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


		struct ConditionScope: Scope {
			public:
				const ConditionOperation* condition;

			protected:
				ConditionScope(index_t startIndex, index_t endIndex, const CodeEnvironment& environment, const ConditionOperation* condition):
						Scope(startIndex, endIndex, environment), condition(condition) {}
		};


		struct IfScope: ConditionScope {
			private:
				struct ElseScope: Scope {
					public:
						const IfScope* const ifScope;

					protected:
						mutable const Operation* ternaryFalseOperation = nullptr;

					public:
						ElseScope(const CodeEnvironment& environment, const index_t endIndex, const IfScope* ifScope):
								Scope(ifScope->endIndex, endIndex, ifScope), ifScope(ifScope) {}

						virtual inline string getHeader(const CodeEnvironment&) const override {
							return " else ";
						}

						virtual string toString(const CodeEnvironment& environment) const override {
							if(code.size() == 1 && instanceof<const IfScope*>(code[0])) {
								return getHeader(environment) + code[0]->toString(environment);
							}
							if(code.size() == 2 && instanceof<const IfScope*>(code[0]) && instanceof<const ElseScope*>(code[1])) {
								return getHeader(environment) + code[0]->toString(environment) + code[1]->toString(environment);
							}
							return this->Scope::toString(environment);
						}

						virtual inline string getFrontSeparator(const ClassInfo&) const override {
							return EMPTY_STRING;
						}
				};

				const ElseScope* const elseScope;

			public:
				IfScope(const CodeEnvironment& environment, index_t startIndex, index_t endIndex, const ConditionOperation* condition):
						ConditionScope(startIndex, endIndex, environment, condition), elseScope(nullptr) {}

				IfScope(const CodeEnvironment& environment, index_t startIndex, index_t endIndex, const ConditionOperation* condition,
						index_t elseScopeEndIndex):
						ConditionScope(startIndex, endIndex, environment, condition),
						elseScope(new ElseScope(environment, elseScopeEndIndex, this)) {
					environment.addScope(elseScope);
				}

			protected:
				virtual string getHeader(const CodeEnvironment& environment) const override {
					return "if(" + condition->toString(environment) + ") ";
				}

				virtual inline string getBackSeparator(const ClassInfo& classinfo) const override {
					return elseScope == nullptr ? this->Scope::getBackSeparator(classinfo) : EMPTY_STRING;
				}

			public:

				/*virtual const Type* getReturnType() const override {
					return isTernary() ? ternaryTrueOperation->getReturnType() : this->Scope::getReturnType();
				}*/

				virtual void addOperation(const Operation* operation, const CodeEnvironment& environment) const override {
					if(operation != elseScope)
						this->Scope::addOperation(operation, environment);
				}
		};



		struct LoopScope;

		struct ContinueOperation: VoidOperation {
			public:
				const LoopScope* const loopScope;

			protected:
				bool hasLabel = false;

			public:
				ContinueOperation(const CodeEnvironment& environment, const LoopScope* loopScope);

				virtual string toString(const CodeEnvironment& environment) const override;
		};



		struct LoopScope: ConditionScope {
			protected:
				mutable bool hasLabel = false;

			public:
				LoopScope(const CodeEnvironment& environment, index_t startIndex, index_t endIndex, const ConditionOperation* condition):
						ConditionScope(startIndex, endIndex, environment, condition) {}

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



		ContinueOperation::ContinueOperation(const CodeEnvironment& environment, const LoopScope* loopScope): loopScope(loopScope) {
			const Scope* currentScope = environment.getCurrentScope();
			while(currentScope != nullptr && !currentScope->isContinuable())
				currentScope = currentScope->parentScope;

			if(currentScope != nullptr && currentScope != loopScope)
				hasLabel = true;
		}


		string ContinueOperation::toString(const CodeEnvironment& environment) const {
			return hasLabel ? "continue " + loopScope->getLabel() : "continue";
		}


		struct WhileScope: LoopScope {
			public:
				WhileScope(const CodeEnvironment& environment, index_t startIndex, index_t endIndex, const ConditionOperation* condition):
						LoopScope(environment, startIndex, endIndex, condition) {}

				virtual string getHeader(const CodeEnvironment& environment) const override {
					return "while(" + condition->toString(environment) + ") ";
				}
		};

		struct ForScope: LoopScope {
			public:
				ForScope(const CodeEnvironment& environment, index_t startIndex, index_t endIndex, const ConditionOperation* condition):
						LoopScope(environment, startIndex, endIndex, condition) {}

				virtual string getHeader(const CodeEnvironment& environment) const override {
					return "for(" + condition->toString(environment) + ") ";
				}
		};


		struct SwitchScope: Scope {
			public:
				const Operation* const value;
				const index_t defaultIndex;
				const map<int32_t, index_t> indexTable;

			protected:
				static const map<int32_t, index_t> offsetTableToIndexTable(const CodeEnvironment& environment, const map<int32_t, offset_t>& offsetTable) {
					map<int32_t, index_t> indexTable;

					for(const auto& entry : offsetTable)
						indexTable[entry.first] = environment.bytecode.posToIndex(environment.pos + entry.second);

					return indexTable;
				}

			public:
				SwitchScope(const CodeEnvironment& environment, offset_t defaultOffset, map<int32_t, offset_t> offsetTable):
						Scope(environment.index,
							environment.bytecode.posToIndex(environment.pos + max(defaultOffset, max_element(offsetTable.begin(), offsetTable.end(),
								[] (auto& e1, auto& e2) { return e1.second < e2.second; })->second)),
							environment),
						value(environment.stack.pop()), defaultIndex(environment.bytecode.posToIndex(environment.pos + defaultOffset)),
						indexTable(offsetTableToIndexTable(environment, offsetTable)) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					environment.classinfo.increaseIndent(2);

					string str = "switch(" + value->toString(environment) + ") {\n";
					const size_t baseSize = str.size();

					const map<uint32_t, index_t>& exprIndexTable = environment.exprIndexTable;

					const index_t defaultExprIndex = exprIndexTable.at(defaultIndex);

					uint32_t i = exprIndexTable.at(this->startIndex);
					for(const Operation* operation : code) {
						if(i == defaultExprIndex) {
							environment.classinfo.reduceIndent();
							str += environment.classinfo.getIndent() + (string)"default:\n";
							environment.classinfo.increaseIndent();
						} else {
							for(auto& entry : indexTable) {
								if(i == exprIndexTable.at(entry.second)) {
									environment.classinfo.reduceIndent();
									str += environment.classinfo.getIndent() + (string)"case " + to_string(entry.first) + ":\n";
									environment.classinfo.increaseIndent();
									break;
								}
							}
						}
						str += environment.classinfo.getIndent() + operation->toString(environment) + (instanceof<const Scope*>(operation) ? "\n" : ";\n");
						i++;
					}

					environment.classinfo.reduceIndent(2);

					if(str.size() == baseSize) {
						str[baseSize - 1] = '}';
						return str;
					}

					return str + environment.classinfo.getIndent() + '}';
				}

				virtual bool isBreakable() const override {
					return true;
				}
		};



		struct EmptyInfiniteLoopScope: Scope {
			EmptyInfiniteLoopScope(const CodeEnvironment& environment):
					Scope(environment.index, environment.index, environment) {}

			virtual string toString(const CodeEnvironment&) const override {
				return "while(true) {}";
			}
		};
	}
}

#endif
