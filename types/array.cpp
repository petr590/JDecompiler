#ifndef JDECOMPILER_ARRAY_TYPE_CPP
#define JDECOMPILER_ARRAY_TYPE_CPP

namespace jdecompiler {

	struct ArrayType final: ReferenceType {
		public:
			const Type *memberType, *elementType;
			uint16_t nestingLevel = 0;

			string braces;

			ArrayType(const string& str): ArrayType(str.c_str()) {}

			ArrayType(const char*&& str): ArrayType(static_cast<const char*&>(str)) {}

			ArrayType(const char*& restrict str) {

				const char* const srcStr = str;

				while(*str == '[')
					++str, ++nestingLevel;

				braces = repeat("[]", nestingLevel);

				const char* const memberTypeStart = str;

				memberType = parseType(str);
				elementType = nestingLevel == 1 ? memberType : new ArrayType(memberType, (uint16_t)(nestingLevel - 1));

				this->name = memberType->getName() + braces;
				this->encodedName = string(srcStr, 0, str - memberTypeStart + nestingLevel); // cut string
			}

			ArrayType(const Type& memberType, uint16_t nestingLevel = 1): ArrayType(&memberType, nestingLevel) {}

			ArrayType(const Type* memberType, uint16_t nestingLevel = 1): memberType(memberType) {
				if(nestingLevel == 0)
					throw IllegalArgumentException("nestingLevel cannot be zero");

				if(instanceof<const ArrayType*>(memberType)) {
					nestingLevel += static_cast<const ArrayType*>(memberType)->nestingLevel;
					memberType = static_cast<const ArrayType*>(memberType)->memberType;
				}

				this->nestingLevel = nestingLevel;

				braces = repeat("[]", nestingLevel);

				this->name = memberType->getName() + braces;
				this->encodedName = string(nestingLevel, '[') + memberType->getEncodedName();

				this->memberType = memberType;
				this->elementType = nestingLevel == 1 ? memberType : new ArrayType(memberType, (uint16_t)(nestingLevel - 1));
			}

			ArrayType(const string& memberName, uint16_t nestingLevel): ArrayType(parseType(memberName), nestingLevel) {}

			virtual string toString(const ClassInfo& classinfo) const override {
				return memberType->toString(classinfo) + braces;
			}

			virtual string toString() const override {
				return "class " + memberType->toString() + braces;
			}

			virtual string getVarName() const override {
				return (memberType->isPrimitive() ? memberType->getName() : memberType->getVarName()) + "Array";
			}

		protected:
			virtual bool isSubtypeOfImpl(const Type* other) const override {
				if(*other == *OBJECT) {
					return true;
				}

				const ArrayType* arrayType = dynamic_cast<const ArrayType*>(other);

				return arrayType != nullptr && ((this->nestingLevel == arrayType->nestingLevel && this->memberType->isSubtypeOf(arrayType->memberType))
						|| this->elementType->isSubtypeOf(arrayType->elementType));
			}
	};

}

#endif