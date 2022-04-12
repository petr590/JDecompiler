#ifndef JDECOMPILER_TYPES_CPP
#define JDECOMPILER_TYPES_CPP

#undef inline
#include <cassert>
#define inline INLINE
#include "const-pool.cpp"
#include "jdecompiler-instance.cpp"
#include "classinfo.cpp"

namespace jdecompiler {

	enum class TypeSize {
		ZERO_BYTES, FOUR_BYTES, EIGHT_BYTES
	};


	static const char* TypeSize_nameOf(const TypeSize typeSize) {
		switch(typeSize) {
			case TypeSize::ZERO_BYTES: return "ZERO_BYTES";
			case TypeSize::FOUR_BYTES: return "FOUR_BYTES";
			case TypeSize::EIGHT_BYTES: return "EIGHT_BYTES";
			default: throw IllegalStateException("Illegal typeSize " + to_string((unsigned int)typeSize));
		}
	}


	struct Type: Stringified {
		protected:
			constexpr Type() noexcept {}

		public:
			virtual string toString() const = 0;

			virtual string toString(const ClassInfo&) const override = 0;

			virtual string getEncodedName() const = 0;

			virtual const string& getName() const = 0;

			virtual string getVarName() const = 0;


			virtual bool isPrimitive() const = 0;

			virtual TypeSize getSize() const = 0;

			virtual bool isBasic() const = 0;

			inline bool isSpecial() const {
				return !isBasic();
			}

		private:
			template<class T>
			static constexpr void checkType() {
				static_assert(is_base_of<Type, T>::value, "template class T must be subclass of class Type");
			}


			template<bool isNoexcept>
			const Type* getNullType(const Type* type) const {
				if constexpr(isNoexcept)
					return nullptr;
				else
					throw DecompilationException("incopatible types: " + this->toString() + " and " + type->toString());
			}


			template<bool isNoexcept>
			const Type* cast0(const Type* type) const {
				const Type* castedType;

				if((castedType = this->castImpl(type)) != nullptr)
					return castedType;

				if(this->canReverseCast(type) && (castedType = type->reversedCastImpl(this)) != nullptr)
					return castedType;

				return getNullType<isNoexcept>(type);
			}

			template<bool isNoexcept>
			const Type* castToNarrowest0(const Type* type) const {
				const Type* castedType;

				if((castedType = this->castToNarrowestImpl(type)) != nullptr)
					return castedType;

				return getNullType<isNoexcept>(type);
			}

		public:
			template<class T>
			inline const T* castTo(const T* type) const {
				checkType<T>();
				return safe_cast<const T*>(cast0<false>(type));
			}

			template<class T>
			inline const T* castNoexcept(const T* type) const {
				checkType<T>();
				return safe_cast<const T*>(cast0<true>(type));
			}


			template<class T>
			inline const T* castToNarrowest(const T* type) const {
				checkType<T>();
				return safe_cast<const T*>(castToNarrowest0<false>(type));
			}

			template<class T>
			inline const T* castToNarrowestNoexcept(const T* type) const {
				checkType<T>();
				return safe_cast<const T*>(castToNarrowest0<true>(type));
			}


			template<class T>
			const T* twoWayCastTo(const T* t) const {
				checkType<T>();

				const Type* castedType;

				if((castedType = this->castImpl(t)) != nullptr)
					return safe_cast<const T*>(castedType);

				if((castedType = ((const Type*)t)->castImpl(this)) != nullptr)
					return safe_cast<const T*>(castedType);

				if((castedType = this->reversedCastImpl(t)) != nullptr)
					return safe_cast<const T*>(castedType);

				if((castedType = ((const Type*)t)->reversedCastImpl(this)) != nullptr)
					return safe_cast<const T*>(castedType);

				throw DecompilationException("incopatible types: " + this->toString() + " and " + t->toString());
			}

			bool isSubtypeOf(const Type* type) const {
				return this->isSubtypeOfImpl(type) || (this->canReverseCast(type) && type->isSubtypeOfImpl(this));
			}

			bool isStrictSubtypeOf(const Type* type) const {
				return this->isStrictSubtypeOfImpl(type) || (this->canReverseCast(type) && type->isStrictSubtypeOfImpl(this));
			}

		protected:
			virtual bool canReverseCast(const Type* other) const {
				return true;
			}

			virtual bool isSubtypeOfImpl(const Type* type) const = 0;

			virtual bool isStrictSubtypeOfImpl(const Type* type) const {
				return isSubtypeOfImpl(type);
			}


			virtual const Type* castImpl(const Type* type) const = 0;

			virtual const Type* castToNarrowestImpl(const Type* type) const {
				return castImpl(type);
			}

			virtual const Type* reversedCastImpl(const Type* type) const {
				return castImpl(type);
			}

		public:
			inline friend bool operator== (const Type& type1, const Type& type2) {
				return &type1 == &type2 || (typeid(type1) == typeid(type2) && type1.getEncodedName() == type2.getEncodedName());
			}

			inline friend bool operator!= (const Type& type1, const Type& type2) {
				return !(type1 == type2);
			}
	};


	struct BasicType: Type {
		protected:
			string encodedName, name;

			BasicType(const string& encodedName, const string& name): encodedName(encodedName), name(name) {}

		public:
			virtual string getEncodedName() const override final {
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
			constexpr SpecialType() noexcept {}

		public:
			virtual bool isBasic() const override final {
				return false;
			}
	};


	template<TypeSize size>
	struct PrimitiveType final: BasicType {
		public:
			const string varName;

			PrimitiveType(const string& encodedName, const string& name, const string& varName): BasicType(encodedName, name), varName(varName) {}

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

			virtual TypeSize getSize() const override final {
				return size;
			}

		protected:
			virtual bool canReverseCast(const Type* other) const override final {
				return other->isSpecial();
			}

			virtual bool isSubtypeOfImpl(const Type* other) const override {
				return this == other;
			}

			virtual bool isStrictSubtypeOfImpl(const Type* other) const override {
				return this == other;
			}

			virtual const Type* castToNarrowestImpl(const Type* other) const override {
				return this->isSubtypeOfImpl(other) ? this : nullptr;
			}

			virtual const Type* castImpl(const Type* other) const override {
				return this->isSubtypeOfImpl(other) ? other : nullptr;
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
			virtual const Type* castImpl(const Type* other) const override {
				return this->isSubtypeOfImpl(other) ? this : nullptr;
			}
	};


	struct ClassType final: ReferenceType {
		public:
			string simpleName, fullSimpleName /* fullSimpleName is a class name including enclosing class name */, packageName;
			vector<const ReferenceType*> parameters;

			const ClassType* enclosingClass;
			bool isAnonymous = false;

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
						case ';': case '\0':
							name = string(name, 0, i);
							encodedName = string(encodedName, 0, i);
							goto ForEnd;
						default:
							throw InvalidClassNameException(encodedName);
					}
				}
				ForEnd:

				this->name = name;
				this->encodedName = 'L' + encodedName;

				simpleName = nameStartPos == 0 ? name : string(name, nameStartPos + 1);

				packageName = name.substr(0, packageEndPos);

				if(enclosingClassNameEndPos == 0) {
					enclosingClass = nullptr;
					fullSimpleName = simpleName;
				} else {
					isAnonymous = all_of(simpleName.begin(), simpleName.end(), [] (unsigned char c) { return isdigit(c); });
					enclosingClass = new ClassType(encodedName.substr(0, enclosingClassNameEndPos));
					if(isAnonymous)
						this->name[enclosingClassNameEndPos] = '$';
					fullSimpleName = enclosingClass->fullSimpleName + (isAnonymous ? '$' : '.') + simpleName;
				}
			}

			virtual string toString(const ClassInfo& classinfo) const override;

			virtual string toString() const override {
				return "ClassType {" + name + '}';
			}

			virtual string getVarName() const override final {
				return toLowerCamelCase(simpleName);
			}

		protected:
			virtual bool isSubtypeOfImpl(const Type* other) const override {
				return instanceof<const ClassType*>(other);
			}
	};


	static const ClassType
			*const OBJECT(new ClassType("java/lang/Object")),
			*const STRING(new ClassType("java/lang/String")),
			*const CLASS(new ClassType("java/lang/Class")),
			*const METHOD_TYPE(new ClassType("java/lang/invoke/MethodType")),
			*const METHOD_HANDLE(new ClassType("java/lang/invoke/MethodHandle")),
			*const THROWABLE(new ClassType("java/lang/Throwable"));


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
				elementType = nestingLevel == 1 ? memberType : new ArrayType(memberType, (uint16_t)(nestingLevel - 1));

				this->name = memberType->getName() + braces;
				this->encodedName = string(name, 0, memberType->getEncodedName().size() + nestingLevel);
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

				for(uint16_t i = 0; i < nestingLevel; i++)
					braces += "[]";

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
				return "ArrayType {" + memberType->toString() + braces + '}';
			}

			virtual string getVarName() const override {
				return memberType->getVarName() + "Array";
			}

		protected:
			virtual bool isSubtypeOfImpl(const Type* other) const override {
				if(*other == *OBJECT) {
					return true;
				}

				const ArrayType* arrayType = dynamic_cast<const ArrayType*>(other);

				return arrayType != nullptr && (this->nestingLevel == arrayType->nestingLevel && this->memberType->isSubtypeOf(arrayType->memberType)
						|| this->elementType->isSubtypeOf(arrayType->elementType));
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

			virtual string toString(const ClassInfo& classinfo) const override final {
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


	/*static constexpr const PrimitiveType<TypeSize::ZERO_BYTES>
			VOID_TYPE("V", "void");

	static constexpr const PrimitiveType<TypeSize::FOUR_BYTES>
			BYTE_TYPE("B", "byte"),
			CHAR_TYPE("C", "char"),
			SHORT_TYPE("S", "short"),
			INT_TYPE("I", "int"),
			FLOAT_TYPE("F", "float"),
			BOOLEAN_TYPE("Z", "boolean");

	static constexpr const PrimitiveType<TypeSize::EIGHT_BYTES>
			LONG_TYPE("J", "long"),
			DOUBLE_TYPE("D", "double");


	static constexpr const PrimitiveType<TypeSize::ZERO_BYTES>
			*const VOID = &VOID_TYPE;

	static constexpr const PrimitiveType<TypeSize::FOUR_BYTES>
			*const BYTE = &BYTE_TYPE,
			*const CHAR = &CHAR_TYPE,
			*const SHORT = &SHORT_TYPE,
			*const INT = &INT_TYPE,
			*const FLOAT = &FLOAT_TYPE,
			*const BOOLEAN = &BOOLEAN_TYPE;

	static constexpr const PrimitiveType<TypeSize::EIGHT_BYTES>
			*const LONG = &LONG_TYPE,
			*const DOUBLE = &DOUBLE_TYPE;*/


	static const PrimitiveType<TypeSize::ZERO_BYTES>
			*const VOID(new PrimitiveType<TypeSize::ZERO_BYTES>("V", "void", "v"));

	static const PrimitiveType<TypeSize::FOUR_BYTES>
			*const BYTE(new PrimitiveType<TypeSize::FOUR_BYTES>("B", "byte", "b")),
			*const CHAR(new PrimitiveType<TypeSize::FOUR_BYTES>("C", "char", "c")),
			*const SHORT(new PrimitiveType<TypeSize::FOUR_BYTES>("S", "short", "s")),
			*const INT(new PrimitiveType<TypeSize::FOUR_BYTES>("I", "int", "n")),
			*const FLOAT(new PrimitiveType<TypeSize::FOUR_BYTES>("F", "float", "f")),
			*const BOOLEAN(new PrimitiveType<TypeSize::FOUR_BYTES>("Z", "boolean", "bool"));

	static const PrimitiveType<TypeSize::EIGHT_BYTES>
			*const LONG(new PrimitiveType<TypeSize::EIGHT_BYTES>("J", "long", "l")),
			*const DOUBLE(new PrimitiveType<TypeSize::EIGHT_BYTES>("D", "double", "d"));


	template<>
	bool PrimitiveType<TypeSize::FOUR_BYTES>::isSubtypeOfImpl(const Type* other) const {
		return this == other || (other == INT && (this == BYTE || this == CHAR || this == SHORT)) || // allow cast byte, char and short to int
						(other == SHORT && this == BYTE); // allow casting byte to short; casting byte to char is not allowed
	}



	struct AmbigousType: SpecialType {
		public:
			const vector<const BasicType*> types;

		protected:
			const bool isPrimitiveTypes;
			const TypeSize size;

		public:
			AmbigousType(const vector<const BasicType*>& types): types(types),
					isPrimitiveTypes(
						all_of(types.begin(), types.end(), [] (auto type) { return  type->isPrimitive(); }) ? true :
						all_of(types.begin(), types.end(), [] (auto type) { return !type->isPrimitive(); }) ? false :
						throw IllegalArgumentException("All types in type list must be only primitive or only non-primitive")
					),
					size(
						all_of(types.begin(), types.end(), [] (auto type) { return type->getSize() == TypeSize::FOUR_BYTES; }) ? TypeSize::FOUR_BYTES :
						all_of(types.begin(), types.end(), [] (auto type) { return type->getSize() == TypeSize::EIGHT_BYTES; }) ? TypeSize::EIGHT_BYTES :
						throw IllegalArgumentException("All types in type list must have same size")
					) {
				if(types.empty())
					throw IllegalArgumentException("Type list cannot be empty");
			}

			AmbigousType(const initializer_list<const BasicType*> typeList): AmbigousType(vector<const BasicType*>(typeList)) {}

			virtual string toString(const ClassInfo& classinfo) const override {
				return types[0]->toString(classinfo);
			}

			virtual string toString() const override {
				return "AmbigousType {" + join<const BasicType*>(types, [] (auto type) { return type->toString(); }) + '}';
			}

			virtual string getEncodedName() const override final {
				return "SAmbigousType(" + join<const BasicType*>(types, [] (auto type) { return type->getEncodedName(); }) + ')';
			}

			virtual const string& getName() const override final {
				return types[0]->getName();
			}

			virtual string getVarName() const override final {
				return types[0]->getVarName();
			}

			virtual bool isPrimitive() const override final {
				return isPrimitiveTypes;
			}

			virtual TypeSize getSize() const override final {
				return size;
			}

			virtual bool isSubtypeOfImpl(const Type* other) const override {
				if(isStrictSubtypeOfImpl(other))
					return true;

				if(other->isBasic())
					for(const BasicType* type : types)
						if(type->isSubtypeOf(other))
							return true;

				/*if(instanceof<const AmbigousType*>(other)) {
					for(const Type* type1 : types)
						for(const Type* type2 : static_cast<const AmbigousType*>(other)->types)
							if(*type1 == *type2)
								return true;
				}*/

				return false;
			}

			virtual bool isStrictSubtypeOfImpl(const Type* other) const override {
				if(*this == *other)
					return true;

				if(other->isBasic())
					for(const BasicType* type : types)
						if(type == other)
							return true;

				if(instanceof<const AmbigousType*>(other)) {
					for(const Type* type1 : types)
						for(const Type* type2 : static_cast<const AmbigousType*>(other)->types)
							if(*type1 == *type2)
								return true;
				}

				return false;
			}

		protected:
			template<bool direct>
			const Type* castImpl0(const Type* other) const {

				if(*this == *other)
					return this;

				if(other->isBasic()) {
					vector<const BasicType*> newTypes;
					for(const BasicType* type : types) {
						const BasicType* castedType = safe_cast<const BasicType*>(
								direct ? type->castToNarrowestNoexcept(other) : other->castToNarrowestNoexcept(type));
						if(castedType != nullptr && find(newTypes.begin(), newTypes.end(), castedType) == newTypes.end()) {
							newTypes.push_back(castedType);
						}
					}
					return newTypes.empty() ? nullptr : newTypes.size() == 1 ? newTypes[0] : (const Type*)new AmbigousType(newTypes);
				}

				if(instanceof<const AmbigousType*>(other)) {
					vector<const BasicType*> newTypes;
					for(const BasicType* type1 : types) {
						for(const BasicType* type2 : static_cast<const AmbigousType*>(other)->types) {
							const BasicType* newType = dynamic_cast<const BasicType*>(direct ? type1->castNoexcept(type2) : type2->castNoexcept(type1));
							if(newType != nullptr && find(newTypes.begin(), newTypes.end(), newType) == newTypes.end())
								newTypes.push_back(newType);
						}
					}
					return newTypes.empty() ? nullptr : new AmbigousType(newTypes);
				}

				return nullptr;
			}

			virtual const Type* castImpl(const Type* other) const override {
				return castImpl0<true>(other);
			}

			virtual const Type* reversedCastImpl(const Type* other) const override {
				return castImpl0<false>(other);
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

			virtual string getEncodedName() const override final {
				return "SAnyType";
			}

			virtual const string& getName() const override final {
				static const string name("java.lang.Object");
				return name;
			}

			virtual string getVarName() const override final {
				return "o";
			}

			virtual bool isPrimitive() const override {
				return false; // ???
			}

			virtual TypeSize getSize() const override {
				return TypeSize::FOUR_BYTES; // ???
			}

			virtual bool isSubtypeOfImpl(const Type* other) const override {
				return true;
			}

		protected:
			virtual const Type* castImpl(const Type* other) const override {
				return other;
			}

		public:
			static const AnyType* getInstance() {
				static const AnyType instance;
				return &instance;
			}

			static const ArrayType* getArrayTypeInstance() {
				static const ArrayType instance(AnyType::getInstance());
				return &instance;
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

			virtual string getEncodedName() const override final {
				return "SAnyObjectType";
			}

			virtual const string& getName() const override final {
				static const string name("java.lang.Object");
				return name;
			}

			virtual string getVarName() const override final {
				return "o";
			}

			virtual bool isPrimitive() const override {
				return false;
			}

			virtual TypeSize getSize() const override {
				return TypeSize::FOUR_BYTES;
			}

			virtual bool isSubtypeOfImpl(const Type* other) const override {
				return this == other || (other->isBasic() && !other->isPrimitive());
			}

		protected:
			virtual const Type* castImpl(const Type* other) const override {
				return this->isSubtypeOfImpl(other) ? other : nullptr;
			}

		public:
			static const AnyObjectType* getInstance() {
				static const AnyObjectType instance;
				return &instance;
			}

			static const ArrayType* getArrayTypeInstance() {
				static const ArrayType instance(AnyObjectType::getInstance());
				return &instance;
			}
	};


	static const AmbigousType
			*const ANY_INT_OR_BOOLEAN(new AmbigousType({BOOLEAN, INT, SHORT, CHAR, BYTE})),
			*const ANY_INT(new AmbigousType({INT, SHORT, CHAR, BYTE})),
			*const BYTE_OR_BOOLEAN(new AmbigousType({BOOLEAN, BYTE})),
			*const INT_OR_BOOLEAN(new AmbigousType({BOOLEAN, INT}));


	struct ExcludingType final: SpecialType {
		public:
			const vector<const BasicType*> types;

			ExcludingType(const vector<const BasicType*>& types): types(types) {
				if(types.empty())
					throw IllegalArgumentException("Type list cannot be empty");
			}

			ExcludingType(const initializer_list<const BasicType*> typeList): ExcludingType(vector<const BasicType*>(typeList)) {}

			virtual string toString() const override {
				return "ExcludingType {" + join<const BasicType*>(types, [] (const BasicType* type) { return type->toString(); }) + '}';
			}

			virtual string toString(const ClassInfo& classinfo) const override final {
				return OBJECT->toString(classinfo);
			}

			virtual string getEncodedName() const override final {
				return "SExcludingType(" + join<const BasicType*>(types, [] (const BasicType* type) { return type->getEncodedName(); }) + ')';
			}

			virtual const string& getName() const override final {
				static const string name("java.lang.Object");
				return name;
			}

			virtual string getVarName() const override final {
				return "o";
			}

			virtual bool isPrimitive() const override {
				return false;
			}

			virtual TypeSize getSize() const override {
				return TypeSize::FOUR_BYTES;
			}

			virtual bool isSubtypeOfImpl(const Type* other) const override {
				if(*this == *other)
					return true;

				if(other->isBasic()) {
					for(const BasicType* type : this->types) {
						if(type == other) {
							return false;
						}
					}

					return true;
				}

				if(instanceof<const AmbigousType*>(other)) {
					for(const BasicType* t1 : this->types) {
						for(const Type* t2 : static_cast<const AmbigousType*>(other)->types) {
							if(!t2->isSubtypeOf(t1)) {
								return true;
							}
						}
					}
				}

				return false;
			}

		protected:
			virtual const Type* castImpl(const Type* other) const override {
				if(*this == *other)
					return this;

				if(other->isBasic()) {
					for(const BasicType* type : this->types) {
						if(type == other) {
							return nullptr;
						}
					}

					return other;
				}

				if(instanceof<const AmbigousType*>(other)) {
					vector<const BasicType*> newTypes = static_cast<const AmbigousType*>(other)->types;
					newTypes.erase(remove_if(newTypes.begin(), newTypes.end(), [this] (const BasicType* t1) {
							for(const BasicType* t2 : types)
								if(*t1 == *t2)
									return true;
							return false;
						}), newTypes.end());

					if(!newTypes.empty())
						return new AmbigousType(newTypes);
				}

				return nullptr;
			}
	};




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
			case 'L': return new ClassType(encodedName + 1);
			case '[': return new ArrayType(encodedName);
			default:
				throw InvalidTypeNameException(encodedName);
		}
	}

	static inline const BasicType* parseType(const string& encodedName) {
		return parseType(encodedName.c_str());
	}


	static const BasicType* parseReturnType(const char* encodedName) {
		if(encodedName[0] == 'V')
			return VOID;
		return parseType(encodedName);
	}


	static const ReferenceType* parseReferenceType(const string& encodedName) {
		return encodedName[0] == '[' ? (const ReferenceType*)new ArrayType(encodedName) : (const ReferenceType*)new ClassType(encodedName);
	}


	static vector<const ReferenceType*> parseParameters(const char* str) {
		if(str[0] != '<')
			throw InvalidTypeNameException(str);

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
				default:
					throw InvalidTypeNameException(str);
			}
			parameters.push_back(parameter);
		}
		ForEnd:
		return parameters;
	}
}

#include "javase.cpp"

#endif
