#ifndef JDECOMPILER_INSTRUCTIONS_CPP
#define JDECOMPILER_INSTRUCTIONS_CPP

#include "operations.cpp"

namespace jdecompiler {

	struct InstructionWithIndex: Instruction {
		public:
			const uint16_t index;

		protected:
			InstructionWithIndex(uint16_t index): index(index) {}
	};


	struct InstructionAndOperation: Instruction, Operation {
		public: virtual const Operation* toOperation(const DecompilationContext&) const override { return this; }
	};

	struct VoidInstructionAndOperation: InstructionAndOperation {
		public: virtual const Type* getReturnType() const override final { return VOID; }
	};

}

#include "instructions/aconst-null.cpp"
#include "instructions/number-const-instructions.cpp"
#include "instructions/ipush-instructions.cpp"
#include "instructions/ldc.cpp"

#include "instructions/load-instructions.cpp"
#include "instructions/array-load-instructions.cpp"

#include "instructions/store-instructions.cpp"
#include "instructions/array-store-instructions.cpp"

#include "instructions/pop.cpp"
#include "instructions/dup.cpp"
#include "instructions/swap.cpp"

#include "instructions/operator-instructions.cpp"
#include "instructions/iinc.cpp"
#include "instructions/cast-instructions.cpp"
#include "instructions/cmp-instructions.cpp"

#include "blocks.cpp"
#include "block-instructions.cpp"

#include "instructions/field-instructions.cpp"
#include "instructions/invoke-instructions.cpp"
#include "instructions/new.cpp"
#include "instructions/new-array-instructions.cpp"
#include "instructions/array-length.cpp"
#include "instructions/athrow.cpp"
#include "instructions/instanceof.cpp"
#include "instructions/return-instructions.cpp"
#include "instructions/number-const-instruction-constents.cpp"

#endif
