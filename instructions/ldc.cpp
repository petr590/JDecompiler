#ifndef JDECOMPILER_LDC_INSTRUCTION_CPP
#define JDECOMPILER_LDC_INSTRUCTION_CPP

namespace jdecompiler {

	template<TypeSize size>
	struct LdcInstruction: InstructionWithIndex {
		public:
			LdcInstruction(const DisassemblerContext& context, uint16_t index): InstructionWithIndex(index) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return context.constPool.get<ConstValueConstant>(index)->toOperation();
			}
	};
}

#endif
