#ifndef JDECOMPILER_INSTRUCTION_H
#define JDECOMPILER_INSTRUCTION_H

namespace jdecompiler {

	struct Instruction {
		protected:
			constexpr Instruction() noexcept {}

		public:
			virtual ~Instruction() {}

			virtual const Operation* toOperation(const DecompilationContext&) const = 0;

			virtual const Block* toBlock(const DisassemblerContext&) const;
	};
}

#endif
