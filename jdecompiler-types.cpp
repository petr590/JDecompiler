#ifndef JDECOMPILER_TYPES_CPP
#define JDECOMPILER_TYPES_CPP

#include <algorithm>

#undef LOG_PREFIX
#define LOG_PREFIX "[ jdecompiler-types.cpp ]"

namespace JDecompiler {
	enum class TypeSize {
		ZERO_BYTES, FOUR_BYTES, EIGHT_BYTES
	};

	static string TypeSize_nameOf(TypeSize typeSize) {
		switch(typeSize) {
			case TypeSize::ZERO_BYTES: return "FOUR_BYTES";
			case TypeSize::FOUR_BYTES: return "FOUR_BYTES";
			case TypeSize::EIGHT_BYTES: return "EIGHT_BYTES";
			default: throw IllegalStateException("Illegal typeSize " + to_string((unsigned int)typeSize));
		}
	}

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

			virtual TypeSize getSize() const {
				return TypeSize::EIGHT_BYTES;
			}

			virtual const Type* getGeneralTypeFor(const Type*) const = 0;

			bool operator==(const Type& type) const {
				return this == &type || encodedName == type.encodedName;
			}
	};


	struct AmbigousType: Type {
		private:
			const vector<const Type*> types;
			const bool isPrimitiveTypes;
			const TypeSize size;

		public:
			AmbigousType(const vector<const Type*>& types): Type(EMPTY_STRING, EMPTY_STRING), types(types),
					isPrimitiveTypes(
						all_of(types.begin(), types.end(), [](auto type) { return  type->isPrimitive(); }) ? true :
						all_of(types.begin(), types.end(), [](auto type) { return !type->isPrimitive(); }) ? false :
						throw IllegalArgumentException("All types in type list must be only primitive or only non-primitive")
					),
					size(
						all_of(types.begin(), types.end(), [](auto type) { return type->getSize() == TypeSize::FOUR_BYTES; }) ? TypeSize::FOUR_BYTES :
						all_of(types.begin(), types.end(), [](auto type) { return type->getSize() == TypeSize::EIGHT_BYTES; }) ? TypeSize::EIGHT_BYTES :
						throw IllegalArgumentException("All types in type list must have same size")
					) {
				if(types.empty())
					throw IllegalArgumentException("Type list cannot be empty");
			}

			AmbigousType(initializer_list<const Type*> typeList): AmbigousType(vector<const Type*>(typeList)) {}

			virtual string toString(const ClassInfo& classinfo) const override {
				return types.back()->toString(classinfo);
			}

			virtual string toString() const override {
				return types.back()->toString();
			}

			virtual bool isPrimitive() const override final {
				return isPrimitiveTypes;
			}

			virtual TypeSize getSize() const override final {
				return size;
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


	template<TypeSize size>
	struct PrimitiveType final: Type {
		public:
			PrimitiveType(const string& encodedName, const string& name): Type(encodedName, name) {}

			virtual string toString() const override {
				return name;
			}

			virtual bool isPrimitive() const override {
				return true;
			}

			virtual TypeSize getSize() const override final {
				return size;
			}

			virtual const Type* getGeneralTypeFor(const Type* other) const override {
				if(*this == *other)
					return this;

				throw DecompilationException("incopatible types " + toString() + " and " + other->toString());
			}
	};

	static const PrimitiveType<TypeSize::FOUR_BYTES>
			*const BYTE = new PrimitiveType<TypeSize::FOUR_BYTES>("B", "byte"),
			*const CHAR = new PrimitiveType<TypeSize::FOUR_BYTES>("C", "char"),
			*const SHORT = new PrimitiveType<TypeSize::FOUR_BYTES>("S", "short"),
			*const INT = new PrimitiveType<TypeSize::FOUR_BYTES>("I", "int"),
			*const FLOAT = new PrimitiveType<TypeSize::FOUR_BYTES>("F", "float"),
			*const BOOLEAN = new PrimitiveType<TypeSize::FOUR_BYTES>("Z", "boolean");
	static const PrimitiveType<TypeSize::ZERO_BYTES>
			*const VOID = new PrimitiveType<TypeSize::ZERO_BYTES>("V", "void");

	static const PrimitiveType<TypeSize::EIGHT_BYTES>
			*const LONG = new PrimitiveType<TypeSize::EIGHT_BYTES>("J", "long"),
			*const DOUBLE = new PrimitiveType<TypeSize::EIGHT_BYTES>("D", "double");


	struct ReferenceType: Type {
		ReferenceType(const string& encodedName, const string& name): Type(encodedName, name) {}

		ReferenceType(): Type(EMPTY_STRING, EMPTY_STRING) {}

		virtual bool isPrimitive() const override final {
			return false;
		}

		virtual TypeSize getSize() const override final {
			return TypeSize::FOUR_BYTES;
		}

		virtual const Type* getGeneralTypeFor(const Type* other) const override final {
			if(*this == *other)
				return this;

			throw DecompilationException("incopatible types " + toString() + " and " + other->toString());
		}
	};


	static vector<const ReferenceType*> parseParameters(const char* str);


	struct ClassType final: ReferenceType {
		public:
			string simpleName, packageName;
			vector<const ReferenceType*> parameters;

			ClassType(const ClassConstant* clazz): ClassType(*clazz->name) {}

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
			*const STRING = new ClassType("java/lang/String"),
			*const CLASS = new ClassType("java/lang/Class"),
			*const METHOD_TYPE = new ClassType("java/lang/invoke/MethodType"),
			*const METHOD_HANDLE = new ClassType("java/lang/invoke/MethodHandle"),
			*const ANY_OBJECT = new ClassType("java/lang/Object"), // TODO
			*const EXCEPTION = new ClassType("java/lang/Exception");


	struct ArrayType final: ReferenceType {
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


	struct ParameterType final: ReferenceType {
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
