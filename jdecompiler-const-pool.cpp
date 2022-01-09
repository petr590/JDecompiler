#ifndef JDECOMPILER_CONST_POOL_CPP
#define JDECOMPILER_CONST_POOL_CPP

#include <string>
#include <vector>
#include "jdecompiler.h"
#include "jdecompiler-util.cpp"
//#include <iostream>

using namespace std;

namespace JDecompiler {
	struct Constant {
		const ConstantPool& constPool;

		Constant(const ConstantPool& constPool): constPool(constPool) {};

		virtual void init() {};
	};


	struct ConstantPool {
		private:
			const uint16_t size;
			Constant** pool;

		public:
			ConstantPool(const int size): size(size), pool(new Constant*[size]) {}

			Constant*& operator[](int index) const {
				if(index < 0 || index >= size)
					throw IndexOutOfBoundsException("Invalid constant pool reference 0x" + hex<4>(index));
				return pool[index];
			}

			template<class T>
			T* get(int index) const {
				static_assert(is_base_of<Constant, T>::value, "T is not subclass of class Constant");
				if(index < 0 || index >= size)
					throw IndexOutOfBoundsException("Invalid constant pool reference 0x" + hex<4>(index));
				T* constant = dynamic_cast<T*>(pool[index]);
				if(constant == nullptr)
					throw DynamicCastException("Invalid constant pool reference 0x" + hex<4>(index) + " at " + typeid(T).name());
				return constant;
			}
	};


	struct Utf8Constant: Constant {
		const uint16_t length;
		const char* bytes;

		Utf8Constant(const ConstantPool& constPool, uint16_t length, const char* bytes): Constant(constPool), length(length), bytes(bytes) {}

		operator string() const {
			return string(bytes, length);
		}
	};

	struct ClassConstant: Constant {
		const uint16_t nameRef;
		const Utf8Constant* name;

		ClassConstant(const ConstantPool& constPool, uint16_t nameRef): Constant(constPool), nameRef(nameRef) {}

		virtual void init() override {
			name = constPool.get<Utf8Constant>(nameRef);
		}
	};


	template<typename T>
	struct ConstValueConstant: Constant {
		T value;
		ConstValueConstant(const ConstantPool& constPool, T value): Constant(constPool), value(value) {}
	};


	struct StringConstant: ConstValueConstant<const Utf8Constant*> {
		const uint16_t valueRef;

		StringConstant(const ConstantPool& constPool, uint16_t valueRef): ConstValueConstant(constPool, nullptr), valueRef(valueRef) {}

		virtual void init() override {
			value = constPool.get<Utf8Constant>(valueRef);
		}

		string toLiteral() const {
			const char* bytes = value->bytes;
			const uint32_t length = strlen(bytes);
			string str = "\"";
			for(uint32_t i = 0; i < length; i++) {
				char32_t ch = bytes[i] & 0xFF;
				char32_t code = ch;
				switch(ch) {
					case '"': str += "\\\""; break;
					case '\b': str += "\\b"; break;
					case '\t': str += "\\t"; break;
					case '\n': str += "\\n"; break;
					case '\f': str += "\\f"; break;
					case '\r': str += "\\r"; break;
					case '\\': str += "\\\\"; break;
					default:
						if((ch & 0xE0) == 0xC0) {
							ch = (ch << 8) | (bytes[++i] & 0xFF);
							code = (ch & 0x1F00) >> 2 | (ch & 0x3F);
						} else if((ch & 0xF0) == 0xE0) {
							if((ch & 0xFF) == 0xED) {
								ch = 0x10000 | (bytes[++i] & 0xF) << 16 | (bytes[++i] & 0x3F) << 10 | (bytes[i+=2] & 0x0F) << 6 | (bytes[++i] & 0x3F);
								str += encodeUtf8(ch);
								continue;
							}
							ch = (ch << 16) | (bytes[++i] & 0xFF) << 8 | (bytes[++i] & 0xFF);
							code = (ch & 0xF0000) >> 4 | (ch & 0x3F00) >> 2 | (ch & 0x3F);
						}
						str += code < 0x20 ? "\\u" + hex<4>(code) : char32ToString(ch);
				}
			}
			return str + '"';
		}
	};


	using IntegerConstant = ConstValueConstant<int32_t>;
	using FloatConstant = ConstValueConstant<float>;
	using LongConstant = ConstValueConstant<int64_t>;
	using DoubleConstant = ConstValueConstant<double>;



	struct NameAndTypeConstant: Constant {
		const uint16_t nameRef;
		const uint16_t descriptorRef;
		const Utf8Constant* name;
		const Utf8Constant* descriptor;

		NameAndTypeConstant(const ConstantPool& constPool, uint16_t nameRef, uint16_t descriptorRef): Constant(constPool), nameRef(nameRef), descriptorRef(descriptorRef) {}

		virtual void init() override {
			name = constPool.get<Utf8Constant>(nameRef);
			descriptor = constPool.get<Utf8Constant>(descriptorRef);
		}
	};

	struct ReferenceConstant: Constant {
		const uint16_t classRef;
		const uint16_t nameAndTypeRef;
		const ClassConstant* clazz;
		const NameAndTypeConstant* nameAndType;

		ReferenceConstant(const ConstantPool& constPool, uint16_t classRef, uint16_t nameAndTypeRef): Constant(constPool), classRef(classRef), nameAndTypeRef(nameAndTypeRef) {}

		virtual void init() override {
			clazz = constPool.get<ClassConstant>(classRef);
			nameAndType = constPool.get<NameAndTypeConstant>(nameAndTypeRef);
		}
	};

	struct FieldrefConstant: ReferenceConstant {
		FieldrefConstant(const ConstantPool& constPool, uint16_t classRef, uint16_t nameAndTypeRef): ReferenceConstant(constPool, classRef, nameAndTypeRef) {}
	};

	struct MethodrefConstant: ReferenceConstant {
		MethodrefConstant(const ConstantPool& constPool, uint16_t classRef, uint16_t nameAndTypeRef): ReferenceConstant(constPool, classRef, nameAndTypeRef) {}
	};

	struct InterfaceMethodrefConstant: ReferenceConstant {
		InterfaceMethodrefConstant(const ConstantPool& constPool, uint16_t classRef, uint16_t nameAndTypeRef): ReferenceConstant(constPool, classRef, nameAndTypeRef) {}
	};

	struct MethodHandleConstant: Constant {
		const uint8_t refKind;
		const uint16_t refRef;

		MethodHandleConstant(const ConstantPool& constPool, uint16_t refKind, uint16_t refRef): Constant(constPool), refKind(refKind), refRef(refRef) {}
	};

	struct MethodTypeConstant: Constant {
		const uint16_t descriptorRef;

		MethodTypeConstant(const ConstantPool& constPool, uint16_t descriptorRef): Constant(constPool), descriptorRef(descriptorRef) {}
	};

	struct InvokeDynamicConstant: Constant {
		const uint16_t bootstrapMethodAttrRef;
		const uint16_t nameAndTypeRef;

		InvokeDynamicConstant(const ConstantPool& constPool, uint16_t bootstrapMethodAttrRef, uint16_t nameAndTypeRef): Constant(constPool), bootstrapMethodAttrRef(bootstrapMethodAttrRef), nameAndTypeRef(nameAndTypeRef) {}
	};
}

#endif
