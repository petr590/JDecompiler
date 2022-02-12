#ifndef JDECOMPILER_TYPES_CPP
#define JDECOMPILER_TYPES_CPP

#include <algorithm>
#include <cassert>

#define inline INLINE_ATTR

#undef LOG_PREFIX
#define LOG_PREFIX "[ jdecompiler-types.cpp ]"

namespace JDecompiler {
	enum class TypeSize {
		ZERO_BYTES, FOUR_BYTES, EIGHT_BYTES
	};

	static string TypeSize_nameOf(TypeSize typeSize) {
		switch(typeSize) {
			case TypeSize::ZERO_BYTES: return "ZERO_BYTES";
			case TypeSize::FOUR_BYTES: return "FOUR_BYTES";
			case TypeSize::EIGHT_BYTES: return "EIGHT_BYTES";
			default: throw IllegalStateException("Illegal typeSize " + to_string((unsigned int)typeSize));
		}
	}

	struct Type: Stringified {
		protected:
			Type() {}

		public:
			virtual string toString() const = 0;

			virtual string toString(const ClassInfo&) const override {
				return toString();
			}

			virtual const string& getEncodedName() const = 0;

			virtual const string& getName() const = 0;


			virtual bool isPrimitive() const = 0;

			virtual TypeSize getSize() const = 0;

			virtual bool isBasic() const = 0;

			inline bool isSpecial() const {
			    assert(this != nullptr);
				return !isBasic();
			}

			template<class T>
			const T* castTo(const T* t) const {
				static_assert(is_base_of<Type, T>::value, "template class T of function Type::castTo is not subclass of class Type");

				if(const Type* castedType = this->isBasic() && t->isSpecial() ? // delegate cast to indefinite type
						t->reversedCastTo((const Type*)this) : this->castToImpl((const Type*)t))
					return safe_cast<const T*>(castedType);
				throw DecompilationException("incopatible types: " + this->toString() + " and " + t->toString());
			}

			const Type* reversedCastTo(const Type* other) const {
				if(const Type* castedType = this->castToImpl(other))
					return castedType;
				throw DecompilationException("incopatible types: " + other->toString() + " and " + this->toString());
			}

			bool isInstanceof(const Type* type) const {
			    return this->isBasic() && type->isSpecial() ? type->isInstanceofImpl(this) : this->isInstanceofImpl(type);
			}

		protected:
			virtual bool isInstanceofImpl(const Type* type) const = 0;

			virtual const Type* castToImpl(const Type*) const = 0;

		public:
			bool operator==(const Type& type) const {
				assert(this != nullptr);
				return this == &type || (typeid(*this) == typeid(type) && this->getEncodedName() == type.getEncodedName());
			}
	};


	struct BasicType: Type {
		protected:
			string encodedName, name;

			BasicType(const string& encodedName, const string& name): encodedName(encodedName), name(name) {}

		public:
			virtual const string& getEncodedName() const override final {
				return encodedName;
			}

			virtual const string& getName() const override final {
				return name;
			}

			virtual bool isBasic() const override final {
				return true;
			}
	};


	struct SpecialType: Type {
		protected:
			SpecialType() {}

		public:
			virtual bool isBasic() const override final {
				return false;
			}
	};


	template<TypeSize size>
	struct PrimitiveType final: BasicType {
		public:
			PrimitiveType(const string& encodedName, const string& name): BasicType(encodedName, name) {}

			virtual string toString() const override {
				return name;
			}

			virtual bool isPrimitive() const override {
				return true;
			}

			virtual TypeSize getSize() const override {
				return size;
			}

		protected:
			virtual bool isInstanceofImpl(const Type* type) const override {
				return this == type || (type->isSpecial() && type->isInstanceof(this));
			}

			virtual const Type* castToImpl(const Type* other) const override {
				return *this == *other ? this : nullptr;
			}
	};


	struct ReferenceType: BasicType {
		protected:
			ReferenceType(const string& encodedName, const string& name): BasicType(encodedName, name) {}

			ReferenceType(): BasicType(EMPTY_STRING, EMPTY_STRING) {}

		public:
			virtual bool isPrimitive() const override final {
				return false;
			}

			virtual TypeSize getSize() const override final {
				return TypeSize::FOUR_BYTES;
			}

		protected:
			virtual const Type* castToImpl(const Type* other) const override {
				return this->isInstanceofImpl(other) ? this : nullptr;
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
						case '[': // DEBUG
						    throw Exception("Illegal class name " + name);
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
				if(packageName != "java.lang" && packageName != classinfo.thisType.packageName)
					classinfo.imports.insert(name);
				return simpleName;
			}

			virtual string toString() const override {
				return "ClassType {" + name + '}';
			}

		protected:
			virtual bool isInstanceofImpl(const Type* other) const override {
				return dynamic_cast<const ClassType*>(other);
			}
	};


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

				this->name = memberType->getName() + braces;
				this->encodedName = string(name, 0, memberType->getEncodedName().size() + nestingLevel);

				assert(elementType != nullptr);
				assert(memberType != nullptr);
			}

			ArrayType(const Type& memberType, uint16_t nestingLevel = 1): ArrayType(&memberType, nestingLevel) {}

			ArrayType(const Type* memberType, uint16_t nestingLevel = 1): memberType(memberType), nestingLevel(nestingLevel) {
				assert(nestingLevel > 0);

				for(uint16_t i = 0; i < nestingLevel; i++)
					braces += "[]";

				this->name = memberType->getName() + braces;
				this->encodedName = string(nestingLevel, '[') + memberType->getEncodedName();

				this->memberType = memberType;
				this->elementType = nestingLevel == 1 ? memberType : new ArrayType(memberType, nestingLevel - 1);

				assert(elementType != nullptr);
				assert(memberType != nullptr);
			}

			ArrayType(const string& memberName, uint16_t nestingLevel): ArrayType(parseType(memberName), nestingLevel) {}

			virtual string toString(const ClassInfo& classinfo) const override {
				return memberType->toString(classinfo) + braces;
			}

			virtual string toString() const override {
				return "ArrayType {" + memberType->toString() + braces + '}';
			}

		protected:
			virtual bool isInstanceofImpl(const Type* other) const override {
				const ArrayType* arrayType = dynamic_cast<const ArrayType*>(other);
				return arrayType != nullptr && this->nestingLevel == arrayType->nestingLevel &&
						this->memberType->isInstanceof(arrayType->memberType);
			}
	};


	struct ParameterType final: ReferenceType {
	    public:
		    ParameterType(const char* encodedName) {
			    string name;
			    uint32_t i = 0;
			    for(char c = encodedName[0]; isLetterOrDigit(c); c = encodedName[++i])
				    name += c;
			    this->encodedName = this->name = name;
		    }

		    virtual string toString() const override {
			    return "ParameterType {" + name + '}';
		    }

		protected:
			virtual bool isInstanceofImpl(const Type* other) const override {
				return *this == *other;
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


	static const ClassType
			*const OBJECT = new ClassType("java/lang/Object"),
			*const STRING = new ClassType("java/lang/String"),
			*const CLASS = new ClassType("java/lang/Class"),
			*const METHOD_TYPE = new ClassType("java/lang/invoke/MethodType"),
			*const METHOD_HANDLE = new ClassType("java/lang/invoke/MethodHandle"),
			*const THROWABLE = new ClassType("java/lang/Throwable");



	struct AmbigousType: SpecialType {
		private:
			const vector<const Type*> types;
			const bool isPrimitiveTypes;
			const TypeSize size;

		public:
			AmbigousType(const vector<const Type*>& types): types(types),
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
				return "AmbigousType {" + join<const Type*>(types, [](auto type) { return type->toString(); }) + '}';
			}

			virtual const string& getEncodedName() const override final {
				return types[0]->getEncodedName();
			}

			virtual const string& getName() const override final {
				return types[0]->getName();
			}

			virtual bool isPrimitive() const override final {
				return isPrimitiveTypes;
			}

			virtual TypeSize getSize() const override final {
				return size;
			}

			virtual bool isInstanceofImpl(const Type* other) const override {
				if(*this == *other)
					return true;

				for(const Type* type : types)
					if(*type == *other)
						return true;

				if(const AmbigousType* ambigousType = dynamic_cast<const AmbigousType*>(other)) {
					for(const Type* type1 : types)
						for(const Type* type2 : ambigousType->types)
							if(*type1 == *type2)
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
					return newTypes.empty() ? nullptr : new AmbigousType(newTypes);
				}

				return nullptr;
			}
	};


	struct AnyType final: SpecialType {
		private: AnyType() {}

		public:
			virtual string toString(const ClassInfo& classinfo) const override {
				return OBJECT->toString(classinfo);
			}

			virtual string toString() const override {
				return "AnyType";
			}

			virtual const string& getEncodedName() const override final {
				static const string encodedName("Ljava/lang/Object");
				return encodedName;
			}

			virtual const string& getName() const override final {
				static const string name("java.lang.Object");
				return name;
			}

			virtual bool isPrimitive() const override {
				return false; // ???
			}

			virtual TypeSize getSize() const override {
				return TypeSize::FOUR_BYTES; // ???
			}

			virtual bool isInstanceofImpl(const Type* other) const override {
				return true;
			}

		protected:
			virtual const Type* castToImpl(const Type* other) const override {
				return other;
			}

		public:
			static AnyType& getInstance() {
				static AnyType instance;
				return instance;
			}

			static ArrayType& getArrayTypeInstance() {
				static ArrayType instance(&AnyType::getInstance());
				return instance;
			}
	};


	struct AnyObjectType final: SpecialType {
		private: AnyObjectType() {}

		public:
			virtual string toString(const ClassInfo& classinfo) const override {
				return OBJECT->toString(classinfo);
			}

			virtual string toString() const override {
				return "AnyObjectType";
			}

			virtual const string& getEncodedName() const override final {
				static const string encodedName("Ljava/lang/Object");
				return encodedName;
			}

			virtual const string& getName() const override final {
				static const string name("java.lang.Object");
				return name;
			}

			virtual bool isPrimitive() const override {
				return false;
			}

			virtual TypeSize getSize() const override {
				return TypeSize::FOUR_BYTES;
			}

			virtual bool isInstanceofImpl(const Type* other) const override {
				return *this == *other || !other->isPrimitive();
			}

		protected:
			virtual const Type* castToImpl(const Type* other) const override {
				return this->isInstanceofImpl(other) ? other : nullptr;
			}

		public:
			static AnyObjectType& getInstance() {
				static AnyObjectType instance;
				return instance;
			}

			static ArrayType& getArrayTypeInstance() {
				static ArrayType instance(&AnyObjectType::getInstance());
				return instance;
			}
	};


	static const AmbigousType
			*const ANY_INT_OR_BOOLEAN = new AmbigousType({BOOLEAN, BYTE, CHAR, SHORT, INT}),
			*const ANY_INT = new AmbigousType({BYTE, CHAR, SHORT, INT});




	static const BasicType* parseType(const char* encodedName) {
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

	static const BasicType* parseType(const string& encodedName) {
		return parseType(encodedName.c_str());
	}


	static const BasicType* parseReturnType(const char* encodedName) {
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
					i += parameter->getEncodedName().size() + 2u;
					break;
				case '[':
					parameter = new ArrayType(&str[i]);
					i += parameter->getEncodedName().size();
					break;
				case 'T':
					parameter = new ParameterType(&str[i]);
					i += parameter->getEncodedName().size() + 1u;
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

#undef inline

#endif
