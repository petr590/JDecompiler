#ifndef JDECOMPILER_PARAMETER_TYPE_CPP
#define JDECOMPILER_PARAMETER_TYPE_CPP

namespace jdecompiler {

	struct ParameterType final: ReferenceType {
		public:
			ParameterType(const char*& restrict str) {
				while(*str != ';')
					name += *(str++);

				++str;

				this->encodedName = name;
			}

			virtual string toString() const override {
				return '<' + name + '>';
			}

			virtual string toString(const ClassInfo&) const override final {
				return name;
			}

			virtual string getVarName() const override final {
				return toLowerCamelCase(name);
			}

		protected:
			virtual bool isSubtypeOfImpl(const Type* other) const override {
				return *this == *other;
			}
	};

}

#endif