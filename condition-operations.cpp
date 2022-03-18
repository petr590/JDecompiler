#ifndef JDECOMPILER_CONDITION_OPERATIONS_CPP
#define JDECOMPILER_CONDITION_OPERATIONS_CPP

namespace jdecompiler {
	namespace operations {

		//template<BasicType type>
		struct CmpOperation: BooleanOperation {
			const Operation *const operand2, *const operand1;

			CmpOperation(const CodeEnvironment& environment, const Type* operandType):
					operand2(environment.stack.pop()), operand1(environment.stack.pop()) {
						operandType = operandType->castTo(operand1->getReturnType())->castTo(operand2->getReturnType());
						operand1->castReturnTypeTo(operandType);
						operand2->castReturnTypeTo(operandType);
					}
		};


		struct LCmpOperation: CmpOperation {
			LCmpOperation(const CodeEnvironment& environment): CmpOperation(environment, LONG) {}

			virtual string toString(const CodeEnvironment& environment) const override { throw Exception("Illegal using of lcmp: toString()"); }
		};

		struct FCmpOperation: CmpOperation {
			FCmpOperation(const CodeEnvironment& environment): CmpOperation(environment, FLOAT) {}

			virtual string toString(const CodeEnvironment& environment) const override { throw Exception("Illegal using of fcmp: toString()"); }
		};

		struct DCmpOperation: CmpOperation {
			DCmpOperation(const CodeEnvironment& environment): CmpOperation(environment, DOUBLE) {}

			virtual string toString(const CodeEnvironment& environment) const override { throw Exception("Illegal using of dcmp: toString()"); }
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
					static const ExcludingType requiredType({BOOLEAN});
					return &requiredType;
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

			public:
				virtual string toString(const CodeEnvironment& environment) const = 0;

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
				}

			public:
				CompareBinaryOperation(const CmpOperation* cmpOperation, const CompareType& compareType):
						CompareOperation(compareType), operand2(cmpOperation->operand2), operand1(cmpOperation->operand1) {
					castOperandsTo(compareType.getRequiredType());
				}

				CompareBinaryOperation(const CodeEnvironment& environment, const Type* requiredType, const CompareType& compareType):
						/* We don't delegate constructor because of undefined order of initialization of the function arguments
						 * which is important in this case */
						CompareOperation(compareType), operand2(environment.stack.pop()), operand1(environment.stack.pop()) {
					castOperandsTo(compareType.getRequiredType()->castTo(requiredType));
				}

				virtual string toString(const CodeEnvironment& environment) const override {
					return toStringPriority(operand1, environment, Associativity::LEFT) + ' ' + compareType.getOperator(inverted) + ' '
							+ toStringPriority(operand2, environment, Associativity::RIGHT);
				}
		};


		struct CompareWithZeroOperation: CompareOperation {
			const Operation* const operand;

			CompareWithZeroOperation(const Operation* operand, const CompareType& compareType): CompareOperation(compareType), operand(operand) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return operand->getReturnType()->isInstanceof(BOOLEAN) && compareType.isEqualsCompareType ? // write `!bool` instead of `bool == false`
						((const EqualsCompareType&)compareType).getUnaryOperator(inverted) +
								toStringPriority(operand, environment, Associativity::RIGHT) :
						toStringPriority(operand, environment, Associativity::LEFT) + ' ' + compareType.getOperator(inverted) + " 0";
			}
		};


		struct CompareWithNullOperation: CompareOperation {
			protected:
				const Operation* const operand;

			public:
				CompareWithNullOperation(const CodeEnvironment& environment, const EqualsCompareType& compareType):
						CompareOperation(compareType), operand(environment.stack.pop()) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return operand->toString(environment) + ' ' + compareType.getOperator(inverted) + " null";
				}
		};


		struct BinaryConditionOperation: ConditionOperation {
			protected:
				const Operation *const operand1, *const operand2;
				const bool isConditionOperands;

				inline BinaryConditionOperation(const Operation* operand1, const Operation* operand2, bool isConditionOperands):
						operand1(operand1), operand2(operand2), isConditionOperands(isConditionOperands) {}

			public:
				BinaryConditionOperation(const Operation* operand1, const Operation* operand2): BinaryConditionOperation(operand1, operand2,
						instanceof<const ConditionOperation*>(operand1) && instanceof<const ConditionOperation*>(operand2)) {}

				BinaryConditionOperation(const ConditionOperation* operand1, const ConditionOperation* operand2):
						BinaryConditionOperation(operand1, operand2, true) {}

			protected:
				virtual inline void onInvert() const override {
					if(isConditionOperands) {
						static_cast<const ConditionOperation*>(operand1)->invert();
						static_cast<const ConditionOperation*>(operand2)->invert();
					}
				}
		};


		struct AndOperation: BinaryConditionOperation {
			public:
				AndOperation(const Operation* operand1, const Operation* operand2): BinaryConditionOperation(operand1, operand2) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					const string strOperand1 = toStringPriority(operand1, environment, Associativity::LEFT),
							strOperand2 = toStringPriority(operand2, environment, Associativity::RIGHT);

					return inverted ? (isConditionOperands ? strOperand1 + " || " + strOperand2 :
							"!(" + strOperand1 + " && " + strOperand2 + ')') :
							strOperand1 + " && " + strOperand2;
				}

				virtual Priority getPriority() const override {
					return inverted && isConditionOperands ? Priority::LOGICAL_OR : Priority::LOGICAL_AND;
				}
		};


		struct OrOperation: BinaryConditionOperation {
			public:
				OrOperation(const Operation* operand1, const Operation* operand2): BinaryConditionOperation(operand1, operand2) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					const string strOperand1 = toStringPriority(operand1, environment, Associativity::LEFT),
							strOperand2 = toStringPriority(operand2, environment, Associativity::RIGHT);

					return inverted ? (isConditionOperands ? strOperand1 + " && " + strOperand2 :
							"!(" + strOperand1 + " || " + strOperand2 + ')') :
							strOperand1 + " || " + strOperand2;
				}

				virtual Priority getPriority() const override {
					return inverted && isConditionOperands ? Priority::LOGICAL_AND : Priority::LOGICAL_OR;
				}
		};


		struct TernaryOperatorOperation: ReturnableOperation<> {
			const ConditionOperation *const condition;
			const Operation *const trueCase, *const falseCase;

			const bool isShort;

			TernaryOperatorOperation(const ConditionOperation* condition, const Operation* trueCase, const Operation* falseCase):
					ReturnableOperation(trueCase->getReturnTypeAs(falseCase->getReturnType())),
					condition(condition), trueCase(trueCase), falseCase(falseCase),
					isShort(instanceof<const IConstOperation*>(trueCase) && static_cast<const IConstOperation*>(trueCase)->value == 1 &&
					        instanceof<const IConstOperation*>(falseCase) && static_cast<const IConstOperation*>(falseCase)->value == 0) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return isShort ? condition->toString(environment) :
						condition->toString(environment) + " ? " + trueCase->toString(environment) + " : " + falseCase->toString(environment);
			}

			virtual Priority getPriority() const override {
				return Priority::TERNARY_OPERATOR;
			}
		};


		struct IfScope;


		struct ContinueOperation: VoidOperation {
			const IfScope* const ifScope;

			ContinueOperation(const CodeEnvironment& environment, const IfScope* ifScope);

			virtual string toString(const CodeEnvironment& environment) const override;
		};


		struct IfScope: Scope {
			private:
				enum ScopeType {
					IF, TERNARY, WHILE, FOR
				};

				static string to_string(const ScopeType type) {
					switch(type) {
						case IF:      return "if";
						case TERNARY: return "ternary operator";
						case WHILE:   return "while";
						case FOR:     return "for";
						default: return "(ScopeType)" + std::to_string((uint32_t)type);
					}
				}

				struct ElseScope: Scope {
					public:
						const IfScope* const ifScope;

					protected:
						mutable const Operation* ternaryFalseOperation = nullptr;

					public:
						ElseScope(const CodeEnvironment& environment, const uint32_t endPos, const IfScope* ifScope):
								Scope(environment.index, endPos, ifScope), ifScope(ifScope) {
						}

						virtual string getHeader(const CodeEnvironment& environment) const override {
							return " else ";
						}

						virtual inline string getFrontSeparator(const ClassInfo& classinfo) const override {
							return EMPTY_STRING;
						}

						virtual void finalize(const CodeEnvironment& environment) const override {
							if(ifScope->isTernary()) {
								ternaryFalseOperation = environment.stack.pop();
								environment.stack.push(new TernaryOperatorOperation(ifScope->condition,
										ifScope->ternaryTrueOperation, ternaryFalseOperation));
							}
						}

						virtual const Type* getReturnType() const override {
							return ifScope->isTernary() ? ternaryFalseOperation->getReturnType() : this->Scope::getReturnType();
						}
				};


				mutable const ConditionOperation* condition;

				friend struct instructions::IfBlock;
				inline void setEnd(uint32_t endPos) const {
					this->endPos = endPos;
				}

				mutable ScopeType type = IF;

				friend struct ElseScope;
				mutable const ElseScope* elseScope = nullptr;

				mutable const Operation* ternaryTrueOperation = nullptr;

				friend struct ContinueOperation;
				mutable bool hasLabel = false;

			public:
				IfScope(const CodeEnvironment& environment, const int32_t offset, const ConditionOperation* condition):
						Scope(environment.exprStartIndex, environment.bytecode.posToIndex(environment.pos + offset) - 1, environment.getCurrentScope()),
						condition(condition) {}

				inline bool isLoop() const {
					return type == WHILE || type == FOR;
				}

				inline bool isFor() const {
					return type == FOR;
				}

				inline bool isTernary() const {
					return type == TERNARY;
				}


			protected:
				friend struct instructions::GotoInstruction;

				void addElseScope(const CodeEnvironment& environment, const uint32_t endPos) const {
					if(elseScope != nullptr)
						throw IllegalStateException("Else scope already added");

					if(type != IF)
						throw IllegalStateException("Cannot add else scope to " + to_string(type));

					this->endPos = endPos;

					elseScope = new ElseScope(environment, endPos, this);
					environment.addScope(elseScope);
				}

				void setLoopType(const CodeEnvironment& environment) const {
					if(type != IF && !isLoop())
						throw IllegalStateException("Cannot make loop from " + to_string(type));
					const Operation* inital = startPos == 0 ? nullptr : environment.getCurrentScope()->code[startPos - 1];
					LOG("startPos = " << startPos);
					type = WHILE;
				}

				virtual string toString(const CodeEnvironment& environment) const override {
					return elseScope == nullptr ? this->Scope::toString(environment) :
							this->Scope::toString(environment) + elseScope->toString(environment);
				}

				virtual string getHeader(const CodeEnvironment& environment) const override {
					return (string)(isLoop() ? ((hasLabel ? getLabel() + ": " : EMPTY_STRING) +
								(isFor() ? "for" : "while")) :
								"if") +
							'(' + condition->toString(environment) + ") ";
				}

				virtual bool canPrintNextOperation(const vector<const Operation*>::const_iterator& i) const override {
					if(next(i) != code.end())
						return true;
					const ContinueOperation* continueOperation = dynamic_cast<const ContinueOperation*>(*i);
					return continueOperation == nullptr || continueOperation->ifScope != this;
				}

				string getLabel() const {
					if(isLoop())
						return "Loop";
					throw DecompilationException("Cannot get label for " + to_string(type));
				}

			public:
				virtual void finalize(const CodeEnvironment& environment) const override {
					if(elseScope != nullptr && !environment.stack.empty() && code.empty()) {
						type = TERNARY;
						ternaryTrueOperation = environment.stack.pop();
					}
				}

				virtual const Type* getReturnType() const override {
					return isTernary() ? ternaryTrueOperation->getReturnType() : this->Scope::getReturnType();
				}

				virtual void add(const Operation* operation, const CodeEnvironment& environment) const {
					if(operation != elseScope)
						this->Scope::add(operation, environment);
				}
		};



		ContinueOperation::ContinueOperation(const CodeEnvironment& environment, const IfScope* ifScope): ifScope(ifScope) {
			if(ifScope != environment.getCurrentScope())
				ifScope->hasLabel = true;
		}


		string ContinueOperation::toString(const CodeEnvironment& environment) const {
			return ifScope->hasLabel ? "continue " + ifScope->getLabel() : "continue";
		}


		struct EmptyInfiniteLoopScope: Scope {
			EmptyInfiniteLoopScope(const CodeEnvironment& environment): Scope(environment.index, environment.index, environment.getCurrentScope()) {}

			virtual string toString(const CodeEnvironment& environment) const override { return "while(true);"; }
		};
	}
}

#endif
