#ifndef JDECOMPILER_CONDITION_OPERATIONS_CPP
#define JDECOMPILER_CONDITION_OPERATIONS_CPP

namespace jdecompiler::operations {

	//template<BasicType type>
	struct CmpOperation: BooleanOperation {
		const Operation *const operand2, *const operand1;

		CmpOperation(const DecompilationContext& context, const Type* operandType):
				operand2(context.stack.pop()), operand1(context.stack.pop()) {
					operandType = operandType->castTo(operand1->getReturnType())->castTo(operand2->getReturnType());
					operand1->castReturnTypeTo(operandType);
					operand2->castReturnTypeTo(operandType);
				}
	};


	struct LCmpOperation: CmpOperation {
		LCmpOperation(const DecompilationContext& context): CmpOperation(context, LONG) {}

		virtual string toString(const StringifyContext& context) const override { throw Exception("Illegal using of lcmp: toString()"); }
	};

	struct FCmpOperation: CmpOperation {
		FCmpOperation(const DecompilationContext& context): CmpOperation(context, FLOAT) {}

		virtual string toString(const StringifyContext& context) const override { throw Exception("Illegal using of fcmp: toString()"); }
	};

	struct DCmpOperation: CmpOperation {
		DCmpOperation(const DecompilationContext& context): CmpOperation(context, DOUBLE) {}

		virtual string toString(const StringifyContext& context) const override { throw Exception("Illegal using of dcmp: toString()"); }
	};


	struct CompareType;

	struct EqualsCompareType;


	struct CompareType {
		static const EqualsCompareType EQUALS, NOT_EQUALS;
		static const CompareType GREATER, LESS_OR_EQUALS, LESS, GREATER_OR_EQUALS;

		protected:
			const char* const binaryOperator;
			const CompareType& invertedType;

		public:
			const bool isEqualsCompareType;

			CompareType(const char* const binaryOperator, const CompareType& invertedType, bool isEqualsCompareType = false):
					binaryOperator(binaryOperator), invertedType(invertedType), isEqualsCompareType(isEqualsCompareType) {}

			inline string getOperator(bool inverted) const {
				return inverted ? invertedType.binaryOperator : binaryOperator;
			}

			virtual const Type* getRequiredType() const {
				return ExcludingBooleanType::getInstance();
			}

			virtual Priority getPriority() const {
				return Priority::GREATER_LESS_COMPARASION;
			}
	};

	struct EqualsCompareType final: CompareType {
		protected:
			const char* const unaryOperator;

		public:
			EqualsCompareType(const char* binaryOperator, const char* unaryOperator, const EqualsCompareType& invertedType):
					CompareType(binaryOperator, invertedType, true), unaryOperator(unaryOperator) {}

			inline string getUnaryOperator(bool inverted) const {
				return inverted ? ((const EqualsCompareType&)invertedType).unaryOperator : unaryOperator;
			}

			virtual const Type* getRequiredType() const override {
				return AnyType::getInstance();
			}

			virtual Priority getPriority() const override {
				return Priority::EQUALS_COMPARASION;
			}
	};


	const EqualsCompareType
			CompareType::EQUALS("==", "!", CompareType::NOT_EQUALS),
			CompareType::NOT_EQUALS("!=", "", CompareType::EQUALS);
	const CompareType
			CompareType::GREATER(">", CompareType::LESS_OR_EQUALS),
			CompareType::LESS_OR_EQUALS("<=", CompareType::GREATER),
			CompareType::LESS("<", CompareType::GREATER_OR_EQUALS),
			CompareType::GREATER_OR_EQUALS(">=", CompareType::LESS);



	struct ConditionOperation: BooleanOperation {
		protected:
			mutable bool inverted = false;

			constexpr ConditionOperation() noexcept {}

		public:
			virtual string toString(const StringifyContext& context) const = 0;

			inline const ConditionOperation* invert() const {
				inverted = !inverted;
				onInvert();
				return this;
			}

		protected:
			virtual inline void onInvert() const {}
	};

	struct CompareOperation: ConditionOperation {
		public:
			const CompareType& compareType;

			CompareOperation(const CompareType& compareType): compareType(compareType) {}

			virtual Priority getPriority() const override {
				return compareType.getPriority();
			}
	};


	struct CompareBinaryOperation: CompareOperation {
		public:
			const Operation *const operand2, *const operand1;

		protected:
			inline void castOperandsTo(const Type* requiredType) {
				operand1->castReturnTypeTo(requiredType);
				operand2->castReturnTypeTo(requiredType);

				if(!compareType.isEqualsCompareType) {
					operand1->allowImplicitCast();
					operand2->allowImplicitCast();
				}

				const Type* generalType = operand1->getReturnType()->castNoexcept(operand2->getReturnType());
				if(generalType == nullptr)
					generalType = operand2->getReturnType()->castNoexcept(operand1->getReturnType());

				if(generalType == nullptr)
					throw DecompilationException("incopatible types for operator " + compareType.getOperator(false) +
							": " + operand1->getReturnType()->toString() + " and " + operand2->getReturnType()->toString());

				operand1->castReturnTypeTo(generalType);
				operand2->castReturnTypeTo(generalType);

				if(generalType->isPrimitive() && (generalType == LONG || generalType == FLOAT || generalType == DOUBLE)) {
					bool canImplCast1 = operand1->canImplicitCast(),
					     canImplCast2 = operand2->canImplicitCast();

					if(canImplCast1 ^ canImplCast2) { // Allow cast when only one of types can be casted implicit
						(canImplCast1 ? operand1 : operand2)->allowImplicitCast();
					}
				}
			}

		public:
			CompareBinaryOperation(const CmpOperation* cmpOperation, const CompareType& compareType):
					CompareOperation(compareType), operand2(cmpOperation->operand2), operand1(cmpOperation->operand1) {
				castOperandsTo(compareType.getRequiredType());
			}

			CompareBinaryOperation(const DecompilationContext& context, const Type* requiredType, const CompareType& compareType):
					/* We don't delegate constructor because of undefined order of initialization of the function arguments
					   which is important in this case */
					CompareOperation(compareType), operand2(context.stack.pop()), operand1(context.stack.pop()) {
				castOperandsTo(compareType.getRequiredType()->castTo(requiredType));
			}

			virtual string toString(const StringifyContext& context) const override {
				return toStringPriority(operand1, context, Associativity::LEFT) + ' ' + compareType.getOperator(inverted) + ' '
						+ toStringPriority(operand2, context, Associativity::RIGHT);
			}
	};


	struct CompareWithZeroOperation: CompareOperation {
		const Operation* const operand;

		CompareWithZeroOperation(const Operation* operand, const CompareType& compareType): CompareOperation(compareType), operand(operand) {
			operand->allowImplicitCast();
		}

		virtual string toString(const StringifyContext& context) const override {
			const Type* const operandType = operand->getReturnType();

			return operandType->isSubtypeOf(BOOLEAN) && !operandType->isSubtypeOf(INT)
					&& compareType.isEqualsCompareType ? // write `!bool` instead of `bool == false`

					((const EqualsCompareType&)compareType).getUnaryOperator(inverted) +
							toStringPriority(operand, context, Associativity::RIGHT) :
					toStringPriority(operand, context, Associativity::LEFT) + ' ' + compareType.getOperator(inverted) + " 0";
		}
	};


	struct CompareWithNullOperation: CompareOperation {
		protected:
			const Operation* const operand;

		public:
			CompareWithNullOperation(const DecompilationContext& context, const EqualsCompareType& compareType):
					CompareOperation(compareType), operand(context.stack.pop()) {}

			virtual string toString(const StringifyContext& context) const override {
				return operand->toString(context) + ' ' + compareType.getOperator(inverted) + " null";
			}
	};


	struct BinaryConditionOperation: ConditionOperation {
		public:
			const ConditionOperation *const operand1, *const operand2;

			BinaryConditionOperation(const ConditionOperation* operand1, const ConditionOperation* operand2):
					operand1(operand1), operand2(operand2) {}

		protected:
			virtual inline void onInvert() const override {
				operand1->invert();
				operand2->invert();
			}
	};


	struct AndOperation: BinaryConditionOperation {
		public:
			AndOperation(const ConditionOperation* operand1, const ConditionOperation* operand2): BinaryConditionOperation(operand1, operand2) {}

			virtual string toString(const StringifyContext& context) const override {
				const string strOperand1 = toStringPriority(operand1, context, Associativity::LEFT),
						strOperand2 = toStringPriority(operand2, context, Associativity::RIGHT);

				return strOperand1 + (inverted ? " || " : " && ") + strOperand2;
			}

			virtual Priority getPriority() const override {
				return inverted ? Priority::LOGICAL_OR : Priority::LOGICAL_AND;
			}
	};


	struct OrOperation: BinaryConditionOperation {
		public:
			OrOperation(const ConditionOperation* operand1, const ConditionOperation* operand2): BinaryConditionOperation(operand1, operand2) {}

			virtual string toString(const StringifyContext& context) const override {
				const string strOperand1 = toStringPriority(operand1, context, Associativity::LEFT),
						strOperand2 = toStringPriority(operand2, context, Associativity::RIGHT);

				return strOperand1 + (inverted ? " && " : " || ") + strOperand2;
			}

			virtual Priority getPriority() const override {
				return inverted ? Priority::LOGICAL_AND : Priority::LOGICAL_OR;
			}
	};


	struct TernaryOperatorOperation: ReturnableOperation<> {
		const ConditionOperation* const condition;
		const Operation *const trueCase, *const falseCase;

		const bool isShort;

		TernaryOperatorOperation(const ConditionOperation* condition, const Operation* trueCase, const Operation* falseCase):
				ReturnableOperation(trueCase->getReturnTypeAs(falseCase->getReturnType())),
				condition(condition), trueCase(trueCase), falseCase(falseCase),
				isShort(instanceof<const IConstOperation*>(trueCase) && static_cast<const IConstOperation*>(trueCase)->value == 1 &&
				        instanceof<const IConstOperation*>(falseCase) && static_cast<const IConstOperation*>(falseCase)->value == 0) {}

		virtual string toString(const StringifyContext& context) const override {
			return isShort && trueCase->getReturnType()->isSubtypeOf(BOOLEAN) && falseCase->getReturnType()->isSubtypeOf(BOOLEAN) ?
					condition->toString(context) :
					condition->toString(context) + " ? " + trueCase->toString(context) + " : " + falseCase->toString(context);
		}

		virtual void onCastReturnType(const Type* newType) const override {
			trueCase->castReturnTypeTo(newType);
			falseCase->castReturnTypeTo(newType);
		}

		virtual Priority getPriority() const override {
			return Priority::TERNARY_OPERATOR;
		}
	};
}

#endif
