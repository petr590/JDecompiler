#ifndef JDECOMPILER_ANY_TYPE_CPP
#define JDECOMPILER_ANY_TYPE_CPP

namespace jdecompiler {

	struct AnyType final: SpecialType {
		private: AnyType() {}

		public:
			virtual string toString(const ClassInfo& classinfo) const override {
				return OBJECT->toString(classinfo);
			}

			virtual string toString() const override {
				return "AnyType";
			}

			virtual string getEncodedName() const override final {
				return "SAnyType";
			}

			virtual const string& getName() const override final {
				static const string name("java.lang.Object");
				return name;
			}

			virtual string getVarName() const override final {
				return "o";
			}

			virtual TypeSize getSize() const override {
				return TypeSize::FOUR_BYTES; // ???
			}

			virtual bool isSubtypeOfImpl(const Type*) const override {
				return true;
			}

		protected:
			virtual const Type* castImpl(const Type* other) const override {
				return other;
			}

			virtual const Type* castToWidestImpl(const Type* other) const override {
				return other->isPrimitive() ? safe_cast<const PrimitiveType*>(other)->toVariableCapacityIntegralType() : other;
			}

		public:
			static const AnyType* getInstance() {
				static const AnyType instance;
				return &instance;
			}

			static const ArrayType* getArrayTypeInstance() {
				static const ArrayType instance(AnyType::getInstance());
				return &instance;
			}
	};

}

#endif