#ifndef JDECOMPILER_NUMBER_CONST_INSTRUCTION_CONSTENTS_INSTRUCTION_CPP
#define JDECOMPILER_NUMBER_CONST_INSTRUCTION_CONSTENTS_INSTRUCTION_CPP

namespace jdecompiler {

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
