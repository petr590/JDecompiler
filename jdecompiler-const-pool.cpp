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
		virtual void init(const ConstantPool& constPool) {};
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

		Utf8Constant(uint16_t length, const char* bytes): length(length), bytes(bytes) {}

		operator string() const {
			return string(bytes, length);
		}
	};

	struct ClassConstant: Constant {
		const uint16_t nameRef;
		const Utf8Constant* name;

		ClassConstant(uint16_t nameRef): nameRef(nameRef) {}

		virtual void init(const ConstantPool& constPool) override {
			name = constPool.get<Utf8Constant>(nameRef);
		}
	};


	template<typename T>
	struct ConstValueConstant: Constant {
		T value;
		ConstValueConstant(T value): value(value) {}
	};


	struct StringConstant: ConstValueConstant<const Utf8Constant*> {
		const uint16_t valueRef;

		StringConstant(uint16_t valueRef): ConstValueConstant(nullptr), valueRef(valueRef) {}

		virtual void init(const ConstantPool& constPool) override {
			value = constPool.get<Utf8Constant>(valueRef);
		}

		string toLiteral() const {
			#define checkLength(n) if(i + n >= length) throw Exception("Unexpected end of the string")
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
							checkLength(1);
							ch = (ch << 8) | (bytes[++i] & 0xFF);
							code = (ch & 0x1F00) >> 2 | (ch & 0x3F);
						} else if((ch & 0xF0) == 0xE0) {
							if(ch == 0xED) {
								checkLength(5);
								str += encodeUtf8(0x10000 | (bytes[++i] & 0xF) << 16 | (bytes[++i] & 0x3F) << 10 | (bytes[i+=2] & 0xF) << 6 | (bytes[++i] & 0x3F));
								continue;
							}
							checkLength(2);
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

		NameAndTypeConstant(uint16_t nameRef, uint16_t descriptorRef): nameRef(nameRef), descriptorRef(descriptorRef) {}

		virtual void init(const ConstantPool& constPool) override {
			name = constPool.get<Utf8Constant>(nameRef);
			descriptor = constPool.get<Utf8Constant>(descriptorRef);
		}
	};

	struct ReferenceConstant: Constant {
		const uint16_t classRef;
		const uint16_t nameAndTypeRef;
		const ClassConstant* clazz;
		const NameAndTypeConstant* nameAndType;

		ReferenceConstant(uint16_t classRef, uint16_t nameAndTypeRef): classRef(classRef), nameAndTypeRef(nameAndTypeRef) {}

		virtual void init(const ConstantPool& constPool) override {
			clazz = constPool.get<ClassConstant>(classRef);
			nameAndType = constPool.get<NameAndTypeConstant>(nameAndTypeRef);
		}
	};

	struct FieldrefConstant: ReferenceConstant {
		FieldrefConstant(uint16_t classRef, uint16_t nameAndTypeRef): ReferenceConstant(classRef, nameAndTypeRef) {}
	};

	struct MethodrefConstant: ReferenceConstant {
		MethodrefConstant(uint16_t classRef, uint16_t nameAndTypeRef): ReferenceConstant(classRef, nameAndTypeRef) {}
	};

	struct InterfaceMethodrefConstant: ReferenceConstant {
		InterfaceMethodrefConstant(uint16_t classRef, uint16_t nameAndTypeRef): ReferenceConstant(classRef, nameAndTypeRef) {}
	};


	struct MethodHandleConstant: Constant {
		enum class ReferenceKind {
			GETFIELD = 1, GETSTATIC, PUTFIELD, PUTSTATIC, INVOKEVIRTUAL, INVOKESTATIC, INVOKESPECIAL, NEWINVOKESPECIAL, INVOKEINTERFACE
		};

		const ReferenceKind referenceKind;
		const uint16_t referenceRef;

		const ReferenceConstant* reference;

		MethodHandleConstant(uint8_t referenceKind, uint16_t referenceRef): referenceKind((ReferenceKind)referenceKind), referenceRef(referenceRef) {
			if(referenceKind < 1 || referenceKind > 9)
				throw IllegalStateException("referenceKind is " + to_string(referenceKind) + ", must be in the range 1 to 9");
		}

		virtual void init(const ConstantPool& constPool) override {
			reference = constPool.get<ReferenceConstant>(referenceRef);
		}
	};


	struct MethodTypeConstant: Constant {
		const uint16_t descriptorRef;

		const Utf8Constant* descriptor;

		MethodTypeConstant(uint16_t descriptorRef): descriptorRef(descriptorRef) {}

		virtual void init(const ConstantPool& constPool) override {
			descriptor = constPool.get<Utf8Constant>(descriptorRef);
		}
	};

	struct InvokeDynamicConstant: Constant {
		const uint16_t bootstrapMethodAttrRef;
		const uint16_t nameAndTypeRef;

		const NameAndTypeConstant* nameAndType;

		InvokeDynamicConstant(uint16_t bootstrapMethodAttrRef, uint16_t nameAndTypeRef): bootstrapMethodAttrRef(bootstrapMethodAttrRef), nameAndTypeRef(nameAndTypeRef) {}

		virtual void init(const ConstantPool& constPool) override {
			nameAndType = constPool.get<NameAndTypeConstant>(nameAndTypeRef);
		}
	};
}

#endif
