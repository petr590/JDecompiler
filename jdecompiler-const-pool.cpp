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
		const ConstantPool* const constPool;

		Constant(const ConstantPool* constPool): constPool(constPool) {};

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
					throw IndexOutOfBoundsException("Invalid constant pool reference 0x" + hex(index, 4));
				return pool[index];
			}

			template<class T> T* get(int index) const {
				if(index < 0 || index >= size)
					throw IndexOutOfBoundsException("Invalid constant pool reference 0x" + hex(index, 4));
				T* constant = dynamic_cast<T*>(pool[index]);
				if(constant == nullptr)
					throw DynamicCastException("Invalid constant pool reference 0x" + hex(index, 4) + " at " + typeid(T).name());
				return constant;
			}
	};


	struct Utf8Constant: Constant {
		const uint16_t length;
		const char* bytes;

		Utf8Constant(ConstantPool* constPool, uint16_t length, const char* bytes): Constant(constPool), length(length), bytes(bytes) {}

		operator string() const {
			return string(bytes, length);
		}
	};

	struct ClassConstant: Constant {
		const uint16_t nameRef;
		const Utf8Constant* name;

		ClassConstant(ConstantPool* constPool, uint16_t nameRef): Constant(constPool), nameRef(nameRef) {}

		virtual void init() override {
			name = constPool->get<Utf8Constant>(nameRef);
		}
	};


	template<typename T>
	struct ConstValueConstant: Constant {
		T value;
		ConstValueConstant(ConstantPool* constPool, T value): Constant(constPool), value(value) {}
	};


	struct StringConstant: ConstValueConstant<const Utf8Constant*> {
		const uint16_t valueRef;

		StringConstant(ConstantPool* constPool, uint16_t valueRef): ConstValueConstant(constPool, nullptr), valueRef(valueRef) {}

		virtual void init() override {
			value = constPool->get<Utf8Constant>(valueRef);
		}

		string toLiteral() const {
			const char* bytes = value->bytes;
			const uint32_t length = strlen(bytes);
			string str = {'"', '\0'};
			for(uint32_t i = 0; i < length; i++) {
				char32_t c = bytes[i] & 0xFF;
				switch(c) {
					case '"': str += "\\\""; break;
					case '\b': str += "\\b"; break;
					case '\t': str += "\\t"; break;
					case '\n': str += "\\n"; break;
					case '\f': str += "\\f"; break;
					case '\r': str += "\\r"; break;
					case '\\': str += "\\\\"; break;
					default:
						//cout << hex << ((c & 0xF0) == 0xE0) << dec << endl;
						if((c & 0xE0) == 0xC0){
							c = (c & 0x1F) << 6 | bytes[++i] & 0x3F;
						}else if((c & 0xF0) == 0xE0) {
							c = (c & 0xFF) == 0xED ?
									0x10000 | (bytes[++i] & 0xF) << 16 | (bytes[++i] & 0x3F) << 10 | (bytes[i+=2] & 0x0F) << 6 | bytes[++i] & 0x3F :
									(c & 0xF) << 12 | (bytes[++i] & 0x3F) << 6 | bytes[++i] & 0x3F;
							//cout << hex << c << dec << endl;
						}
						//cout << hex << "😎️" << dec << endl;
						//cout << hex << (uint32_t)'九' << dec << endl;
						str += c < 0x20 ? "\\u" + hex(c) : (char*)&c;
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

		NameAndTypeConstant(ConstantPool* constPool, uint16_t nameRef, uint16_t descriptorRef): Constant(constPool), nameRef(nameRef), descriptorRef(descriptorRef) {}

		virtual void init() override {
			name = constPool->get<Utf8Constant>(nameRef);
			descriptor = constPool->get<Utf8Constant>(descriptorRef);
		}
	};

	struct ReferenceConstant: Constant {
		const uint16_t classRef;
		const uint16_t nameAndTypeRef;
		const ClassConstant* clazz;
		const NameAndTypeConstant* nameAndType;

		ReferenceConstant(ConstantPool* constPool, uint16_t classRef, uint16_t nameAndTypeRef): Constant(constPool), classRef(classRef), nameAndTypeRef(nameAndTypeRef) {}

		virtual void init() override {
			clazz = constPool->get<ClassConstant>(classRef);
			nameAndType = constPool->get<NameAndTypeConstant>(nameAndTypeRef);
		}
	};

	struct FieldrefConstant: ReferenceConstant {
		FieldrefConstant(ConstantPool* constPool, uint16_t classRef, uint16_t nameAndTypeRef): ReferenceConstant(constPool, classRef, nameAndTypeRef) {}
	};

	struct MethodrefConstant: ReferenceConstant {
		MethodrefConstant(ConstantPool* constPool, uint16_t classRef, uint16_t nameAndTypeRef): ReferenceConstant(constPool, classRef, nameAndTypeRef) {}
	};

	struct InterfaceMethodrefConstant: ReferenceConstant {
		InterfaceMethodrefConstant(ConstantPool* constPool, uint16_t classRef, uint16_t nameAndTypeRef): ReferenceConstant(constPool, classRef, nameAndTypeRef) {}
	};

	struct MethodHandleConstant: Constant {
		const uint8_t refKind;
		const uint16_t refRef;

		MethodHandleConstant(ConstantPool* constPool, uint16_t refKind, uint16_t refRef): Constant(constPool), refKind(refKind), refRef(refRef) {}
	};

	struct MethodTypeConstant: Constant {
		const uint16_t descriptorRef;

		MethodTypeConstant(ConstantPool* constPool, uint16_t descriptorRef): Constant(constPool), descriptorRef(descriptorRef) {}
	};

	struct InvokeDynamicConstant: Constant {
		const uint16_t bootstrapMethodAttrRef;
		const uint16_t nameAndTypeRef;

		InvokeDynamicConstant(ConstantPool* constPool, uint16_t bootstrapMethodAttrRef, uint16_t nameAndTypeRef): Constant(constPool), bootstrapMethodAttrRef(bootstrapMethodAttrRef), nameAndTypeRef(nameAndTypeRef) {}
	};
}

#endif