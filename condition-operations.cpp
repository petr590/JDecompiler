#ifndef JDECOMPILER_CONDITION_OPERATIONS_CPP
#define JDECOMPILER_CONDITION_OPERATIONS_CPP

namespace jdecompiler {
	namespace operations {

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
				}

			public:
				CompareBinaryOperation(const CmpOperation* cmpOperation, const CompareType& compareType):
						CompareOperation(compareType), operand2(cmpOperation->operand2), operand1(cmpOperation->operand1) {
					castOperandsTo(compareType.getRequiredType());
				}

				CompareBinaryOperation(const DecompilationContext& context, const Type* requiredType, const CompareType& compareType):
						/* We don't delegate constructor because of undefined order of initialization of the function arguments
						 * which is important in this case */
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

			CompareWithZeroOperation(const Operation* operand, const CompareType& compareType): CompareOperation(compareType), operand(operand) {}

			virtual string toString(const StringifyContext& context) const override {
				return operand->getReturnType()->isSubtypeOf(BOOLEAN) && compareType.isEqualsCompareType ? // write `!bool` instead of `bool == false`
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

				virtual string toString(const StringifyContext& context) const override {
					const string strOperand1 = toStringPriority(operand1, context, Associativity::LEFT),
							strOperand2 = toStringPriority(operand2, context, Associativity::RIGHT);

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

				virtual string toString(const StringifyContext& context) const override {
					const string strOperand1 = toStringPriority(operand1, context, Associativity::LEFT),
							strOperand2 = toStringPriority(operand2, context, Associativity::RIGHT);

					return inverted ? (isConditionOperands ? strOperand1 + " && " + strOperand2 :
							"!(" + strOperand1 + " || " + strOperand2 + ')') :
							strOperand1 + " || " + strOperand2;
				}

				virtual Priority getPriority() const override {
					return inverted && isConditionOperands ? Priority::LOGICAL_AND : Priority::LOGICAL_OR;
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
				return isShort ? condition->toString(context) :
						condition->toString(context) + " ? " + trueCase->toString(context) + " : " + falseCase->toString(context);
			}

			virtual Priority getPriority() const override {
				return Priority::TERNARY_OPERATOR;
			}
		};


		struct ConditionScope: Scope {
			protected:
				mutable const ConditionOperation* condition;

				ConditionScope(index_t startIndex, index_t endIndex, const DecompilationContext& context, const ConditionOperation* condition):
						Scope(startIndex, endIndex, context), condition(condition) {}

			public:
				inline const ConditionOperation* getCondition() const {
					return condition;
				}
		};


		struct IfScope: ConditionScope {
			private:
				struct ElseScope: Scope {
					public:
						const IfScope* const ifScope;

					protected:
						mutable const Operation* ternaryFalseCase = nullptr;
						mutable bool isTernary = false;

					public:
						ElseScope(const DecompilationContext& context, const index_t endIndex, const IfScope* ifScope):
								Scope(ifScope->endIndex, endIndex, ifScope->parentScope), ifScope(ifScope) {}

						virtual inline string getHeader(const StringifyContext&) const override {
							return " else ";
						}

						virtual string toStringImpl(const StringifyContext& context) const override {
							if(code.size() == 1 && instanceof<const IfScope*>(code[0])) { // else if
								return getHeader(context) + code[0]->toString(context);
							}
							if(code.size() == 2 && instanceof<const IfScope*>(code[0]) && instanceof<const ElseScope*>(code[1])) { // else if ... else
								return getHeader(context) + code[0]->toString(context) + code[1]->toString(context);
							}
							return this->Scope::toStringImpl(context);
						}

						virtual inline string getFrontSeparator(const ClassInfo&) const override {
							return EMPTY_STRING;
						}

						virtual void finalize(const DecompilationContext& context) const override {
							if(ifScope->isTernary) {
								if(!context.stack.empty()) {
									isTernary = true;
									ternaryFalseCase = context.stack.pop();
									context.stack.push(new TernaryOperatorOperation(ifScope->condition, ifScope->ternaryTrueCase, ternaryFalseCase));
								} else {
									context.warning("fail to indicate false case of ternary operator");
									ifScope->isTernary = false;
								}

								/*log('[');
								for(int i = 0; i < context.stack.size(); i++)
									log(typeNameOf(context.stack.lookup(i)));
								log(']');*/
							}
						}

						virtual const Type* getReturnType() const override {
							return isTernary ? ternaryFalseCase->getReturnType() : Scope::getReturnType();
						}

				};


				const ElseScope* const elseScope;
				mutable bool isTernary = false;
				mutable const Operation* ternaryTrueCase = nullptr;

			public:
				IfScope(const DecompilationContext& context, index_t endIndex, const ConditionOperation* condition):
						ConditionScope(context.exprStartIndex, endIndex, context, condition), elseScope(nullptr) {
				}

				IfScope(const DecompilationContext& context, index_t endIndex, const ConditionOperation* condition, index_t elseScopeEndIndex):
						ConditionScope(context.exprStartIndex, endIndex, context, condition),
						elseScope(new ElseScope(context, elseScopeEndIndex, this)) {
					context.addScope(elseScope);
				}

			protected:
				virtual string getHeader(const StringifyContext& context) const override {
					return "if(" + condition->toString(context) + ") ";
				}

				virtual inline string getBackSeparator(const ClassInfo& classinfo) const override {
					return elseScope == nullptr ? this->Scope::getBackSeparator(classinfo) : EMPTY_STRING;
				}

			public:
				virtual void addOperation(const Operation* operation, const StringifyContext& context) const override {
					if(operation != elseScope)
						this->Scope::addOperation(operation, context);
				}

				virtual void finalize(const DecompilationContext& context) const override {
					if(elseScope != nullptr && code.empty() && elseScope->code.empty() && !context.stack.empty()) {
						isTernary = true;
						ternaryTrueCase = context.stack.pop();
					}
				}

				virtual const Type* getReturnType() const override {
					return isTernary ? ternaryTrueCase->getReturnType() : Scope::getReturnType();
				}
		};



		struct LoopScope;

		struct ContinueOperation: VoidOperation {
			public:
				const LoopScope* const loopScope;

			protected:
				bool hasLabel = false;

			public:
				ContinueOperation(const DecompilationContext& context, const LoopScope* loopScope);

				virtual string toString(const StringifyContext& context) const override;
		};



		struct LoopScope: ConditionScope {
			protected:
				mutable bool hasLabel = false;

			public:
				LoopScope(const DecompilationContext& context, index_t startIndex, index_t endIndex, const ConditionOperation* condition):
						ConditionScope(startIndex, endIndex, context, condition) {}

				string getLabel() const {
					if(!hasLabel)
						hasLabel = true;
					return "Label1";
				}

				virtual bool isBreakable() const override {
					return true;
				}

				virtual bool isContinuable() const override {
					return true;
				}

			protected:
				virtual bool canPrintNextOperation(const vector<const Operation*>::const_iterator& i) const override {
					if(next(i) != code.end())
						return true;
					const ContinueOperation* continueOperation = dynamic_cast<const ContinueOperation*>(*i);
					return continueOperation == nullptr || continueOperation->loopScope != this;
				}
		};



		ContinueOperation::ContinueOperation(const DecompilationContext& context, const LoopScope* loopScope): loopScope(loopScope) {
			const Scope* currentScope = context.getCurrentScope();
			while(currentScope != nullptr && !currentScope->isContinuable())
				currentScope = currentScope->parentScope;

			if(currentScope != nullptr && currentScope != loopScope)
				hasLabel = true;
		}


		string ContinueOperation::toString(const StringifyContext& context) const {
			return hasLabel ? "continue " + loopScope->getLabel() : "continue";
		}


		struct WhileScope: LoopScope {
			public:
				WhileScope(const DecompilationContext& context, index_t startIndex, index_t endIndex, const ConditionOperation* condition):
						LoopScope(context, startIndex, endIndex, condition) {}

				virtual string getHeader(const StringifyContext& context) const override {
					return "while(" + condition->toString(context) + ") ";
				}
		};

		struct ForScope: LoopScope {
			public:
				ForScope(const DecompilationContext& context, index_t startIndex, index_t endIndex, const ConditionOperation* condition):
						LoopScope(context, startIndex, endIndex, condition) {}

				virtual string getHeader(const StringifyContext& context) const override {
					return "for(" + condition->toString(context) + ") ";
				}
		};


		struct InfiniteLoopScope: LoopScope {
			public:
				struct TrueConstOperation: ConditionOperation {
					private: TrueConstOperation() {}

					public:
						virtual const Type* getReturnType() const override {
							return BOOLEAN;
						}

						virtual string toString(const StringifyContext&) const override {
							return "true";
						}

						static const TrueConstOperation* getInstance() {
							static TrueConstOperation INSTANCE;
							return &INSTANCE;
						}
				};


				InfiniteLoopScope(const DecompilationContext& context, index_t startIndex, index_t endIndex):
						LoopScope(context, startIndex, endIndex, TrueConstOperation::getInstance()) {}


				virtual string getHeader(const StringifyContext& context) const override {
					return "while(" + condition->toString(context) + ") ";
				}


				virtual void finalize(const DecompilationContext&) const override {
					if(code.size() == 1 && instanceof<const IfScope*>(code[0])) {
						const IfScope* ifScope = static_cast<const IfScope*>(code[0]);
						condition = ifScope->getCondition();
						code = ifScope->getCode();
					}
				}
		};


		struct SwitchScope: Scope {
			public:
				const Operation* const value;
				const index_t defaultIndex;
				const map<int32_t, index_t> indexTable;

			protected:
				static const map<int32_t, index_t> offsetTableToIndexTable(const DecompilationContext& context, const map<int32_t, offset_t>& offsetTable) {
					map<int32_t, index_t> indexTable;

					for(const auto& entry : offsetTable)
						indexTable[entry.first] = context.posToIndex(context.pos + entry.second);

					return indexTable;
				}

			public:
				SwitchScope(const DecompilationContext& context, offset_t defaultOffset, map<int32_t, offset_t> offsetTable):
						Scope(context.index,
							context.posToIndex(context.pos + max(defaultOffset, max_element(offsetTable.begin(), offsetTable.end(),
								[] (auto& e1, auto& e2) { return e1.second < e2.second; })->second)),
							context),
						value(context.stack.popAs(ANY_INT)), defaultIndex(context.posToIndex(context.pos + defaultOffset)),
						indexTable(offsetTableToIndexTable(context, offsetTable)) {}

				virtual string toStringImpl(const StringifyContext& context) const override {
					context.classinfo.increaseIndent(2);

					string str = "switch(" + value->toString(context) + ") {\n";
					const size_t baseSize = str.size();

					const map<uint32_t, index_t>& exprIndexTable = context.exprIndexTable;

					const index_t defaultExprIndex = exprIndexTable.at(defaultIndex);

					uint32_t i = exprIndexTable.at(this->startIndex);
					for(const Operation* operation : code) {
						if(i == defaultExprIndex) {
							context.classinfo.reduceIndent();
							str += context.classinfo.getIndent() + (string)"default:\n";
							context.classinfo.increaseIndent();
						} else {
							for(auto& entry : indexTable) {
								if(i == exprIndexTable.at(entry.second)) {
									context.classinfo.reduceIndent();
									str += context.classinfo.getIndent() + (string)"case " + to_string(entry.first) + ":\n";
									context.classinfo.increaseIndent();
									break;
								}
							}
						}
						str += context.classinfo.getIndent() + operation->toString(context) + (instanceof<const Scope*>(operation) ? "\n" : ";\n");
						i++;
					}

					context.classinfo.reduceIndent(2);

					if(str.size() == baseSize) {
						str[baseSize - 1] = '}';
						return str;
					}

					return str + context.classinfo.getIndent() + '}';
				}

				virtual bool isBreakable() const override {
					return true;
				}
		};



		struct EmptyInfiniteLoopScope: Scope {
			EmptyInfiniteLoopScope(const DecompilationContext& context):
					Scope(context.index, context.index, context) {}

			virtual string toStringImpl(const StringifyContext&) const override {
				return "while(true) {}";
			}
		};
	}
}

#endif
