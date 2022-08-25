#ifndef JDECOMPILER_IPUSH_INSTRUCTIONS_CPP
#define JDECOMPILER_IPUSH_INSTRUCTIONS_CPP

namespace jdecompiler {

	template<typename T>
	struct IPushInstruction: Instruction {
		public:
			const T value;

			IPushInstruction(T value): value(value) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return new IConstOperation(value);
			}
	};

	using BIPushInstruction = IPushInstruction<jbyte>;
	using SIPushInstruction = IPushInstruction<jshort>;
}

#endif
