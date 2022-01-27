#ifndef JDECOMPILER_ATTRIBUTES_CPP
#define JDECOMPILER_ATTRIBUTES_CPP

#include "jdecompiler.h"
#include "jdecompiler-util.cpp"
#include "jdecompiler-const-pool.cpp"

#undef LOG_PREFIX
#define LOG_PREFIX "[ jdecompiler-attributes.cpp ]"

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

	template<class T> bool has() const {
		return get<T>() != nullptr;
	}

	Attributes(BinaryInputStream& instream, const ConstantPool& constPool, uint16_t attributeCount);
};


struct UnknownAttribute: Attribute {
	const char* const bytes;
	UnknownAttribute(const string& name, uint32_t length, BinaryInputStream& instream): Attribute(name, length), bytes(instream.readBytes(length)) {}
};

struct ConstantValueAttribute: Attribute {
	const ConstValueConstant* const value;

	ConstantValueAttribute(uint32_t length, BinaryInputStream& instream, const ConstantPool& constPool): Attribute("ConstantValue", length), value(constPool.get<ConstValueConstant>(instream.readShort())) {
		if(length != 2) throw IllegalAttributeException("Length of ConstantValue attribute must be 2");
	}
};

struct CodeAttribute: Attribute {
	struct ExceptionAttribute {
		const uint16_t startPos, endPos, handlerPos;
		const ClassConstant* const catchType;

		ExceptionAttribute(BinaryInputStream& instream, const ConstantPool& constPool): startPos(instream.readShort()), endPos(instream.readShort()), handlerPos(instream.readShort()), catchType(constPool.get<ClassConstant>(instream.readShort())) {}
	};

	static vector<ExceptionAttribute*> readExceptionTable(BinaryInputStream& instream, const ConstantPool& constPool, uint16_t length) {
		vector<ExceptionAttribute*> exceptionTable;
		exceptionTable.reserve(length);

		for(uint16_t i = 0; i < length; i++)
			exceptionTable.push_back(new ExceptionAttribute(instream, constPool));

		return exceptionTable;
	}

	const uint16_t maxStack, maxLocals;
	const uint32_t codeLength;
	const char* const code;
	const uint16_t exceptionTableLength;
	vector<ExceptionAttribute*> exceptionTable;
	const Attributes* const attributes;

	CodeAttribute(uint32_t length, BinaryInputStream& instream, const ConstantPool& constPool): Attribute("Code", length), maxStack(instream.readShort()), maxLocals(instream.readShort()), codeLength(instream.readInt()), code(instream.readBytes(codeLength)), exceptionTableLength(instream.readShort()), exceptionTable(readExceptionTable(instream, constPool, exceptionTableLength)), attributes(new Attributes(instream, constPool, instream.readShort())) {}
};


struct Annotation: Stringified {
	private:
		struct Element {
			private:
				struct Value: Stringified {};

				template<typename T, typename ConstT = T>
				struct NumberValue: Value {
					const T value;

					NumberValue(const NumberConstant<ConstT>* constant): value(constant->value) {}
					NumberValue(BinaryInputStream& instream, const ConstantPool& constPool): NumberValue(constPool.get<NumberConstant<ConstT>>(instream.readShort())) {}

					virtual string toString(const ClassInfo& classinfo) const override {
						return primitiveToString(value);
					}
				};

				using BooleanValue = NumberValue<bool, int32_t>;
				using CharValue = NumberValue<char, int32_t>;
				using IntegerValue = NumberValue<int32_t>;
				using FloatValue = NumberValue<float>;
				using LongValue = NumberValue<int64_t>;
				using DoubleValue = NumberValue<double>;


				struct StringValue: Value {
					const StringConstant* const value;

					StringValue(BinaryInputStream& instream, const ConstantPool& constPool): value(constPool.get<StringConstant>(instream.readShort())) {}

					virtual string toString(const ClassInfo& classinfo) const override {
						return value->toString(classinfo);
					}
				};

				struct EnumValue: Value {
					const ClassType* const type;
					const string name;

					EnumValue(const ClassType* type, const string& name): type(type), name(name) {}

					virtual string toString(const ClassInfo& classinfo) const override {
						return type->toString(classinfo) + "." + name;
					}
				};

				struct ClassValue: Value {
					const ReferenceType* const type;

					ClassValue(const ReferenceType* type): type(type) {}

					virtual string toString(const ClassInfo& classinfo) const override {
						return type->toString(classinfo) + ".class";
					}
				};

				struct AnnotationValue: Value {
					const Annotation* const annotation;

					AnnotationValue(BinaryInputStream& instream, const ConstantPool& constPool): annotation(new Annotation(instream, constPool)) {}

					virtual string toString(const ClassInfo& classinfo) const override {
						return annotation->toString(classinfo);
					}
				};

				struct ArrayValue: Value {
					const uint16_t length;
					vector<const Value*> elements;

					ArrayValue(BinaryInputStream& instream, const ConstantPool& constPool): length(instream.readShort()) {
						elements.reserve(length);
						for(uint16_t i = 0; i < length; i++)
							elements.push_back(getValue(instream, constPool, instream.readByte()));
					}

					virtual string toString(const ClassInfo& classinfo) const override {
						return "{" + join<const Value*>(elements, [&classinfo] (const Value* element) { return element->toString(classinfo); }) + "}";
					}
				};

			public:
				const string name;
				const uint8_t typeTag;
				const Value* value;
				Element(BinaryInputStream& instream, const ConstantPool& constPool): name(constPool.getUtf8Constant(instream.readShort())), typeTag(instream.readByte()), value(getValue(instream, constPool, typeTag)) {}

			private: static inline const Value* getValue(BinaryInputStream& instream, const ConstantPool& constPool, uint8_t typeTag) {
				switch(typeTag) {
					case 'B': case 'S': case 'I': return new IntegerValue(constPool.get<IntegerConstant>(instream.readShort()));
					case 'C': return new CharValue(instream, constPool);
					case 'F': return new FloatValue(instream, constPool);
					case 'J': return new LongValue(instream, constPool);
					case 'D': return new DoubleValue(instream, constPool);
					case 'Z': return new BooleanValue(instream, constPool);
					case 's': return new StringValue(instream, constPool);
					case 'e': return new EnumValue(new ClassType(constPool.getUtf8Constant(instream.readShort())), constPool.getUtf8Constant(instream.readShort()));
					case 'c': return new ClassValue(parseReferenceType(*constPool.get<ClassConstant>(instream.readShort())->name));
					case '@': return new AnnotationValue(instream, constPool);
					case '[': return new ArrayValue(instream, constPool);
					default:
						throw DecompilationException((string)"Illegal annotation element value type: '" + (char)typeTag + "' (0x" + hex(typeTag) + ")");
				}
			}
		};

	public:
		const ClassType* const type;
		const uint16_t elementCount;
		vector<const Element*> elements;

		Annotation(BinaryInputStream& instream, const ConstantPool& constPool): type(getAnnotationType(constPool.getUtf8Constant(instream.readShort()))), elementCount(instream.readShort()) {
			elements.reserve(elementCount);
			for(int i = 0; i < elementCount; i++)
				elements.push_back(new Element(instream, constPool));
		}

		virtual string toString(const ClassInfo& classinfo) const override {
			return "@" + type->toString(classinfo) + (elementCount == 0 ? "" : "(" + join<const Element*>(elements, [&classinfo] (const Element* element) { return element->name + "=" + element->value->toString(classinfo); }) + ")");
		}

		private: static inline const ClassType* getAnnotationType(const string& descriptor) {
			const Type* type = parseType(descriptor);
			if(const ClassType* classType = dynamic_cast<const ClassType*>(type))
				return classType;
			throw DecompilationException("Illegal annotation type descriptor :" + descriptor);
		}
};


struct AnnotationsAttribute: Attribute, Stringified {
	const uint16_t annotationsCount;
	vector<const Annotation*> annotations;

	AnnotationsAttribute(string& name, uint32_t length, BinaryInputStream& instream, const ConstantPool& constPool): Attribute(name, length), annotationsCount(instream.readShort()) {
		annotations.reserve(annotationsCount);
		for(int i = 0; i < annotationsCount; i++)
			annotations.push_back(new Annotation(instream, constPool));
	}

	virtual string toString(const ClassInfo& classinfo) const override {
		string str;
		for(const Annotation* annotation : annotations)
			str += annotation->toString(classinfo) + "\n" + classinfo.getIndent();
		return str;
	}
};


/*
struct XXXAttribute: Attribute {
	XXXAttribute(string name, uint32_t length, BinaryInputStream& instream): Attribute("", length) {}
};
*/


struct ExceptionsAttribute: Attribute {
	vector<const ClassConstant*> exceptions;

	ExceptionsAttribute(uint32_t length, BinaryInputStream& instream, const ConstantPool& constPool): Attribute("Exceptions", length) {
		for(uint16_t i = instream.readShort(); i > 0; i--)
			exceptions.push_back(constPool.get<const ClassConstant>(instream.readShort()));
	}
};


struct DeprecatedAttribute: Attribute {
	DeprecatedAttribute(uint32_t length): Attribute("Deprecated", length) {
		if(length != 0) throw IllegalAttributeException("Length of Deprecated attribute must be 0");
	}
};


struct ClassSignature {
	vector<ReferenceType> parameters;
};


/*struct SignatureAttribute: Attribute {
	const ClassSignature* classSignature;

	SignatureAttribute(uint32_t length, BinaryInputStream& instream, const ConstantPool& constPool): Attribute("Signature", length), classSignature(new ClassSignature(constPool.get<Utf8Constant>(instream.readShort()))) {
		if(length != 2) throw IllegalAttributeException("Length of Signature attribute must be 2");
	}
};*/

struct BootstrapMethod {
	const MethodHandleConstant* const methodHandle;
	vector<uint16_t> argumentIndexes;
	vector<const ConstValueConstant*> arguments;

	BootstrapMethod(BinaryInputStream& instream, const ConstantPool& constPool):
			methodHandle(constPool.get<MethodHandleConstant>(instream.readShort())) {
		uint16_t argumentsCount = instream.readShort();

		argumentIndexes.reserve(argumentsCount);
		arguments.reserve(argumentsCount);

		for(uint16_t i = 0; i < argumentsCount; i++) {
			uint16_t index = instream.readShort();
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
			for(uint16_t i = instream.readShort(); i > 0; i--)
				bootstrapMethods.push_back(new BootstrapMethod(instream, constPool));
		}

		inline const BootstrapMethod* operator[](uint16_t index) const {
			return bootstrapMethods[index];
		}
};


Attributes::Attributes(BinaryInputStream& instream, const ConstantPool& constPool, uint16_t attributeCount) {
	this->reserve(attributeCount);

	for(uint16_t i = 0; i < attributeCount; i++) {
		string name = constPool.getUtf8Constant(instream.readShort());
		const uint32_t length = instream.readInt();
		const streampos pos = instream.getPos() + (streampos)length;
		const Attribute* attribute;
		if(name == "ConstantValue")
			attribute = new ConstantValueAttribute(length, instream, constPool);
		else if(name == "Code")
			attribute = new CodeAttribute(length, instream, constPool);
		else if(name == "Exceptions")
			attribute = new ExceptionsAttribute(length, instream, constPool);
		else if(name == "Deprecated")
			attribute = new DeprecatedAttribute(length);
		else if(name == "RuntimeVisibleAnnotations" || name == "RuntimeInvisibleAnnotations")
			attribute = new AnnotationsAttribute(name, length, instream, constPool);
		/*else if(name == "Signature")
			attribute = new SignatureAttribute(length, instream, constPool);*/
		else if(name == "BootstrapMethods")
			attribute = new BootstrapMethodsAttribute(length, instream, constPool);
		else
			attribute = new UnknownAttribute(name, length, instream);

		instream.setPosTo(pos);

		this->push_back(attribute);
	}
};

#endif
