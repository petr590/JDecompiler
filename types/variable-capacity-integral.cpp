#ifndef JDECOMPILER_VARIABLE_CAPACITY_INTEGRAL_TYPE_CPP
#define JDECOMPILER_VARIABLE_CAPACITY_INTEGRAL_TYPE_CPP

namespace jdecompiler {

	struct VariableCapacityIntegralType: SpecialType {
		public:
			const uint8_t minCapacity, maxCapacity;
			const bool includeBoolean, includeChar;

			static const uint8_t INCLUDE_BOOLEAN = 1, INCLUDE_CHAR = 2;
			static const uint8_t CHAR_CAPACITY = 2;

		private:
			const PrimitiveType* const highPrimitiveType;

			const string encodedName;

			static inline constexpr const PrimitiveType* primitiveTypeByCapacity(uint8_t capacity, bool includeChar) {
				if(includeChar && capacity == CHAR_CAPACITY) {
					return CHAR;
				}

				switch(capacity) {
					case 1: return BYTE;
					case 2: return SHORT;
					case 4: return INT;
					default:
						throw IllegalStateException((string)"Cannot find " + (includeChar ? "unsigned" : "signed") +
								" integral type for capacity " + to_string(capacity));
				}
			}

			VariableCapacityIntegralType(uint8_t minCapacity, uint8_t maxCapacity, bool includeBoolean, bool includeChar):
					minCapacity(minCapacity), maxCapacity(maxCapacity), includeBoolean(includeBoolean), includeChar(includeChar),
					highPrimitiveType(primitiveTypeByCapacity(maxCapacity, includeChar)),
					encodedName("SVariableCapacityIntegralType:" + to_string(minCapacity) + ':' + to_string(maxCapacity) + ':' +
							(char)('0' + includeBoolean + (includeChar << 1))) {}

		public:
			static const VariableCapacityIntegralType* getInstance(uint8_t minCapacity, uint8_t maxCapacity, bool includeBoolean, bool includeChar) {
				static vector<const VariableCapacityIntegralType*> instances;

				if(minCapacity > maxCapacity)
					return nullptr;

				for(const VariableCapacityIntegralType* instance : instances) {
					if(instance->minCapacity == minCapacity && instance->maxCapacity == maxCapacity &&
						instance->includeBoolean == includeBoolean && instance->includeChar == includeChar) {

						return instance;
					}
				}

				const VariableCapacityIntegralType* instance = new VariableCapacityIntegralType(minCapacity, maxCapacity, includeBoolean, includeChar);
				instances.push_back(instance);
				return instance;
			}

			static inline const VariableCapacityIntegralType* getInstance(uint8_t minCapacity, uint8_t maxCapacity, uint8_t flags = 0) {
				const VariableCapacityIntegralType* instance = getInstance(minCapacity, maxCapacity, flags & INCLUDE_BOOLEAN, flags & INCLUDE_CHAR);
				return instance != nullptr ? instance : throw IllegalArgumentException(
						(string)"minCapacity = " + to_string(minCapacity) + ", maxCapacity = " + to_string(maxCapacity) + ", flags = 0x" + hex<1>(flags));
			}

			virtual string toString(const ClassInfo& classinfo) const override {
				return highPrimitiveType->toString(classinfo);
			}

			virtual string toString() const override {
				return "VariableCapacityIntegralType(" + to_string(minCapacity) + ", " + to_string(maxCapacity) +
						(includeBoolean ? ", boolean" : "") + (includeChar ? ", char" : "") + ')';
			}

			virtual string getEncodedName() const override {
				return encodedName;
			}

			virtual const string& getName() const override {
				return highPrimitiveType->getName();
			}

			virtual string getVarName() const override {
				return highPrimitiveType->getVarName();
			}

			virtual TypeSize getSize() const override {
				return TypeSize::FOUR_BYTES;
			}

		protected:
			virtual bool canReverseCast(const Type* other) const {
				return true;
			}

			virtual bool isSubtypeOfImpl(const Type* other) const override {
				if(*this == *other || (other == BOOLEAN && includeBoolean) || other == highPrimitiveType)
					return true;

				if(other == CHAR)
					return includeChar || maxCapacity > CHAR_CAPACITY;

				if(other->isIntegral()) {
					const uint8_t capacity = safe_cast<const IntegralType*>(other)->getCapacity();
					return capacity >= minCapacity;
				}

				if(instanceof<const VariableCapacityIntegralType*>(other)) {
					return static_cast<const VariableCapacityIntegralType*>(other)->maxCapacity >= minCapacity;
				}

				return false;
			}

			virtual bool isStrictSubtypeOfImpl(const Type* other) const {
				if(*this == *other || (minCapacity == maxCapacity && other == highPrimitiveType && !includeBoolean && !includeChar))
					return true;

				if(minCapacity == maxCapacity && other->isIntegral())
					return safe_cast<const IntegralType*>(other)->getCapacity() == minCapacity;

				return false;
			}

			template<bool widest>
			static const Type* castImpl0(const VariableCapacityIntegralType* type, const Type* other) {

				if(other->isPrimitive()) {

					if(other == BOOLEAN)
						return type->includeBoolean ? other : nullptr;

					if(other == type->highPrimitiveType)
						return widest ? type : other;

					if(other == CHAR)
						return type->includeChar ? other : nullptr;

					if(other->isIntegral()) {
						const uint8_t capacity = safe_cast<const IntegralType*>(other)->getCapacity();

						if(capacity == type->minCapacity)
							return widest ? type : other;

						if(capacity == type->maxCapacity)
							return widest ? type : other;

						if(capacity > type->minCapacity)
							return getInstance(type->minCapacity, min(capacity, type->maxCapacity), false, type->includeChar && capacity > CHAR_CAPACITY);
					}
				}

				if(instanceof<const VariableCapacityIntegralType*>(other)) {
					return castImpl0(type, static_cast<const VariableCapacityIntegralType*>(other));
				}

				return nullptr;
			}

			static const Type* castImpl0(const VariableCapacityIntegralType* type, const VariableCapacityIntegralType* other) {
				return getInstance(type->minCapacity, min(other->maxCapacity, type->maxCapacity),
						type->includeBoolean && other->includeBoolean, type->includeChar && other->includeChar);
			}

			template<bool widest>
			const Type* reversedCastImpl0(const Type* other) const {

				if(other->isPrimitive()) {

					if(other == BOOLEAN)
						return includeBoolean ? other : nullptr;

					if(other == highPrimitiveType)
						return widest ? this : other;

					if(other == CHAR)
						return includeChar || maxCapacity > CHAR_CAPACITY ?
								(widest ? getInstance(CHAR_CAPACITY * 2, maxCapacity, false, includeChar) : other) : nullptr;

					if(other->isIntegral()) {
						const uint8_t capacity = safe_cast<const IntegralType*>(other)->getCapacity();

						if(widest ? capacity <= minCapacity : capacity >= maxCapacity)
							return this;

						if(widest ? capacity <= maxCapacity : capacity >= minCapacity) {
							return widest ? getInstance(max(capacity, minCapacity), capacity, false, includeChar) :
											getInstance(capacity, min(capacity, maxCapacity), false, includeChar);
						}
					}
				}

				if(instanceof<const VariableCapacityIntegralType*>(other))
					return castImpl0<false>(static_cast<const VariableCapacityIntegralType*>(other), this);

				return nullptr;
			}


			virtual const Type* castImpl(const Type* other) const override {
				return castImpl0<false>(this, other);
			}

			virtual const Type* reversedCastImpl(const Type* other) const override {
				return reversedCastImpl0<false>(other);
			}


			virtual const Type* castToWidestImpl(const Type* other) const override {
				return castImpl0<true>(this, other);
			}

			virtual const Type* reversedCastToWidestImpl(const Type* other) const override {
				return reversedCastImpl0<true>(other);
			}

			virtual const Type* getReducedType() const {
				return includeBoolean ? BOOLEAN : highPrimitiveType;
			}
	};


	static const VariableCapacityIntegralType
			*const   ANY_INT_OR_BOOLEAN = VariableCapacityIntegralType::getInstance(1, 4,
					VariableCapacityIntegralType::INCLUDE_BOOLEAN | VariableCapacityIntegralType::INCLUDE_CHAR),
			*const              ANY_INT = VariableCapacityIntegralType::getInstance(1, 4, VariableCapacityIntegralType::INCLUDE_CHAR),
			*const       ANY_SIGNED_INT = VariableCapacityIntegralType::getInstance(1, 4),
			*const CHAR_OR_SHORT_OR_INT = VariableCapacityIntegralType::getInstance(2, 4, VariableCapacityIntegralType::INCLUDE_CHAR),
			*const          CHAR_OR_INT = VariableCapacityIntegralType::getInstance(4, 4, VariableCapacityIntegralType::INCLUDE_CHAR),
			*const         SHORT_OR_INT = VariableCapacityIntegralType::getInstance(2, 4),
			*const      BYTE_OR_BOOLEAN = VariableCapacityIntegralType::getInstance(1, 1, VariableCapacityIntegralType::INCLUDE_BOOLEAN),
			*const       INT_OR_BOOLEAN = VariableCapacityIntegralType::getInstance(4, 4, VariableCapacityIntegralType::INCLUDE_BOOLEAN);


	const Type* ByteType::toVariableCapacityIntegralType() const {
		return ANY_INT;
	}

	const Type* CharType::toVariableCapacityIntegralType() const {
		return CHAR_OR_INT;
	}

	const Type* ShortType::toVariableCapacityIntegralType() const {
		return SHORT_OR_INT;
	}


}

#endif
