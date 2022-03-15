#ifndef JDECOMPILER_CODE_CPP
#define JDECOMPILER_CODE_CPP

#include "types.cpp"

namespace jdecompiler {

	string ClassConstant::toString(const ClassInfo& classinfo) const {
		return (new ClassType(*name))->toString(classinfo) + ".class";
	}


	namespace BuiltinTypes {
		/* Serves as a marker for the struct TypeByBuiltinType */
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
	struct TypeByBuiltinType {
		static_assert(is_one_of<T, bool, int32_t, int64_t, float, double, BuiltinTypes::MarkerStruct>::value, "illegal basic type T");
	};

	template<> struct TypeByBuiltinType<bool>    { static const Type* const value; };
	template<> struct TypeByBuiltinType<int32_t> { static const Type* const value; };
	template<> struct TypeByBuiltinType<int64_t> { static const Type* const value; };
	template<> struct TypeByBuiltinType<float>   { static const Type* const value; };
	template<> struct TypeByBuiltinType<double>  { static const Type* const value; };
	template<> struct TypeByBuiltinType<BuiltinTypes::Object>       { static const Type* const value; };
	template<> struct TypeByBuiltinType<BuiltinTypes::String>       { static const Type* const value; };
	template<> struct TypeByBuiltinType<BuiltinTypes::Class>        { static const Type* const value; };
	template<> struct TypeByBuiltinType<BuiltinTypes::MethodType>   { static const Type* const value; };
	template<> struct TypeByBuiltinType<BuiltinTypes::MethodHandle> { static const Type* const value; };

	const Type* const TypeByBuiltinType<bool>::value =    BOOLEAN;
	const Type* const TypeByBuiltinType<int32_t>::value = ANY_INT_OR_BOOLEAN;
	const Type* const TypeByBuiltinType<int64_t>::value = LONG;
	const Type* const TypeByBuiltinType<float>::value =   FLOAT;
	const Type* const TypeByBuiltinType<double>::value =  DOUBLE;
	const Type* const TypeByBuiltinType<BuiltinTypes::Object>::value =       AnyObjectType::getInstance();
	const Type* const TypeByBuiltinType<BuiltinTypes::String>::value =       STRING;
	const Type* const TypeByBuiltinType<BuiltinTypes::Class>::value =        CLASS;
	const Type* const TypeByBuiltinType<BuiltinTypes::MethodType>::value =   METHOD_TYPE;
	const Type* const TypeByBuiltinType<BuiltinTypes::MethodHandle>::value = METHOD_HANDLE;


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
			static inline const Operation* getArray(const CodeEnvironment& environment, const Type* elementType);

			Operation() noexcept {}

			virtual ~Operation() {}

		public:
			virtual string toString(const CodeEnvironment& environment) const = 0;

			virtual string toArrayInitString(const CodeEnvironment& environment) const {
				return toString(environment);
			}

			virtual const Type* getReturnType() const = 0;

			template<class T>
			const T* getReturnTypeAs(const T* type) const {
				const T* newType = getReturnType()->castTo(type);
				onCastReturnType(newType);
				return newType;
			}

			inline void castReturnTypeTo(const Type* type) const {
				onCastReturnType(getReturnType()->castTo(type));
			}

		protected:
			virtual void onCastReturnType(const Type* newType) const {}


		public:
			template<class O>
			static inline O castOperationTo(const Operation* operation);


			template<class D, class... Ds>
			static bool checkDup(const CodeEnvironment& environment, const Operation* operation);


			template<class D, class... Ds>
			static inline const Type* getDupReturnType(const CodeEnvironment& environment, const Operation* operation, const Type* defaultType = VOID) {
				return checkDup<D, Ds...>(environment, operation) ? operation->getReturnType() : defaultType;
			}


		public:
			virtual Priority getPriority() const {
				return Priority::DEFAULT_PRIORITY;
			}

			inline string toStringPriority(const Operation* operation, const CodeEnvironment& environment, const Associativity associativity) const {
				const Priority thisPriority = this->getPriority(),
				               otherPriority = operation->getPriority();

				if(otherPriority < thisPriority || (otherPriority == thisPriority && getAssociativityByPriority(otherPriority) != associativity))
					return '(' + operation->toString(environment) + ')';
				return operation->toString(environment);
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
			static inline constexpr Associativity getAssociativityByPriority(Priority priority) {
				return priority == Priority::ASSIGNMENT || priority == Priority::TERNARY_OPERATOR || priority == Priority::CAST
						|| priority == Priority::UNARY ? Associativity::RIGHT : Associativity::LEFT;
			}
	};

	struct Instruction {
		public:
			virtual const Operation* toOperation(const CodeEnvironment& environment) const = 0;

		protected:
			Instruction() {}

			virtual ~Instruction() {}
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
			mutable bool declared;

			mutable vector<const Operation*> linkedOperations;

			/*friend struct operations::LoadOperation;
			friend struct operations::StoreOperation;
			friend struct operations::IIncOperation;*/



		public:
			void linkWith(const Operation* operation) const {
				linkedOperations.push_back(operation);
				castTypeTo(operation->getReturnType());
			}

			const Type* setType(const Type* newType) const {
				for(const Operation* operation : linkedOperations)
					newType = operation->getReturnTypeAs(newType);
				return this->type = newType;
			}

			inline const Type* castTypeTo(const Type* requiredType) const {
				return setType(type->castTo(requiredType));
			}

			const Type* getType() const {
				return type;
			}

			bool isDeclared() const {
				return declared;
			}

			bool setDeclared(bool declared) const {
				return this->declared = declared;
			}

			Variable(const Type* type, bool declared): type(type), declared(declared) {}


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
			NamedVariable(const Type* type, bool declared, const string& name): Variable(type, declared), name(name) {}

			virtual string getName() const override {
				return name;
			}

			virtual void addName(const string& name) const override {}
	};


	struct UnnamedVariable: Variable {
		protected:
			mutable vector<string> names;

		public:
			UnnamedVariable(const Type* type, bool declared): Variable(type, declared) {}

			UnnamedVariable(const Type* type, bool declared, const string& name): Variable(type, declared), names({name}) {}

			virtual string getName() const override {
				return names.size() == 1 ? names[0] : getNameByType(type);
			}

			virtual void addName(const string& name) const override {
				names.push_back(name);
			}
	};



	struct Bytecode {
		public:
			const uint32_t length;
			const uint8_t* const bytes;

		private:
			uint32_t pos = 0;
			vector<Instruction*> instructions;
			vector<uint32_t> posMap;

		public:
			Bytecode(const uint32_t length, const uint8_t* bytes): length(length), bytes(bytes) {
				instructions.reserve(length);
			}

			inline const vector<Instruction*>& getInstructions() const {
				return instructions;
			}

			const Instruction* getInstructionNoexcept(uint32_t index) const {
				if(index >= instructions.size())
					return nullptr;
				return instructions[index];
			}

			inline const vector<uint32_t>& getPosMap() const {
				return posMap;
			}

			inline int8_t nextByte() {
				return (int8_t)bytes[++pos];
			}

			inline uint8_t nextUByte() {
				return (uint8_t)bytes[++pos];
			}

			inline int16_t nextShort() {
				return (int16_t)(nextUByte() << 8 | nextUByte());
			}

			inline uint16_t nextUShort() {
				return (uint16_t)(nextUByte() << 8 | nextUByte());
			}

			inline int32_t nextInt() {
				return (int32_t)(nextUByte() << 24 | nextUByte() << 16 | nextUByte() << 8 | nextUByte());
			}

			inline uint32_t nextUInt() {
				return (uint32_t)(nextUByte() << 24 | nextUByte() << 16 | nextUByte() << 8 | nextUByte());
			}

			inline uint8_t current() const {
				return bytes[pos];
			}

			inline uint32_t getPos() const {
				return pos;
			}

			inline bool available() const {
				return length - pos > 0;
			}

			inline void skip(int32_t count) {
				pos += (uint32_t)count;
			}

			const Instruction* getInstruction(uint32_t index) const {
				if(index >= instructions.size())
					throw IndexOutOfBoundsException(index, instructions.size());
				return instructions[index];
			}

			uint32_t posToIndex(uint32_t pos) const {
				uint32_t i = 0, index = 0;
				const uint32_t max = posMap.size();
				while(index != pos && i < max)
					index = posMap[++i];
				if(index != pos)
					throw BytecodeIndexOutOfBoundsException(pos, length);
				return i;
			}

			inline uint32_t indexToPos(uint32_t index) const {
				return posMap[index];
			}

			inline Instruction* nextInstruction() {
				posMap.push_back(pos);

				Instruction* instruction = nextInstruction0();
				if(instruction != nullptr) {
					instructions.push_back(instruction);
				}
				return instruction;
			}

			private: Instruction* nextInstruction0();
	};

	template<typename T>
	struct Stack {
		private:
			class Entry {
				public:
					const T value;
					const Entry* const next;

					Entry(T value, const Entry* next): value(value), next(next) {}

					void deleteNext() const {
						if(next != nullptr) {
							next->deleteNext();
							delete next;
						}
					}
			};

			const Entry* firstEntry;
			uint16_t length;

		protected:
			inline void checkEmptyStack() const {
				if(firstEntry == nullptr)
					throw EmptyStackException();
			}

		public:
			Stack(): firstEntry(nullptr), length(0) {}

			Stack(T value): firstEntry(new Entry(value, nullptr)), length(1) {}

			void push(T value) {
				firstEntry = new Entry(value, firstEntry);
				length++;
			}

			inline void push(T value, T operations...) {
				push(value);
				push(operations);
			}

			T pop() {
				checkEmptyStack();

				const Entry copiedEntry = *firstEntry;
				delete firstEntry;
				firstEntry = copiedEntry.next;
				length--;
				return copiedEntry.value;
			}

			T top() const {
				checkEmptyStack();
				return firstEntry->value;
			}

			T lookup(uint16_t index) const {
				checkEmptyStack();

				if(index >= length)
					throw StackIndexOutOfBoundsException(index, length);

				const Entry* currentEntry = firstEntry;
				for(uint16_t i = 0; i < index; i++)
					currentEntry = currentEntry->next;
				return currentEntry->value;
			}

			inline uint16_t size() const {
				return length;
			}

			inline bool empty() const {
				return length == 0;
			}

			~Stack() {
				if(firstEntry != nullptr) {
					firstEntry->deleteNext();
					delete firstEntry;
				}
			}
	};


	struct CodeStack: Stack<const Operation*> {
		const Operation* popAs(const Type* type) {
			checkEmptyStack();

			const Operation* operation = this->pop();
			operation->castReturnTypeTo(type);
			return operation;
		}

		inline const Operation* pop() {
			return Stack<const Operation*>::pop();
		}

		template<class O>
		O pop() {
			checkEmptyStack();

			O operation = Stack<const Operation*>::pop();
			if(const O* t = dynamic_cast<const O*>(operation))
				return t;
			throw DecompilationException((string)"Illegal operation type " + typeid(O).name() + " for operation " + typeid(*operation).name());
		}
	};


	struct CodeEnvironment {
		public:
			const Bytecode& bytecode;
			const ClassInfo& classinfo;
			const ConstantPool& constPool;
			CodeStack& stack;
			MethodScope& methodScope;

		private:
			Stack<const Scope*> currentScopes;

		public:
			const uint16_t modifiers;
			const MethodDescriptor& descriptor;
			const Attributes& attributes;
			uint32_t pos = 0, index = 0, exprStartIndex = 0;
			map<uint32_t, uint32_t> exprIndexTable;

		private:
			mutable vector<const Scope*> scopes;

		public:
			CodeEnvironment(const Bytecode& bytecode, const ClassInfo& classinfo, MethodScope* methodScope, uint16_t modifiers,
					const MethodDescriptor& descriptor, const Attributes& attributes, uint16_t maxLocals);

			CodeEnvironment(const CodeEnvironment&) = delete;

			CodeEnvironment& operator=(const CodeEnvironment&) = delete;


			void checkCurrentScope();

			inline const Scope* getCurrentScope() const {
				return currentScopes.top();
			}

			inline void addScope(const Scope* scope) const {
				scopes.push_back(scope);
			}

			~CodeEnvironment() {
				delete &stack;
			}

		protected:
			template<typename Arg, typename... Args>
			inline void print(ostream& out, Arg arg, Args... args) const {
				if constexpr(sizeof...(Args) != 0)
					print(out << arg, args...);
				else
					out << arg << endl;
			}

		public:
			template<typename... Args>
			inline void warning(Args... args) const {
				print(cerr << descriptor.toString() << ':' << pos << ": warning: ", args...);
			}
	};


	const Operation* Operation::getArray(const CodeEnvironment& environment, const Type* elementType) {
		return environment.stack.popAs(new ArrayType(elementType));
	}


	template<class D, class... Ds>
	bool Operation::checkDup(const CodeEnvironment& environment, const Operation* operation) {
		if(instanceof<const D*>(operation)) {
			if(static_cast<const D*>(operation)->operation != environment.stack.pop())
				throw DecompilationException("Illegal stack state after dup operation");
			return true;
		}

		if constexpr(sizeof...(Ds) == 0)
			return false;
		else
			return checkDup<Ds...>(environment, operation);
	}


	struct Scope: Operation {
		protected:
			mutable uint32_t startPos, endPos;

		public:
			const Scope *const parentScope;

		protected:
			const uint16_t localsCount;

			mutable vector<Variable*> variables;
			mutable uint16_t lastAddedVarIndex = 0;

			friend struct operations::IfScope;

			mutable vector<const Operation*> code;
			mutable map<const Variable*, string> varNames;
			mutable vector<const Scope*> innerScopes;

		public:

			Scope(uint32_t startPos, uint32_t endPos, const Scope* parentScope, uint16_t localsCount):
					startPos(startPos), endPos(endPos), parentScope(parentScope), localsCount(localsCount), variables(localsCount) {}

			Scope(uint32_t startPos, uint32_t endPos, const Scope* parentScope): Scope(startPos, endPos, parentScope, parentScope->localsCount) {}


			inline uint32_t start() const {
				return startPos;
			}

			inline uint32_t end() const {
				return endPos;
			}

		protected:
			const Variable* findVariable(uint32_t index) const {
				const Variable* var = variables[index];
				return var == nullptr && parentScope != nullptr ? parentScope->findVariable(index) : var;
			}

			const Variable* findDeclaredVariable(uint32_t index) const {
				const Variable* var = findVariable(index);
				if(var == nullptr) {
					for(const Scope* scope : innerScopes) {
						var = scope->findDeclaredVariable(index);
						if(var != nullptr)
							return var;
						LOG(typeid(*scope).name() << ' ' << scope->start() << ".." << scope->end() << ' ' << var);
						LOG(join<Variable*>(scope->variables, [] (Variable* v) { return to_string((uint64_t)v); }));
					}
				}
				return var;
			}

		public:
			virtual const Variable& getVariable(uint32_t index, bool isDeclared) const {
				if(index >= variables.size()) {
					if(parentScope == nullptr)
						throw IndexOutOfBoundsException(index, variables.size());
					return parentScope->getVariable(index, isDeclared);
				}

				const Variable* var = findVariable(index);
				if(var == nullptr) {
					if(isDeclared) {
						var = findDeclaredVariable(index);
						if(var == nullptr)
							throw DecompilationException("Variable #" + to_string(index) + " not found");
					} else {
						var = variables[index] = new UnnamedVariable(AnyType::getInstance(), false);
						lastAddedVarIndex = localsCount;
					}
				}
				return *var;
			}

			bool hasVariable(const string& name) const {
				for(Variable* var : variables)
					if(name == var->getName())
						return true;
				return parentScope == nullptr ? false : parentScope->hasVariable(name);
			}

			void addVariable(Variable* var) {
				if(lastAddedVarIndex >= localsCount)
					throw IllegalStateException("cannot add variable: no free place");

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

			//virtual void initiate(const CodeEnvironment&) {}

			virtual void finalize(const CodeEnvironment&) const {}

			virtual string toString(const CodeEnvironment& environment) const override {
				environment.classinfo.increaseIndent();

				string str = getHeader(environment) + "{\n";
				const size_t baseSize = str.size();

				for(auto i = code.begin(); i != code.end(); ++i) {
					if(canPrintNextOperation(i)) {
						const Operation* operation = *i;
						if(operation->getReturnType() == VOID)
							str += operation->getFrontSeparator(environment.classinfo) + operation->toString(environment) +
									operation->getBackSeparator(environment.classinfo);
					}
				}

				environment.classinfo.reduceIndent();

				if(str.size() == baseSize) {
					str[baseSize - 1] = '}';
					return str;
				}

				return str + environment.classinfo.getIndent() + '}';
			}

		protected:
			virtual inline bool canPrintNextOperation(const vector<const Operation*>::const_iterator& i) const { return true; }

			virtual string getHeader(const CodeEnvironment& environment) const {
				return EMPTY_STRING;
			}

			virtual inline string getBackSeparator(const ClassInfo& classinfo) const override {
				return "\n";
			}

		public:
			uint32_t getVariablesCount() const {
				return variables.size();
			}

			virtual void add(const Operation* operation, const CodeEnvironment& environment) const {
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
	};


	struct MethodScope: Scope {
		public:
			MethodScope(uint32_t startPos, uint32_t endPos, uint16_t localsCount): Scope(startPos, endPos, nullptr, localsCount) {
				variables.reserve(localsCount);
			}
	};


	struct StaticInitializerScope: MethodScope {
		private:
			mutable bool fieldsInitialized = false;

		public:
			StaticInitializerScope(uint32_t startPos, uint32_t endPos, uint16_t localsCount):
					MethodScope(startPos, endPos, localsCount) {}

			virtual void add(const Operation* operation, const CodeEnvironment& environment) const override;

			inline bool isFieldsInitialized() const {
				return fieldsInitialized;
			}
	};

}

#endif
