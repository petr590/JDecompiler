#ifndef JDECOMPILER_CODE_CPP
#define JDECOMPILER_CODE_CPP

#include "types.cpp"

namespace jdecompiler {

	string ClassConstant::toString(const ClassInfo& classinfo) const {
		return parseReferenceType(name)->toString(classinfo) + ".class";
	}


	namespace BuiltinTypes {
		/* Serves as a marker for the struct typeByBuiltinType */
		struct MarkerStruct {
			MarkerStruct() = delete;
			MarkerStruct(const MarkerStruct&) = delete;
			MarkerStruct& operator= (const MarkerStruct&) = delete;
		};

		struct Object: MarkerStruct {};
		struct String: MarkerStruct {};
		struct Class: MarkerStruct {};
		struct MethodType: MarkerStruct {};
		struct MethodHandle: MarkerStruct {};
	};

	template<typename T>
	static const Type* typeByBuiltinType();

	template<> inline const Type* typeByBuiltinType<bool>() { return BOOLEAN; }
	template<> inline const Type* typeByBuiltinType<int32_t>() { return ANY_INT_OR_BOOLEAN; }
	template<> inline const Type* typeByBuiltinType<int64_t>() { return LONG; }
	template<> inline const Type* typeByBuiltinType<float>() { return FLOAT; }
	template<> inline const Type* typeByBuiltinType<double>() { return DOUBLE; }
	template<> inline const Type* typeByBuiltinType<BuiltinTypes::Object>() { return AnyObjectType::getInstance(); }
	template<> inline const Type* typeByBuiltinType<BuiltinTypes::String>() { return STRING; }
	template<> inline const Type* typeByBuiltinType<BuiltinTypes::Class>() { return CLASS; }
	template<> inline const Type* typeByBuiltinType<BuiltinTypes::MethodType>() { return METHOD_TYPE; }
	template<> inline const Type* typeByBuiltinType<BuiltinTypes::MethodHandle>() { return METHOD_HANDLE; }
	template<> inline const Type* typeByBuiltinType<string>() { return STRING; }

	template<typename T>
	static const Type* exactTypeByBuiltinType() {
		return typeByBuiltinType<T>();
	}

	template<>
	inline const Type* exactTypeByBuiltinType<int32_t>() { return INT; }


	enum class Associativity { LEFT, RIGHT };


	enum class Priority {
		DEFAULT_PRIORITY         = 16,
		POST_INCREMENT           = 15,
		UNARY                    = 14, PRE_INCREMENT = UNARY, BIT_NOT = UNARY, LOGICAL_NOT = UNARY, UNARY_PLUS = UNARY, UNARY_MINUS = UNARY,
		CAST                     = 13,
		MUL_DIV_REM              = 12, MULTIPLE = MUL_DIV_REM, DIVISION = MUL_DIV_REM, REMAINDER = MUL_DIV_REM,
		PLUS_MINUS               = 11, PLUS = PLUS_MINUS, MINUS = PLUS_MINUS,
		SHIFT                    = 10,
		GREATER_LESS_COMPARASION = 9, INSTANCEOF = GREATER_LESS_COMPARASION,
		EQUALS_COMPARASION       = 8,
		BIT_AND                  = 7,
		BIT_XOR                  = 6,
		BIT_OR                   = 5,
		LOGICAL_AND              = 4,
		LOGICAL_OR               = 3,
		TERNARY_OPERATOR         = 2,
		ASSIGNMENT               = 1,
		LAMBDA                   = 0,
	};


	struct Operation {
		protected:
			Operation() noexcept {}

			virtual ~Operation() {}

		public:
			virtual string toString(const StringifyContext& context) const = 0;

			virtual string toArrayInitString(const StringifyContext& context) const {
				return toString(context);
			}

			virtual const Type* getReturnType() const = 0;

			template<class T>
			const T* getReturnTypeAs(const T* type) const {
				const T* newType = getReturnType()->castTo(type);
				onCastReturnType(newType);
				return newType;
			}

			template<class T>
			const T* getReturnTypeAsWidest(const T* type) const {
				const T* newType = getReturnType()->castToWidest(type);
				onCastReturnType(newType);
				return newType;
			}

			inline void castReturnTypeTo(const Type* type) const {
				onCastReturnType(getReturnType()->castTo(type));
			}

			inline void twoWayCastReturnTypeTo(const Type* type) const {
				onCastReturnType(getReturnType()->twoWayCastTo(type));
			}

		protected:
			virtual void onCastReturnType(const Type* newType) const {}


		public:
			virtual const Operation* getOriginalOperation() const {
				return this;
			}


			template<class D, class... Ds>
			static bool checkDup(const DecompilationContext& context, const Operation* operation);


			template<class D, class... Ds>
			static inline const Type* getDupReturnType(const DecompilationContext& context, const Operation* operation, const Type* defaultType = VOID) {
				return checkDup<D, Ds...>(context, operation) ? operation->getReturnType() : defaultType;
			}


		public:
			virtual Priority getPriority() const {
				return Priority::DEFAULT_PRIORITY;
			}

			inline string toStringPriority(const Operation* operation, const StringifyContext& context, const Associativity associativity) const {
				const Priority thisPriority = this->getPriority(),
				               otherPriority = operation->getPriority();

				if(otherPriority < thisPriority || (otherPriority == thisPriority && getAssociativityByPriority(otherPriority) != associativity))
					return '(' + operation->toString(context) + ')';
				return operation->toString(context);
			}

			virtual inline string getFrontSeparator(const ClassInfo& classinfo) const {
				return classinfo.getIndent();
			}

			virtual inline string getBackSeparator(const ClassInfo& classinfo) const {
				return ";\n";
			}


			virtual bool canAddToCode() const {
				return true;
			}

		private:
			mutable bool removed = false;

		public:
			inline void remove() const {
				removed = true;
			}

			inline bool isRemoved() const {
				return removed;
			}

			virtual void addVariableName(const string& name) const {}

		private:
			static inline constexpr Associativity getAssociativityByPriority(Priority priority) {
				return priority == Priority::ASSIGNMENT || priority == Priority::TERNARY_OPERATOR || priority == Priority::CAST
						|| priority == Priority::UNARY ? Associativity::RIGHT : Associativity::LEFT;
			}
	};



	struct Variable {
		protected:
			static string getRawNameByType(const Type* type, bool* unchecked) {
				if(const ClassType* classType = dynamic_cast<const ClassType*>(type)) {
					if(classType->simpleName == "Boolean") return "bool";
					if(classType->simpleName == "Byte") return "b";
					if(classType->simpleName == "Character") return "ch";
					if(classType->simpleName == "Short") return "sh";
					if(classType->simpleName == "Integer") return "n";
					if(classType->simpleName == "Long") return "l";
					if(classType->simpleName == "Float") return "f";
					if(classType->simpleName == "Double") return "d";
				}
				*unchecked = true;
				return type->getVarName();
			}

			static string getNameByType(const Type* type) {
				bool unchecked = false;

				const string name = getRawNameByType(type, &unchecked);
				if(unchecked) {
					static const map<const char*, const char*> keywords {
						{"boolean", "bool"}, {"byte", "b"}, {"char", "ch"}, {"short", "sh"}, {"int", "n"}, {"long", "l"},
						{"float", "f"}, {"double", "d"}, {"void", "v"},
						{"public", "pub"}, {"protected", "prot"}, {"private", "priv"}, {"static", "stat"}, {"final", "f"}, {"abstract", "abs"},
						{"transient", "trans"}, {"volatile", "vol"}, {"native", "nat"}, {"synchronized", "sync"},
						{"class", "clazz"}, {"interface", "interf"}, {"enum", "en"}, {"this", "t"}, {"super", "sup"}, {"extends", "ext"}, {"implements", "impl"},
						{"import", "imp"}, {"package", "pack"}, {"instanceof", "inst"}, {"new", "n"},
						{"if", "cond"}, {"else", "el"}, {"while", "whl"}, {"do", "d"}, {"for", "f"}, {"switch", "sw"}, {"case", "cs"}, {"default", "def"},
						{"break", "brk"}, {"continue", "cont"}, {"return", "ret"},
						{"try", "tr"}, {"catch", "c"}, {"finally", "f"}, {"throw", "thr"}, {"throws", "thrs"}, {"assert", "assrt"},
						{"true", "tr"}, {"false", "fls"}, {"null", "nll"},
						{"strictfp", "strict"}, {"const", "cnst"}, {"goto", "gt"}
					};

					for(const auto& keyword : keywords)
						if(name == keyword.first)
							return keyword.second;
				}

				return name;
			}


		protected:
			mutable const Type* type;
			const bool isFixedType;

			mutable bool declared;

			mutable vector<const Operation*> bindedOperations;

			/*friend struct operations::LoadOperation;
			friend struct operations::StoreOperation;
			friend struct operations::IIncOperation;*/

			mutable bool typeSettingLocked = false;



		public:

			const Type* getType() const {
				return type;
			}

			const Type* setType(const Type* newType) const {

				if(isFixedType) {
					if(type->isSubtypeOf(newType))
						return this->type;
					throw IncopatibleTypesException(type, newType);
				}

				if(typeSettingLocked) // To avoid infinite recursion
					return this->type;

				typeSettingLocked = true;

				for(const Operation* operation : bindedOperations)
					newType = operation->getReturnTypeAsWidest(newType);

				typeSettingLocked = false;

				return this->type = (instanceof<const PrimitiveType*>(newType) ? static_cast<const PrimitiveType*>(newType)->toAmbigousType() : newType);
			}

			inline const Type* castTypeTo(const Type* requiredType) const {
				return setType(type->castTo(requiredType));
			}

			void bindTo(const Operation* operation) const {
				bindedOperations.push_back(operation);
				setType(operation->getReturnTypeAsWidest(type));
			}


			bool isDeclared() const {
				return declared;
			}

			void setDeclared(bool declared) const {
				this->declared = declared;
			}

			Variable(const Type* type, bool declared, bool isFixedType): type(type), declared(declared), isFixedType(isFixedType) {}


			virtual string getName() const = 0;

			virtual void addName(const string&) const = 0;

			inline bool operator==(const Variable& other) const {
				return this == &other;
			}


			virtual ~Variable() {}
	};


	struct NamedVariable: Variable {
		protected:
			const string name;

		public:
			NamedVariable(const Type* type, bool declared, const string& name, bool isFixedType = false):
					Variable(type, declared, isFixedType), name(name) {}

			virtual string getName() const override {
				return name;
			}

			virtual void addName(const string& name) const override {}
	};


	struct UnnamedVariable: Variable {
		protected:
			mutable vector<string> names;

		public:
			UnnamedVariable(const Type* type, bool declared, bool isFixedType = false):
					Variable(type, declared, isFixedType) {}

			UnnamedVariable(const Type* type, bool declared, const string& name, bool isFixedType = false):
					Variable(type, declared, isFixedType), names({name}) {}

			virtual string getName() const override {
				return names.size() == 1 ? names[0] : getNameByType(type);
			}

			virtual void addName(const string& name) const override {
				names.push_back(name);
			}
	};





	struct Instruction {
		public:
			virtual const Operation* toOperation(const DecompilationContext&) const = 0;

		protected:
			Instruction() {}

			virtual ~Instruction() {}

		public:
			virtual const Block* toBlock(const DisassemblerContext&) const {
				return nullptr;
			}

			//virtual int32_t getStackChange() const = 0;
	};



	struct Block {
		public:
			mutable index_t startIndex, endIndex;
			const Block* parentBlock;

		protected:
			mutable vector<const Block*> innerBlocks;

		public:
			Block(index_t startIndex, index_t endIndex, const Block* parentBlock) noexcept:
					startIndex(startIndex), endIndex(endIndex), parentBlock(parentBlock) {}

			Block(index_t startIndex, index_t endIndex, const DisassemblerContext& context) noexcept;

			Block(index_t startIndex, index_t endIndex) noexcept:
					Block(startIndex, endIndex, nullptr) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const = 0;

			inline index_t start() const {
				return startIndex;
			}

			inline index_t end() const {
				return endIndex;
			}

			inline void addInnerBlock(const Block* block) const {
				innerBlocks.push_back(block);
			}

			string toDebugString() const {
				return typeNameOf(*this) + " {" + to_string(startIndex) + ".." + to_string(endIndex) + '}';
			}
	};

	struct RootBlock: Block {
		RootBlock(index_t endIndex) noexcept: Block(0, endIndex) {}

		virtual const Operation* toOperation(const DecompilationContext&) const override {
			return nullptr;
		}
	};
}

#include "disassembler-context.cpp"

namespace jdecompiler {

	Block::Block(index_t startIndex, index_t endIndex, const DisassemblerContext& context) noexcept:
			Block(startIndex, endIndex, context.getCurrentBlock()) {}


	struct CodeStack: stack<const Operation*> {
		const Operation* popAs(const Type* type) {
			const Operation* operation = this->pop();
			operation->castReturnTypeTo(type);
			return operation;
		}

		CodeStack() {}

		inline const Operation* pop() {
			return stack<const Operation*>::pop();
		}

		template<class O>
		O pop() {
			checkEmptyStack();

			O operation = stack<const Operation*>::pop();
			if(const O* t = dynamic_cast<const O*>(operation))
				return t;
			throw DecompilationException("Illegal operation type " + typeNameOf<O>() + " for operation " + typeNameOf(*operation));
		}

		CodeStack(const CodeStack&) = delete;
		CodeStack& operator=(const CodeStack&) = delete;
	};
}

#include "decompilation-context.cpp"
#include "stringify-context.cpp"

namespace jdecompiler {

	template<class D, class... Ds>
	bool Operation::checkDup(const DecompilationContext& context, const Operation* operation) {
		if(instanceof<const D*>(operation)) {
			if(static_cast<const D*>(operation)->operation != context.stack.pop())
				throw DecompilationException("Illegal stack state after dup operation");
			return true;
		}

		if constexpr(sizeof...(Ds) == 0)
			return false;
		else
			return checkDup<Ds...>(context, operation);
	}


	struct Scope: Operation {
		public:
			const index_t startIndex, endIndex;

			const Scope *const parentScope;

		protected:
			mutable vector<Variable*> variables;
			mutable uint16_t lastAddedVarIndex = 0;

			friend struct operations::IfScope;

			mutable vector<const Operation*> code;
			mutable map<const Variable*, string> varNames;
			mutable vector<const Scope*> innerScopes;

		private:
			Scope(index_t startIndex, index_t endIndex, const Scope* parentScope, uint16_t variablesCount):
					startIndex(startIndex), endIndex(endIndex), parentScope(parentScope), variables(variablesCount) {}

		public:

			Scope(index_t startIndex, index_t endIndex, uint16_t variablesCount):
					Scope(startIndex, endIndex, nullptr, variablesCount) {}

			Scope(index_t startIndex, index_t endIndex, const Scope* parentScope):
					Scope(startIndex, endIndex, parentScope, parentScope->variables.size()) {}

			Scope(index_t startIndex, index_t endIndex, const DecompilationContext& context):
					Scope(startIndex, endIndex, context.getCurrentScope()) {}


			inline index_t start() const {
				return startIndex;
			}

			inline index_t end() const {
				return endIndex;
			}

			inline vector<const Operation*>& getCode() const {
				return code;
			}

		protected:
			const Variable* findVariable(index_t index) const {
				const Variable* var = variables[index];
				return var == nullptr && parentScope != nullptr ? parentScope->findVariable(index) : var;
			}

			const Variable* findVariableAtInnerScopes(index_t index) const {
				const Variable* var = variables[index];

				if(var == nullptr) {
					for(const Scope* scope : innerScopes) {
						var = scope->findVariableAtInnerScopes(index);
						if(var != nullptr)
							return var;
					}
				} else {
					variables[index] = nullptr;
				}

				return var;
			}

		public:
			virtual const Variable& getVariable(index_t index, bool isDeclared) const {
				if(index >= variables.size()) {
					if(parentScope == nullptr)
						throw IndexOutOfBoundsException(index, variables.size());
					return parentScope->getVariable(index, isDeclared);
				}

				const Variable* var = findVariable(index);
				if(var == nullptr) {
					if(isDeclared) {
						for(const Scope* scope : innerScopes) {
							var = scope->findVariableAtInnerScopes(index);
							if(var != nullptr)
								return *var;
						}

						throw DecompilationException("Variable #" + to_string(index) + " is not found");

					} else {
						var = variables[index] = new UnnamedVariable(AnyType::getInstance(), false);
						lastAddedVarIndex = variables.size();
					}
				}
				return *var;
			}

			bool hasVariable(const string& name) const {
				for(Variable* var : variables)
					if(var != nullptr && name == var->getName())
						return true;
				return parentScope == nullptr ? false : parentScope->hasVariable(name);
			}

			void addVariable(Variable* var) {
				if(lastAddedVarIndex >= variables.size())
					throw IllegalStateException("cannot add variable to " + this->toDebugString() + ": no free place");

				variables[lastAddedVarIndex] = var;
				switch(var->getType()->getSize()) {
					case TypeSize::FOUR_BYTES: lastAddedVarIndex += 1; break;
					case TypeSize::EIGHT_BYTES: lastAddedVarIndex += 2; break;
					default: throw IllegalStateException((string)"illegal variable type size " + TypeSize_nameOf(var->getType()->getSize()));
				}
			}

			inline string getNameFor(const Variable& var) const {
				return getNameFor(&var);
			}

			string getNameFor(const Variable* var) const {
				if(!has(variables, var)) {
					return parentScope != nullptr ? parentScope->getNameFor(var) :
							throw IllegalStateException("Variable of type " + var->getType()->toString() + " is not found");
				}

				const auto varName = varNames.find(var);
				if(varName != varNames.end()) {
					return varName->second;
				}

				const string baseName = var->getName();
				string name = baseName;
				uint_fast16_t i = 1;

				while(find_if(varNames.begin(), varNames.end(), [&name] (const auto& it) { return it.second == name; }) != varNames.end()) {
					name = baseName + to_string(++i);
				}

				varNames[var] = name;

				return name;
			}

			inline const vector<Variable*>& getVariables() const {
				return variables;
			}

			inline uint32_t getVariablesCount() const {
				return variables.size();
			}

			//virtual void initiate(const DecompilationContext&) {}

			virtual void finalize(const DecompilationContext&) const {}

			virtual string toString(const StringifyContext& context) const override final {

				context.enterScope(this);
				const string str = toStringImpl(context);
				context.exitScope(this);
				return str;
			}

			virtual string toStringImpl(const StringifyContext& context) const {
				context.classinfo.increaseIndent();

				string str = (label.empty() ? EMPTY_STRING : label + ": ") + getHeader(context) + "{\n";
				const size_t baseSize = str.size();

				for(auto i = code.begin(); i != code.end(); ++i) {
					const Operation* operation = *i;

					if(!operation->isRemoved() && canPrintNextOperation(i)) {
						if(operation->getReturnType() == VOID)
							str += operation->getFrontSeparator(context.classinfo) + operation->toString(context) +
									operation->getBackSeparator(context.classinfo);
					}
				}

				context.classinfo.reduceIndent();

				if(str.size() == baseSize) {
					str[baseSize - 1] = '}';
					return str;
				}

				return str + context.classinfo.getIndent() + '}';
			}

		protected:
			virtual inline bool canPrintNextOperation(const vector<const Operation*>::const_iterator& i) const { return true; }

			virtual string getHeader(const StringifyContext& context) const {
				return EMPTY_STRING;
			}

			virtual inline string getBackSeparator(const ClassInfo& classinfo) const override {
				return "\n";
			}

		public:
			virtual void addOperation(const Operation* operation, const StringifyContext& context) const {
				code.push_back(operation);
				if(instanceof<const Scope*>(operation))
					innerScopes.push_back(static_cast<const Scope*>(operation));
			}

			inline bool isEmpty() const {
				return code.empty();
			}

			virtual const Type* getReturnType() const override {
				return VOID;
			}

			virtual bool canAddToCode() const override {
				return false;
			}


		protected:
			mutable string label = EMPTY_STRING;

			virtual string getLabelName() const { return "L"; }

		public:
			void makeLabel() const {
				assert(parentScope != nullptr);

				if(label.empty())
					label = getLabelName(); // TODO
			}

			string getLabel() const {
				assert(!label.empty());

				return label;
			}


			virtual bool isBreakable() const {
				return false;
			}

			virtual bool isContinuable() const {
				return false;
			}

			string toDebugString() const {
				return typeNameOf(*this) + " {" + to_string(startIndex) + ".." + to_string(endIndex) + '}';
			}

			friend ostream& operator<< (ostream& out, const Scope& scope) {
				return out << (&scope != nullptr ? scope.toDebugString() : "null");
			}
	};


	struct MethodScope: Scope {
		public:
			MethodScope(index_t startIndex, index_t endIndex, uint16_t localsCount): Scope(startIndex, endIndex, localsCount) {
				variables.reserve(localsCount);
			}
	};


	struct StaticInitializerScope: MethodScope {
		private:
			mutable bool fieldsInitialized = false;

		public:
			StaticInitializerScope(index_t startIndex, index_t endIndex, uint16_t localsCount):
					MethodScope(startIndex, endIndex, localsCount) {}

			virtual void addOperation(const Operation* operation, const StringifyContext& context) const override;

			inline bool isFieldsInitialized() const {
				return fieldsInitialized;
			}
	};
}

#endif
