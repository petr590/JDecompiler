#ifndef JDECOMPILER_STORE_INSTRUCTIONS_CPP
#define JDECOMPILER_STORE_INSTRUCTIONS_CPP

namespace jdecompiler {

	struct StoreInstruction: InstructionWithIndex {
		public: StoreInstruction(uint16_t index): InstructionWithIndex(index) {}
	};

	struct IStoreInstruction: StoreInstruction {
		IStoreInstruction(uint16_t index): StoreInstruction(index) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new IStoreOperation(context, index); }
	};

	struct LStoreInstruction: StoreInstruction {
		LStoreInstruction(uint16_t index): StoreInstruction(index) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new LStoreOperation(context, index); }
	};

	struct FStoreInstruction: StoreInstruction {
		FStoreInstruction(uint16_t index): StoreInstruction(index) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new FStoreOperation(context, index); }
	};

	struct DStoreInstruction: StoreInstruction {
		DStoreInstruction(uint16_t index): StoreInstruction(index) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new DStoreOperation(context, index); }
	};

	struct AStoreInstruction: StoreInstruction {
		AStoreInstruction(uint16_t index): StoreInstruction(index) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new AStoreOperation(context, index); }
	};
}

#endif
