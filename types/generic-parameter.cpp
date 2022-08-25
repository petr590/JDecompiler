#ifndef JDECOMPILER_GENERIC_PARAMETER_TYPE_CPP
#define JDECOMPILER_GENERIC_PARAMETER_TYPE_CPP

namespace jdecompiler {

	struct GenericParameter: Stringified {
		public:
			const string name;
			const vector<const ReferenceType*> types;

		private:
			static string parseName(const char*& restrict str) {
				string name;

				while(*str != ':')
					name += *(str++);
				++str;

				if(name.empty())
					throw InvalidSignatureException(str);

				return name;
			}

			static vector<const ReferenceType*> parseTypes(const char*& restrict str) {
				if(*str == ':')
					++str;

				vector<const ReferenceType*> types { parseParameter(str) };

				while(*str == ':') {
					types.push_back(parseParameter(str += 1));
				}

				return types;
			}

		public:
			GenericParameter(const char*& restrict str): name(parseName(str)), types(parseTypes(str)) {}

			virtual string toString(const ClassInfo& classinfo) const override {
				return types.size() == 1 && *types[0] == *OBJECT ? name : name + " extends " +
						join<const ReferenceType*>(types, [&classinfo] (const ReferenceType* type) { return type->toString(classinfo); }, " & ");
			}
	};

}

#endif