#ifndef JDECOMPILER_CONST_POOL_CPP
#define JDECOMPILER_CONST_POOL_CPP

#include <string>
#include <vector>
#include <iostream>
#include <type_traits>
#include "jdecompiler.h"
#include "jdecompiler-util.cpp"

#define inline INLINE_ATTR

#undef LOG_PREFIX
#define LOG_PREFIX "[ jdecompiler-const-pool.cpp ]"

namespace JDecompiler {

	using namespace std;


	struct Constant {
		virtual void init(const ConstantPool& constPool) {};

		virtual ~Constant() {}
	};


	struct ConstantPool {
		private:
			const uint16_t size;
			Constant** const pool;

			#define checkTemplate() static_assert(is_base_of<Constant, T>::value, "template type T of method ConstantPool::get is not subclass of class Constant")

		public:
			ConstantPool(const uint16_t size): size(size), pool(new Constant*[size]) {}

		private:
			inline void checkIndex(uint16_t index) const {
				if(index >= size)
					throw ConstantPoolIndexOutOfBoundsException(index, size);
			}

			template<class T>
			inline const T* get0(uint16_t index) const {
				if(const T* constant = dynamic_cast<const T*>(pool[index]))
					return constant;
				throw DynamicCastException("Invalid constant pool referenceConstant 0x" + hex<4>(index) + " at " + typeid(T).name());
			}

		public:

			Constant*& operator[](uint16_t index) const {
				checkIndex(index);
				return pool[index];
			}

			template<class T>
			const T* getNullablle(uint16_t index) const {
				checkTemplate();
				checkIndex(index);
				return dynamic_cast<const T*>(pool[index]);
			}

			template<class T>
			const T* get(uint16_t index) const {
				checkTemplate();
				checkIndex(index);
				return get0<T>(index);
			}

			template<class T>
			const T* getOrDefault(uint16_t index, function<const T*()> defaultValueGetter) const {
				checkTemplate();
				checkIndex(index);
				if(index == 0)
					return defaultValueGetter();
				return get0<T>(index);
			}

			inline const Utf8Constant& getUtf8Constant(uint16_t index) const {
				return *get<Utf8Constant>(index);
			}

			#undef checkTemplate
	};


	struct Utf8Constant: Constant, string {
		Utf8Constant(const char* str, size_t length): string(str, length) {}

		Utf8Constant(const char* str): string(str) {}
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

		ClassConstant(const Utf8Constant* name): nameRef(0), name(name) {}

		virtual void init(const ConstantPool& constPool) override {
			name = constPool.get<Utf8Constant>(nameRef);
		}

		virtual string toString(const ClassInfo&) const override;
	};


	struct StringConstant: ConstValueConstant {
		const uint16_t valueRef;
		const Utf8Constant* value;

		StringConstant(uint16_t valueRef): valueRef(valueRef) {}

		StringConstant(const Utf8Constant* value): valueRef(0), value(value) {}

		virtual void init(const ConstantPool& constPool) override {
			value = constPool.get<Utf8Constant>(valueRef);
		}

		virtual string toString(const ClassInfo&) const override {
			return stringToLiteral(*value);
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

	struct InterfaceMethodrefConstant: MethodrefConstant {
		InterfaceMethodrefConstant(uint16_t classRef, uint16_t nameAndTypeRef): MethodrefConstant(classRef, nameAndTypeRef) {}
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
			const ReferenceConstant* referenceConstant;

		private:
			static KindType getKindType(const ReferenceKind referenceKind) {
				switch(referenceKind) {
					case ReferenceKind::GETFIELD: case ReferenceKind::GETSTATIC: case ReferenceKind::PUTFIELD: case ReferenceKind::PUTSTATIC:
						return KindType::FIELD;
					default: return KindType::METHOD;
				}
		}

		public:
			MethodHandleConstant(uint8_t referenceKind, uint16_t referenceRef):
					referenceKind((ReferenceKind)referenceKind), kindType(getKindType((ReferenceKind)referenceKind)), referenceRef(referenceRef) {
				if(referenceKind < 1 || referenceKind > 9)
					throw IllegalStateException("referenceKind is " + to_string(referenceKind) + ", must be in the range 1 to 9");
			}

			virtual void init(const ConstantPool& constPool) override {
				referenceConstant = constPool.get<ReferenceConstant>(referenceRef);
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

		InvokeDynamicConstant(uint16_t bootstrapMethodAttrIndex, uint16_t nameAndTypeRef):
				bootstrapMethodAttrIndex(bootstrapMethodAttrIndex), nameAndTypeRef(nameAndTypeRef) {}

		virtual void init(const ConstantPool& constPool) override {
			nameAndType = constPool.get<NameAndTypeConstant>(nameAndTypeRef);
		}
	};
}

#undef inline

#endif
