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





	struct Instruction {
		public:
			virtual const Operation* toOperation(const CodeEnvironment&) const = 0;

		protected:
			Instruction() {}

			virtual ~Instruction() {}

		public:
			virtual const Block* toBlock(const Bytecode&) const {
				return nullptr;
			}
	};



	struct Block {
		public:
			mutable index_t startIndex, endIndex;
			const Block* parentBlock;

		protected:
			mutable vector<const Block*> innerBlocks;

		public:
			Block(index_t startIndex, index_t endIndex, const Bytecode& bytecode) noexcept;

			Block(index_t startIndex, index_t endIndex) noexcept:
					startIndex(startIndex), endIndex(endIndex), parentBlock(nullptr) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const = 0;

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

		virtual const Operation* toOperation(const CodeEnvironment&) const override {
			return nullptr;
		}
	};



	struct Bytecode {
		public:
			const uint32_t length;
			const uint8_t* const bytes;

		private:
			pos_t pos = 0;
			index_t index = 0;
			vector<Instruction*> instructions;
			mutable vector<const Block*> blocks, inactiveBlocks;
			const Block* currentBlock = nullptr;
			map<pos_t, index_t> indexMap;
			map<index_t, pos_t> posMap;

		public:
			inline const Block* getCurrentBlock() const {
				return currentBlock;
			}

			Bytecode(const uint32_t length, const uint8_t* bytes): length(length), bytes(bytes) {

				while(available()) {
					indexMap[pos] = index;
					posMap[index] = pos;
					index++;

					instructions.push_back(nextInstruction());

					next();
				}

				index_t lastIndex = index;
				assert(lastIndex == instructions.size());

				currentBlock = new RootBlock(lastIndex);

				index = 0;

				for(const Instruction* instruction = instructions[0]; index < lastIndex; instruction = instructions[++index]) {
					pos = posMap[index];

					if(instruction != nullptr) {
						const Block* block = instruction->toBlock(*this);
						if(block != nullptr)
							addBlock(block);
					}

					updateBlocks();
				}
			}

			inline const vector<Instruction*>& getInstructions() const {
				return instructions;
			}

			inline const vector<const Block*>& getBlocks() const {
				return blocks;
			}

			const Instruction* getInstruction(index_t index) const {
				if(index >= instructions.size())
					throw IndexOutOfBoundsException(index, instructions.size());
				return instructions[index];
			}

			const Instruction* getInstructionNoexcept(index_t index) const noexcept {
				if(index >= instructions.size())
					return nullptr;
				return instructions[index];
			}

			inline void addBlock(const Block* block) const {
				blocks.push_back(block);
				inactiveBlocks.push_back(block);
			}

		protected:
			void updateBlocks() {

				while(index >= currentBlock->end()) {
					if(currentBlock->parentBlock == nullptr)
						throw DecompilationException("Unexpected end of global function block " + currentBlock->toDebugString());
					LOG("End of " << currentBlock->toDebugString());
					currentBlock = currentBlock->parentBlock;
				}

				for(auto i = inactiveBlocks.begin(); i != inactiveBlocks.end(); ) {
					const Block* block = *i;
					if(block->start() <= index) {
						if(block->end() > currentBlock->end()) {
							throw DecompilationException("Block " + block->toDebugString() +
									" is out of bounds of the parent block " + currentBlock->toDebugString());
						}

						LOG("Start of " << block->toDebugString());

						currentBlock->addInnerBlock(block);
						currentBlock = block;
						inactiveBlocks.erase(i);
					} else
						++i;
				}
			}

			inline uint8_t next() {
				return (uint8_t)bytes[++pos];
			}


			inline int8_t nextByte() {
				return (int8_t)next();
			}

			inline uint8_t nextUByte() {
				return (uint8_t)next();
			}

			inline int16_t nextShort() {
				return (int16_t)(next() << 8 | next());
			}

			inline uint16_t nextUShort() {
				return (uint16_t)(next() << 8 | next());
			}

			inline int32_t nextInt() {
				return (int32_t)(next() << 24 | next() << 16 | next() << 8 | next());
			}

			inline uint32_t nextUInt() {
				return (uint32_t)(next() << 24 | next() << 16 | next() << 8 | next());
			}

			inline uint8_t current() const {
				return bytes[pos];
			}

			inline bool available() const {
				return length - pos > 0;
			}

			Instruction* nextInstruction();

		public:
			inline pos_t getPos() const {
				return pos;
			}

			inline index_t getIndex() const {
				return index;
			}

			inline void skip(int32_t count) {
				pos += (uint32_t)count;
			}

			index_t posToIndex(pos_t pos) const {
				const auto foundPos = indexMap.find(pos);
				if(foundPos != indexMap.end())
					return foundPos->second;

				throw BytecodePosOutOfBoundsException(pos, length);
			}

			pos_t indexToPos(index_t index) const {
				const auto foundIndex = posMap.find(index);
				if(foundIndex != posMap.end())
					return foundIndex->second;

				throw BytecodeIndexOutOfBoundsException(index, length);
			}
	};

	Block::Block(index_t startIndex, index_t endIndex, const Bytecode& bytecode) noexcept:
			startIndex(startIndex), endIndex(endIndex), parentBlock(bytecode.getCurrentBlock()) {}


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
			throw DecompilationException("Illegal operation type " + typeNameOf<O>() + " for operation " + typeNameOf(*operation));
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
			const Scope* currentScope;

		public:
			const uint16_t modifiers;
			const MethodDescriptor& descriptor;
			const Attributes& attributes;
			pos_t pos = 0;
			index_t index = 0, exprStartIndex = 0;
			map<uint32_t, index_t> exprIndexTable;

		private:
			mutable vector<const Scope*> inactiveScopes;

		public:
			CodeEnvironment(const Bytecode& bytecode, const ClassInfo& classinfo, MethodScope* methodScope, uint16_t modifiers,
					const MethodDescriptor& descriptor, const Attributes& attributes, uint16_t maxLocals);

			CodeEnvironment(const CodeEnvironment&) = delete;

			CodeEnvironment& operator=(const CodeEnvironment&) = delete;

			/*bool addOperation(const Operation* operation) {
				if(operation == nullptr)
					return false;

				//LOG(typeNameOf(operation));

				bool status = false;

				if(operation->getReturnType() != VOID) {
					stack.push(operation);
				} else if(operation->canAddToCode() && !(index == instructions.size() - 1 && operation == &VReturn::getInstance())) {
					currentScope->addOperation(operation, *this);
					status = true;
				}

				if(instanceof<const Scope*>(operation))
					addScope(static_cast<const Scope*>(operation));

				return status;
			}*/


			void updateScopes();

			inline const Scope* getCurrentScope() const {
				return currentScope;
			}

			inline void addScope(const Scope* scope) const {
				inactiveScopes.push_back(scope);
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
		public:
			const index_t startIndex, endIndex;

			const Scope *const parentScope;

		protected:
			const uint16_t localsCount;

			mutable vector<Variable*> variables;
			mutable uint16_t lastAddedVarIndex = 0;

			friend struct operations::IfScope;

			mutable vector<const Operation*> code;
			mutable map<const Variable*, string> varNames;
			mutable vector<const Scope*> innerScopes;

		private:
			Scope(index_t startIndex, index_t endIndex, const Scope* parentScope, uint16_t localsCount):
					startIndex(startIndex), endIndex(endIndex), parentScope(parentScope), localsCount(localsCount), variables(localsCount) {}

		public:

			Scope(index_t startIndex, index_t endIndex, uint16_t localsCount):
					Scope(startIndex, endIndex, nullptr, localsCount) {}

			Scope(index_t startIndex, index_t endIndex, const Scope* parentScope):
					Scope(startIndex, endIndex, parentScope, parentScope->localsCount) {}

			Scope(index_t startIndex, index_t endIndex, const CodeEnvironment& environment):
					Scope(startIndex, endIndex, environment.getCurrentScope(), environment.getCurrentScope()->localsCount) {}


			inline index_t start() const {
				return startIndex;
			}

			inline index_t end() const {
				return endIndex;
			}

		protected:
			const Variable* findVariable(index_t index) const {
				const Variable* var = variables[index];
				return var == nullptr && parentScope != nullptr ? parentScope->findVariable(index) : var;
			}

			const Variable* findDeclaredVariable(index_t index) const {
				const Variable* var = findVariable(index);
				if(var == nullptr) {
					for(const Scope* scope : innerScopes) {
						var = scope->findDeclaredVariable(index);
						if(var != nullptr)
							return var;
						LOG(scope->toDebugString() << ' ' << var);
						LOG(join<Variable*>(scope->variables, [] (Variable* v) { return to_string((uint64_t)v); }));
					}
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
					if(var != nullptr && name == var->getName())
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
			inline uint32_t getVariablesCount() const {
				return variables.size();
			}

			virtual void addOperation(const Operation* operation, const CodeEnvironment& environment) const {
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

			virtual bool isBreakable() const {
				return false;
			}

			virtual bool isContinuable() const {
				return false;
			}

			string toDebugString() const {
				return typeNameOf(*this) + " {" + to_string(startIndex) + ".." + to_string(endIndex) + '}';
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

			virtual void addOperation(const Operation* operation, const CodeEnvironment& environment) const override;

			inline bool isFieldsInitialized() const {
				return fieldsInitialized;
			}
	};
}

#endif
