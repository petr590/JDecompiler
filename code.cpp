#ifndef JDECOMPILER_CODE_CPP
#define JDECOMPILER_CODE_CPP

#include "types.cpp"

namespace jdecompiler {

	string ClassConstant::toString(const ClassInfo& classinfo) const {
		return ClassType(*name).toString(classinfo) + ".class";
	}


	struct Variable {
		protected:
			static string getRawNameByType(const Type* type, bool& unchecked) {

				if(type->isPrimitive()) {
					if(type->isInstanceof(BOOLEAN)) return "bool";
					if(type->isInstanceof(BYTE)) return "b";
					if(type->isInstanceof(CHAR)) return "c";
					if(type->isInstanceof(SHORT)) return "s";
					if(type->isInstanceof(INT)) return "n";
					if(type->isInstanceof(LONG)) return "l";
					if(type->isInstanceof(FLOAT)) return "f";
					if(type->isInstanceof(DOUBLE)) return "d";
				}
				if(const ClassType* classType = dynamic_cast<const ClassType*>(type)) {
					if(classType->simpleName == "Boolean") return "bool";
					if(classType->simpleName == "Byte") return "b";
					if(classType->simpleName == "Character") return "ch";
					if(classType->simpleName == "Short") return "sh";
					if(classType->simpleName == "Integer") return "n";
					if(classType->simpleName == "Long") return "l";
					if(classType->simpleName == "Float") return "f";
					if(classType->simpleName == "Double") return "d";
					unchecked = true;
					return toLowerCamelCase(classType->simpleName);
				}
				if(const ArrayType* arrayType = dynamic_cast<const ArrayType*>(type)) {
					if(const ClassType* classMemberType = dynamic_cast<const ClassType*>(arrayType->memberType))
						return toLowerCamelCase(classMemberType->simpleName) + "Array";
					return toLowerCamelCase(arrayType->memberType->getName()) + "Array";
				}
				return "o";
			}

			static string getNameByType(const Type* type) {
				bool unchecked = false;

				const string name = getRawNameByType(type, unchecked);
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


		public:
			mutable const Type* type;


			Variable(const Type* type): type(type) {}

			virtual ~Variable() {}


			virtual string getName() const = 0;

			virtual void addName(const string&) const = 0;

			inline bool operator==(const Variable& other) const {
				return this == &other;
			}
	};


	struct NamedVariable: Variable {
		protected:
			const string name;

		public:
			NamedVariable(const Type* type, const string& name): Variable(type), name(name) {}

			virtual string getName() const override {
				return name;
			}

			virtual void addName(const string& name) const override {}
	};


	struct UnnamedVariable: Variable {
		protected:
			mutable vector<string> names;

		public:
			UnnamedVariable(const Type* type): Variable(type) {}

			UnnamedVariable(const Type* type, const string& name): Variable(type), names({name}) {}

			virtual string getName() const override {
				return names.size() == 1 ? names[0] : getNameByType(type);
			}

			virtual void addName(const string& name) const override {
				names.push_back(name);
			}
	};


	namespace BuiltinTypes {
		/* Serves as a marker for the struct TypeByBuiltinType */
		struct MarkerStruct { MarkerStruct() = delete; };

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


	struct Operation {
		public:
			static const uint16_t DEFAULT_PRIORITY = 15;

			const uint16_t priority;
			const Associativity associativity;

		protected:
			Operation(uint16_t priority = DEFAULT_PRIORITY): priority(priority), associativity(getAssociativityByPriority(priority)) {}

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
			string toString(const CodeEnvironment& environment, uint16_t priority, const Associativity associativity) const {
				if(this->priority < priority || (this->priority == priority && this->associativity != associativity))
					return '(' + this->toString(environment) + ')';
				return this->toString(environment);
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
			static inline constexpr Associativity getAssociativityByPriority(uint16_t priority) {
				return priority == 1 || priority == 2 || priority == 13 ? Associativity::RIGHT : Associativity::LEFT;
			}
	};

	struct Instruction {
		public:
			virtual const Operation* toOperation(const CodeEnvironment& environment) const = 0;

		protected:
			Instruction() {}

			virtual ~Instruction() {}
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

			const Operation* operation = Stack<const Operation*>::pop();
			operation->getReturnTypeAs(type);
			return operation;
		}

		inline const Operation* popAs(const Type& type) {
			return popAs(&type);
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
			Stack<Scope*> currentScopes;
			const uint16_t modifiers;
			const MethodDescriptor& descriptor;
			const Attributes& attributes;
			uint32_t pos = 0, index = 0, exprStartIndex = 0;
			map<uint32_t, uint32_t> exprIndexTable;

		private:
			mutable vector<Scope*> scopes;

		public:
			CodeEnvironment(const Bytecode& bytecode, const ClassInfo& classinfo, MethodScope* methodScope, uint16_t modifiers,
					const MethodDescriptor& descriptor, const Attributes& attributes, uint16_t maxLocals);

			CodeEnvironment(const CodeEnvironment&) = delete;

			CodeEnvironment& operator=(const CodeEnvironment&) = delete;


			void checkCurrentScope();

			inline Scope* getCurrentScope() const {
				return currentScopes.top();
			}

			inline void addScope(Scope* scope) const {
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
				print(cerr << "Warning while decompiling method " << descriptor.toString() << " at pos " << pos << ": ", args...);
			}
	};


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
			uint32_t startPos, endPos;

		public:
			Scope *const parentScope;

		protected:
			vector<Variable*> variables;
			vector<const Operation*> code;
			map<const Variable*, string> varNames;

		public:
			vector<Scope*> innerScopes;

			Scope(uint32_t startPos, uint32_t endPos, Scope* parentScope): startPos(startPos), endPos(endPos), parentScope(parentScope) {}


			inline uint32_t start() const {
				return startPos;
			}

			inline uint32_t end() const {
				return endPos;
			}


			virtual const Variable& getVariable(uint32_t index) const {
				if(index >= variables.size() && parentScope == nullptr)
					throw IndexOutOfBoundsException(index, variables.size());
				return index >= variables.size() ? parentScope->getVariable(index) : *variables[index];
			}

			bool hasVariable(const string& name) const {
				for(Variable* var : variables)
					if(name == var->getName())
						return true;
				return parentScope == nullptr ? false : parentScope->hasVariable(name);
			}

			void addVariable(Variable* var) {
				variables.push_back(var);
			}

			inline string getNameFor(const Variable& var) {
				return getNameFor(&var);
			}

			string getNameFor(const Variable* var) {
				const auto varName = varNames.find(var);
				if(varName != varNames.end()) {
					return varName->second;
				}

				const string baseName = var->getName();
				string name = baseName;
				unsigned int i = 1;

				while(find_if(varNames.begin(), varNames.end(), [&name] (const auto& it) { return it.second == name; }) != varNames.end()) {
					name = baseName + to_string(++i);
				}

				varNames[var] = name;

				return name;
			}

			//virtual void initiate(const CodeEnvironment&) {}

			virtual void finalize(const CodeEnvironment&) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				environment.classinfo.increaseIndent();

				string str = getHeader(environment) + "{\n";
				const size_t baseSize = str.size();

				for(auto i = code.begin(); i != code.end(); ++i) {
					if(printNextOperation(i)) {
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
			virtual inline bool printNextOperation(const vector<const Operation*>::const_iterator i) const { return true; }

			virtual inline string getBackSeparator(const ClassInfo& classinfo) const override {
				return "\n";
			}

			virtual string getHeader(const CodeEnvironment& environment) const {
				return EMPTY_STRING;
			}

		public:
			uint32_t getVariablesCount() const {
				return variables.size();
			}

			virtual void add(const Operation* operation, const CodeEnvironment& environment) {
				code.push_back(operation);
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
			MethodScope(uint32_t startPos, uint32_t endPos, uint16_t localsCount): Scope(startPos, endPos, nullptr) {
				variables.reserve(localsCount);
			}
	};


	struct StaticInitializerScope: MethodScope {
		private:
			bool fieldsInitialized = false;

		public:
			StaticInitializerScope(uint32_t startPos, uint32_t endPos, uint16_t localsCount):
					MethodScope(startPos, endPos, localsCount) {}

			virtual void add(const Operation* operation, const CodeEnvironment& environment) override;

			inline bool isFieldsInitialized() const {
				return fieldsInitialized;
			}
	};

}

#endif
