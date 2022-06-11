#ifndef JDECOMPILER_STORE_OPERATIONS_CPP
#define JDECOMPILER_STORE_OPERATIONS_CPP

namespace jdecompiler::operations {

	struct StoreOperation: TransientReturnableOperation {
		public:
			const Operation* const value;
			const uint16_t index;
			const Variable& variable;

		protected:
			int_fast32_t postInc = 0;
			const BinaryOperatorOperation* shortFormOperator = nullptr;
			const Operation* revokeIncrementOperation = nullptr;
			bool canDeclare = false;

		public:
			StoreOperation(const Type* requiredType, const DecompilationContext& context, uint16_t index):
					value(context.stack.popAs(requiredType)), index(index), variable(context.getCurrentScope()->getVariable(index, false)) {

				initReturnType<Dup1Operation, Dup2Operation>(context, value);

				// Check we have load operation on stack
				if(returnType == VOID && !context.stack.empty()) {
					const Operation* operation = context.stack.top();
					if(instanceof<const LoadOperation*>(operation) && static_cast<const LoadOperation*>(operation)->index == index) {
						returnType = context.stack.pop()->getReturnType();
						//log("returnType =", getReturnType());
					}
				}

				value->allowImplicitCast();

				variable.bind(value);

				const Operation* rawValue = value->getOriginalOperation();
				if(instanceof<const InvokeOperation*>(rawValue)) {
					const string methodName = static_cast<const InvokeOperation*>(rawValue)->descriptor.name;
					if(methodName.size() > 3 && stringStartsWith(methodName, "get")) {
						variable.addName(toLowerCamelCase(methodName.substr(3)));
					}
				}

				if(!variable.isDeclared() && returnType == VOID) {
					variable.setDeclared(true);
					canDeclare = true;
				} else {
					const BinaryOperatorOperation* binaryOperator = dynamic_cast<const BinaryOperatorOperation*>(rawValue);

					if(binaryOperator == nullptr && instanceof<const CastOperation*>(rawValue))
						binaryOperator = dynamic_cast<const BinaryOperatorOperation*>(static_cast<const CastOperation*>(rawValue)->value);

					//log("1", binaryOperator != nullptr);

					if(binaryOperator != nullptr) {
						if(const LoadOperation* loadOperation = dynamic_cast<const LoadOperation*>(binaryOperator->operand1->getOriginalOperation())) {
							//log("2", loadOperation->index == index);

							if(loadOperation->index == index) {
								shortFormOperator = binaryOperator;

								const AbstractConstOperation* constOperation = dynamic_cast<const AbstractConstOperation*>(binaryOperator->operand2);

								//log("3", constOperation != nullptr);
								if(constOperation != nullptr) { // post increment
									int_fast8_t incValue = instanceof<const AddOperatorOperation*>(binaryOperator) ? 1 :
												instanceof<const SubOperatorOperation*>(binaryOperator) ? -1 : 0;
									//log("4", (int)incValue);

									if(incValue != 0) {
										initReturnType<Dup1Operation, Dup2Operation>(context, binaryOperator->operand1);

										bool isPostInc =
											instanceof<const IConstOperation*>(constOperation) ?
												static_cast<const IConstOperation*>(constOperation)->value == 1 :
											instanceof<const LConstOperation*>(constOperation) ?
												static_cast<const LConstOperation*>(constOperation)->value == 1 :
											instanceof<const FConstOperation*>(constOperation) ?
												static_cast<const FConstOperation*>(constOperation)->value == 1 :
											instanceof<const DConstOperation*>(constOperation) ?
												static_cast<const DConstOperation*>(constOperation)->value == 1 : false;

										//log("5", isPostInc);
										if(isPostInc) {
											postInc = incValue;
										} else if(returnType != VOID) {
											revokeIncrementOperation = constOperation;
											context.warning("Cannot decompile bytecode exactly: it contains post-increment by a number, other than one or minus one");
										}
									}
								}
							}
						}
					}
				}
				//log("returnType =", getReturnType());
			}

			virtual string toString(const StringifyContext& context) const override {
				//log("A", shortFormOperator != nullptr, postInc);

				return (canDeclare ? variable.getType()->toString(context.classinfo) + ' ' : EMPTY_STRING) +
						// Increment
						(shortFormOperator != nullptr ?
							(postInc != 0 ? (context.getCurrentScope()->getNameFor(variable) + (postInc > 0 ? "++" : "--")) :
							revokeIncrementOperation != nullptr ?
							'(' + shortFormOperator->toShortFormString(context, variable) + ") " + // x++ is equivalent (x += 1) - 1
									shortFormOperator->getOppositeOperator() + ' ' + revokeIncrementOperation->toString(context) :
							shortFormOperator->toShortFormString(context, variable)) :

						context.getCurrentScope()->getNameFor(variable) + " = " +

						// Short form array
						(canDeclare && JDecompiler::getInstance().canUseShortArrayInitializing() ?
								value->toArrayInitString(context) : value->toString(context)));
			}

			virtual Priority getPriority() const override {
				return Priority::ASSIGNMENT;
			}
	};

	struct IStoreOperation: StoreOperation {
		IStoreOperation(const DecompilationContext& context, uint16_t index): StoreOperation(ANY_INT_OR_BOOLEAN, context, index) {}
	};

	struct LStoreOperation: StoreOperation {
		LStoreOperation(const DecompilationContext& context, uint16_t index): StoreOperation(LONG, context, index) {}
	};

	struct FStoreOperation: StoreOperation {
		FStoreOperation(const DecompilationContext& context, uint16_t index): StoreOperation(FLOAT, context, index) {}
	};

	struct DStoreOperation: StoreOperation {
		DStoreOperation(const DecompilationContext& context, uint16_t index): StoreOperation(DOUBLE, context, index) {}
	};

	struct AStoreOperation: StoreOperation {
		AStoreOperation(const DecompilationContext& context, uint16_t index): StoreOperation(AnyObjectType::getInstance(), context, index) {}
	};
}

#endif
