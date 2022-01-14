#ifndef JDECOMPILER_INSTRUCTIONS_CPP
#define JDECOMPILER_INSTRUCTIONS_CPP

namespace Instructions {
	using namespace Operations;

	struct InstructionWithIndex: Instruction {
		protected:
			const uint16_t index;
			InstructionWithIndex(uint16_t index): index(index) {}
	};


	struct InstructionAndOperation: Instruction, Operation {
		public: virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return this; }
	};

	struct VoidInstructionAndOperation: InstructionAndOperation {
		public: virtual const Type* getReturnType() const override { return VOID; }
	};


	struct AConstNull: VoidInstructionAndOperation {
		private: AConstNull() {}

		public:
			virtual string toString(const CodeEnvironment& environment) const override { return "null"; }

			static inline AConstNull& getInstance() {
				static AConstNull instance;
				return instance;
			}
	};

	template<typename T>
	struct NumberConstInstruction: Instruction {
		const T value;

		NumberConstInstruction(T value): value(value) {}
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
		protected: const T value;

		public:
			IPushInstruction(T value): value(value) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new IPushOperation<T>(value); }
	};

	using BIPushInstruction = IPushInstruction<int8_t>;

	using SIPushInstruction = IPushInstruction<int16_t>;


	struct LdcInstruction: InstructionWithIndex {
		LdcInstruction(uint16_t index): InstructionWithIndex(index) {}

		virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new LdcOperation(environment, index); }
	};

	struct Ldc2Instruction: InstructionWithIndex {
		Ldc2Instruction(uint16_t index): InstructionWithIndex(index) {}

		virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new Ldc2Operation(environment, index); }
	};


	struct LoadInstruction: InstructionWithIndex {
		public: LoadInstruction(uint16_t index): InstructionWithIndex(index) {}
	};

	struct ILoadInstruction: LoadInstruction {
		ILoadInstruction(uint16_t index): LoadInstruction(index) {}

		virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new ILoadOperation(environment, index); }
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


	struct PopInstruction: Instruction {
		virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new PopOperation(environment); }
	};

	struct DupInstruction: Instruction {
		virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new DupOperation(environment); }
	};

	struct SwapInstruction: Instruction {
		virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new SwapOperation(environment); }
	};


	static const Type* getTypeByCode(uint16_t code) {
		switch(code) {
			case 0: return INT;
			case 1: return LONG;
			case 2: return FLOAT;
			case 3: return DOUBLE;
			default: throw Exception("Illegal type code: 0x" + hex(code));
		}
	}


	template<char32_t operation, uint16_t priority>
	struct OperatorInstruction: Instruction {
		protected:
			const Type *const type;

		public: OperatorInstruction(uint16_t typeCode): type(getTypeByCode(typeCode)) {}
	};


	template<char32_t operation, uint16_t priority>
	struct BinaryOperatorInstruction: OperatorInstruction<operation, priority> {
		BinaryOperatorInstruction(uint16_t typeCode): OperatorInstruction<operation, priority>(typeCode) {}

		virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
			return new BinaryOperatorOperation(OperatorInstruction<operation, priority>::type, environment, operation, priority);
		}
	};


	template<char32_t operation, uint16_t priority>
	struct UnaryOperatorInstruction: OperatorInstruction<operation, priority> {
		UnaryOperatorInstruction(uint16_t typeCode): OperatorInstruction<operation, priority>(typeCode) {}

		virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
			return new UnaryOperatorOperation(OperatorInstruction<operation, priority>::type, environment, operation, priority);
		}
	};

	using AddOperatorInstruction = BinaryOperatorInstruction<'+', 11>;

	using SubOperatorInstruction = BinaryOperatorInstruction<'-', 11>;

	using MulOperatorInstruction = BinaryOperatorInstruction<'*', 12>;

	using DivOperatorInstruction = BinaryOperatorInstruction<'/', 12>;

	using RemOperatorInstruction = BinaryOperatorInstruction<'%', 12>;

	using NegOperatorInstruction = UnaryOperatorInstruction<'-', 13>;

	using ShiftLeftOperatorInstruction = BinaryOperatorInstruction<'<<', 10>;

	using ShiftRightOperatorInstruction = BinaryOperatorInstruction<'>>', 10>;

	using UShiftRightOperatorInstruction = BinaryOperatorInstruction<'>>>', 10>;

	using AndOperatorInstruction = BinaryOperatorInstruction<'&', 7>;

	using OrOperatorInstruction = BinaryOperatorInstruction<'|', 5>;

	using XorOperatorInstruction = BinaryOperatorInstruction<'^', 6>;



	struct IIncInstruction: InstructionWithIndex {
		const int16_t value;

		IIncInstruction(uint16_t index, int16_t value): InstructionWithIndex(index), value(value) {}

		virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new IIncOperation(environment, index, value); }
	};


	template<bool required>
	struct CastInstruction: Instruction {
		protected:
			const Type *const type;

		public: CastInstruction(const Type* type): type(type) {}

		virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new CastOperation<required>(environment, type); }
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


	struct IfInstruction: Instruction {
		const int16_t offset;

		IfInstruction(const int32_t offset): offset(offset) {}
	};

	struct IfCmpInstruction: IfInstruction {
		const CompareType& compareType;

		IfCmpInstruction(const int16_t offset, const CompareType& compareType): IfInstruction(offset), compareType(compareType) {}

		virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
			return new IfCmpScope(environment, offset, compareType);
		}
	};


	struct IfEqInstruction: IfCmpInstruction {
		IfEqInstruction(const int16_t offset): IfCmpInstruction(offset, CompareType::EQUALS) {};
	};

	struct IfNotEqInstruction: IfCmpInstruction {
		IfNotEqInstruction(const int16_t offset): IfCmpInstruction(offset, CompareType::NOT_EQUALS) {}
	};

	struct IfGtInstruction: IfCmpInstruction {
		IfGtInstruction(const int16_t offset): IfCmpInstruction(offset, CompareType::GREATER) {}
	};

	struct IfGeInstruction: IfCmpInstruction {
		IfGeInstruction(const int16_t offset): IfCmpInstruction(offset, CompareType::GREATER_OR_EQUALS) {}
	};

	struct IfLtInstruction: IfCmpInstruction {
		IfLtInstruction(const int16_t offset): IfCmpInstruction(offset, CompareType::LESS) {}
	};

	struct IfLeInstruction: IfCmpInstruction {
		IfLeInstruction(const int16_t offset): IfCmpInstruction(offset, CompareType::LESS_OR_EQUALS) {}
	};


	struct IfICmpInstruction: IfCmpInstruction {
		IfICmpInstruction(int16_t offset, const CompareType& compareType): IfCmpInstruction(offset, compareType) {}

		virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
			environment.stack.push(new ICmpOperation(environment));
			return new IfCmpScope(environment, offset, compareType);
		}
	};


	struct IfIEqInstruction: IfICmpInstruction {
		IfIEqInstruction(const int16_t offset): IfICmpInstruction(offset, CompareType::EQUALS) {}
	};

	struct IfINotEqInstruction: IfICmpInstruction {
		IfINotEqInstruction(const int16_t offset): IfICmpInstruction(offset, CompareType::NOT_EQUALS) {}
	};

	struct IfIGtInstruction: IfICmpInstruction {
		IfIGtInstruction(const int16_t offset): IfICmpInstruction(offset, CompareType::GREATER) {}
	};

	struct IfIGeInstruction: IfICmpInstruction {
		IfIGeInstruction(const int16_t offset): IfICmpInstruction(offset, CompareType::GREATER_OR_EQUALS) {}
	};

	struct IfILtInstruction: IfICmpInstruction {
		IfILtInstruction(const int16_t offset): IfICmpInstruction(offset, CompareType::LESS) {}
	};

	struct IfILeInstruction: IfICmpInstruction {
		IfILeInstruction(const int16_t offset): IfICmpInstruction(offset, CompareType::LESS_OR_EQUALS) {}
	};


	struct IfACmpInstruction: IfCmpInstruction {
		IfACmpInstruction(int16_t offset, const EqualsCompareType& compareType): IfCmpInstruction(offset, compareType) {}

		virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
			environment.stack.push(new ACmpOperation(environment));
			return new IfCmpScope(environment, offset, compareType);
		}
	};


	struct IfAEqInstruction: IfACmpInstruction {
		IfAEqInstruction(const int16_t offset): IfACmpInstruction(offset, CompareType::EQUALS) {}
	};

	struct IfANotEqInstruction: IfACmpInstruction {
		IfANotEqInstruction(const int16_t offset): IfACmpInstruction(offset, CompareType::NOT_EQUALS) {}
	};


	struct GotoInstruction: Instruction {
		const int32_t offset;

		GotoInstruction(const int32_t offset): offset(offset) {}

		virtual inline const Operation* toOperation(const CodeEnvironment& environment) const override {
			if(offset == 0) return new EmptyInfiniteLoopScope(environment);

			const uint32_t index = environment.bytecode.posToIndex(offset + environment.pos);

			const IfScope* ifScope = dynamic_cast<const IfScope*>(environment.getCurrentScope());

			if(ifScope != nullptr && environment.index == ifScope->to) {
				if(offset > 0)
					return new ElseScope(environment, min(index, ifScope->parentScope->to) - 1, ifScope);

				if(index == ifScope->from) {
					if(offset < 0) {
						ifScope->isLoop = true;
						return new ContinueOperation(/*ifScope*/);
					}
				}
			}

			throw DecompilationException("illegal using of goto instruction");
		}
	};


	struct LookupswitchInstruction: Instruction {
		protected:
			int32_t defaultOffset;
			map<int32_t, int32_t> offsetTable;

		public:
			LookupswitchInstruction(int32_t defaultOffset, map<int32_t, int32_t> offsetTable): defaultOffset(defaultOffset), offsetTable(offsetTable) {}

		virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
			return new SwitchScope(environment, defaultOffset, offsetTable);
		}
	};


	struct ReturnInstruction: Instruction {
		virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new ReturnOperation(environment); }
	};

	struct VReturn: VoidInstructionAndOperation {
		private: VReturn() {}

		public:
			virtual string toString(const CodeEnvironment& environment) const override { return "return"; }

			static inline VReturn& getInstance() {
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
		InvokeinterfaceInstruction(uint16_t index): InvokeInstruction(index) {}

		virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
			return new InvokeinterfaceOperation(environment, index);
		}
	};


	struct InvokedynamicInstruction: InvokeInstruction {
		InvokedynamicInstruction(uint16_t index): InvokeInstruction(index) {}

		virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
			return new InvokedynamicOperation(environment, index);
		}
	};



	struct NewInstruction: InstructionWithIndex {
		NewInstruction(uint16_t classIndex): InstructionWithIndex(classIndex) {}

		virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new NewOperation(environment, index); }
	};


	static const PrimitiveType* getArrayTypeByCode(uint8_t code) {
		switch(code) {
			case 0x4: return BOOLEAN;
			case 0x5: return CHAR;
			case 0x6: return FLOAT;
			case 0x7: return DOUBLE;
			case 0x8: return BYTE;
			case 0x9: return SHORT;
			case 0xA: return INT;
			case 0xB: return LONG;
			default: throw Exception("Illegal array type code: 0x" + hex(code));
		}
	}


	struct NewArrayInstruction: Instruction {
		protected: const PrimitiveType *const memberType;
		public: NewArrayInstruction(uint8_t code): memberType(getArrayTypeByCode(code)) {}

		virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new NewArrayOperation(environment, memberType); }
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


	struct IfNullInstruction: IfInstruction {
		IfNullInstruction(const uint16_t offset): IfInstruction(offset) {}

		virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new IfNullScope(environment, offset); }
	};

	struct IfNonNullInstruction: IfInstruction {
		IfNonNullInstruction(const uint16_t offset): IfInstruction(offset) {}

		virtual const Operation* toOperation(const CodeEnvironment& environment) const override { return new IfNonNullScope(environment, offset); }
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

#endif
