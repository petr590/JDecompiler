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

			virtual bool isInstanceof(const Type* type) const = 0;

			bool operator==(const Type& type) const {
				return this == &type || this->encodedName == type.encodedName;
			}


			template<class T>
			const T* castTo(const T* t) const {
				static_assert(is_base_of<Type, T>::value, "template class T of function Type::castTo is not subclass of class Type");
				return safe_cast<const T*>(castToImpl(static_cast<const Type*>(t)));
			}

		protected:
			virtual const Type* castToImpl(const Type*) const = 0;
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
				return types[0]->toString(classinfo);
			}

			virtual string toString() const override {
				return "Ambigous type {" + join<const Type*>(types, [](auto type) { return type->toString(); }) + '}';
			}

			virtual bool isPrimitive() const override final {
				return isPrimitiveTypes;
			}

			virtual TypeSize getSize() const override final {
				return size;
			}

			virtual bool isInstanceof(const Type* type) const override {
				if(*this == *type)
					return true;

				for(const Type* type : types)
					if(*type == *type)
						return true;

				if(const AmbigousType* ambigousType = dynamic_cast<const AmbigousType*>(type)) {
					for(const Type* type : ambigousType->types)
						if(find(types.begin(), types.end(), type) == types.end())
							return false;
					return true;
				}

				return false;
			}

		protected:
			virtual const Type* castToImpl(const Type* other) const override {
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

			virtual TypeSize getSize() const override {
				return size;
			}

			virtual bool isInstanceof(const Type* type) const override {
				return this == type;
			}

		protected:
			virtual const Type* castToImpl(const Type* other) const override {
				if(*this == *other)
					return this;

				throw DecompilationException("incopatible types " + toString() + " and " + other->toString());
			}
	};

	static const PrimitiveType<TypeSize::ZERO_BYTES>
			*const VOID = new PrimitiveType<TypeSize::ZERO_BYTES>("V", "void");

	static const PrimitiveType<TypeSize::FOUR_BYTES>
			*const BYTE = new PrimitiveType<TypeSize::FOUR_BYTES>("B", "byte"),
			*const CHAR = new PrimitiveType<TypeSize::FOUR_BYTES>("C", "char"),
			*const SHORT = new PrimitiveType<TypeSize::FOUR_BYTES>("S", "short"),
			*const INT = new PrimitiveType<TypeSize::FOUR_BYTES>("I", "int"),
			*const FLOAT = new PrimitiveType<TypeSize::FOUR_BYTES>("F", "float"),
			*const BOOLEAN = new PrimitiveType<TypeSize::FOUR_BYTES>("Z", "boolean");

	static const PrimitiveType<TypeSize::EIGHT_BYTES>
			*const LONG = new PrimitiveType<TypeSize::EIGHT_BYTES>("J", "long"),
			*const DOUBLE = new PrimitiveType<TypeSize::EIGHT_BYTES>("D", "double");

	static const AmbigousType
			*const ANY_INT_OR_BOOLEAN = new AmbigousType({BOOLEAN, BYTE, CHAR, SHORT, INT}),
			*const ANY_INT = new AmbigousType({BYTE, CHAR, SHORT, INT});


	struct ReferenceType: Type {
		protected:
			ReferenceType(const string& encodedName, const string& name): Type(encodedName, name) {}

			ReferenceType(): Type(EMPTY_STRING, EMPTY_STRING) {}

		public:
			virtual bool isPrimitive() const override final {
				return false;
			}

			virtual TypeSize getSize() const override final {
				return TypeSize::FOUR_BYTES;
			}

			virtual bool isInstanceof(const Type* type) const override {
				return this == type;
			}

		protected:
			virtual const Type* castToImpl(const Type* other) const override {
				if(*this == *other)
					return this;

				throw DecompilationException("incopatible types " + this->toString() + " and " + other->toString());
			}
	};


	static vector<const ReferenceType*> parseParameters(const char* str);


	struct ClassType final: ReferenceType {
		public:
			string simpleName, fullSimpleName /* fullSimpleName is a class name including enclosing class name */, packageName;
			const ClassType* enclosingClass;
			vector<const ReferenceType*> parameters;

			ClassType(const ClassConstant* clazz): ClassType(*clazz->name) {}

			ClassType(string encodedName) {
				const uint32_t length = encodedName.size();

				string name = encodedName;

				uint32_t nameStartPos = 0,
				         packageEndPos = 0,
				         enclosingClassNameEndPos = 0;

				for(uint32_t i = 0; i < length; i++) {
					char c = name[i];
					if(isLetterOrDigit(c))
						continue;

					switch(c) {
						case '/':
							nameStartPos = packageEndPos = i;
							name[i] = '.';
							break;
						case '$':
							nameStartPos = enclosingClassNameEndPos = i;
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
				this->encodedName = 'L' + encodedName;

				simpleName = nameStartPos == 0 ? name : string(name, nameStartPos + 1);

				packageName = string(name, 0, packageEndPos);

				if(enclosingClassNameEndPos == 0) {
					enclosingClass = nullptr;
					fullSimpleName = simpleName;
				} else {
					enclosingClass = new ClassType(string(name, 0, enclosingClassNameEndPos));
					fullSimpleName = simpleName + '.' + enclosingClass->fullSimpleName;
				}
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
			*const OBJECT = new ClassType("java/lang/Object"),
			*const STRING = new ClassType("java/lang/String"),
			*const CLASS = new ClassType("java/lang/Class"),
			*const METHOD_TYPE = new ClassType("java/lang/invoke/MethodType"),
			*const METHOD_HANDLE = new ClassType("java/lang/invoke/MethodHandle"),
			*const EXCEPTION = new ClassType("java/lang/Exception");


	struct ArrayType final: ReferenceType {
		public:
			const Type *memberType, *elementType;
			uint16_t nestingLevel = 0;

		private: string braces;

		public:
			ArrayType(const string& name) {
				size_t i = 0;
				for(char c = name[0]; c == '['; c = name[++i]) {
					nestingLevel++;
					braces += "[]";
				}

				memberType = parseType(&name[i]);
				elementType = nestingLevel == 1 ? memberType : new ArrayType(memberType, nestingLevel - 1);

				this->name = memberType->name + braces;
				this->encodedName = string(name, 0, memberType->encodedName.size() + nestingLevel);
			}

			ArrayType(const Type* memberType, uint16_t nestingLevel = 1): memberType(memberType), nestingLevel(nestingLevel) {
				for(uint16_t i = 0; i < nestingLevel; i++)
					braces += "[]";

				this->name = memberType->name + braces;
				this->encodedName = string(nestingLevel, '[') + memberType->encodedName;
			}

			ArrayType(const string& memberName, uint16_t nestingLevel): ArrayType(parseType(memberName), nestingLevel) {}

			virtual string toString(const ClassInfo& classinfo) const override {
				return memberType->toString(classinfo) + braces;
			}

			virtual string toString() const override {
				return memberType->toString() + braces;
			}
	};


	struct AnyType final: ReferenceType {
		private: AnyType(): ReferenceType("Ljava/lang/Object", "java.lang.Object") {}

		protected:
			virtual const Type* castToImpl(const Type* other) const override {
				return other;
			}

		public:
			virtual string toString(const ClassInfo& classinfo) const override {
				return OBJECT->toString(classinfo);
			}

			virtual string toString() const override {
				return OBJECT->toString();
			}

			static AnyType& getInstance() {
				static AnyType instance;
				return instance;
			}

			static ArrayType& getArrayTypeInstance() {
				static ArrayType instance(&AnyType::getInstance());
				return instance;
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
			case 'F': return FLOAT;
			case 'D': return DOUBLE;
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

		for(size_t i = 0; true; i++) {
			const ReferenceType* parameter;
			switch(str[i]) {
				case 'L':
					parameter = new ClassType(&str[i + 1]);
					i += parameter->encodedName.size() + 2u;
					break;
				case '[':
					parameter = new ArrayType(&str[i]);
					i += parameter->encodedName.size();
					break;
				case 'T':
					parameter = new ParameterType(&str[i]);
					i += parameter->encodedName.size() + 1u;
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
