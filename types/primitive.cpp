#ifndef JDECOMPILER_PRIMITIVE_TYPE_CPP
#define JDECOMPILER_PRIMITIVE_TYPE_CPP

namespace jdecompiler {

	struct PrimitiveType: BasicType {
		public:
			const string varName;

		private:
			PrimitiveType(const string& encodedName, const string& name, const string& varName):
					BasicType(encodedName, name), varName(varName) {}

			PrimitiveType(const PrimitiveType&) = delete;
			PrimitiveType& operator=(const PrimitiveType&) = delete;

			/* Allow only these types to inherit from PrimitiveType */
			friend struct VoidType;
			friend struct BooleanType;
			friend struct CharType;
			friend struct IntegralType;
			friend struct LongType;
			friend struct FloatType;
			friend struct DoubleType;

		public:
			virtual string toString() const override final {
				return name;
			}

			virtual string toString(const ClassInfo&) const override final {
				return name;
			}

			virtual string getVarName() const override final {
				return varName;
			}

			virtual bool isPrimitive() const override final {
				return true;
			}

		protected:
			virtual bool canReverseCast(const Type* other) const override final {
				return other->isSpecial();
			}

			virtual bool isSubtypeOfImpl(const Type* other) const override {
				return this == other;
			}

			virtual bool isStrictSubtypeOfImpl(const Type* other) const override final {
				return this == other;
			}

			virtual const Type* castImpl(const Type* other) const override {
				return this->isSubtypeOfImpl(other) ? this : nullptr;
			}

			virtual const Type* castToWidestImpl(const Type* other) const override {
				return this->isSubtypeOfImpl(other) ? other : nullptr;
			}

		public:
			virtual const Type* toVariableCapacityIntegralType() const {
				return this;
			}

			virtual status_t implicitCastStatus(const Type*) const override;

			virtual const ClassType& getWrapperType() const = 0;
	};
}

#endif
