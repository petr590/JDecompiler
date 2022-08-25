#ifndef JDECOMPILER_ARRAY_LOAD_INSTRUCTIONS_CPP
#define JDECOMPILER_ARRAY_LOAD_INSTRUCTIONS_CPP

namespace jdecompiler {

	struct ArrayLoadInstruction: Instruction {};

	struct IALoadInstruction: ArrayLoadInstruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new IALoadOperation(context); }
	};

	struct LALoadInstruction: ArrayLoadInstruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new LALoadOperation(context); }
	};

	struct FALoadInstruction: ArrayLoadInstruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new FALoadOperation(context); }
	};

	struct DALoadInstruction: ArrayLoadInstruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new DALoadOperation(context); }
	};

	struct AALoadInstruction: ArrayLoadInstruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new AALoadOperation(context); }
	};

	struct BALoadInstruction: ArrayLoadInstruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new BALoadOperation(context); }
	};

	struct CALoadInstruction: ArrayLoadInstruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new CALoadOperation(context); }
	};

	struct SALoadInstruction: ArrayLoadInstruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new SALoadOperation(context); }
	};
}

#endif
