#ifndef JDECOMPILER_TYPES_CPP
#define JDECOMPILER_TYPES_CPP

#include "const-pool.cpp"
#include "classinfo.cpp"

namespace jdecompiler {

	enum class TypeSize {
		ZERO_BYTES = 0, FOUR_BYTES = 1, EIGHT_BYTES = 2
	};


	static const char* TypeSize_nameOf(const TypeSize typeSize) {
		switch(typeSize) {
			case TypeSize::ZERO_BYTES: return "ZERO_BYTES";
			case TypeSize::FOUR_BYTES: return "FOUR_BYTES";
			case TypeSize::EIGHT_BYTES: return "EIGHT_BYTES";
			default: throw IllegalStateException("Illegal typeSize " + to_string((unsigned)typeSize));
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


			virtual bool isBasic() const = 0;

			inline bool isSpecial() const {
				return !isBasic();
			}

			/* Only for subtypes of class PrimitiveType */
			virtual bool isPrimitive() const {
				return false;
			}

			/* Only for subtypes of class IntegralType */
			virtual bool isIntegral() const {
				return false;
			}

			virtual TypeSize getSize() const = 0;

		private:
			template<class T>
			static constexpr void checkType() noexcept {
				static_assert(is_base_of<Type, T>::value, "Class T must be subclass of class Type");
			}

		protected:
			template<bool widest>
			static inline constexpr auto getCastImplFunction() {
				return widest ? &Type::castToWidestImpl : &Type::castImpl;
			}

			template<bool widest>
			static inline constexpr auto getReversedCastImplFunction() {
				return widest ? &Type::reversedCastToWidestImpl : &Type::reversedCastImpl;
			}


		public:
			template<bool isNoexcept, bool widest>
			const Type* cast0(const Type* type) const {

				// I love such constructions in C++ :)
				// It's a pointer to a member function
				static constexpr const Type* (Type::* castImplFunc)(const Type*) const = getCastImplFunction<widest>();
				static constexpr const Type* (Type::* reversedCastImplFunc)(const Type*) const = getReversedCastImplFunction<widest>();

				const Type* castedType;

				if((castedType = (this->*castImplFunc)(type)) != nullptr) {
					return castedType;
				}

				(type->*reversedCastImplFunc)(this);

				if(this->canReverseCast(type) && (castedType = (type->*reversedCastImplFunc)(this)) != nullptr) {
					return castedType;
				}

				if constexpr(isNoexcept)
					return nullptr;
				else
					throw IncopatibleTypesException(this, type);
			}

		public:
			template<class T>
			inline const T* castTo(const T* type) const {
				checkType<T>();
				return safe_cast<const T*>(cast0<false, false>(type));
			}

			template<class T>
			inline const T* castNoexcept(const T* type) const {
				checkType<T>();
				return safe_cast<const T*>(cast0<true, false>(type));
			}

			template<class T>
			inline const T* castToWidest(const T* type) const {
				checkType<T>();
				return safe_cast<const T*>(cast0<false, true>(type));
			}

			template<class T>
			inline const T* castToWidestNoexcept(const T* type) const {
				checkType<T>();
				return safe_cast<const T*>(cast0<true, true>(type));
			}


			template<class T>
			const T* twoWayCastTo(const T* t) const {
				checkType<T>();

				const Type* castedType;

				if((castedType = this->castToWidestImpl(t)) != nullptr)
					return safe_cast<const T*>(castedType);

				if((castedType = ((const Type*)t)->castToWidestImpl(this)) != nullptr)
					return safe_cast<const T*>(castedType);

				if((castedType = this->reversedCastToWidestImpl(t)) != nullptr)
					return safe_cast<const T*>(castedType);

				if((castedType = ((const Type*)t)->reversedCastToWidestImpl(this)) != nullptr)
					return safe_cast<const T*>(castedType);

				throw IncopatibleTypesException(this, t);
			}

			bool isSubtypeOf(const Type* type) const {
				return this->isSubtypeOfImpl(type) || (this->canReverseCast(type) && type->isSubtypeOfImpl(this));
			}

			bool isStrictSubtypeOf(const Type* type) const {
				return this->isStrictSubtypeOfImpl(type) || (this->canReverseCast(type) && type->isStrictSubtypeOfImpl(this));
			}

		protected:
			virtual bool canReverseCast(const Type*) const {
				return true;
			}

			virtual bool isSubtypeOfImpl(const Type*) const = 0;

			virtual bool isStrictSubtypeOfImpl(const Type* other) const {
				return isSubtypeOfImpl(other);
			}


			virtual const Type* castImpl(const Type*) const = 0;

			virtual const Type* reversedCastImpl(const Type* other) const {
				return castImpl(other);
			}

			virtual const Type* castToWidestImpl(const Type* other) const {
				return castImpl(other);
			}

			virtual const Type* reversedCastToWidestImpl(const Type* other) const {
				return castToWidestImpl(other);
			}

		public:
			inline friend bool operator==(const Type& type1, const Type& type2) {
				return &type1 == &type2 || (typeid(type1) == typeid(type2) && type1.getEncodedName() == type2.getEncodedName());
			}

			inline friend bool operator!=(const Type& type1, const Type& type2) {
				return !(type1 == type2);
			}

			inline friend ostream& operator<<(ostream& out, const Type* type) {
				return out << (type != nullptr ? type->toString() : "null");

				/*const char* data = reinterpret_cast<const char*>(type);
				for(size_t i = 0; i < 240 ; i++) // 240 = sizeof(ClassType)
					out << hex<2>(data[i]) << ' ';
				out << endl << typenameof(type);

				return out;*/
			}

			inline friend ostream& operator<<(ostream& out, const Type& type) {
				return out << &type;
			}

			/* The status determines the priority when overloading methods are resolved
			 * N_STATUS determines that the type has no conversion to another type */
			typedef uint_fast8_t status_t;

			static constexpr status_t
					N_STATUS = 0,
					SAME_STATUS = 1,
					EXTEND_STATUS = 2, // Extend of argument (String -> Object or char -> int)
					AUTOBOXING_STATUS = 3,
					OBJECT_AUTOBOXING_STATUS = 4, // Autoboxing into Object (int -> Integer -> Object)
					VARARGS_STATUS = 5;

			virtual status_t implicitCastStatus(const Type* other) const {
				return *this == *other ? SAME_STATUS : this->isSubtypeOf(other) ? EXTEND_STATUS : N_STATUS;
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


	struct PrimitiveType: BasicType {
		public:
			const string varName;

		private:
			PrimitiveType(const string& encodedName, const string& name, const string& varName):
					BasicType(encodedName, name), varName(varName) {}

			PrimitiveType(const PrimitiveType&) = delete;
			PrimitiveType& operator=(const PrimitiveType&) = delete;

			/* Allow only these types to inherit from PrimitiveType */
			friend struct VoidType;
			friend struct BooleanType;
			friend struct CharType;
			friend struct IntegralType;
			friend struct LongType;
			friend struct FloatType;
			friend struct DoubleType;

		public:
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

		protected:
			virtual bool canReverseCast(const Type* other) const override final {
				return other->isSpecial();
			}

			virtual bool isSubtypeOfImpl(const Type* other) const override {
				return this == other;
			}

			virtual bool isStrictSubtypeOfImpl(const Type* other) const override final {
				return this == other;
			}

			virtual const Type* castImpl(const Type* other) const override {
				return this->isSubtypeOfImpl(other) ? this : nullptr;
			}

			virtual const Type* castToWidestImpl(const Type* other) const override {
				return this->isSubtypeOfImpl(other) ? other : nullptr;
			}

		public:
			virtual const Type* toVariableCapacityIntegralType() const {
				return this;
			}

			virtual status_t implicitCastStatus(const Type*) const override;

			virtual const ClassType& getWrapperType() const = 0;
	};


	/*
		An integral type is an signed integer type that occupies 4 bytes on the stack: int, short and byte.
		Boolean and char are not included in this list, they are processed separately
	*/
	struct IntegralType: PrimitiveType {
		private:
			IntegralType(const string& encodedName, const string& name, const string& varName):
					PrimitiveType(encodedName, name, varName) {}

			friend struct ByteType;
			friend struct ShortType;
			friend struct IntType;
			friend struct LongType;

		public:
			virtual uint8_t getCapacity() const = 0;

			virtual bool isIntegral() const override final {
				return true;
			}
	};


	struct VoidType final: PrimitiveType {
		VoidType(): PrimitiveType("V", "void", "v") {}

		virtual TypeSize getSize() const override final { return TypeSize::ZERO_BYTES; }

		virtual const ClassType& getWrapperType() const override;

		static const VoidType& getInstance() {
			static const VoidType instance;
			return instance;
		}
	};

	struct BooleanType final: PrimitiveType {
		BooleanType(): PrimitiveType("Z", "boolean", "bool") {}

		virtual TypeSize getSize() const override final { return TypeSize::FOUR_BYTES; }

		virtual const ClassType& getWrapperType() const override;

		static const BooleanType& getInstance() {
			static const BooleanType instance;
			return instance;
		}
	};

	struct ByteType final: IntegralType {
		ByteType(): IntegralType("B", "byte", "b") {}

		virtual TypeSize getSize() const override final { return TypeSize::FOUR_BYTES; }
		virtual uint8_t getCapacity() const override { return 1; }

		virtual bool isSubtypeOfImpl(const Type*) const override;
		virtual const Type* toVariableCapacityIntegralType() const override;

		virtual const ClassType& getWrapperType() const override;

		static const ByteType& getInstance() {
			static const ByteType instance;
			return instance;
		}
	};

	struct CharType final: PrimitiveType {
		CharType(): PrimitiveType("C", "char", "c") {}

		virtual TypeSize getSize() const override final { return TypeSize::FOUR_BYTES; }

		virtual bool isSubtypeOfImpl(const Type*) const override;
		virtual const Type* toVariableCapacityIntegralType() const override;

		virtual const ClassType& getWrapperType() const override;

		static const CharType& getInstance() {
			static const CharType instance;
			return instance;
		}
	};

	struct ShortType final: IntegralType {
		ShortType(): IntegralType("S", "short", "s") {}

		virtual TypeSize getSize() const override final { return TypeSize::FOUR_BYTES; }
		virtual uint8_t getCapacity() const override { return 2; }

		virtual bool isSubtypeOfImpl(const Type*) const override;
		virtual const Type* toVariableCapacityIntegralType() const override;

		virtual const ClassType& getWrapperType() const override;

		static const ShortType& getInstance() {
			static const ShortType instance;
			return instance;
		}
	};

	struct IntType final: IntegralType {
		IntType(): IntegralType("I", "int", "n") {}

		virtual TypeSize getSize() const override final { return TypeSize::FOUR_BYTES; }
		virtual uint8_t getCapacity() const override { return 4; }

		virtual const ClassType& getWrapperType() const override;

		static const IntType& getInstance() {
			static const IntType instance;
			return instance;
		}
	};

	struct LongType final: PrimitiveType {
		LongType(): PrimitiveType("J", "long", "l") {}

		virtual TypeSize getSize() const override final { return TypeSize::EIGHT_BYTES; }

		virtual const ClassType& getWrapperType() const override;

		static const LongType& getInstance() {
			static const LongType instance;
			return instance;
		}
	};

	struct FloatType final: PrimitiveType {
		FloatType(): PrimitiveType("F", "float", "f") {}

		virtual TypeSize getSize() const override final { return TypeSize::FOUR_BYTES; }

		virtual const ClassType& getWrapperType() const override;

		static const FloatType& getInstance() {
			static const FloatType instance;
			return instance;
		}
	};

	struct DoubleType final: PrimitiveType {
		DoubleType(): PrimitiveType("D", "double", "d") {}

		virtual TypeSize getSize() const override final { return TypeSize::EIGHT_BYTES; }

		virtual const ClassType& getWrapperType() const override;

		static const DoubleType& getInstance() {
			static const DoubleType instance;
			return instance;
		}
	};


	static const VoidType *const VOID = &VoidType::getInstance();
	static const BooleanType *const BOOLEAN = &BooleanType::getInstance();
	static const ByteType *const BYTE = &ByteType::getInstance();
	static const CharType *const CHAR = &CharType::getInstance();
	static const ShortType *const SHORT = &ShortType::getInstance();
	static const IntType *const INT = &IntType::getInstance();
	static const LongType *const LONG = &LongType::getInstance();
	static const FloatType *const FLOAT = &FloatType::getInstance();
	static const DoubleType *const DOUBLE = &DoubleType::getInstance();


	bool ByteType::isSubtypeOfImpl(const Type* other) const {
		return other == BYTE || other == SHORT || other == INT;
	}

	bool CharType::isSubtypeOfImpl(const Type* other) const {
		return other == CHAR || other == INT;
	}

	bool ShortType::isSubtypeOfImpl(const Type* other) const {
		return other == SHORT || other == INT;
	}


	struct ReferenceType: BasicType {
		protected:
			ReferenceType(const string& encodedName, const string& name): BasicType(encodedName, name) {}

			ReferenceType(): BasicType(EMPTY_STRING, EMPTY_STRING) {}

		public:
			virtual TypeSize getSize() const override final {
				return TypeSize::FOUR_BYTES;
			}

			virtual string getClassEncodedName() const {
				return encodedName;
			}

		protected:
			virtual const Type* castImpl(const Type* other) const override {
				return this->isSubtypeOfImpl(other) ? this : nullptr;
			}
	};


	struct ClassType final: ReferenceType {
		public:
			string classEncodedName,
					simpleName,
					fullSimpleName, // fullSimpleName is a class name including enclosing class name
					packageName;

			vector<const ReferenceType*> parameters;

			const ClassType* enclosingClass;
			bool isNested = false, isAnonymous = false;

			ClassType(const ClassConstant* clazz): ClassType(clazz->name) {}

			ClassType(const string& str): ClassType(str.c_str()) {}

			ClassType(const char*&& str): ClassType(static_cast<const char*&>(str)) {}

			ClassType(const char*& restrict str) {

				const char* const srcStr = str;

				string encodedName = str;
				string name = encodedName;

				uint32_t nameStartPos = 0,
				         packageEndPos = 0,
				         enclosingClassNameEndPos = 0;

				for(uint32_t i = 0;;) {

					switch(*str) {
						case '/':
							packageEndPos = i;
							nameStartPos = i + 1;
							name[i] = '.';
							break;

						case '$':
							enclosingClassNameEndPos = i;
							nameStartPos = i + 1;
							name[i] = '.';
							break;

						case '<':
							parameters = parseParameters(str);

							switch(*str) {
								case ';':
									++str;
								case '\0':
									name = name.substr(0, i);
									encodedName = encodedName.substr(0, i);
									goto End; // break loop

								default:
									throw InvalidClassNameException(srcStr, i);
							}

						case ';':
							++str;
						case '\0':
							name = name.substr(0, i);
							encodedName = encodedName.substr(0, i);
							goto End;
						// invalid chars
						case '\t': case '\n': case '\v': case '\f': case '\r': case ' ': case '!':
						case '"': case '#': case '%': case '&': case '\'': case '(': case ')': case '*': case '+':
						case ',': case '-': case '.': case ':': case '=':  case '?': case '@': case '[': case '\\':
						case ']': case '^': case '`': case '{': case '|':  case '}': case '~': case '\x7F': // DEL
							throw InvalidClassNameException(srcStr, i);
					}

					++i, ++str;
				}
				End:

				this->name = name;

				this->encodedName = 'L' + encodedName + ';';
				this->classEncodedName = encodedName;

				simpleName = name.substr(nameStartPos);

				packageName = name.substr(0, packageEndPos);

				if(enclosingClassNameEndPos == 0) {
					enclosingClass = nullptr;
					fullSimpleName = simpleName;
				} else {
					isNested = true;
					isAnonymous = all_of(simpleName.begin(), simpleName.end(), [] (unsigned char c) { return isdigit(c); });
					enclosingClass = new ClassType(encodedName.substr(0, enclosingClassNameEndPos));

					fullSimpleName = enclosingClass->fullSimpleName + (isAnonymous ? '$' : '.') + simpleName;

					if(isAnonymous) {
						this->name[enclosingClassNameEndPos] = '$';
						simpleName = enclosingClass->simpleName + '$' + simpleName;
					}
				}
			}

			virtual string toString(const ClassInfo&) const override;

			virtual string toString() const override {
				return "class " + (parameters.empty() ? name : name +
						'<' + join<const ReferenceType*>(parameters, [] (const ReferenceType* type) { return type->toString(); }) + '>');
			}

			virtual string getVarName() const override final {
				return toLowerCamelCase(simpleName);
			}

			virtual status_t implicitCastStatus(const Type*) const override;

			virtual string getClassEncodedName() const override {
				return classEncodedName;
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
			*const ENUM(new ClassType("java/lang/Enum")),
			*const THROWABLE(new ClassType("java/lang/Throwable")),
			*const EXCEPTION(new ClassType("java/lang/Exception")),
			*const METHOD_TYPE(new ClassType("java/lang/invoke/MethodType")),
			*const METHOD_HANDLE(new ClassType("java/lang/invoke/MethodHandle"));
}

#include "javase.cpp"

namespace jdecompiler {

	Type::status_t PrimitiveType::implicitCastStatus(const Type* other) const {
		return *other == *OBJECT ? OBJECT_AUTOBOXING_STATUS : *other == getWrapperType() ? AUTOBOXING_STATUS : Type::implicitCastStatus(other);
	}

	const ClassType& VoidType::getWrapperType() const {
		return javaLang::Void;
	}

	const ClassType& BooleanType::getWrapperType() const {
		return javaLang::Boolean;
	}

	const ClassType& ByteType::getWrapperType() const {
		return javaLang::Byte;
	}

	const ClassType& CharType::getWrapperType() const {
		return javaLang::Character;
	}

	const ClassType& ShortType::getWrapperType() const {
		return javaLang::Short;
	}

	const ClassType& IntType::getWrapperType() const {
		return javaLang::Integer;
	}

	const ClassType& LongType::getWrapperType() const {
		return javaLang::Long;
	}

	const ClassType& FloatType::getWrapperType() const {
		return javaLang::Float;
	}

	const ClassType& DoubleType::getWrapperType() const {
		return javaLang::Double;
	}


	Type::status_t ClassType::implicitCastStatus(const Type* other) const {
		return other->isPrimitive() && *this == safe_cast<const PrimitiveType*>(other)->getWrapperType() ?
						AUTOBOXING_STATUS : Type::implicitCastStatus(other);
	}


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


	struct GenericType: ReferenceType {

		virtual string getVarName() const override {
			throw IllegalStateException("Seriously? Variable of unknown generic type?");
		}

		virtual bool isSubtypeOfImpl(const Type* other) const override {
			return instanceof<const ReferenceType*>(other);
		}
	};


	struct DefinedGenericType: GenericType {
		const ReferenceType* const type;

		DefinedGenericType(const char*& str): type(parseParameter(str)) {}
	};


	struct ExtendingGenericType: DefinedGenericType {
		ExtendingGenericType(const char*& str): DefinedGenericType(str) {}

		virtual string toString(const ClassInfo& classinfo) const override {
			return "? extends " + type->toString(classinfo);
		}

		virtual string toString() const override {
			return "ExtendingGenericType(" + type->toString() + ')';
		}
	};


	struct SuperGenericType: DefinedGenericType {
		SuperGenericType(const char*& str): DefinedGenericType(str) {}

		virtual string toString(const ClassInfo& classinfo) const override {
			return "? super " + type->toString(classinfo);
		}

		virtual string toString() const override {
			return "SuperGenericType(" + type->toString() + ')';
		}
	};


	struct AnyGenericType: GenericType {
		private:
			AnyGenericType() noexcept {}

		public:
			static const AnyGenericType* getInstance() {
				static const AnyGenericType instance;
				return &instance;
			}

			virtual string toString(const ClassInfo&) const override {
				return "?";
			}

			virtual string toString() const override {
				return "AnyGenericType";
			}
	};


	struct VariableCapacityIntegralType: SpecialType {
		public:
			const uint8_t minCapacity, maxCapacity;
			const bool includeBoolean, includeChar;

			static const uint8_t INCLUDE_BOOLEAN = 1, INCLUDE_CHAR = 2;
			static const uint8_t CHAR_CAPACITY = 2;

		private:
			const PrimitiveType* const primitiveType;

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
					primitiveType(primitiveTypeByCapacity(maxCapacity, includeChar)),
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
				return primitiveType->toString(classinfo);
			}

			virtual string toString() const override {
				return "VariableCapacityIntegralType(" + to_string(minCapacity) + ", " + to_string(maxCapacity) +
						(includeBoolean ? ", boolean" : "") + (includeChar ? ", char" : "") + ')';
			}

			virtual string getEncodedName() const override {
				return encodedName;
			}

			virtual const string& getName() const override {
				return primitiveType->getName();
			}

			virtual string getVarName() const override {
				return primitiveType->getVarName();
			}

			virtual TypeSize getSize() const override {
				return TypeSize::FOUR_BYTES;
			}

		protected:
			virtual bool canReverseCast(const Type* other) const {
				return true;
			}

			virtual bool isSubtypeOfImpl(const Type* other) const override {
				if(*this == *other || (other == BOOLEAN && includeBoolean) || other == primitiveType)
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
				if(*this == *other || (other == BOOLEAN && includeBoolean) || other == primitiveType)
					return true;

				if(other == CHAR)
					return includeChar || (minCapacity <= CHAR_CAPACITY && maxCapacity > CHAR_CAPACITY);

				if(other->isIntegral()) {
					const uint8_t capacity = safe_cast<const IntegralType*>(other)->getCapacity();
					return capacity >= minCapacity && capacity <= maxCapacity;
				}

				return false;
			}

			template<bool widest>
			static const Type* castImpl0(const VariableCapacityIntegralType* type, const Type* other) {

				if(other->isPrimitive()) {

					if(other == BOOLEAN)
						return type->includeBoolean ? other : nullptr;

					if(other == type->primitiveType)
						return type;

					if(other == CHAR)
						return type->includeChar ? other : nullptr;

					if(other->isIntegral()) {
						const uint8_t capacity = safe_cast<const IntegralType*>(other)->getCapacity();

						if(capacity == type->minCapacity)
							return widest ? type : other;

						if(capacity == type->maxCapacity)
							return type;

						if(capacity > type->minCapacity)
							return getInstance(type->minCapacity, min(capacity, type->maxCapacity), type->includeBoolean, type->includeChar);
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

					if(other == primitiveType)
						return widest ? this : other;

					if(other == CHAR)
						return includeChar || maxCapacity > CHAR_CAPACITY ?
								(widest ? getInstance(CHAR_CAPACITY * 2, maxCapacity, false, includeChar) : other) : nullptr;

					if(other->isIntegral()) {
						const uint8_t capacity = safe_cast<const IntegralType*>(other)->getCapacity();

						if(widest ? capacity <= minCapacity : capacity >= maxCapacity)
							return this;

						if(widest ? capacity <= maxCapacity : capacity >= minCapacity) {
							return widest ? getInstance(max(capacity, minCapacity), capacity, includeBoolean, includeChar) :
											getInstance(capacity, min(capacity, maxCapacity), includeBoolean, includeChar);
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

			virtual TypeSize getSize() const override {
				return TypeSize::FOUR_BYTES; // ???
			}

			virtual bool isSubtypeOfImpl(const Type*) const override {
				return true;
			}

		protected:
			virtual const Type* castImpl(const Type* other) const override {
				return other;
			}

			virtual const Type* castToWidestImpl(const Type* other) const override {
				return other->isPrimitive() ? safe_cast<const PrimitiveType*>(other)->toVariableCapacityIntegralType() : other;
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




	static const BasicType* parseType(const char*& restrict str) {
		switch(str[0]) {
			case 'B': ++str; return BYTE;
			case 'C': ++str; return CHAR;
			case 'S': ++str; return SHORT;
			case 'I': ++str; return INT;
			case 'J': ++str; return LONG;
			case 'F': ++str; return FLOAT;
			case 'D': ++str; return DOUBLE;
			case 'Z': ++str; return BOOLEAN;
			case 'L': return new ClassType(str += 1);
			case '[': return new ArrayType(str);
			case 'T': return new ParameterType(str += 1);
			default:
				throw InvalidTypeNameException(str);
		}
	}

	static inline const BasicType* parseType(const char*&& str) {
		return parseType(static_cast<const char*&>(str));
	}

	static inline const BasicType* parseType(const string& str) {
		return parseType(str.c_str());
	}

	/* Крч, тесты показали, что если указать компилятору restrict, то он сам введет доп. переменную для значения указателя.
	   Поэтому можно спокойно забить на оптимизацию вручную и просто указывать restrict где попало 😆️ */
	static vector<const Type*> parseMethodArguments(const char*& restrict str) {
		if(str[0] != '(')
			throw IllegalMethodDescriptorException(str);
		++str;

		vector<const Type*> arguments;

		while(true) {
			if(*str == ')') {
				++str;
				return arguments;
			}

			arguments.push_back(parseType(str));
		}
	}


	static const BasicType* parseReturnType(const char* str) {
		return str[0] == 'V' ? VOID : parseType(str);
	}

	static inline const BasicType* parseReturnType(const string& str) {
		return parseReturnType(str.c_str());
	}


	static const ReferenceType* parseReferenceType(const char* str) {
		return str[0] == '[' ? (const ReferenceType*)new ArrayType(str) : (const ReferenceType*)new ClassType(str);
	}

	static inline const ReferenceType* parseReferenceType(const string& str) {
		return parseReferenceType(str.c_str());
	}


	static inline const ClassType* parseClassType(const char*& restrict str) {
		return str[0] == 'L' ? new ClassType(str += 1) : throw InvalidSignatureException(str, 0);
	}


	static const ReferenceType* parseParameter(const char*& restrict str) {
		const ReferenceType* parameter;

		switch(str[0]) {
			case 'L': return new ClassType(str += 1);
			case '[': return new ArrayType(str);
			case 'T': return new ParameterType(str += 1);
			default:
				throw InvalidTypeNameException(str);
		}
	}


	static vector<const ReferenceType*> parseParameters(const char*& restrict str) {
		if(str[0] != '<')
			throw InvalidTypeNameException(str, 0);

		vector<const ReferenceType*> parameters;

		for(const char* const srcStr = str++; ;) {
			switch(*str) {
				case '>':
					++str;
					return parameters;

				case '+':
					parameters.push_back(new ExtendingGenericType(str += 1));
					break;
				case '-':
					parameters.push_back(new SuperGenericType(str += 1));
					break;
				case '*':
					++str;
					parameters.push_back(AnyGenericType::getInstance());
					break;

				case '\0':
					throw InvalidTypeNameException(srcStr, str - srcStr);

				default:
					parameters.push_back(parseParameter(str));
			}
		}
	}


	static vector<const GenericParameter*> parseGeneric(const char*& restrict str) {
		if(str[0] != '<')
			return vector<const GenericParameter*>();

		vector<const GenericParameter*> genericParameters;

		for(const char* const srcStr = str++; ;) {
			switch(*str) {
				case '>':
					++str;
					return genericParameters;

				default:
					genericParameters.push_back(new GenericParameter(str));
					break;

				case '\0':
					throw InvalidTypeNameException(srcStr, str - srcStr);
			}
		}

		return genericParameters;
	}


	IncopatibleTypesException::IncopatibleTypesException(const Type* type1, const Type* type2):
			DecompilationException("Incopatible types: " + type1->toString() + " and " + type2->toString()) {}

	string ClassConstant::toString(const ClassInfo& classinfo) const {
		return parseReferenceType(name)->toString(classinfo) + ".class";
	}
}

#endif
