#ifndef JDECOMPILER_INSTRUCTIONS_CPP
#define JDECOMPILER_INSTRUCTIONS_CPP

#include "operations.cpp"

namespace jdecompiler {
	namespace instructions {

		using namespace operations;


		struct InstructionWithIndex: Instruction {
			public:
				const uint16_t index;

			protected:
				InstructionWithIndex(uint16_t index): index(index) {}
		};


		struct InstructionAndOperation: Instruction, Operation {
			public: virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return this; }
		};

		struct VoidInstructionAndOperation: InstructionAndOperation {
			public: virtual const Type* getReturnType() const override { return VOID; }
		};


		struct AConstNull final: InstructionAndOperation {
			private: AConstNull() {}

			public:
				virtual string toString(const CodeEnvironment& environment) const override {
					return "null";
				}

				virtual const Type* getReturnType() const override {
					return AnyType::getInstance();
				}

				static inline AConstNull& getInstance() {
					static AConstNull instance;
					return instance;
				}
		};

		template<typename T>
		struct NumberConstInstruction: Instruction {
			static_assert(is_fundamental<T>::value, "template type T of struct NumberConstInstruction is not primitive");

			const T value;

			NumberConstInstruction(const T value): value(value) {}
		};


		struct IConstInstruction: NumberConstInstruction<int32_t> {
			IConstInstruction(int32_t value): NumberConstInstruction(value) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new IConstOperation(value); }
		};

		struct LConstInstruction: NumberConstInstruction<int64_t> {
			LConstInstruction(int64_t value): NumberConstInstruction(value) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new LConstOperation(value); }
		};

		struct FConstInstruction: NumberConstInstruction<float> {
			FConstInstruction(float value): NumberConstInstruction(value) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new FConstOperation(value); }
		};

		struct DConstInstruction: NumberConstInstruction<double> {
			DConstInstruction(double value): NumberConstInstruction(value) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new DConstOperation(value); }
		};


		template<typename T>
		struct IPushInstruction: Instruction {
			public:
				const T value;

				IPushInstruction(T value): value(value) {}

				virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new IConstOperation(value); }
		};

		using BIPushInstruction = IPushInstruction<int8_t>;

		using SIPushInstruction = IPushInstruction<int16_t>;



		template<TypeSize size>
		struct LdcInstruction: InstructionWithIndex {
			LdcInstruction(uint16_t index): InstructionWithIndex(index) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
				return LdcOperation_valueOf(environment, index);
			}
		};


		struct LoadInstruction: InstructionWithIndex {
			public: LoadInstruction(uint16_t index): InstructionWithIndex(index) {}
		};

		struct ILoadInstruction: LoadInstruction {
			ILoadInstruction(uint16_t index): LoadInstruction(index) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
				return isNullOperation ? nullptr : new ILoadOperation(environment, index);
			}

			private:
				friend operations::IIncOperation::IIncOperation(const CodeEnvironment& environment, uint16_t index, int16_t value);

				mutable bool isNullOperation = false;

				inline void setNullOperation() const {
					isNullOperation = true;
				}
		};

		struct LLoadInstruction: LoadInstruction {
			LLoadInstruction(uint16_t index): LoadInstruction(index) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new LLoadOperation(environment, index); }
		};

		struct FLoadInstruction: LoadInstruction {
			FLoadInstruction(uint16_t index): LoadInstruction(index) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new FLoadOperation(environment, index); }
		};

		struct DLoadInstruction: LoadInstruction {
			DLoadInstruction(uint16_t index): LoadInstruction(index) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new DLoadOperation(environment, index); }
		};

		struct ALoadInstruction: LoadInstruction {
			ALoadInstruction(uint16_t index): LoadInstruction(index) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new ALoadOperation(environment, index); }
		};


		struct ArrayLoadInstruction: Instruction {};

		struct IALoadInstruction: ArrayLoadInstruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new IALoadOperation(environment); }
		};

		struct LALoadInstruction: ArrayLoadInstruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new LALoadOperation(environment); }
		};

		struct FALoadInstruction: ArrayLoadInstruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new FALoadOperation(environment); }
		};

		struct DALoadInstruction: ArrayLoadInstruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new DALoadOperation(environment); }
		};

		struct AALoadInstruction: ArrayLoadInstruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new AALoadOperation(environment); }
		};

		struct BALoadInstruction: ArrayLoadInstruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new BALoadOperation(environment); }
		};

		struct CALoadInstruction: ArrayLoadInstruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new CALoadOperation(environment); }
		};

		struct SALoadInstruction: ArrayLoadInstruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new SALoadOperation(environment); }
		};


		struct StoreInstruction: InstructionWithIndex {
			public: StoreInstruction(uint16_t index): InstructionWithIndex(index) {}
		};

		struct IStoreInstruction: StoreInstruction {
			IStoreInstruction(uint16_t index): StoreInstruction(index) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new IStoreOperation(environment, index); }
		};

		struct LStoreInstruction: StoreInstruction {
			LStoreInstruction(uint16_t index): StoreInstruction(index) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new LStoreOperation(environment, index); }
		};

		struct FStoreInstruction: StoreInstruction {
			FStoreInstruction(uint16_t index): StoreInstruction(index) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new FStoreOperation(environment, index); }
		};

		struct DStoreInstruction: StoreInstruction {
			DStoreInstruction(uint16_t index): StoreInstruction(index) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new DStoreOperation(environment, index); }
		};

		struct AStoreInstruction: StoreInstruction {
			AStoreInstruction(uint16_t index): StoreInstruction(index) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new AStoreOperation(environment, index); }
		};


		struct ArrayStoreInstruction: Instruction {};

		struct IAStoreInstruction: ArrayStoreInstruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new IAStoreOperation(environment); }
		};

		struct LAStoreInstruction: ArrayStoreInstruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new LAStoreOperation(environment); }
		};

		struct FAStoreInstruction: ArrayStoreInstruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new FAStoreOperation(environment); }
		};

		struct DAStoreInstruction: ArrayStoreInstruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new DAStoreOperation(environment); }
		};

		struct AAStoreInstruction: ArrayStoreInstruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new AAStoreOperation(environment); }
		};

		struct BAStoreInstruction: ArrayStoreInstruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new BAStoreOperation(environment); }
		};

		struct CAStoreInstruction: ArrayStoreInstruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new CAStoreOperation(environment); }
		};

		struct SAStoreInstruction: ArrayStoreInstruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new SAStoreOperation(environment); }
		};


		template<TypeSize size>
		struct PopInstruction: Instruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new PopOperation<size>(environment); }
		};


		template<TypeSize size>
		struct DupInstruction: Instruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new DupOperation<size>(environment); }
		};

		using Dup1Instruction = DupInstruction<TypeSize::FOUR_BYTES>;
		using Dup2Instruction = DupInstruction<TypeSize::EIGHT_BYTES>;

		struct DupX1Instruction: Instruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new DupX1Operation(environment); }
		};

		struct DupX2Instruction: Instruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new DupX2Operation(environment); }
		};

		struct Dup2X1Instruction: Instruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new Dup2X1Operation(environment); }
		};

		struct Dup2X2Instruction: Instruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new Dup2X2Operation(environment); }
		};


		struct SwapInstruction: Instruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
				environment.stack.push(environment.stack.pop(), environment.stack.pop());
				return nullptr;
			}
		};


		template<char32_t operation, Priority priority, bool canUseBoolean = false>
		struct OperatorInstruction: Instruction {
			protected:
				const Type *const type;

				static inline const Type* getTypeByCode(uint16_t code) {
					switch(code) {
						case 0: return canUseBoolean ? ANY_INT_OR_BOOLEAN : ANY_INT;
						case 1: return LONG;
						case 2: return FLOAT;
						case 3: return DOUBLE;
						default: throw Exception("Illegal type code: " + hexWithPrefix(code));
					}
				}

			public:
				OperatorInstruction(uint16_t typeCode): type(getTypeByCode(typeCode)) {}
		};


		template<char32_t operation, Priority priority, bool canUseBoolean = false>
		struct BinaryOperatorInstruction: OperatorInstruction<operation, priority, canUseBoolean> {
			BinaryOperatorInstruction(uint16_t typeCode): OperatorInstruction<operation, priority, canUseBoolean>(typeCode) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
				return new BinaryOperatorOperationImpl<operation, priority>(this->type, environment);
			}
		};


		template<char32_t operation, Priority priority>
		struct ShiftOperatorInstruction: BinaryOperatorInstruction<operation, priority> {
			ShiftOperatorInstruction(uint16_t typeCode): BinaryOperatorInstruction<operation, priority>(typeCode) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
				return new BinaryOperatorOperationImpl<operation, priority>(this->type, INT, environment);
			}
		};


		template<char32_t operation, Priority priority>
		struct BooleanBinaryOperatorInstruction: BinaryOperatorInstruction<operation, priority, true> {
			BooleanBinaryOperatorInstruction(uint16_t typeCode): BinaryOperatorInstruction<operation, priority, true>(typeCode) {}
		};



		template<char32_t operation, Priority priority>
		struct UnaryOperatorInstruction: OperatorInstruction<operation, priority> {
			UnaryOperatorInstruction(uint16_t typeCode): OperatorInstruction<operation, priority>(typeCode) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
				return new UnaryOperatorOperation<operation, priority>(this->type, environment);
			}
		};

		using AddOperatorInstruction = BinaryOperatorInstruction<'+', Priority::PLUS>;
		using SubOperatorInstruction = BinaryOperatorInstruction<'-', Priority::MINUS>;
		using MulOperatorInstruction = BinaryOperatorInstruction<'*', Priority::MULTIPLE>;
		using DivOperatorInstruction = BinaryOperatorInstruction<'/', Priority::DIVISION>;
		using RemOperatorInstruction = BinaryOperatorInstruction<'%', Priority::REMAINDER>;

		using NegOperatorInstruction = UnaryOperatorInstruction<'-', Priority::UNARY_MINUS>;

		using ShiftLeftOperatorInstruction = ShiftOperatorInstruction<"<<"_c32, Priority::SHIFT>;
		using ShiftRightOperatorInstruction = ShiftOperatorInstruction<">>"_c32, Priority::SHIFT>;
		using UShiftRightOperatorInstruction = ShiftOperatorInstruction<">>>"_c32, Priority::SHIFT>;

		using AndOperatorInstruction = BooleanBinaryOperatorInstruction<'&', Priority::BIT_AND>;
		using OrOperatorInstruction  = BooleanBinaryOperatorInstruction<'|', Priority::BIT_OR>;
		using XorOperatorInstruction = BooleanBinaryOperatorInstruction<'^', Priority::BIT_XOR>;


		/* maybe TODO
		template<PrimitiveType type> using AddOperatorInstruction = BinaryOperatorInstruction<type, '+', 11>;

		template<PrimitiveType type> using SubOperatorInstruction = BinaryOperatorInstruction<type, '-', 11>;

		template<PrimitiveType type> using MulOperatorInstruction = BinaryOperatorInstruction<type, '*', 12>;

		template<PrimitiveType type> using DivOperatorInstruction = BinaryOperatorInstruction<type, '/', 12>;

		template<PrimitiveType type> using RemOperatorInstruction = BinaryOperatorInstruction<type, '%', 12>;

		template<PrimitiveType type> using NegOperatorInstruction = UnaryOperatorInstruction<type, '-', 13>;

		template<PrimitiveType type> using ShiftLeftOperatorInstruction = BinaryOperatorInstruction<type, '<<', 10>;

		template<PrimitiveType type> using ShiftRightOperatorInstruction = BinaryOperatorInstruction<type, '>>', 10>;

		template<PrimitiveType type> using UShiftRightOperatorInstruction = BinaryOperatorInstruction<type, '>>>', 10>;

		template<PrimitiveType type> using AndOperatorInstruction = BinaryOperatorInstruction<type, '&', 7>;

		template<PrimitiveType type> using OrOperatorInstruction = BinaryOperatorInstruction<type, '|', 5>;

		template<PrimitiveType type> using XorOperatorInstruction = BinaryOperatorInstruction<type, '^', 6>;
		*/



		struct IIncInstruction: InstructionWithIndex {
			const int16_t value;

			IIncInstruction(uint16_t index, int16_t value): InstructionWithIndex(index), value(value) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new IIncOperation(environment, index, value); }
		};

	}

	namespace operations {
		IIncOperation::IIncOperation(const CodeEnvironment& environment, uint16_t index, int16_t value):
				variable(environment.getCurrentScope()->getVariable(index, true)), value(value),
				isShortInc(value == 1 || value == -1) /* isShortInc true when we can write ++ or -- */ {

			using namespace instructions;

			const Type* variableType = variable.castTypeTo(ANY_INT);

			const ILoadOperation* iloadOperation = environment.stack.empty() ? nullptr : dynamic_cast<const ILoadOperation*>(environment.stack.top());

			if(isShortInc && iloadOperation != nullptr && iloadOperation->variable == variable) {
				environment.stack.pop();
				returnType = variableType;
				isPostInc = true;
			} else {
				const ILoadInstruction* iloadInstruction =
						dynamic_cast<const ILoadInstruction*>(environment.bytecode.getInstructionNoexcept(environment.index + 1));
				if(iloadInstruction != nullptr && iloadInstruction->index == index) {
					iloadInstruction->setNullOperation();
					returnType = variableType;
				} else {
					returnType = VOID;
				}
			}
		}
	}

	namespace instructions {

		template<bool required>
		struct CastInstruction: Instruction {
			protected:
				const Type *const requiredType, *const type;

			public:
				CastInstruction(const Type* requiredType, const Type* type): requiredType(requiredType), type(type) {}

				virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
					return new CastOperation<required>(environment, requiredType, type);
				}
		};


		struct LCmpInstruction: Instruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new LCmpOperation(environment); }
		};

		struct FCmpInstruction: Instruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new FCmpOperation(environment); }
		};

		struct DCmpInstruction: Instruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new DCmpOperation(environment); }
		};


		struct IfBlock: Instruction {
			const int32_t offset;

			IfBlock(/*const Bytecode& bytecode, */const int32_t offset): /*Block(bytecode.getPos(), bytecode.getPos() + offset),*/ offset(offset) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override final {
				const Scope* currentScope = environment.getCurrentScope();

				const uint32_t index = environment.bytecode.posToIndex(environment.pos + offset);

				if(const IfScope* ifScope = dynamic_cast<const IfScope*>(currentScope)) {
					if(offset > 0) {
						//LOG(environment.index << ' ' << index << ' ' << ifScope->end());

						if(index - 1 == ifScope->end()) {
							ifScope->condition = new AndOperation(ifScope->condition, getCondition(environment)->invert());
							return nullptr;
						}

						if(environment.index == ifScope->end()) {
							ifScope->condition = new OrOperation(ifScope->condition->invert(), getCondition(environment)->invert());
							ifScope->setEnd(index);
							return nullptr;
						}
					}
				}

				return new IfScope(environment, offset, getCondition(environment)->invert());
			}

			virtual const ConditionOperation* getCondition(const CodeEnvironment& environment) const = 0;
		};

		struct IfCmpBlock: IfBlock {
			const CompareType& compareType;

			IfCmpBlock(const int32_t offset, const CompareType& compareType): IfBlock(offset), compareType(compareType) {}

			virtual const ConditionOperation* getCondition(const CodeEnvironment& environment) const override {
				const Operation* operation = environment.stack.pop();
				if(const CmpOperation* cmpOperation = Operation::castOperationTo<const CmpOperation*>(operation))
					return new CompareBinaryOperation(cmpOperation, compareType);
				return new CompareWithZeroOperation(operation, compareType);
			}
		};


		struct IfEqBlock: IfCmpBlock {
			IfEqBlock(const int32_t offset): IfCmpBlock(offset, CompareType::EQUALS) {};
		};

		struct IfNotEqBlock: IfCmpBlock {
			IfNotEqBlock(const int32_t offset): IfCmpBlock(offset, CompareType::NOT_EQUALS) {}
		};

		struct IfGtBlock: IfCmpBlock {
			IfGtBlock(const int32_t offset): IfCmpBlock(offset, CompareType::GREATER) {}
		};

		struct IfGeBlock: IfCmpBlock {
			IfGeBlock(const int32_t offset): IfCmpBlock(offset, CompareType::GREATER_OR_EQUALS) {}
		};

		struct IfLtBlock: IfCmpBlock {
			IfLtBlock(const int32_t offset): IfCmpBlock(offset, CompareType::LESS) {}
		};

		struct IfLeBlock: IfCmpBlock {
			IfLeBlock(const int32_t offset): IfCmpBlock(offset, CompareType::LESS_OR_EQUALS) {}
		};


		struct IfICmpBlock: IfCmpBlock {
			IfICmpBlock(int32_t offset, const CompareType& compareType): IfCmpBlock(offset, compareType) {}

			virtual const ConditionOperation* getCondition(const CodeEnvironment& environment) const override {
				return new CompareBinaryOperation(environment, INT, compareType);
			}
		};


		struct IfIEqBlock: IfICmpBlock {
			IfIEqBlock(int32_t offset): IfICmpBlock(offset, CompareType::EQUALS) {}
		};

		struct IfINotEqBlock: IfICmpBlock {
			IfINotEqBlock(int32_t offset): IfICmpBlock(offset, CompareType::NOT_EQUALS) {}
		};

		struct IfIGtBlock: IfICmpBlock {
			IfIGtBlock(int32_t offset): IfICmpBlock(offset, CompareType::GREATER) {}
		};

		struct IfIGeBlock: IfICmpBlock {
			IfIGeBlock(int32_t offset): IfICmpBlock(offset, CompareType::GREATER_OR_EQUALS) {}
		};

		struct IfILtBlock: IfICmpBlock {
			IfILtBlock(int32_t offset): IfICmpBlock(offset, CompareType::LESS) {}
		};

		struct IfILeBlock: IfICmpBlock {
			IfILeBlock(int32_t offset): IfICmpBlock(offset, CompareType::LESS_OR_EQUALS) {}
		};


		struct IfACmpBlock: IfCmpBlock {
			IfACmpBlock(int32_t offset, const EqualsCompareType& compareType): IfCmpBlock(offset, compareType) {}

			virtual const ConditionOperation* getCondition(const CodeEnvironment& environment) const override {
				return new CompareBinaryOperation(environment, AnyObjectType::getInstance(), compareType);
			}
		};


		struct IfAEqBlock: IfACmpBlock {
			IfAEqBlock(int32_t offset): IfACmpBlock(offset, CompareType::EQUALS) {}
		};

		struct IfANotEqBlock: IfACmpBlock {
			IfANotEqBlock(int32_t offset): IfACmpBlock(offset, CompareType::NOT_EQUALS) {}
		};


		struct IfNullBlock: IfBlock {
			IfNullBlock(const int32_t offset): IfBlock(offset) {}

			virtual const ConditionOperation* getCondition(const CodeEnvironment& environment) const override {
				return new CompareWithNullOperation(environment, CompareType::EQUALS);
			}
		};

		struct IfNonNullBlock: IfBlock {
			IfNonNullBlock(const int32_t offset): IfBlock(offset) {}

			virtual const ConditionOperation* getCondition(const CodeEnvironment& environment) const override {
				return new CompareWithNullOperation(environment, CompareType::NOT_EQUALS);
			}
		};


		struct GotoInstruction: Instruction {
			const int32_t offset;

			GotoInstruction(int32_t offset): offset(offset) {}


			virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
				if(offset == 0) return new EmptyInfiniteLoopScope(environment);

				const uint32_t index = environment.bytecode.posToIndex(environment.pos + offset);

				const Scope* const currentScope = environment.getCurrentScope();


				if(const IfScope* ifScope = dynamic_cast<const IfScope*>(currentScope)) {
					// Here goto instruction creates else scope
					if(offset > 0 && !ifScope->isLoop() && environment.index == ifScope->end() /* check if goto instruction in the end of ifScope */) {
						const Scope* parentScope = ifScope->parentScope;

						/* I don't remember why there is index minus 1 instead of index,
						   but since I wrote that, then it should be so :) */
						if(index - 1 <= parentScope->end()) {
							ifScope->addElseScope(environment, index - 1);
							return nullptr;
						}

						const GotoInstruction* gotoInstruction = dynamic_cast<const GotoInstruction*>(environment.bytecode.getInstruction(parentScope->end()));
						if(gotoInstruction && environment.bytecode.posToIndex(environment.bytecode.indexToPos(parentScope->end()) + gotoInstruction->offset) == index) {
							ifScope->addElseScope(environment, parentScope->end() - 1);
							return nullptr;
						}
					} else {
						// Here goto creates operator continue
						do {
							//LOG(index << ' ' << ifScope->start());
							if(index == ifScope->start()) {
								ifScope->setLoopType(environment);
								return new ContinueOperation(environment, ifScope);
							}
							ifScope = dynamic_cast<const IfScope*>(ifScope->parentScope);
						} while(ifScope != nullptr);
					}
				} else if(const TryScope* tryScope = dynamic_cast<const TryScope*>(currentScope)) {
					if(environment.index == tryScope->end()) return nullptr;
						//return new CatchScope(environment, tryScope);
				}

				//cerr << "GOTO LOG!!! " << offset << ' ' << index << ' ' << currentScope->start() << "  " << index << ' ' << currentScope->end() << '\n' << typeid(*currentScope).name() << endl;

				if(offset > 0 && index >= currentScope->start() && index - 1 <= currentScope->end()) { // goto in the borders of current scope
					const_cast<Bytecode&>(environment.bytecode).skip(offset);
					return nullptr;
				}

				throw DecompilationException("Illegal using of goto instruction: goto from " +
						to_string(environment.pos) + " to " + to_string(environment.pos + offset));
			}
		};


		struct SwitchInstruction: Instruction {
			protected:
				const int32_t defaultOffset;
				map<int32_t, int32_t> offsetTable;

			public:
				SwitchInstruction(int32_t defaultOffset, map<int32_t, int32_t> offsetTable): defaultOffset(defaultOffset), offsetTable(offsetTable) {}

				virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
					return new SwitchScope(environment, defaultOffset, offsetTable);
				}
		};


		template<class ReturnOperation>
		struct ReturnInstruction: Instruction {
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
				return new ReturnOperation(environment);
			}
		};

		using IReturnInstruction = ReturnInstruction<IReturnOperation>;
		using LReturnInstruction = ReturnInstruction<LReturnOperation>;
		using FReturnInstruction = ReturnInstruction<FReturnOperation>;
		using DReturnInstruction = ReturnInstruction<DReturnOperation>;
		using AReturnInstruction = ReturnInstruction<AReturnOperation>;


		struct VReturn: VoidInstructionAndOperation {
			private: VReturn() {}

			public:
				virtual string toString(const CodeEnvironment& environment) const override { return "return"; }

				static VReturn& getInstance() {
					static VReturn instance;
					return instance;
				}
		};


		struct GetStaticFieldInstruction: InstructionWithIndex {
			GetStaticFieldInstruction(uint16_t index): InstructionWithIndex(index) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new GetStaticFieldOperation(environment, index); }
		};


		struct PutStaticFieldInstruction: InstructionWithIndex {
			PutStaticFieldInstruction(uint16_t index): InstructionWithIndex(index) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new PutStaticFieldOperation(environment, index); }
		};


		struct GetInstanceFieldInstruction: InstructionWithIndex {
			GetInstanceFieldInstruction(uint16_t index): InstructionWithIndex(index) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new GetInstanceFieldOperation(environment, index); }
		};


		struct PutInstanceFieldInstruction: InstructionWithIndex {
			PutInstanceFieldInstruction(uint16_t index): InstructionWithIndex(index) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new PutInstanceFieldOperation(environment, index); }
		};



		struct InvokeInstruction: InstructionWithIndex {
			InvokeInstruction(uint16_t index): InstructionWithIndex(index) {}
		};


		struct InvokevirtualInstruction: InvokeInstruction {
			InvokevirtualInstruction(uint16_t index): InvokeInstruction(index) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
				return new InvokevirtualOperation(environment, index);
			}
		};


		struct InvokespecialInstruction: InvokeInstruction {
			InvokespecialInstruction(uint16_t index): InvokeInstruction(index) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
				return new InvokespecialOperation(environment, index);
			}
		};


		struct InvokestaticInstruction: InvokeInstruction {
			InvokestaticInstruction(uint16_t index): InvokeInstruction(index) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
				return new InvokestaticOperation(environment, index);
			}
		};


		struct InvokeinterfaceInstruction: InvokeInstruction {
			InvokeinterfaceInstruction(uint16_t index, uint16_t count, uint16_t zeroByte, const Bytecode& bytecode): InvokeInstruction(index) {
				if(count == 0)
					cerr << "warning: illegal format of instruction invokeinterface at pos " << hexWithPrefix(bytecode.getPos()) <<
							": by specification, count must not be zero" << endl;
				if(zeroByte != 0)
					cerr << "warning: illegal format of instruction invokeinterface at pos " << hexWithPrefix(bytecode.getPos()) <<
							": by specification, fourth byte must be zero" << endl;
			}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
				return new InvokeinterfaceOperation(environment, index);
			}
		};


		struct InvokedynamicInstruction: InvokeInstruction {
			InvokedynamicInstruction(uint16_t index, uint16_t zeroShort, const Bytecode& bytecode): InvokeInstruction(index) {
				if(zeroShort != 0)
					cerr << "warning: illegal format of instruction invokedynamic at pos " << hexWithPrefix(bytecode.getPos()) <<
							": by specification, third and fourth bytes must be zero" << endl;
			}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
				const InvokeDynamicConstant* invokeDynamicConstant = environment.constPool.get<InvokeDynamicConstant>(index);
				const BootstrapMethod* bootstrapMethod =
						(*environment.classinfo.attributes.getExact<BootstrapMethodsAttribute>())[invokeDynamicConstant->bootstrapMethodAttrIndex];

				const uint16_t index = bootstrapMethod->methodHandle->referenceRef;

				typedef MethodHandleConstant::ReferenceKind RefKind;
				typedef MethodHandleConstant::KindType KindType;

				switch(bootstrapMethod->methodHandle->kindType) {
					case KindType::FIELD:
						switch(bootstrapMethod->methodHandle->referenceKind) {
							case RefKind::GETFIELD: return new GetInstanceFieldOperation(environment, index);
							case RefKind::GETSTATIC: return new GetStaticFieldOperation(environment, index);
							case RefKind::PUTFIELD: return new PutInstanceFieldOperation(environment, index);
							case RefKind::PUTSTATIC: return new PutStaticFieldOperation(environment, index);
							default: throw IllegalStateException((string)"Illegal referenceConstant kind " +
									to_string((unsigned int)bootstrapMethod->methodHandle->referenceKind));
						}
					case KindType::METHOD: {
						const MethodDescriptor descriptor(*bootstrapMethod->methodHandle->referenceConstant->clazz->name,
								*invokeDynamicConstant->nameAndType->name, *invokeDynamicConstant->nameAndType->descriptor);

						vector<const Operation*> arguments;

						static const ArrayType OBJECT_ARRAY(OBJECT);
						static const ClassType CALL_SITE("java/lang/invoke/CallSite");
						static const ClassType LOOKUP("java/lang/invoke/MethodHandles$Lookup");
						static const ClassType STRING_CONCAT_FACTORY("java/lang/invoke/StringConcatFactory");

						// pop arguments that already on stack
						for(uint32_t i = descriptor.arguments.size(); i > 0; i--)
							arguments.push_back(environment.stack.pop());


						if(bootstrapMethod->methodHandle->referenceKind == RefKind::INVOKESTATIC && descriptor.name == "makeConcatWithConstants" &&
							MethodDescriptor(bootstrapMethod->methodHandle->referenceConstant) ==
							MethodDescriptor(STRING_CONCAT_FACTORY, "makeConcatWithConstants", &CALL_SITE, {&LOOKUP, STRING, METHOD_TYPE, STRING, &OBJECT_ARRAY}))
						{
							// push static arguments on stack
							for(uint32_t i = 0, argumentsCount = bootstrapMethod->arguments.size(); i < argumentsCount; i++)
								environment.stack.push(LdcOperation_valueOf(bootstrapMethod->argumentIndexes[i], bootstrapMethod->arguments[i]));

							// push non-static arguments on stack
							for(const Operation* operation : arguments)
								environment.stack.push(operation);

							return new ConcatStringsOperation(environment, *new MethodDescriptor(descriptor));
						}


						// push lookup argument
						environment.stack.push(new InvokestaticOperation(environment,
								*new MethodDescriptor(STRING_CONCAT_FACTORY, "publicLookup", CALL_SITE, {})));

						StringConstant* nameArgument = new StringConstant(invokeDynamicConstant->nameAndType->nameRef);
						nameArgument->init(environment.constPool);
						environment.stack.push(new StringConstOperation(nameArgument));

						MethodTypeConstant* typeArgument = new MethodTypeConstant(invokeDynamicConstant->nameAndType->descriptorRef);
						typeArgument->init(environment.constPool);
						environment.stack.push(new MethodTypeConstOperation(typeArgument));

						// push static arguments on stack
						for(uint32_t i = 0, argumentsCount = bootstrapMethod->arguments.size(); i < argumentsCount; i++)
							environment.stack.push(LdcOperation_valueOf(bootstrapMethod->argumentIndexes[i], bootstrapMethod->arguments[i]));

						// push non-static arguments on stack
						for(const Operation* operation : arguments)
							environment.stack.push(operation);

						switch(bootstrapMethod->methodHandle->referenceKind) {
							case RefKind::INVOKEVIRTUAL: return new InvokevirtualOperation(environment, index);
							case RefKind::INVOKESTATIC: return new InvokestaticOperation(environment, index);
							case RefKind::INVOKESPECIAL: return new InvokespecialOperation(environment, index);
							case RefKind::NEWINVOKESPECIAL: return new InvokespecialOperation(environment,
										new NewOperation(environment, bootstrapMethod->methodHandle->referenceConstant->clazz), index);
							case RefKind::INVOKEINTERFACE: return new InvokeinterfaceOperation(environment, index);
							default: throw IllegalStateException((string)"Illegal referenceConstant kind " +
									to_string((unsigned int)bootstrapMethod->methodHandle->referenceKind));
						}
					}
					default: throw IllegalStateException((string)"Illegal kind type " + to_string((unsigned int)bootstrapMethod->methodHandle->kindType));
				}
			}
		};



		struct NewInstruction: InstructionWithIndex {
			NewInstruction(uint16_t classIndex): InstructionWithIndex(classIndex) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new NewOperation(environment, index); }
		};


		static const Type* getArrayTypeByCode(uint8_t code) {
			switch(code) {
				case 0x4: return BOOLEAN;
				case 0x5: return CHAR;
				case 0x6: return FLOAT;
				case 0x7: return DOUBLE;
				case 0x8: return BYTE;
				case 0x9: return SHORT;
				case 0xA: return INT;
				case 0xB: return LONG;
				default: throw DecompilationException("Illegal array type code: " + hexWithPrefix(code));
			}
		}


		struct NewArrayInstruction: Instruction {
			protected:
				const Type *const memberType;

			public:
				NewArrayInstruction(uint8_t code): memberType(getArrayTypeByCode(code)) {}

				virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
					return new NewArrayOperation(environment, memberType);
				}
		};

		struct ANewArrayInstruction: InstructionWithIndex {
			ANewArrayInstruction(uint16_t index): InstructionWithIndex(index) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new ANewArrayOperation(environment, index); }
		};


		struct ArrayLengthInstruction: Instruction {
			public: virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new ArrayLengthOperation(environment); }
		};


		struct AThrowInstruction: Instruction {
			public: virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new AThrowOperation(environment); }
		};


		struct CheckCastInstruction: InstructionWithIndex {
			CheckCastInstruction(uint16_t index): InstructionWithIndex(index) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new CheckCastOperation(environment, index); }
		};

		struct InstanceofInstruction: InstructionWithIndex {
			InstanceofInstruction(uint16_t index): InstructionWithIndex(index) {}
			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new InstanceofOperation(environment, index); }
		};


		struct MultiANewArrayInstruction: InstructionWithIndex {
			protected: const uint16_t dimensions;

			public: MultiANewArrayInstruction(uint16_t index, uint16_t dimensions): InstructionWithIndex(index), dimensions(dimensions) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new MultiANewArrayOperation(environment, index, dimensions); }
		};



		static IConstInstruction
				*const ICONST_M1 = new IConstInstruction(-1),
				*const ICONST_0 = new IConstInstruction(0),
				*const ICONST_1 = new IConstInstruction(1),
				*const ICONST_2 = new IConstInstruction(2),
				*const ICONST_3 = new IConstInstruction(3),
				*const ICONST_4 = new IConstInstruction(4),
				*const ICONST_5 = new IConstInstruction(5);
		static LConstInstruction
				*const LCONST_0 = new LConstInstruction(0),
				*const LCONST_1 = new LConstInstruction(1);
		static FConstInstruction
				*const FCONST_0 = new FConstInstruction(0),
				*const FCONST_1 = new FConstInstruction(1),
				*const FCONST_2 = new FConstInstruction(2);
		static DConstInstruction
				*const DCONST_0 = new DConstInstruction(0),
				*const DCONST_1 = new DConstInstruction(1);
	}
}

#endif
