#ifndef JDECOMPILER_RETURN_INSTRUCTION_CPP
#define JDECOMPILER_RETURN_INSTRUCTION_CPP

namespace jdecompiler {

	template<class ReturnOperation>
	struct ReturnInstruction: Instruction {
		virtual const Operation* toOperation(const DecompilationContext& context) const override {
			return new ReturnOperation(context);
		}
	};

	using IReturnInstruction = ReturnInstruction<IReturnOperation>;
	using LReturnInstruction = ReturnInstruction<LReturnOperation>;
	using FReturnInstruction = ReturnInstruction<FReturnOperation>;
	using DReturnInstruction = ReturnInstruction<DReturnOperation>;
	using AReturnInstruction = ReturnInstruction<AReturnOperation>;

	struct VReturn: VoidInstructionAndOperation {
		private:
			constexpr VReturn() noexcept {}

		public:
			virtual string toString(const StringifyContext&) const override { return "return"; }

			static VReturn* getInstance() {
				static VReturn instance;
				return &instance;
			}
	};
}

#endif
