#ifndef JDECOMPILER_ATTRIBUTES_CPP
#define JDECOMPILER_ATTRIBUTES_CPP

#include "types.cpp"

namespace jdecompiler {

	using namespace std;


	struct Attribute {
		const string name;
		const uint32_t length;

		protected:
			Attribute(const string& name, uint32_t length): name(name), length(length) {}

			virtual ~Attribute() {}
	};


	struct Attributes: vector<const Attribute*> {
		template<class T>
		const T* get() const {
			for(const Attribute* attribute : *this)
				if(const T* t = dynamic_cast<const T*>(attribute)) return t;
			return nullptr;
		}

		template<class T>
		const T* getExact() const {
			for(const Attribute* attribute : *this)
				if(const T* t = dynamic_cast<const T*>(attribute)) return t;
			throw AttributeNotFoundException(typeid(T).name());
		}

		template<class T>
		inline bool has() const {
			return get<T>() != nullptr;
		}

		static inline const Attribute* readAttribute(BinaryInputStream& instream, const ConstantPool& constPool, const string& name, const uint32_t length);

		Attributes(BinaryInputStream& instream, const ConstantPool& constPool, uint16_t attributeCount) {
			this->reserve(attributeCount);

			for(uint16_t i = 0; i < attributeCount; i++) {
				const string& name = constPool.getUtf8Constant(instream.readUShort());
				const uint32_t length = instream.readUInt();
				const streampos pos = instream.getPos() + (streampos)length;

				const Attribute* attribute = readAttribute(instream, constPool, name, length);

				instream.setPosTo(pos);

				this->push_back(attribute);
			}
		}
	};


	struct UnknownAttribute: Attribute {
		const uint8_t* const bytes;
		UnknownAttribute(const string& name, uint32_t length, BinaryInputStream& instream):
				Attribute(name, length), bytes(instream.readBytes(length)) {}

		virtual ~UnknownAttribute() override {
			delete bytes;
		}
	};

	struct ConstantValueAttribute: Attribute, Stringified {
		const ConstValueConstant* const value;

		ConstantValueAttribute(uint32_t length, BinaryInputStream& instream, const ConstantPool& constPool):
				Attribute("ConstantValue", length), value(constPool.get<ConstValueConstant>(instream.readUShort())) {
			if(length != 2) throw IllegalAttributeException("Length of ConstantValue attribute must be 2");
		}

		virtual string toString(const ClassInfo& classinfo) const override {
			return value->toString(classinfo);
		}
	};

	struct CodeAttribute: Attribute {
		struct ExceptionHandler final {
			public:
				const uint16_t startPos, endPos, handlerPos;
				const ClassType* const catchType;

			protected:
				static inline const ClassType* getCatchType(const ClassConstant* classConstant) {
					return classConstant == nullptr ? nullptr : new ClassType(classConstant);
				}

			public:
				ExceptionHandler(BinaryInputStream& instream, const ConstantPool& constPool):
						startPos(instream.readUShort()), endPos(instream.readUShort()), handlerPos(instream.readUShort()),
						catchType(getCatchType(constPool.getNullablle<ClassConstant>(instream.readUShort()))) {}

				~ExceptionHandler() {
					if(catchType != nullptr)
						delete catchType;
				}
		};

		static const vector<const ExceptionHandler*> readExceptionTable(BinaryInputStream& instream, const ConstantPool& constPool) {
			const uint16_t length = instream.readUShort();

			vector<const ExceptionHandler*> exceptionTable;
			exceptionTable.reserve(length);

			for(uint16_t i = 0; i < length; i++)
				exceptionTable.push_back(new ExceptionHandler(instream, constPool));

			return exceptionTable;
		}

		const uint16_t maxStack, maxLocals;
		const uint32_t codeLength;
		const uint8_t* const code;
		const vector<const ExceptionHandler*> exceptionTable;
		const Attributes& attributes;

		CodeAttribute(uint32_t length, BinaryInputStream& instream, const ConstantPool& constPool):
				Attribute("Code", length), maxStack(instream.readUShort()), maxLocals(instream.readUShort()),
				codeLength(instream.readUInt()), code(instream.readBytes(codeLength)),
				exceptionTable(readExceptionTable(instream, constPool)),
				attributes(*new Attributes(instream, constPool, instream.readUShort())) {}
	};



	struct AnnotationValue: Stringified {
		static const AnnotationValue& readValue(BinaryInputStream& instream, const ConstantPool& constPool, uint8_t typeTag);
	};

	struct Annotation: Stringified {
		private:
			struct Element {
				public:
					const string name;
					const AnnotationValue& value;
					Element(BinaryInputStream& instream, const ConstantPool& constPool): name(constPool.getUtf8Constant(instream.readUShort())),
							value(AnnotationValue::readValue(instream, constPool, instream.readUByte())) {}
			};

		public:
			const ClassType* const type;
			vector<const Element*> elements;

			Annotation(BinaryInputStream& instream, const ConstantPool& constPool):
					type(getAnnotationType(constPool.getUtf8Constant(instream.readUShort()))) {
				const uint16_t elementCount = instream.readUShort();
				elements.reserve(elementCount);
				for(uint16_t i = 0; i < elementCount; i++)
					elements.push_back(new Element(instream, constPool));
			}

			virtual string toString(const ClassInfo& classinfo) const override {
				return '@' + type->toString(classinfo) + (elements.size() == 0 ? EMPTY_STRING : '(' + join<const Element*>(elements,
						[&classinfo] (auto element) { return element->name + '=' + element->value.toString(classinfo); }) + ')');
			}

		private:
			static inline const ClassType* getAnnotationType(const string& descriptor) {
				const Type* type = parseType(descriptor);
				if(const ClassType* classType = dynamic_cast<const ClassType*>(type))
					return classType;
				throw DecompilationException("Illegal annotation type descriptor :" + descriptor);
			}
	};


	template<typename T, typename ConstT = T>
	struct NumberAnnotationValue: AnnotationValue {
		const T value;

		static inline T castExact(ConstT value) {
			if((ConstT)(T)value != value)
				cerr << "warning: integer overflow while reaing value from constant pool: value " << to_string(value)
						<< " is out of bounds for size " << sizeof(T) << endl;
			return (T)value;
		}

		NumberAnnotationValue(const NumberConstant<ConstT>* constant):
				value(sizeof(ConstT) > sizeof(T) ? castExact(constant->value) : constant->value) {}

		NumberAnnotationValue(BinaryInputStream& instream, const ConstantPool& constPool):
				NumberAnnotationValue(constPool.get<NumberConstant<ConstT>>(instream.readUShort())) {}

		virtual string toString(const ClassInfo& classinfo) const override {
			return primitiveToString(value);
		}
	};

	using BooleanAnnotationValue = NumberAnnotationValue<bool, int32_t>;
	using CharAnnotationValue = NumberAnnotationValue<char, int32_t>;
	using IntegerAnnotationValue = NumberAnnotationValue<int32_t>;
	using FloatAnnotationValue = NumberAnnotationValue<float>;
	using LongAnnotationValue = NumberAnnotationValue<int64_t>;
	using DoubleAnnotationValue = NumberAnnotationValue<double>;


	struct StringAnnotationValue: AnnotationValue {
		const Utf8Constant& value;

		StringAnnotationValue(BinaryInputStream& instream, const ConstantPool& constPool):
				value(constPool.getUtf8Constant(instream.readUShort())) {}

		virtual string toString(const ClassInfo& classinfo) const override {
			return stringToLiteral(value);
		}
	};

	struct EnumAnnotationValue: AnnotationValue {
		const Type* const type;
		const string name;

		EnumAnnotationValue(const Type* type, const string& name): type(type), name(name) {}

		virtual string toString(const ClassInfo& classinfo) const override {
			return type->toString(classinfo) + '.' + name;
		}
	};

	struct ClassAnnotationValue: AnnotationValue {
		const ReferenceType* const type;

		ClassAnnotationValue(const ReferenceType* type): type(type) {}

		virtual string toString(const ClassInfo& classinfo) const override {
			return type->toString(classinfo) + ".class";
		}
	};

	struct AnnotationAnnotationValue: AnnotationValue {
		const Annotation* const annotation;

		AnnotationAnnotationValue(BinaryInputStream& instream, const ConstantPool& constPool): annotation(new Annotation(instream, constPool)) {}

		virtual string toString(const ClassInfo& classinfo) const override {
			return annotation->toString(classinfo);
		}
	};

	struct ArrayAnnotationValue: AnnotationValue {
		const uint16_t length;
		vector<const AnnotationValue*> elements;

		ArrayAnnotationValue(BinaryInputStream& instream, const ConstantPool& constPool): length(instream.readUShort()) {
			elements.reserve(length);
			for(uint16_t i = 0; i < length; i++)
				elements.push_back(&readValue(instream, constPool, instream.readUByte()));
		}

		virtual string toString(const ClassInfo& classinfo) const override {
			return '{' + join<const AnnotationValue*>(elements, [&classinfo] (const AnnotationValue* element) { return element->toString(classinfo); }) + '}';
		}
	};


	const AnnotationValue& AnnotationValue::readValue(BinaryInputStream& instream, const ConstantPool& constPool, uint8_t typeTag) {
		switch(typeTag) {
			case 'B': case 'S': case 'I': return *new IntegerAnnotationValue(constPool.get<IntegerConstant>(instream.readUShort()));
			case 'C': return *new CharAnnotationValue(instream, constPool);
			case 'F': return *new FloatAnnotationValue(instream, constPool);
			case 'J': return *new LongAnnotationValue(instream, constPool);
			case 'D': return *new DoubleAnnotationValue(instream, constPool);
			case 'Z': return *new BooleanAnnotationValue(instream, constPool);
			case 's': return *new StringAnnotationValue(instream, constPool);
			case 'e': return *new EnumAnnotationValue(parseType(constPool.getUtf8Constant(instream.readUShort())),
					constPool.getUtf8Constant(instream.readUShort()));
			case 'c': return *new ClassAnnotationValue(parseReferenceType(*constPool.get<ClassConstant>(instream.readUShort())->name));
			case '@': return *new AnnotationAnnotationValue(instream, constPool);
			case '[': return *new ArrayAnnotationValue(instream, constPool);
			default:
				throw IllegalAttributeException((string)"Illegal annotation element value type: '" + (char)typeTag + "' (U+" + hex<4>(typeTag) + ')');
		}
	}


	struct AnnotationsAttribute: Attribute, Stringified {
		vector<const Annotation*> annotations;

		AnnotationsAttribute(const string& name, uint32_t length, BinaryInputStream& instream, const ConstantPool& constPool):
				Attribute(name, length) {
			const uint16_t annotationsCount = instream.readUShort();
			annotations.reserve(annotationsCount);
			for(uint16_t i = 0; i < annotationsCount; i++)
				annotations.push_back(new Annotation(instream, constPool));
		}

		virtual string toString(const ClassInfo& classinfo) const override {
			string str;
			for(const Annotation* annotation : annotations)
				str += classinfo.getIndent() + annotation->toString(classinfo) + '\n';
			return str;
		}
	};


	struct AnnotationDefaultAttribute: Attribute, Stringified {
		const AnnotationValue& value;

		AnnotationDefaultAttribute(uint32_t length, BinaryInputStream& instream, const ConstantPool& constPool):
				Attribute("AnnotationDefault", length), value(AnnotationValue::readValue(instream, constPool, instream.readUByte())) {}

		virtual string toString(const ClassInfo& classinfo) const override {
			return value.toString(classinfo);
		}
	};


	/*
	struct XXXAttribute: Attribute {
		XXXAttribute(string name, uint32_t length, BinaryInputStream& instream): Attribute(EMPTY_STRING, length) {}
	};
	*/


	struct ExceptionsAttribute: Attribute {
		vector<const ClassConstant*> exceptions;

		ExceptionsAttribute(uint32_t length, BinaryInputStream& instream, const ConstantPool& constPool): Attribute("Exceptions", length) {
			for(uint16_t i = instream.readUShort(); i > 0; i--)
				exceptions.push_back(constPool.get<const ClassConstant>(instream.readUShort()));
		}
	};


	struct DeprecatedAttribute: Attribute {
		DeprecatedAttribute(uint32_t length): Attribute("Deprecated", length) {
			if(length != 0) throw IllegalAttributeException("Length of Deprecated attribute must be 0");
		}
	};



	struct LocalVariableTableAttribute: Attribute {
		private:
			struct LocalVariable {
				const uint16_t startPos, endPos;
				const string& name;
				const Type& type;
				const uint16_t index;

				LocalVariable(BinaryInputStream& instream, const ConstantPool& constPool):
						startPos(instream.readUShort()), endPos(startPos + instream.readUShort()), name(constPool.getUtf8Constant(instream.readUShort())),
						type(*parseType(constPool.getUtf8Constant(instream.readUShort()))), index(instream.readUShort()) {}
			};

			vector<const LocalVariable*> localVariableTable;

		public:
			LocalVariableTableAttribute(uint32_t length, BinaryInputStream& instream, const ConstantPool& constPool):
					Attribute("LocalVariableTable", length) {

				const uint16_t localVariableTableLength = instream.readUShort();
				localVariableTable.reserve(localVariableTableLength);
				for(uint16_t i = 0; i < localVariableTableLength; i++)
					localVariableTable.push_back(new LocalVariable(instream, constPool));
			}
	};


	struct ClassSignature {
		vector<ReferenceType> parameters;
	};


	/*struct SignatureAttribute: Attribute {
		const ClassSignature* classSignature;

		SignatureAttribute(uint32_t length, BinaryInputStream& instream, const ConstantPool& constPool): Attribute("Signature", length), classSignature(new ClassSignature(constPool.get<Utf8Constant>(instream.readUShort()))) {
			if(length != 2) throw IllegalAttributeException("Length of Signature attribute must be 2");
		}
	};*/


	struct BootstrapMethod {
		const MethodHandleConstant* const methodHandle;
		vector<uint16_t> argumentIndexes;
		vector<const ConstValueConstant*> arguments;

		BootstrapMethod(BinaryInputStream& instream, const ConstantPool& constPool):
				methodHandle(constPool.get<MethodHandleConstant>(instream.readUShort())) {
			uint16_t argumentsCount = instream.readUShort();

			argumentIndexes.reserve(argumentsCount);
			arguments.reserve(argumentsCount);

			for(uint16_t i = 0; i < argumentsCount; i++) {
				uint16_t index = instream.readUShort();
				argumentIndexes.push_back(index);
				arguments.push_back(constPool.get<ConstValueConstant>(index));
			}
		}
	};


	struct BootstrapMethodsAttribute: Attribute {
		protected:
			vector<const BootstrapMethod*> bootstrapMethods;

		public:
			BootstrapMethodsAttribute(uint32_t length, BinaryInputStream& instream, const ConstantPool& constPool): Attribute("BootstrapMethods", length) {
				for(uint16_t i = instream.readUShort(); i > 0; i--)
					bootstrapMethods.push_back(new BootstrapMethod(instream, constPool));
			}

			inline const BootstrapMethod* operator[] (uint16_t index) const {
				return bootstrapMethods[index];
			}
	};


	inline const Attribute* Attributes::readAttribute(BinaryInputStream& instream, const ConstantPool& constPool, const string& name, const uint32_t length) {
		if(name == "ConstantValue") return new ConstantValueAttribute(length, instream, constPool);
		if(name == "Code") return new CodeAttribute(length, instream, constPool);
		if(name == "Exceptions") return new ExceptionsAttribute(length, instream, constPool);
		if(name == "Deprecated") return new DeprecatedAttribute(length);
		if(name == "RuntimeVisibleAnnotations" || name == "RuntimeInvisibleAnnotations") return new AnnotationsAttribute(name, length, instream, constPool);
		//if(name == "Signature") return new SignatureAttribute(length, instream, constPool);
		if(name == "BootstrapMethods") return new BootstrapMethodsAttribute(length, instream, constPool);
		if(name == "AnnotationDefault") return new AnnotationDefaultAttribute(length, instream, constPool);
		if(name == "LocalVariableTable") return new LocalVariableTableAttribute(length, instream, constPool);
		return new UnknownAttribute(name, length, instream);
	}
}

#endif
