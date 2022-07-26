#ifndef JDECOMPILER_CONDITION_SCOPES_CPP
#define JDECOMPILER_CONDITION_SCOPES_CPP

#include "conditions.cpp"

namespace jdecompiler::operations {

	struct ConditionScope: Scope {
		protected:
			mutable const ConditionOperation* condition;

			ConditionScope(index_t startIndex, index_t endIndex, const DecompilationContext& context, const ConditionOperation* condition):
					Scope(startIndex, endIndex, context), condition(condition) {}

			ConditionScope(index_t startIndex, index_t endIndex, const Scope* parentScope, const ConditionOperation* condition):
					Scope(startIndex, endIndex, parentScope), condition(condition) {}

		public:
			inline const ConditionOperation* getCondition() const {
				return condition;
			}
	};


	struct IfScope: ConditionScope {
		protected:
			inline void setCondition(const ConditionOperation* condition) const {
				this->condition = condition;
			}

			friend struct instructions::IfBlock;

		private:
			struct ElseScope: Scope {
				public:
					const IfScope* const ifScope;

				protected:
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
								context.stack.push(new TernaryOperatorOperation(ifScope->condition, ifScope->ternaryTrueCase, context.stack.pop()));
							} else {
								context.warning("fail to indicate false case of ternary operator");
								ifScope->isTernary = false;
							}
						}
					}

					virtual bool canStringify() const override {
						return !isTernary;
					}

			};

		private:
			mutable index_t bodyStartIndex;
			friend struct instructions::IfBlock;

			const ElseScope* const elseScope;
			mutable const Operation* assertOperation = nullptr;
			mutable bool isTernary = false;
			mutable const Operation* ternaryTrueCase = nullptr;

			IfScope(const DecompilationContext& context, index_t endIndex, const ConditionOperation* condition,
					const Scope* parentScope, const function<const ElseScope*()>& elseScopeGetter):
					ConditionScope(context.exprStartIndex, endIndex, parentScope, condition), bodyStartIndex(context.index + 1),
					elseScope(elseScopeGetter()) {}

		public:
			IfScope(const DecompilationContext& context, index_t endIndex, const ConditionOperation* condition, const Scope* parentScope):
					IfScope(context, endIndex, condition, parentScope, [] () { return nullptr; }) {}

			IfScope(const DecompilationContext& context, index_t endIndex, const ConditionOperation* condition, const Scope* parentScope,
					index_t elseScopeEndIndex):
					IfScope(context, endIndex, condition, parentScope,
						[&context, elseScopeEndIndex, this] () { return new ElseScope(context, elseScopeEndIndex, this); }) {
				context.addScope(elseScope);
			}

		protected:
			virtual string getHeader(const StringifyContext& context) const override {
				return "if(" + condition->toString(context) + ") ";
			}

			virtual string toStringImpl(const StringifyContext& context) const override {
				if(assertOperation != nullptr)
					return "assert " + assertOperation->toString(context) + ';';

				return ConditionScope::toStringImpl(context);
			}

			virtual inline string getBackSeparator(const ClassInfo& classinfo) const override {
				return elseScope == nullptr ? this->Scope::getBackSeparator(classinfo) : EMPTY_STRING;
			}

		public:
			virtual void addOperation(const Operation* operation, const DecompilationContext& context) const override {
				if(operation != elseScope)
					this->Scope::addOperation(operation, context);
			}

			virtual void finalize(const DecompilationContext& context) const override {

				if(elseScope == nullptr) {

					if(const AndOperation* andOperation = dynamic_cast<const AndOperation*>(condition)) {

						if(const CompareWithZeroOperation* operand1 = dynamic_cast<const CompareWithZeroOperation*>(andOperation->operand1)) {

							if(const GetStaticFieldOperation* getStatic = dynamic_cast<const GetStaticFieldOperation*>(operand1->operand)) {

								static const FieldDescriptor assertionsDisabledDescriptor("$assertionsDisabled", BOOLEAN);

								if(getStatic->clazz == context.classinfo.thisType && getStatic->descriptor == assertionsDisabledDescriptor) {

									const Field* assertionsDisabledField = JDecompiler::getInstance()
											.getClass(context.classinfo.thisType.getEncodedName())->getField(assertionsDisabledDescriptor);

									if(assertionsDisabledField != nullptr && assertionsDisabledField->isSynthetic()) {
										if(code.size() == 1) {
											if(const AThrowOperation* athrow = dynamic_cast<const AThrowOperation*>(code[0])) {
												if(const InvokespecialOperation* constructorInvoke =
														dynamic_cast<const InvokespecialOperation*>(athrow->exception)) {

													static const MethodDescriptor
															AssertionErrorConstructor(*new ClassType("java/lang/AssertionError"), "<init>", VOID);

													if(constructorInvoke->descriptor == AssertionErrorConstructor) {
														assertOperation = andOperation->operand2->invert();
														return;
													}
												}
											}
										}

										context.warning("Cannot decompile assertion: illegal code in assertion block");
									}
								}
							}
						}
					}

				} else {
					if(code.empty() && elseScope->code.empty() && !context.stack.empty()) {
						isTernary = true;
						ternaryTrueCase = context.stack.pop();
					}
				}
			}

			virtual bool canStringify() const override {
				return !isTernary;
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

		public:
			LoopScope(const DecompilationContext& context, index_t startIndex, index_t endIndex, const ConditionOperation* condition):
					ConditionScope(startIndex, endIndex, context, condition) {}

			virtual string defaultLabelName() const override {
				return "Loop";
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
				private: constexpr TrueConstOperation() noexcept {}

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

		protected:
			mutable const Operation* initializing = nullptr;
			mutable vector<const Operation*> updatingOperations;

		public:
			InfiniteLoopScope(const DecompilationContext& context, index_t startIndex, index_t endIndex):
					LoopScope(context, startIndex, endIndex, TrueConstOperation::getInstance()) {}


			virtual string getHeader(const StringifyContext& context) const override {
				return initializing != nullptr || !updatingOperations.empty() ?
						"for(" + (initializing != nullptr ? initializing->toString(context) : EMPTY_STRING) + "; " + condition->toString(context) + ';'
								+ (!updatingOperations.empty() ? ' ' + rjoin<const Operation*>(updatingOperations,
										[&context] (const Operation* operation) { return operation->toString(context); }, ", ") : EMPTY_STRING) + ") " :
						"while(" + condition->toString(context) + ") ";
			}


			virtual void finalize(const DecompilationContext&) const override {
				if(code.size() == 1 && instanceof<const IfScope*>(code[0])) {
					const IfScope* ifScope = static_cast<const IfScope*>(code[0]);
					condition = ifScope->getCondition();
					code = ifScope->getCode();

					const vector<Variable*>& ifScopeVariables = ifScope->getVariables();
					if(variables.size() < ifScopeVariables.size())
						variables.resize(ifScopeVariables.size());

					for(size_t i = 0; i < ifScopeVariables.size(); i++) {
						Variable* var = ifScopeVariables[i];

						if(var != nullptr)
							variables[i] = var;
					}
				}

				if(!code.empty() && code.back()->isIncrement()) {
					do {
						updatingOperations.push_back(code.back());
						code.pop_back();
					} while(!code.empty() && code.back()->isIncrement());

					if(parentScope->code.size() >= 2) {
						const auto penultimateIter = parentScope->code.end() - 2;
						if(instanceof<const StoreOperation*>(*penultimateIter)) {
							const StoreOperation* store = static_cast<const StoreOperation*>(*penultimateIter);
							if(store->isDeclare()) {
								store->variable.makeCounter();
							}

							initializing = store;
							parentScope->code.erase(penultimateIter);
						}
					}
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

	struct BreakOperation: VoidOperation {
		const Scope* const scope;
		bool hasLabel;

		BreakOperation(const DecompilationContext& context, const Scope* scope):
				scope(scope), hasLabel([&context, scope] () {
					for(const Scope* currentScope = context.getCurrentScope(); currentScope != nullptr; currentScope = currentScope->parentScope) {
						if(currentScope->isBreakable() && currentScope != scope)
							return true;
					}
					return false;
				}()) {
					if(hasLabel)
						scope->makeLabel();
				}

		virtual string toString(const StringifyContext&) const override {
			return hasLabel ? "break " + scope->getLabel() : "break";
		}
	};
}

#endif
