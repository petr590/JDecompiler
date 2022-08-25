#ifndef JDECOMPILER_ANY_OBJECT_TYPE_CPP
#define JDECOMPILER_ANY_OBJECT_TYPE_CPP

namespace jdecompiler {

	struct AnyObjectType final: SpecialType {
		private: AnyObjectType() {}

		public:
			virtual string toString(const ClassInfo& classinfo) const override {
				return OBJECT->toString(classinfo);
			}

			virtual string toString() const override {
				return "AnyObjectType";
			}

			virtual string getEncodedName() const override final {
				return "SAnyObjectType";
			}

			virtual const string& getName() const override final {
				static const string name("java.lang.Object");
				return name;
			}

			virtual string getVarName() const override final {
				return "o";
			}

			virtual TypeSize getSize() const override {
				return TypeSize::FOUR_BYTES;
			}

			virtual bool isSubtypeOfImpl(const Type* other) const override {
				return this == other || (other->isBasic() && !other->isPrimitive());
			}

		protected:
			virtual const Type* castImpl(const Type* other) const override {
				return this->isSubtypeOfImpl(other) ? other : nullptr;
			}

		public:
			static const AnyObjectType* getInstance() {
				static const AnyObjectType instance;
				return &instance;
			}

			static const ArrayType* getArrayTypeInstance() {
				static const ArrayType instance(AnyObjectType::getInstance());
				return &instance;
			}
	};


}

#endif