#ifndef JDECOMPILER_OPERATOR_OPERATIONS_CPP
#define JDECOMPILER_OPERATOR_OPERATIONS_CPP

namespace jdecompiler {

	struct OperatorOperation: ReturnableOperation<> {
		public:
			const char* const stringOperator;

		private:
			const char* const oppositeOperator;

			const Priority priority;

			static const char* oppositeOperatorOf(char32_t charOperator) {
				switch(charOperator) {
					case '+': return "-";
					case '-': return "+";
					case '*': return "/";
					case '/': return "*";
					default: return "";
				}
			}

		public:
			OperatorOperation(char32_t charOperator, Priority priority, const Type* type):
					ReturnableOperation(type), stringOperator(char32ToString(charOperator)),
					oppositeOperator(oppositeOperatorOf(charOperator)), priority(priority) {}

			virtual Priority getPriority() const override {
				return priority;
			}

			const char* getOppositeOperator() const {
				if(*oppositeOperator != '\0')
					return oppositeOperator;
				throw IllegalStateException((string)"Cannot get opposite operator for operator '" + stringOperator + '\'');
			}
	};


	struct BinaryOperatorOperation: OperatorOperation {
		public:
			const Operation *const operand2, *const operand1;

		private:
			inline void castOperands(const Type* type) {
				const Type* requiredType = operand1->getReturnType()->twoWayCastTo(operand2->getReturnType())->castTo(type);
				operand1->twoWayCastReturnTypeTo(requiredType);
				operand2->twoWayCastReturnTypeTo(requiredType);

				operand1->allowImplicitCast();
				operand2->allowImplicitCast();
			}

		public:
			BinaryOperatorOperation(char32_t charOperator, Priority priority, const Type* type1, const Type* type2, const DecompilationContext& context):
						OperatorOperation(charOperator, priority, type1),
						operand2(context.stack.popAs(type2)), operand1(context.stack.popAs(type1)) {}

			BinaryOperatorOperation(char32_t charOperator, Priority priority, const Type* type, const DecompilationContext& context):
					OperatorOperation(charOperator, priority, type), operand2(context.stack.pop()), operand1(context.stack.pop()) {
				castOperands(type);
			}

			BinaryOperatorOperation(char32_t charOperator, Priority priority, const Type* type, const Operation* operand1, const Operation* operand2):
					OperatorOperation(charOperator, priority, type), operand2(operand2), operand1(operand1) {
				castOperands(type);
			}

			virtual string toString(const StringifyContext& context) const override {
				return toStringPriority(operand1, context, Associativity::LEFT) + ' ' + stringOperator + ' ' +
						toStringPriority(operand2, context, Associativity::RIGHT);
			}

			string toShortFormString(const StringifyContext& context, const string& name) const {
				return name + ' ' + stringOperator + "= " + operand2->toString(context);
			}
	};


	/* The template parameter named priority_parameter is named to avoid
	   naming conflicts with field OperatorOperation::priority */
	template<char32_t charOperator, Priority priority_parameter>
	struct BinaryOperatorOperationImpl: BinaryOperatorOperation {
		BinaryOperatorOperationImpl(const Type* type1, const Type* type2, const DecompilationContext& context):
				BinaryOperatorOperation(charOperator, priority_parameter, type1, type2, context) {}

		BinaryOperatorOperationImpl(const Type* type, const DecompilationContext& context):
				BinaryOperatorOperation(charOperator, priority_parameter, type, context) {}

		BinaryOperatorOperationImpl(const Type* type, const Operation* operand1, const Operation* operand2):
				BinaryOperatorOperation(charOperator, priority_parameter, type, operand1, operand2) {}
	};


	/* Operator bit not realized in java through operator xor with value -1 */
	template<Priority priority_parameter>
	struct BinaryOperatorOperationImpl<'^', priority_parameter>: BinaryOperatorOperation {
		const bool isBitNot;

		BinaryOperatorOperationImpl(const Type* type, const DecompilationContext& context):
					BinaryOperatorOperation('^', priority_parameter, type, context),
					isBitNot((instanceof<const IConstOperation*>(operand2) && static_cast<const IConstOperation*>(operand2)->value == -1)
						|| (instanceof<const LConstOperation*>(operand2) && static_cast<const LConstOperation*>(operand2)->value == -1)) {}

		virtual string toString(const StringifyContext& context) const override {
			return isBitNot ? '~' + this->toStringPriority(operand1, context, Associativity::RIGHT):
					this->toStringPriority(operand1, context, Associativity::LEFT) + ' ' + this->stringOperator + ' ' +
					this->toStringPriority(operand2, context, Associativity::RIGHT);
		}

		virtual Priority getPriority() const override {
			return isBitNot ? Priority::BIT_NOT : priority_parameter;
		}
	};


	template<char32_t charOperator, Priority priority_parameter>
	struct UnaryOperatorOperation: OperatorOperation {
		public:
			const Operation* const operand;

			UnaryOperatorOperation(const Type* type, const Operation* operand):
					OperatorOperation(charOperator, priority_parameter, type), operand(operand) {
				operand->allowImplicitCast();
			}

			UnaryOperatorOperation(const Type* type, const DecompilationContext& context):
					UnaryOperatorOperation(type, context.stack.popAs(type)) {}

			virtual string toString(const StringifyContext& context) const override {
				return this->stringOperator + this->toStringPriority(operand, context, Associativity::RIGHT);
			}
	};


	using AddOperatorOperation = BinaryOperatorOperationImpl<'+', Priority::PLUS>;
	using SubOperatorOperation = BinaryOperatorOperationImpl<'-', Priority::MINUS>;
	using MulOperatorOperation = BinaryOperatorOperationImpl<'*', Priority::MULTIPLE>;
	using DivOperatorOperation = BinaryOperatorOperationImpl<'/', Priority::DIVISION>;
	using RemOperatorOperation = BinaryOperatorOperationImpl<'%', Priority::REMAINDER>;

	using NegOperatorOperation = UnaryOperatorOperation<'-', Priority::UNARY_MINUS>;

	/*using ShiftLeftOperatorOperation = ShiftOperatorOperation<"<<"_c32, Priority::SHIFT>;
	using ShiftRightOperatorOperation = ShiftOperatorOperation<">>"_c32, Priority::SHIFT>;
	using UShiftRightOperatorOperation = ShiftOperatorOperation<">>>"_c32, Priority::SHIFT>;

	using AndOperatorOperation = BooleanBinaryOperatorOperation<'&', Priority::BIT_AND>;
	using OrOperatorOperation  = BooleanBinaryOperatorOperation<'|', Priority::BIT_OR>;
	using XorOperatorOperation = BooleanBinaryOperatorOperation<'^', Priority::BIT_XOR>;*/

}

#endif
