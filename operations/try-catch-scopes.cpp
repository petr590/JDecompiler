#ifndef JDECOMPILER_TRY_CATCH_SCOPES_CPP
#define JDECOMPILER_TRY_CATCH_SCOPES_CPP

namespace jdecompiler::operations {

	struct CatchBlockDataHolder {
		index_t startIndex;
		vector<const ClassType*> catchTypes;

		CatchBlockDataHolder(index_t startIndex, const ClassType* catchType): startIndex(startIndex), catchTypes{catchType} {}
	};


	struct TryScope: Scope {
		public:
			TryScope(const DecompilationContext& context, index_t startIndex, index_t endIndex):
					Scope(startIndex, endIndex, context) {}

			virtual string getHeader(const StringifyContext& context) const override {
				return "try ";
			}

			virtual string getBackSeparator(const ClassInfo& classinfo) const override {
				return EMPTY_STRING;
			}
	};


	struct CatchScope: Scope {
		public:
			const vector<const ClassType*> catchTypes;
			const ClassType* const catchType;

		protected:
			mutable vector<const Operation*> tmpStack;
			mutable Variable* exceptionVariable = nullptr;
			mutable uint16_t exceptionVariableIndex;
			const bool hasNext;

		public:
			CatchScope(const DecompilationContext& context, index_t startIndex, index_t endIndex,
					const vector<const ClassType*>& catchTypes, bool hasNext):
					Scope(startIndex, endIndex, context), catchTypes(catchTypes), catchType(catchTypes[0]), hasNext(hasNext) {}

			CatchScope(const DecompilationContext& context, index_t startIndex, index_t endIndex,
					const initializer_list<const ClassType*>& catchTypes, bool hasNext):
					CatchScope(context, startIndex, endIndex, vector<const ClassType*>(catchTypes), hasNext) {}


			virtual void addOperation(const Operation* operation, const DecompilationContext& context) const override {
				if(exceptionVariable == nullptr) {
					if(instanceof<const StoreOperation*>(operation)) {
						exceptionVariableIndex = static_cast<const StoreOperation*>(operation)->index;
						exceptionVariable = new NamedVariable(catchType, true, "ex");
						return;
					} /*else {
						//context.classinfo.warning("first instruction in the catch or finally block should be `astore`"); // TODO
					}*/
				}
				Scope::addOperation(operation, context);
			}

			virtual const Variable& getVariable(index_t index, bool isDeclared) const override {
				if(exceptionVariable != nullptr && index == exceptionVariableIndex)
					return *exceptionVariable;
				return Scope::getVariable(index, isDeclared);
			}


			virtual string getFrontSeparator(const ClassInfo& classinfo) const override {
				return " ";
			}

			virtual string getHeader(const StringifyContext& context) const override {
				return catchType == nullptr ? "finally" :
						"catch(" + join<const ClassType*>(catchTypes,
								[&context] (const ClassType* catchType) { return catchType->toString(context.classinfo); }, " | ") +
									' ' + context.getCurrentScope()->getNameFor(*exceptionVariable) + ") ";
			}

			virtual string getBackSeparator(const ClassInfo& classinfo) const override {
				return hasNext ? EMPTY_STRING : "\n";
			}

			void initiate(const DecompilationContext& context) {
				tmpStack.reserve(context.stack.size());
				while(!context.stack.empty())
					tmpStack.push_back(context.stack.pop());

				context.stack.push(new LoadCatchedExceptionOperation(catchTypes.empty() ? catchType : THROWABLE));
			}

		protected:
			struct LoadCatchedExceptionOperation: Operation {
				const ClassType* const catchType;

				LoadCatchedExceptionOperation(const ClassType* catchType): catchType(catchType) {}

				virtual const Type* getReturnType() const override {
					return catchType;
				}

				virtual string toString(const StringifyContext& context) const override {
					throw Exception("Illegal using of LoadCatchedExceptionOperation: toString()");
				}
			};

		public:
			virtual void finalize(const DecompilationContext& context) const override {
				reverse(tmpStack.begin(), tmpStack.end());
				for(const Operation* operation : tmpStack)
					context.stack.push(operation);
			}
	};
}

#endif
