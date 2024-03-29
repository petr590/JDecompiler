#ifndef JDECOMPILER_ATTRIBUTES_CPP
#define JDECOMPILER_ATTRIBUTES_CPP

namespace jdecompiler {

	struct Attribute {
		const string name;
		const uint32_t length;

		protected:
			Attribute(const string& name, uint32_t length): name(name), length(length) {}

			virtual ~Attribute() {}
	};


	enum class AttributesType {
		CLASS, FIELD, METHOD, ATTRIBUTE
	};


	struct Attributes: vector<const Attribute*> {
		public:
			template<class T>
			const T* get() const {
				for(const Attribute* attribute : *this) {
					if(instanceof<const T*>(attribute)) {
						return static_cast<const T*>(attribute);
					}
				}

				return nullptr;
			}

			template<class T>
			const T* getExact() const {
				const T* t = get<T>();

				return t != nullptr ? t : throw AttributeNotFoundException(typenameof<T>());
			}

			template<class T>
			inline bool has() const {
				return get<T>() != nullptr;
			}

			static const Attribute* readAttribute(ClassInputStream&, const ConstantPool&, const string&, uint32_t, AttributesType);

			Attributes(ClassInputStream& instream, const ConstantPool& constPool, uint16_t attributeCount, AttributesType attributesType) {

				this->reserve(attributeCount);

				for(uint16_t i = 0; i < attributeCount; i++) {
					const uint16_t addr = instream.readUShort();

					const string& name = constPool.getUtf8Constant(addr);

					const uint32_t length = instream.readUInt();
					const streampos pos = instream.getPos() + (streampos)length;

					const Attribute* attribute = readAttribute(instream, constPool, name, length, attributesType);

					if(instream.getPos() != pos) {
						throw DecompilationException("When reading the " + attribute->name +
								" attribute, a different number of bytes was read than its length");
					}

					this->push_back(attribute);
				}
			}

			Attributes(const Attributes&) = delete;

		private:
			Attributes() noexcept {}

		public:
			static const Attributes& getEmptyInstance() {
				static Attributes EMPTY_INSTANCE;
				return EMPTY_INSTANCE;
			}
	};


	struct UnknownAttribute: Attribute {
		const uint8_t* const bytes;
		UnknownAttribute(const string& name, uint32_t length, ClassInputStream& instream):
				Attribute(name, length), bytes(instream.readBytes(length)) {}

		virtual ~UnknownAttribute() override {
			delete bytes;
		}
	};

	struct ConstantValueAttribute: Attribute/*, Stringified*/ {
		const ConstValueConstant* const value;

		ConstantValueAttribute(uint32_t length, ClassInputStream& instream, const ConstantPool& constPool):
				Attribute("ConstantValue", length), value(constPool.get<ConstValueConstant>(instream.readUShort())) {

			if(length != 2)
				throw IllegalAttributeException("Length of ConstantValue attribute must be 2");
		}

		/*virtual string toString(const ClassInfo& classinfo) const override {
			return value->toString(classinfo);
		}*/

		const Operation* getInitializer() const {
			return value->toOperation();
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
				ExceptionHandler(ClassInputStream& instream, const ConstantPool& constPool):
						startPos(instream.readUShort()), endPos(instream.readUShort()), handlerPos(instream.readUShort()),
						catchType(getCatchType(constPool.getNullable<ClassConstant>(instream.readUShort()))) {}

				~ExceptionHandler() {
					if(catchType != nullptr)
						delete catchType;
				}
		};

		static const vector<const ExceptionHandler*> readExceptionTable(ClassInputStream& instream, const ConstantPool& constPool) {
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

		CodeAttribute(uint32_t length, ClassInputStream& instream, const ConstantPool& constPool):
				Attribute("Code", length), maxStack(instream.readUShort()), maxLocals(instream.readUShort()),
				codeLength(instream.readUInt()), code(instream.readBytes(codeLength)),
				exceptionTable(readExceptionTable(instream, constPool)),
				attributes(*new Attributes(instream, constPool, instream.readUShort(), AttributesType::ATTRIBUTE)) {}
	};



	struct AnnotationValue: Stringified {
		static const AnnotationValue& readValue(ClassInputStream&, const ConstantPool&, uint8_t);
	};

	struct Annotation: Stringified {
		private:
			struct Element {
				public:
					const string name;
					const AnnotationValue& value;
					Element(ClassInputStream& instream, const ConstantPool& constPool): name(constPool.getUtf8Constant(instream.readUShort())),
							value(AnnotationValue::readValue(instream, constPool, instream.readUByte())) {}
			};

		public:
			const ClassType* const type;
			vector<const Element*> elements;

			Annotation(ClassInputStream& instream, const ConstantPool& constPool):
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
				if(instanceof<const ClassType*>(type))
					return static_cast<const ClassType*>(type);
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

		NumberAnnotationValue(ClassInputStream& instream, const ConstantPool& constPool):
				NumberAnnotationValue(constPool.get<NumberConstant<ConstT>>(instream.readUShort())) {}

		virtual string toString(const ClassInfo&) const override {
			return primitiveToString(value);
		}
	};

	using BooleanAnnotationValue = NumberAnnotationValue<jbool, jint>;
	using CharAnnotationValue    = NumberAnnotationValue<jchar, jint>;
	using IntegerAnnotationValue = NumberAnnotationValue<jint>;
	using FloatAnnotationValue   = NumberAnnotationValue<jfloat>;
	using LongAnnotationValue    = NumberAnnotationValue<jlong>;
	using DoubleAnnotationValue  = NumberAnnotationValue<jdouble>;


	struct StringAnnotationValue: AnnotationValue {
		const Utf8Constant& value;

		StringAnnotationValue(ClassInputStream& instream, const ConstantPool& constPool):
				value(constPool.getUtf8Constant(instream.readUShort())) {}

		virtual string toString(const ClassInfo&) const override {
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

		AnnotationAnnotationValue(ClassInputStream& instream, const ConstantPool& constPool): annotation(new Annotation(instream, constPool)) {}

		virtual string toString(const ClassInfo& classinfo) const override {
			return annotation->toString(classinfo);
		}
	};

	struct ArrayAnnotationValue: AnnotationValue {
		const uint16_t length;
		vector<const AnnotationValue*> elements;

		ArrayAnnotationValue(ClassInputStream& instream, const ConstantPool& constPool): length(instream.readUShort()) {
			elements.reserve(length);
			for(uint16_t i = 0; i < length; i++)
				elements.push_back(&readValue(instream, constPool, instream.readUByte()));
		}

		virtual string toString(const ClassInfo& classinfo) const override {
			return '{' + join<const AnnotationValue*>(elements, [&classinfo] (const AnnotationValue* element) { return element->toString(classinfo); }) + '}';
		}
	};


	const AnnotationValue& AnnotationValue::readValue(ClassInputStream& instream, const ConstantPool& constPool, uint8_t typeTag) {
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
			case 'c': return *new ClassAnnotationValue(parseReferenceType(constPool.get<ClassConstant>(instream.readUShort())->name));
			case '@': return *new AnnotationAnnotationValue(instream, constPool);
			case '[': return *new ArrayAnnotationValue(instream, constPool);
			default:
				throw IllegalAttributeException((string)"Illegal annotation element value type: '" + (char)typeTag + "' (U+" + hex<4>(typeTag) + ')');
		}
	}


	struct AnnotationsAttribute: Attribute, Stringified {
		vector<const Annotation*> annotations;

		AnnotationsAttribute(const string& name, uint32_t length, ClassInputStream& instream, const ConstantPool& constPool):
				Attribute(name, length) {
			const uint16_t annotationsCount = instream.readUShort();
			annotations.reserve(annotationsCount);
			for(uint16_t i = 0; i < annotationsCount; i++)
				annotations.push_back(new Annotation(instream, constPool));
		}

		virtual string toString(const ClassInfo& classinfo) const override {
			return join<const Annotation*>(annotations,
					[&classinfo] (const Annotation* annotation) { return classinfo.getIndent() + annotation->toString(classinfo); }, "\n");
		}
	};


	struct ParameterAnnotationsAttribute: Attribute {
		vector<vector<const Annotation*>> parameterAnnotations;

		ParameterAnnotationsAttribute(const string& name, uint32_t length, ClassInputStream& instream, const ConstantPool& constPool):
				Attribute(name, length) {

			uint8_t parametersLength = instream.readUByte();
			parameterAnnotations.reserve(parametersLength);
			for(uint8_t i = 0; i < parametersLength; i++) {

				vector<const Annotation*> annotations;
				uint16_t annotationsLength = instream.readUShort();
				parameterAnnotations.reserve(annotationsLength);
				for(uint16_t j = 0; j < annotationsLength; j++) {
					annotations.push_back(new Annotation(instream, constPool));
				}

				parameterAnnotations.push_back(annotations);
			}
		}

		string parameterAnnotationsToString(size_t index, const ClassInfo& classinfo) const {
			if(index >= parameterAnnotations.size())
				return EMPTY_STRING;

			const vector<const Annotation*> annotations = parameterAnnotations[index];

			if(annotations.empty())
				return EMPTY_STRING;

			if(!JDecompiler::getInstance().printNewLineInParameterAnnotations()) {
				return join<const Annotation*>(annotations, [&classinfo] (const Annotation* annotation) {
						return annotation->toString(classinfo);
					}, " ") + ' ';
			}

			classinfo.increaseIndent(2);

			string str("\n");
			for(const Annotation* annotation : annotations)
				str += classinfo.getIndent() + annotation->toString(classinfo) + '\n';

			str += classinfo.getIndent();

			classinfo.reduceIndent(2);

			return str;
		}
	};


	struct AnnotationDefaultAttribute: Attribute, Stringified {
		const AnnotationValue& value;

		AnnotationDefaultAttribute(uint32_t length, ClassInputStream& instream, const ConstantPool& constPool):
				Attribute("AnnotationDefault", length), value(AnnotationValue::readValue(instream, constPool, instream.readUByte())) {}

		virtual string toString(const ClassInfo& classinfo) const override {
			return value.toString(classinfo);
		}
	};


	/*
	struct XXXAttribute: Attribute {
		XXXAttribute(string name, uint32_t length, ClassInputStream& instream): Attribute(name, length) {}
	};
	*/


	struct ExceptionsAttribute: Attribute {
		vector<const ClassConstant*> exceptions;

		ExceptionsAttribute(uint32_t length, ClassInputStream& instream, const ConstantPool& constPool): Attribute("Exceptions", length) {
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
		struct LocalVariable {
			const uint16_t startPos, endPos;
			const string& name;
			const Type& type;

			LocalVariable(ClassInputStream& instream, const ConstantPool& constPool):
					startPos(instream.readUShort()), endPos(startPos + instream.readUShort()), name(constPool.getUtf8Constant(instream.readUShort())),
					type(*parseType(constPool.getUtf8Constant(instream.readUShort()))) {}
		};

		vector<vector<const LocalVariable*>> localVariableTable;

		LocalVariableTableAttribute(uint32_t length, ClassInputStream& instream, const ConstantPool& constPool):
				Attribute("LocalVariableTable", length) {

			for(uint16_t i = instream.readUShort(); i > 0; i--) {
				const LocalVariable* localVar = new LocalVariable(instream, constPool);
				uint16_t index = instream.readUShort();

				if(index >= localVariableTable.size()) {
					uint16_t j = localVariableTable.size();

					localVariableTable.resize(index + 1);

					for(; j <= index; j++) {
						localVariableTable[j] = vector<const LocalVariable*>();
					}
				}

				localVariableTable[index].push_back(localVar);
			}
		}
	};


	struct Signature {
		public:
			const vector<const GenericParameter*> genericParameters;

		protected:
			Signature(const char*& str): genericParameters(parseGeneric(str)) {}
	};


	struct ClassSignature: Signature {
		const ClassType& superClass;
		vector<const ClassType*> interfaces;

		ClassSignature(const char* str):
				Signature(str), superClass(*parseClassType(str)) {

			const char* const srcStr = str;

			while(*str != '\0') {
				if(str[0] != 'L')
					throw InvalidSignatureException(srcStr, str - srcStr);
				interfaces.push_back(new ClassType(str += 1));
			}
		}
	};


	struct FieldSignature {
		const Type& type;

		FieldSignature(const char* str): type(*parseParameter(str)) {}
	};


	struct MethodSignature: Signature {
		const vector<const Type*> arguments;
		const Type* const returnType;

		MethodSignature(const char* str): Signature(str), arguments(parseMethodArguments(str)), returnType(parseReturnType(str)) {}
	};


	template<class Signature>
	struct SignatureAttribute: Attribute {

		const Signature& signature;

		SignatureAttribute(uint32_t length, ClassInputStream& instream, const ConstantPool& constPool): Attribute("Signature", length),
				signature(*new Signature(constPool.getUtf8Constant(instream.readUShort()).c_str())) {

			if(length != 2) throw IllegalAttributeException("Length of Signature attribute must be 2");
		}
	};


	typedef SignatureAttribute<ClassSignature>   ClassSignatureAttribute;
	typedef SignatureAttribute<FieldSignature>   FieldSignatureAttribute;
	typedef SignatureAttribute<MethodSignature> MethodSignatureAttribute;


	struct BootstrapMethod {
		const MethodHandleConstant* const methodHandle;
		vector<uint16_t> argumentIndexes;
		vector<const ConstValueConstant*> arguments;

		BootstrapMethod(ClassInputStream& instream, const ConstantPool& constPool):
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
			BootstrapMethodsAttribute(uint32_t length, ClassInputStream& instream, const ConstantPool& constPool):
					Attribute("BootstrapMethods", length) {

				for(uint16_t i = instream.readUShort(); i > 0; i--)
					bootstrapMethods.push_back(new BootstrapMethod(instream, constPool));
			}

			inline const BootstrapMethod* operator[](uint16_t index) const {
				return bootstrapMethods[index];
			}
	};


	struct InnerClass {
		const ClassType classType;
		const ClassConstant* const outerClass;
		const Utf8Constant* const innerName;
		const modifiers_t modifiers;

		InnerClass(ClassInputStream& instream, const ConstantPool& constPool):
				classType(constPool.get<ClassConstant>(instream.readUShort())), outerClass(constPool.getNullable<ClassConstant>(instream.readUShort())),
				innerName(constPool.getNullable<Utf8Constant>(instream.readUShort())), modifiers(instream.readUShort()) {}
	};


	struct InnerClassesAttribute: Attribute {
		vector<const InnerClass*> classes;

		InnerClassesAttribute(uint32_t length, ClassInputStream& instream, const ConstantPool& constPool):
				Attribute("InnerClasses", length) {

			const uint16_t size = instream.readUShort();
			classes.reserve(size);

			for(uint16_t i = size; i > 0; i--)
				classes.push_back(new InnerClass(instream, constPool));
		}

		inline const InnerClass* find(const ClassType& classType) const {
			const auto result = find_if(classes.begin(), classes.end(),
					[&classType] (const InnerClass* innerClass) { return innerClass->classType == classType; });
			return result == classes.end() ? nullptr : *result;
		}
	};


	struct NestMembersAttribute: Attribute {
		vector<const ClassType*> nestMembers;

		NestMembersAttribute(uint32_t length, ClassInputStream& instream, const ConstantPool& constPool):
				Attribute("NestMembers", length) {

			const uint16_t size = instream.readUShort();
			nestMembers.reserve(size);

			for(uint16_t i = size; i > 0; i--)
				nestMembers.push_back(new ClassType(constPool.get<ClassConstant>(instream.readUShort())));
		}
	};


	struct SourceFileAttribute: Attribute {
		const string& sourceFile;

		SourceFileAttribute(uint32_t length, ClassInputStream& instream, const ConstantPool& constPool):
				Attribute("SourceFile", length), sourceFile(constPool.getUtf8Constant(instream.readUShort())) {

			if(length != 2)
				throw IllegalAttributeException("Length of SourceFile attribute must be 2");
		}
	};


	inline const Attribute* Attributes::readAttribute(ClassInputStream& instream, const ConstantPool& constPool,
			const string& name, uint32_t length, AttributesType attributesType) {

		if(attributesType != AttributesType::ATTRIBUTE) {
			if(name == "Deprecated") return new DeprecatedAttribute(length);
			if(name == "RuntimeVisibleAnnotations" || name == "RuntimeInvisibleAnnotations")
				return new AnnotationsAttribute(name, length, instream, constPool);
			if(name == "RuntimeVisibleParameterAnnotations" || name == "RuntimeInvisibleParameterAnnotations")
				return new ParameterAnnotationsAttribute(name, length, instream, constPool);
			if(name == "Signature") {
				switch(attributesType) {
					case AttributesType::CLASS:  return new  ClassSignatureAttribute(length, instream, constPool);
					case AttributesType::FIELD:  return new  FieldSignatureAttribute(length, instream, constPool);
					case AttributesType::METHOD: return new MethodSignatureAttribute(length, instream, constPool);
					default: throw Exception("Seriously?");
				}
			}
		}

		switch(attributesType) {
			case AttributesType::CLASS:
				if(name == "BootstrapMethods") return new BootstrapMethodsAttribute(length, instream, constPool);
				if(name == "InnerClasses") return new InnerClassesAttribute(length, instream, constPool);
				if(name == "NestMembers") return new NestMembersAttribute(length, instream, constPool);
				if(name == "SourceFile") return new SourceFileAttribute(length, instream, constPool);
				break;

			case AttributesType::FIELD:
				if(name == "ConstantValue") return new ConstantValueAttribute(length, instream, constPool);
				break;

			case AttributesType::METHOD:
				if(name == "Code") return new CodeAttribute(length, instream, constPool);
				if(name == "Exceptions") return new ExceptionsAttribute(length, instream, constPool);
				if(name == "AnnotationDefault") return new AnnotationDefaultAttribute(length, instream, constPool);
				break;

			case AttributesType::ATTRIBUTE:
				if(name == "LocalVariableTable") return new LocalVariableTableAttribute(length, instream, constPool);
		}

		return new UnknownAttribute(name, length, instream);
	}
}

#endif
