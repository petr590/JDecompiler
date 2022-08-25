#ifndef JDECOMPILER_TYPE_CPP
#define JDECOMPILER_TYPE_CPP

#include "const-pool.cpp"
#include "stringified.cpp"
#include "classinfo.cpp"
#include "type-size.cpp"

namespace jdecompiler {

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
			inline virtual bool isPrimitive() const {
				return false;
			}

			/* Only for subtypes of class IntegralType */
			inline virtual bool isIntegral() const {
				return false;
			}

			virtual TypeSize getSize() const = 0;

		private:
			template<class T>
			static inline constexpr void checkType() noexcept {
				static_assert(is_base_of<Type, T>(), "Class T must be subclass of class Type");
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
			const Type* cast0(const Type*) const;

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

				return safe_cast<const T*>(cast0<false, true>(type));
			}

			template<class T>
			inline const T* castToWidestNoexcept(const T* type) const {
				checkType<T>();
				return safe_cast<const T*>(cast0<true, true>(type));
			}


			template<class T>
			const T* twoWayCastTo(const T*) const;

			inline bool isSubtypeOf(const Type* type) const {
				return this->isSubtypeOfImpl(type) || (this->canReverseCast(type) && type->isSubtypeOfImpl(this));
			}

			inline bool isStrictSubtypeOf(const Type* type) const {
				return this->isStrictSubtypeOfImpl(type) || (this->canReverseCast(type) && type->isStrictSubtypeOfImpl(this));
			}

		protected:
			inline virtual bool canReverseCast(const Type*) const {
				return true;
			}

			virtual bool isSubtypeOfImpl(const Type*) const = 0;

			inline virtual bool isStrictSubtypeOfImpl(const Type* other) const {
				return *this == *other;
			}


			virtual const Type* castImpl(const Type*) const = 0;

			inline virtual const Type* reversedCastImpl(const Type* other) const {
				return castImpl(other);
			}

			inline virtual const Type* castToWidestImpl(const Type* other) const {
				return castImpl(other);
			}

			inline virtual const Type* reversedCastToWidestImpl(const Type* other) const {
				return castToWidestImpl(other);
			}

		public:
			inline virtual const Type* getReducedType() const {
				return this;
			}

			inline friend bool operator==(const Type& type1, const Type& type2) {
				return &type1 == &type2 || (typeid(type1) == typeid(type2) && type1.getEncodedName() == type2.getEncodedName());
			}

			inline friend bool operator!=(const Type& type1, const Type& type2) {
				return !(type1 == type2);
			}

			inline friend ostream& operator<<(ostream& out, const Type* type) {
				return out << (type != nullptr ? type->toString() : "null");
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

			inline virtual status_t implicitCastStatus(const Type* other) const {
				return *this == *other ? SAME_STATUS : this->isSubtypeOf(other) ? EXTEND_STATUS : N_STATUS;
			}
	};
}

#endif
