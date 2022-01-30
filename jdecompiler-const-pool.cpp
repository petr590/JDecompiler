#ifndef JDECOMPILER_CONST_POOL_CPP
#define JDECOMPILER_CONST_POOL_CPP

#include <string>
#include <vector>
#include "jdecompiler.h"
#include "jdecompiler-util.cpp"
#include <iostream>

#undef LOG_PREFIX
#define LOG_PREFIX "[ jdecompiler-const-pool.cpp ]"

using namespace std;

namespace JDecompiler {
	struct Constant {
		virtual void init(const ConstantPool& constPool) {};
	};


	struct ConstantPool {
		private:
			const uint16_t size;
			Constant** const pool;

		public:
			ConstantPool(const uint16_t size): size(size), pool(new Constant*[size]) {}

			Constant*& operator[](int index) const {
				if(index < 0 || index >= size)
					throw ConstantPoolIndexOutOfBoundsException(index, size);
				return pool[index];
			}

			template<class T>
			T* get(uint16_t index) const {
				static_assert(is_base_of<Constant, T>::value, "template type T of method ConstantPool::get is not subclass of class Constant");
				if(index < 0 || index >= size)
					throw ConstantPoolIndexOutOfBoundsException(index, size);
				T* constant = dynamic_cast<T*>(pool[index]);
				if(constant == nullptr)
					throw DynamicCastException("Invalid constant pool reference 0x" + hex<4>(index) + " at " + typeid(T).name());
				return constant;
			}

			inline const Utf8Constant& getUtf8Constant(uint16_t index) const {
				return *get<Utf8Constant>(index);
			}
	};


	struct Utf8Constant: Constant, string {
		Utf8Constant(const char* str, size_t length): string(str, length) {}
	};


	struct ConstValueConstant: Constant {
		virtual string toString(const ClassInfo& classinfo) const = 0;
	};


	template<typename T>
	struct NumberConstant: ConstValueConstant {
		const T value;
		NumberConstant(T value): value(value) {}

		virtual string toString(const ClassInfo& classinfo) const override {
			return primitiveToString(value);
		}
	};


	using IntegerConstant = NumberConstant<int32_t>;
	using FloatConstant = NumberConstant<float>;
	using LongConstant = NumberConstant<int64_t>;
	using DoubleConstant = NumberConstant<double>;


	struct ClassConstant: ConstValueConstant {
		const uint16_t nameRef;
		const Utf8Constant* name;

		ClassConstant(uint16_t nameRef): nameRef(nameRef) {}

		virtual void init(const ConstantPool& constPool) override {
			name = constPool.get<Utf8Constant>(nameRef);
		}

		virtual string toString(const ClassInfo&) const override;
	};


	struct StringConstant: ConstValueConstant {
		const uint16_t valueRef;
		const Utf8Constant* value;

		StringConstant(uint16_t valueRef): valueRef(valueRef) {}

		virtual void init(const ConstantPool& constPool) override {
			value = constPool.get<Utf8Constant>(valueRef);
		}

		virtual string toString(const ClassInfo& classinfo) const override {
			#define checkLength(n) if(i + n >= length) throw Exception("Unexpected end of the string")
			const char* bytes = value->c_str();
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


	struct MethodHandleConstant: ConstValueConstant {
		public:
			enum class ReferenceKind {
				GETFIELD = 1, GETSTATIC, PUTFIELD, PUTSTATIC, INVOKEVIRTUAL, INVOKESTATIC, INVOKESPECIAL, NEWINVOKESPECIAL, INVOKEINTERFACE
			};

			enum class KindType { FIELD, METHOD };

			const ReferenceKind referenceKind;
			const KindType kindType;

			const uint16_t referenceRef;
			const ReferenceConstant* reference;

		private: static KindType getKindType(const ReferenceKind referenceKind) {
			switch(referenceKind) {
				case ReferenceKind::GETFIELD: case ReferenceKind::GETSTATIC: case ReferenceKind::PUTFIELD: case ReferenceKind::PUTSTATIC:
					return KindType::FIELD;
				default: return KindType::METHOD;
			}
		}

		public:
			MethodHandleConstant(uint8_t referenceKind, uint16_t referenceRef): referenceKind((ReferenceKind)referenceKind), kindType(getKindType((ReferenceKind)referenceKind)), referenceRef(referenceRef) {
				if(referenceKind < 1 || referenceKind > 9)
					throw IllegalStateException("referenceKind is " + to_string(referenceKind) + ", must be in the range 1 to 9");
			}

			virtual void init(const ConstantPool& constPool) override {
				reference = constPool.get<ReferenceConstant>(referenceRef);
			}

			virtual string toString(const ClassInfo& classinfo) const override {
				return "#MethodHandle#";
			}
	};


	struct MethodTypeConstant: ConstValueConstant {
		const uint16_t descriptorRef;

		const Utf8Constant* descriptor;

		MethodTypeConstant(uint16_t descriptorRef): descriptorRef(descriptorRef) {}

		virtual void init(const ConstantPool& constPool) override {
			descriptor = constPool.get<Utf8Constant>(descriptorRef);
		}

		virtual string toString(const ClassInfo&) const override;
	};

	struct InvokeDynamicConstant: Constant {
		const uint16_t bootstrapMethodAttrIndex;
		const uint16_t nameAndTypeRef;

		const NameAndTypeConstant* nameAndType;

		InvokeDynamicConstant(uint16_t bootstrapMethodAttrIndex, uint16_t nameAndTypeRef): bootstrapMethodAttrIndex(bootstrapMethodAttrIndex), nameAndTypeRef(nameAndTypeRef) {}

		virtual void init(const ConstantPool& constPool) override {
			nameAndType = constPool.get<NameAndTypeConstant>(nameAndTypeRef);
		}
	};
}

#endif
