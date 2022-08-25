#ifndef JDECOMPILER_ARRAY_STORE_INSTRUCTIONS_CPP
#define JDECOMPILER_ARRAY_STORE_INSTRUCTIONS_CPP

namespace jdecompiler {

	struct ArrayStoreInstruction: Instruction {};

	struct IAStoreInstruction: ArrayStoreInstruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new IAStoreOperation(context); }
	};

	struct LAStoreInstruction: ArrayStoreInstruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new LAStoreOperation(context); }
	};

	struct FAStoreInstruction: ArrayStoreInstruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new FAStoreOperation(context); }
	};

	struct DAStoreInstruction: ArrayStoreInstruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new DAStoreOperation(context); }
	};

	struct AAStoreInstruction: ArrayStoreInstruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new AAStoreOperation(context); }
	};

	struct BAStoreInstruction: ArrayStoreInstruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new BAStoreOperation(context); }
	};

	struct CAStoreInstruction: ArrayStoreInstruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new CAStoreOperation(context); }
	};

	struct SAStoreInstruction: ArrayStoreInstruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new SAStoreOperation(context); }
	};
}

#endif
