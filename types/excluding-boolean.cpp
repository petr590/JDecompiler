#ifndef JDECOMPILER_EXCLUDING_BOOLEAN_TYPE_CPP
#define JDECOMPILER_EXCLUDING_BOOLEAN_TYPE_CPP

namespace jdecompiler {

	struct ExcludingBooleanType: SpecialType {
		private:
			constexpr ExcludingBooleanType() noexcept {}

		public:
			static const ExcludingBooleanType* getInstance() {
				static const ExcludingBooleanType instance;
				return &instance;
			}

			virtual string toString(const ClassInfo&) const override {
				return "ExcludingBooleanType";
			}

			virtual string toString() const override {
				return "ExcludingBooleanType";
			}

			virtual string getEncodedName() const override {
				return "SExcludingBooleanType";
			}

			virtual const string& getName() const override {
				static string name("ExcludingBooleanType");
				return name;
			}

			virtual string getVarName() const override {
				return "e";
			}

			virtual TypeSize getSize() const override {
				return TypeSize::FOUR_BYTES;
			}

			virtual bool isSubtypeOfImpl(const Type* other) const override {
				return castImpl(other) != nullptr;
			}

			virtual const Type* castImpl(const Type* other) const override {

				if(instanceof<const VariableCapacityIntegralType*>(other)) {
					const VariableCapacityIntegralType* intergalType = static_cast<const VariableCapacityIntegralType*>(other);

					return !intergalType->includeBoolean ? intergalType :
							VariableCapacityIntegralType::getInstance(intergalType->minCapacity, intergalType->maxCapacity, false, intergalType->includeChar);
				}

				return other != BOOLEAN ? other : nullptr;
			}

	};


}

#endif