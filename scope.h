#ifndef JDECOMPILER_SCOPE_H
#define JDECOMPILER_SCOPE_H

namespace jdecompiler {

	struct Scope: Operation {
		public:
			const index_t startIndex, endIndex;

			const Scope *const parentScope;

		protected:
			mutable vector<Variable*> variables;
			mutable uint16_t lastAddedVarIndex = 0;

			friend struct IfScope;
			friend struct InfiniteLoopScope;

			mutable vector<const Operation*> code, tempCode;
			mutable umap<const Variable*, string> varNames;
			mutable vector<const Scope*> innerScopes;

		private:
			Scope(index_t, index_t, const Scope*, uint16_t variablesCount);

		public:
			inline Scope(index_t startIndex, index_t endIndex, uint16_t variablesCount):
					Scope(startIndex, endIndex, nullptr, variablesCount) {}

			inline Scope(index_t startIndex, index_t endIndex, const Scope* parentScope):
					Scope(startIndex, endIndex, parentScope, parentScope->variables.size()) {}

			inline Scope(index_t startIndex, index_t endIndex, const DecompilationContext& context):
					Scope(startIndex, endIndex, context.getCurrentScope()) {}


			inline index_t start() const {
				return startIndex;
			}

			inline index_t end() const {
				return endIndex;
			}

			// unsafe, but who cares?
			inline vector<const Operation*>& getCode() const {
				return code;
			}

			inline vector<const Operation*>& getTempCode() const {
				return tempCode;
			}

		protected:
			const Variable* findVariable(index_t) const;

			const Variable* findVariableAtInnerScopes(index_t) const;

		public:
			virtual const Variable& getVariable(index_t, bool isDeclared) const;

			inline bool hasVariable(const string& name) const {
				return find_if(varNames.begin(), varNames.end(), [&name] (const auto& it) { return it.second == name; }) != varNames.end() ||
						(parentScope != nullptr && parentScope->hasVariable(name));
			}

			void addVariable(Variable*);

			inline string getNameFor(const Variable& var) const {
				return getNameFor(&var);
			}

			string getNameFor(const Variable*) const;

			inline const vector<Variable*>& getVariables() const {
				return variables;
			}

			inline uint32_t getVariablesCount() const {
				return variables.size();
			}

			//virtual void initiate(const DecompilationContext&) {}

			virtual void finalize(const DecompilationContext&) const;

			virtual string toString(const StringifyContext&) const override final;

			virtual string toStringImpl(const StringifyContext&) const;


			bool bracketsOmitted() const;

			size_t getStringifiedOperationsCount() const;

		protected:
			virtual inline bool canPrintNextOperation(const vector<const Operation*>::const_iterator&) const {
				return true;
			}

			virtual inline bool canOmitBrackets() const {
				return false;
			}

			virtual inline string getHeader(const StringifyContext&) const {
				return EMPTY_STRING;
			}

			virtual inline string getBackSeparator(const ClassInfo&) const override {
				return bracketsOmitted() ? "\n" : "\n\n";
			}

		public:
			virtual void addOperation(const Operation*, const DecompilationContext&) const;

		protected:
			bool removeOperation(const Operation*) const;

		public:
			void update(const DecompilationContext&) const;

			inline bool isEmpty() const {
				return code.empty() || all_of(code.begin(), code.end(),
						[] (const Operation* operation) { return !operation->canStringify(); });
			}

			virtual const Type* getReturnType() const override;

			virtual bool canAddToCode() const override;

			void reduceVariableTypes() const {
				for(const Variable* variable : variables)
					if(variable != nullptr)
						variable->setTypeShrinking(variable->getType()->getReducedType());

				for(const Scope* scope : innerScopes)
					scope->reduceVariableTypes();
			}


		protected:
			mutable string label = EMPTY_STRING;

			virtual inline string defaultLabelName() const { return "L"; }

		public:
			void makeLabel() const;

			inline string getLabel() const {
				assert(!label.empty());

				return label;
			}


			virtual bool isBreakable() const;

			virtual bool isContinuable() const;

			inline string toDebugString() const {
				return short_typenameof(*this) + " {" + to_string(startIndex) + ".." + to_string(endIndex) + '}';
			}

			inline friend ostream& operator<<(ostream& out, const Scope* scope) {
				return out << (scope != nullptr ? scope->toDebugString() : "null");
			}

			inline friend ostream& operator<<(ostream& out, const Scope& scope) {
				return out << &scope;
			}
	};


	struct MethodScope: Scope {
		MethodScope(index_t, index_t, uint16_t localsCount);

		void removeOperation(const Operation* operation, const DecompilationContext&) const {
			if(!Scope::removeOperation(operation))
				throw DecompilationException("Cannot remove operation of type " + typenameof(*operation) + ": it was not found");
		}
	};


	struct StaticInitializerScope: MethodScope {
		private:
			mutable bool fieldsInitialized = false;

		public:
			StaticInitializerScope(index_t, index_t, uint16_t localsCount);

			virtual void addOperation(const Operation*, const DecompilationContext&) const override;

			inline bool isFieldsInitialized() const {
				return fieldsInitialized;
			}
	};
}

#endif
