#ifndef JDECOMPILER_TYPES_CPP
#define JDECOMPILER_TYPES_CPP

#include <algorithm>

#undef LOG_PREFIX
#define LOG_PREFIX "[ jdecompiler-types.cpp ]"

namespace JDecompiler {
	struct Type: Stringified {
		public:
			string encodedName, name;

		protected:
			Type(const string& encodedName, const string& name): encodedName(encodedName), name(name) {}

		public:
			virtual string toString() const = 0;

			virtual string toString(const ClassInfo&) const override {
				return toString();
			}

			virtual bool isPrimitive() const = 0;

			virtual const Type* getGeneralTypeFor(const Type*) const = 0;

			bool operator==(const Type& type) const {
				return this == &type || encodedName == type.encodedName;
			}
	};


	struct AmbigousType: Type {
		private:
			const vector<const Type*> types;
			const bool fieldIsPrimitive;

		public:
			AmbigousType(const vector<const Type*>& types): Type(EMPTY_STRING, EMPTY_STRING), types(types),
					fieldIsPrimitive(all_of(types.begin(), types.end(), [] (const Type* type) { return type->isPrimitive(); })) {
				if(types.empty())
					throw IllegalArgumentException("type list cannot be empty");
			}

			AmbigousType(initializer_list<const Type*> typeList): AmbigousType(vector<const Type*>(typeList)) {}

			virtual string toString(const ClassInfo& classinfo) const override {
				return types.back()->toString(classinfo);
			}

			virtual string toString() const override {
				return types.back()->toString();
			}

			virtual bool isPrimitive() const override {
				return fieldIsPrimitive;
			}

			virtual const Type* getGeneralTypeFor(const Type* other) const override {
				if(*this == *other)
					return this;

				for(const Type* type : types)
					if(*type == *other)
						return type;

				if(const AmbigousType* ambigousType = dynamic_cast<const AmbigousType*>(other)) {
					vector<const Type*> newTypes;
					for(const Type* type1 : types)
						for(const Type* type2 : ambigousType->types)
							if(*type1 == *type2)
								newTypes.push_back(type1);
					return new AmbigousType(newTypes);
				}

				throw DecompilationException("incopatible types " + toString() + " and " + other->toString());
			}
	};


	struct PrimitiveType: Type {
		public:
			PrimitiveType(const string& encodedName, const string& name): Type(encodedName, name) {}

			virtual string toString() const override {
				return name;
			}

			virtual bool isPrimitive() const override {
				return true;
			}

			virtual const Type* getGeneralTypeFor(const Type* other) const override {
				if(*this == *other)
					return this;

				throw DecompilationException("incopatible types " + toString() + " and " + other->toString());
			}
	};

	static const PrimitiveType
			* const BYTE = new PrimitiveType("B", "byte"),
			* const CHAR = new PrimitiveType("C", "char"),
			* const SHORT = new PrimitiveType("S", "short"),
			* const INT = new PrimitiveType("I", "int"),
			* const LONG = new PrimitiveType("J", "long"),
			* const DOUBLE = new PrimitiveType("D", "double"),
			* const FLOAT = new PrimitiveType("F", "float"),
			* const BOOLEAN = new PrimitiveType("Z", "boolean"),
			* const VOID = new PrimitiveType("V", "void");


	struct ReferenceType: Type {
		ReferenceType(const string& encodedName, const string& name): Type(encodedName, name) {}

		ReferenceType(): Type(EMPTY_STRING, EMPTY_STRING) {}

		virtual bool isPrimitive() const override {
			return false;
		}

		virtual const Type* getGeneralTypeFor(const Type* other) const override {
			if(*this == *other)
				return this;

			throw DecompilationException("incopatible types " + toString() + " and " + other->toString());
		}
	};


	static vector<const ReferenceType*> parseParameters(const char* str);


	struct ClassType: ReferenceType {
		public:
			string simpleName, packageName;
			vector<const ReferenceType*> parameters;

			ClassType(string encodedName) {
				const int length = encodedName.size();

				string name = encodedName;

				int pointPos = 0;

				for(int i = 0; i < length; i++) {
					char c = name[i];
					if(isLetterOrDigit(c)) continue;
					switch(c) {
						case '/': case '$':
							pointPos = i;
							name[i] = '.';
							break;
						case '<':
							parameters = parseParameters(&name[i]);
							break;
						case ';':
							name = string(name, 0, i);
							encodedName = string(encodedName, 0, i);
							goto ForEnd;
					}
				}
				ForEnd:

				this->name = name;
				this->encodedName = "L" + encodedName;

				simpleName = pointPos == 0 ? name : string(name, pointPos + 1);

				packageName = string(name, 0, pointPos);
			}

			virtual string toString(const ClassInfo& classinfo) const override {
				if(packageName != "java.lang" && packageName != classinfo.type->packageName)
					classinfo.imports.insert(name);
				return simpleName;
			}

			virtual string toString() const override {
				return name;
			}
	};

	static const ClassType
			* const STRING = new ClassType("java/lang/String"),
			* const CLASS = new ClassType("java/lang/Class"),
			* const METHOD_TYPE = new ClassType("java/lang/inkoke/MethodType"),
			* const METHOD_HANDLE = new ClassType("java/lang/inkoke/MethodHandle"),
			* const ANY_OBJECT = new ClassType("java/lang/Object"); // TODO


	struct ArrayType: ReferenceType {
		public:
			const Type *memberType, *elementType;
			uint16_t nestingLevel = 0;

		private: string braces;

		public:
			ArrayType(const string& name) {
				int i = 0;
				for(char c = name[0]; c == '['; c = name[++i]) {
					nestingLevel++;
					braces += "[]";
				}

				memberType = parseType(&name[i]);
				elementType = nestingLevel == 1 ? memberType : new ArrayType(memberType, nestingLevel - 1);

				this->name = memberType->name + braces;
				this->encodedName = string(name, 0, memberType->encodedName.size() + nestingLevel);
			}

			ArrayType(const Type* const memberType, uint16_t nestingLevel = 1): memberType(memberType), nestingLevel(nestingLevel) {
				for(uint16_t i = 0; i < nestingLevel; i++)
					braces += "[]";

				this->name = memberType->name + braces;
				this->encodedName = string('[', nestingLevel) + memberType->encodedName;
			}

			ArrayType(const string& memberName, uint16_t nestingLevel): ArrayType(parseType(memberName), nestingLevel) {}

			virtual string toString(const ClassInfo& classinfo) const override {
				return memberType->toString(classinfo) + braces;
			}

			virtual string toString() const override {
				return memberType->toString() + braces;
			}
	};


	struct ParameterType: ReferenceType {
		ParameterType(const char* encodedName) {
			string name;
			uint32_t i = 0;
			for(char c = encodedName[0]; isLetterOrDigit(c); c = encodedName[++i])
				name += c;
			this->encodedName = this->name = name;
		}

		virtual string toString() const override {
			return name;
		}
	};



	static const Type* parseType(const char* encodedName) {
		switch(encodedName[0]) {
			case 'B': return BYTE;
			case 'C': return CHAR;
			case 'S': return SHORT;
			case 'I': return INT;
			case 'J': return LONG;
			case 'D': return DOUBLE;
			case 'F': return FLOAT;
			case 'Z': return BOOLEAN;
			case 'L': return new ClassType(&encodedName[1]);
			case '[': return new ArrayType(encodedName);
			default:
				throw IllegalTypeNameException(encodedName);
		}
	}

	static const Type* parseType(const string& encodedName) {
		return parseType(encodedName.c_str());
	}


	static const Type* parseReturnType(const char* encodedName) {
		switch(encodedName[0]) {
			case 'V': return VOID;
			default: return parseType(encodedName);
		}
	}


	static const ReferenceType* parseReferenceType(const string& encodedName) {
		if(encodedName[0] == '[')
			return new ArrayType(encodedName);
		return new ClassType(encodedName);
	}


	static vector<const ReferenceType*> parseParameters(const char* str) {
		if(str[0] != '<')
			throw IllegalTypeNameException(str);

		vector<const ReferenceType*> parameters;

		for(int i = 0; true; i++) {
			const ReferenceType* parameter;
			switch(str[i]) {
				case 'L':
					parameter = new ClassType(&str[i + 1]);
					i += parameter->encodedName.size() + 2;
					break;
				case '[':
					parameter = new ArrayType(&str[i]);
					i += parameter->encodedName.size();
					break;
				case 'T':
					parameter = new ParameterType(&str[i]);
					i += parameter->encodedName.size() + 1;
					break;
				case '>':
					parameters = parseParameters(&str[i]);
					goto ForEnd;
			}
			parameters.push_back(parameter);
		}
		ForEnd:
		return parameters;
	}
}

#endif
