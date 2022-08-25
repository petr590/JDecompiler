#ifndef JDECOMPILER_OPERATION_H
#define JDECOMPILER_OPERATION_H

namespace jdecompiler {

	enum class Associativity { LEFT, RIGHT };


	enum class Priority {
		DEFAULT_PRIORITY         = 16,
		POST_INCREMENT           = 15,
		UNARY                    = 14, PRE_INCREMENT = UNARY, BIT_NOT = UNARY, LOGICAL_NOT = UNARY, UNARY_PLUS = UNARY, UNARY_MINUS = UNARY,
		CAST                     = 13,
		MUL_DIV_REM              = 12, MULTIPLE = MUL_DIV_REM, DIVISION = MUL_DIV_REM, REMAINDER = MUL_DIV_REM,
		PLUS_MINUS               = 11, PLUS = PLUS_MINUS, MINUS = PLUS_MINUS,
		SHIFT                    = 10,
		GREATER_LESS_COMPARASION = 9,  INSTANCEOF = GREATER_LESS_COMPARASION,
		EQUALS_COMPARASION       = 8,
		BIT_AND                  = 7,
		BIT_XOR                  = 6,
		BIT_OR                   = 5,
		LOGICAL_AND              = 4,
		LOGICAL_OR               = 3,
		TERNARY_OPERATOR         = 2,
		ASSIGNMENT               = 1,
		LAMBDA                   = 0,
	};


	struct Operation {
		protected:
			explicit constexpr Operation() noexcept {}

		public:
			virtual ~Operation() {}


			virtual string toString(const StringifyContext&) const = 0;

			virtual string toArrayInitString(const StringifyContext& context) const {
				return toString(context);
			}


			virtual const Type* getReturnType() const = 0;

			template<class T>
			const T* getReturnTypeAs(const T*) const;

			template<class T>
			const T* getReturnTypeAsWidest(const T*) const;

			inline void castReturnTypeTo(const Type* type) const {
				onCastReturnType(getReturnType()->castTo(type));
			}

			inline void twoWayCastReturnTypeTo(const Type* type) const {
				onCastReturnType(getReturnType()->twoWayCastTo(type));
			}

		protected:
			virtual void onCastReturnType(const Type*) const {}


		public:
			virtual const Operation* getOriginalOperation() const;


			template<class D, class... Ds>
			static bool checkDup(const DecompilationContext&, const Operation*);


			template<class D, class... Ds>
			static inline const Type* getDupReturnType(const DecompilationContext& context, const Operation* operation, const Type* defaultType = VOID) {
				return checkDup<D, Ds...>(context, operation) ? operation->getReturnType() : defaultType;
			}


		public:
			virtual Priority getPriority() const;

			string toStringPriority(const Operation*, const StringifyContext&, const Associativity) const;

			virtual inline string getFrontSeparator(const ClassInfo& classinfo) const {
				return classinfo.getIndent();
			}

			virtual inline string getBackSeparator(const ClassInfo&) const {
				return ";\n";
			}


			virtual bool canAddToCode() const;

			void remove(const DecompilationContext&) const;

			virtual bool canStringify() const;

			virtual void addVariableName(const string&) const {}


			virtual void allowImplicitCast() const {} // For CastOperation, LConstOperation

			virtual const Type* getImplicitType() const { // For CastOperation, LConstOperation
				return getReturnType();
			}

			inline bool canImplicitCast() const {
				return *this->getImplicitType() != *this->getReturnType();
			}

			virtual bool isReferenceToThis(const StringifyContext&) const { // For ALoadOperation
				return false;
			}

			virtual bool isIncrement() const { // For IIncOperation, store operations
				return false;
			}

			virtual bool isAbstractConstOperation() const { // For AbstractConstOperation
				return false;
			}

			virtual string toString(const StringifyContext&, const ConstantDecompilationContext&) const { // For AbstractConstOperation
				throw IllegalStateException("Can call this function only for AbstractConstOperation");
			}

		private:
			static inline constexpr Associativity getAssociativityByPriority(Priority priority) {
				return priority == Priority::ASSIGNMENT || priority == Priority::TERNARY_OPERATOR || priority == Priority::CAST
						|| priority == Priority::UNARY ? Associativity::RIGHT : Associativity::LEFT;
			}
	};
}

#endif
