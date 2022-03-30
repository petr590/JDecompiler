#ifndef JDECOMPILER_CONST_POOL_CPP
#define JDECOMPILER_CONST_POOL_CPP

#undef inline
#include <type_traits>
#define inline FORCE_INLINE
#include "util.cpp"

#define DEFINE_CONSTANT_NAME(name)\
	static constexpr const char* CONSTANT_NAME = #name;\
	virtual const char* getConstantName() const override {\
		return CONSTANT_NAME;\
	}

namespace jdecompiler {

	struct Constant {
		virtual const char* getConstantName() const = 0;

		virtual void init(const ConstantPool& constPool) {};

		virtual ~Constant() {}
	};


	struct ConstantPool {
		private:
			const uint16_t size;
			Constant** const pool;

			#define checkTemplate() static_assert(is_base_of<Constant, T>::value,\
					"template type T of method ConstantPool::get is not subclass of class Constant")

		public:
			ConstantPool(const uint16_t size): size(size), pool(new Constant*[size]) {
				for(uint16_t i = 0; i < size; i++)
					pool[i] = nullptr;
			}

		private:
			inline void checkIndex(uint16_t index) const {
				if(index >= size)
					throw ConstantPoolIndexOutOfBoundsException(index, size);
			}

			template<class T>
			inline const T* get0(uint16_t index) const {
				const Constant* const constant = pool[index];
				if(instanceof<const T*>(constant))
					return static_cast<const T*>(constant);
				throw InvalidConstantPoolReferenceException("Invalid constant pool reference " + hexWithPrefix<4>(index) +
						": expected " + T::CONSTANT_NAME + ", got " + (constant == nullptr ? "null" : constant->getConstantName()));
						//": expected " + typeNameOf(T) + ", got " + (constant == nullptr ? "null" : typeNameOf(*constant)));
			}

		public:

			Constant*& operator[] (uint16_t index) const {
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
		DEFINE_CONSTANT_NAME(Utf8);

		Utf8Constant(const char* str, size_t length): string(str, length) {}

		Utf8Constant(const char* str): string(str) {}
	};


	struct ConstValueConstant: Constant {
		DEFINE_CONSTANT_NAME(ConstantValue);

		virtual string toString(const ClassInfo& classinfo) const = 0;

		virtual const Operation* toOperation() const = 0;
	};


	template<typename T>
	struct NumberConstant: ConstValueConstant {
		const T value;
		NumberConstant(const T value): value(value) {}

		virtual string toString(const ClassInfo& classinfo) const override {
			return primitiveToString(value);
		}
	};


	struct IntegerConstant: NumberConstant<int32_t> { IntegerConstant(const int32_t value): NumberConstant(value) {}; DEFINE_CONSTANT_NAME(Integer);
		virtual const Operation* toOperation() const override;
	};
	struct FloatConstant:   NumberConstant<float>   {   FloatConstant(const   float value): NumberConstant(value) {}; DEFINE_CONSTANT_NAME(Float);
		virtual const Operation* toOperation() const override;};
	struct LongConstant:    NumberConstant<int64_t> {    LongConstant(const int64_t value): NumberConstant(value) {}; DEFINE_CONSTANT_NAME(Long);
		virtual const Operation* toOperation() const override;};
	struct DoubleConstant:  NumberConstant<double>  {  DoubleConstant(const  double value): NumberConstant(value) {}; DEFINE_CONSTANT_NAME(Double);
		virtual const Operation* toOperation() const override;};


	struct ClassConstant: ConstValueConstant {
		DEFINE_CONSTANT_NAME(Class);

		const uint16_t nameRef;
		const Utf8Constant* name;

		ClassConstant(uint16_t nameRef): nameRef(nameRef) {}

		ClassConstant(const Utf8Constant* name): nameRef(0), name(name) {}

		virtual void init(const ConstantPool& constPool) override {
			name = constPool.get<Utf8Constant>(nameRef);
		}

		virtual string toString(const ClassInfo&) const override;

		virtual const Operation* toOperation() const override;
	};


	struct StringConstant: ConstValueConstant {
		DEFINE_CONSTANT_NAME(String);

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

		virtual const Operation* toOperation() const override;
	};



	struct NameAndTypeConstant: Constant {
		DEFINE_CONSTANT_NAME(NameAndType);

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
		DEFINE_CONSTANT_NAME(Reference);

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
		DEFINE_CONSTANT_NAME(Fieldref);

		FieldrefConstant(uint16_t classRef, uint16_t nameAndTypeRef): ReferenceConstant(classRef, nameAndTypeRef) {}
	};

	struct MethodrefConstant: ReferenceConstant {
		DEFINE_CONSTANT_NAME(Methodref);

		MethodrefConstant(uint16_t classRef, uint16_t nameAndTypeRef): ReferenceConstant(classRef, nameAndTypeRef) {}
	};

	struct InterfaceMethodrefConstant: MethodrefConstant {
		DEFINE_CONSTANT_NAME(InterfaceMethodref);

		InterfaceMethodrefConstant(uint16_t classRef, uint16_t nameAndTypeRef): MethodrefConstant(classRef, nameAndTypeRef) {}
	};


	struct MethodHandleConstant: ConstValueConstant {
		DEFINE_CONSTANT_NAME(MethodHandle);

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

			virtual const Operation* toOperation() const override;
	};


	struct MethodTypeConstant: ConstValueConstant {
		DEFINE_CONSTANT_NAME(MethodType);

		const uint16_t descriptorRef;

		const Utf8Constant* descriptor;

		MethodTypeConstant(uint16_t descriptorRef): descriptorRef(descriptorRef) {}

		virtual void init(const ConstantPool& constPool) override {
			descriptor = constPool.get<Utf8Constant>(descriptorRef);
		}

		virtual string toString(const ClassInfo&) const override;

		virtual const Operation* toOperation() const override;
	};

	struct InvokeDynamicConstant: Constant {
		DEFINE_CONSTANT_NAME(InvokeDynamic);

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

#undef DEFINE_CONSTANT_NAME
#endif
