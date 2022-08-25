#ifndef JDECOMPILER_STORE_OPERATIONS_CPP
#define JDECOMPILER_STORE_OPERATIONS_CPP

namespace jdecompiler {

	struct StoreOperation: TransientReturnableOperation {
		/* The changed order of the fields is needed to optimize the size of the object (size decreased from 72 to 56 bytes)
		   Yes, I have nothing else to do but optimize the size of c++ objects */
		public:
			const Operation* const value;
			const Variable& variable;

			const uint16_t index;

		protected:
			bool declare = false;
			mutable int_fast8_t inc = 0;
			bool isStringAppend = false;

			const BinaryOperatorOperation* shortFormOperator = nullptr;
			const Operation* revokeIncrementOperation = nullptr;

		public:
			StoreOperation(const Type* requiredType, const DecompilationContext& context, uint16_t index):
					value(context.stack.popAs(requiredType)), variable(context.getCurrentScope()->getVariable(index, false)), index(index) {

				initReturnType<Dup1Operation, Dup2Operation>(context, value);

				bool isPostInc = returnType == VOID;

				// Check we have load operation on stack
				if(returnType == VOID && !context.stack.empty()) {
					const Operation* operation = context.stack.top();
					if(instanceof<const LoadOperation*>(operation) && static_cast<const LoadOperation*>(operation)->index == index) {
						returnType = context.stack.pop()->getReturnType();
					}
				}

				value->allowImplicitCast();

				variable.bind(value);

				const Operation* rawValue = value->getOriginalOperation();
				if(instanceof<const InvokeOperation*>(rawValue)) {
					const string methodName = static_cast<const InvokeOperation*>(rawValue)->descriptor.name;
					if(stringStartsWith(methodName, "get") && methodName.size() > 3) {
						variable.addName(toLowerCamelCase(methodName.substr(3)));
					} else if(stringStartsWith(methodName, "as") && methodName.size() > 2) {
						variable.addName(toLowerCamelCase(methodName.substr(2)));
					} else if(stringStartsWith(methodName, "is") && methodName.size() > 2) {
						variable.addName(methodName);
					}
				}

				if(!variable.isDeclared()) {
					if(returnType == VOID) {
						variable.setDeclared(true);
						declare = true;
					} else {
						context.getCurrentScope()->addOperation(new DeclareVariableOperation(variable), context);
					}
				}

				if(!declare) {
					const BinaryOperatorOperation* binaryOperator = dynamic_cast<const BinaryOperatorOperation*>(rawValue);

					if(binaryOperator == nullptr && instanceof<const CastOperation*>(rawValue))
						binaryOperator = dynamic_cast<const BinaryOperatorOperation*>(static_cast<const CastOperation*>(rawValue)->value);

					if(binaryOperator != nullptr) {
						if(const LoadOperation* loadOperation = dynamic_cast<const LoadOperation*>(binaryOperator->operand1->getOriginalOperation())) {

							if(loadOperation->index == index && !loadOperation->isIncrement()) {
								shortFormOperator = binaryOperator;

								const AbstractConstOperation* constOperation = dynamic_cast<const AbstractConstOperation*>(binaryOperator->operand2);

								if(constOperation != nullptr) { // post increment
									int_fast8_t incValue = instanceof<const AddOperatorOperation*>(binaryOperator) ? 1 :
												instanceof<const SubOperatorOperation*>(binaryOperator) ? -1 : 0;

									if(incValue != 0) {
										initReturnType<Dup1Operation, Dup2Operation>(context, binaryOperator->operand1);

										bool isShortInc =
											instanceof<const IConstOperation*>(constOperation) ?
												static_cast<const IConstOperation*>(constOperation)->value == 1 :
											instanceof<const LConstOperation*>(constOperation) ?
												static_cast<const LConstOperation*>(constOperation)->value == 1 :
											instanceof<const FConstOperation*>(constOperation) ?
												static_cast<const FConstOperation*>(constOperation)->value == 1 :
											instanceof<const DConstOperation*>(constOperation) ?
												static_cast<const DConstOperation*>(constOperation)->value == 1 : false;

										if(isShortInc) {
											inc = incValue << isPostInc; // post-inc if return type is void, else pre-inc
										} else if(returnType != VOID) {
											revokeIncrementOperation = constOperation;
											context.warning("cannot decompile bytecode exactly: it contains post-increment by a number,"
															"other than one or minus one");
										}
									}
								}
							}
						}

					} else if(const ConcatStringsOperation* concatOperation = dynamic_cast<const ConcatStringsOperation*>(rawValue)) {
						vector<const Operation*>& operands = concatOperation->operands;

						if(const ALoadOperation* aloadOperation = dynamic_cast<const ALoadOperation*>(operands[0])) {
							if(aloadOperation->index == index) {
								isStringAppend = true;
								operands.erase(operands.begin());
								if(operands.size() > 1)
									concatOperation->insertEmptyStringIfNecessary();
							}
						}
					}
				}
			}

			inline string valueToString(const StringifyContext& context) const {
				return (declare && JDecompiler::getInstance().useShortArrayInitializing() ?
						value->toArrayInitString(context) : value->toString(context));
			}

			inline bool isDeclare() const {
				return declare;
			}


			virtual string toString(const StringifyContext& context) const override {

				if(declare) {
					return variableDeclarationToString(variable.getType(), context.classinfo, context.getCurrentScope()->getNameFor(variable)) +
							" = " + valueToString(context);
				}

				if(shortFormOperator != nullptr) { // Increment
					if(inc != 0) {
						const string
								name = context.getCurrentScope()->getNameFor(variable),
								incStr = inc > 0 ? "++" : "--";

						return inc & 0x1 ? incStr + name : name + incStr;
					}

					return revokeIncrementOperation != nullptr ?
						'(' + shortFormOperator->toShortFormString(context, context.getCurrentScope()->getNameFor(variable)) + ") " + // x++ is equivalent (x += 1) - 1
								shortFormOperator->getOppositeOperator() + ' ' + revokeIncrementOperation->toString(context) :
						shortFormOperator->toShortFormString(context, context.getCurrentScope()->getNameFor(variable));
				}

				if(isStringAppend) {
					return context.getCurrentScope()->getNameFor(variable) + " += " + valueToString(context);
				}

				return context.getCurrentScope()->getNameFor(variable) + " = " + valueToString(context);
			}

		protected:
			void bind(const LoadOperation* loadOperation) const {
				assert(returnType == VOID && loadOperation->index == index);
				returnType = loadOperation->getReturnType();
				inc >>= 1;
			}

			friend LoadOperation::LoadOperation(const Type*, const DecompilationContext&, uint16_t);

		public:
			virtual Priority getPriority() const override {
				return Priority::ASSIGNMENT;
			}

			virtual bool isIncrement() const override {
				return inc != 0;
			}
	};


	LoadOperation::LoadOperation(const Type* requiredType, const DecompilationContext& context, uint16_t index):
			index(index), variable(context.getCurrentScope()->getVariable(index, true)) {
		variable.castTypeTo(requiredType);

		vector<const Operation*>& tempCode = context.getCurrentScope()->getTempCode();
		if(!tempCode.empty()) {

			const Operation* operation = tempCode.back();
			if(operation->isIncrement() && instanceof<const StoreOperation*>(operation) && static_cast<const StoreOperation*>(operation)->index == index) {

				static_cast<const StoreOperation*>(operation)->bind(this);
				incOperation = operation;
				tempCode.pop_back();
			}
		}
	}


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
		protected:
			bool isCatchScopeHandler = false;

			void handleCatchScope();

		public:
			AStoreOperation(const DecompilationContext& context, uint16_t index): StoreOperation(AnyObjectType::getInstance(), context, index) {
				handleCatchScope();
			}

			virtual bool canAddToCode() const override;
	};
}

#endif
