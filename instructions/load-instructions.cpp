#ifndef JDECOMPILER_LOAD_INSTRUCTIONS_CPP
#define JDECOMPILER_LOAD_INSTRUCTIONS_CPP

namespace jdecompiler {

	struct LoadInstruction: InstructionWithIndex {
		LoadInstruction(uint16_t index): InstructionWithIndex(index) {}
	};

	struct ILoadInstruction: LoadInstruction {
		ILoadInstruction(uint16_t index): LoadInstruction(index) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override {
			return isNullOperation ? nullptr : new ILoadOperation(context, index);
		}

		private:
			friend IIncOperation::IIncOperation(const DecompilationContext&, uint16_t, int16_t);

			mutable bool isNullOperation = false;

			inline void setNullOperation() const noexcept {
				isNullOperation = true;
			}
	};

	struct LLoadInstruction: LoadInstruction {
		LLoadInstruction(uint16_t index): LoadInstruction(index) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new LLoadOperation(context, index); }
	};

	struct FLoadInstruction: LoadInstruction {
		FLoadInstruction(uint16_t index): LoadInstruction(index) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new FLoadOperation(context, index); }
	};

	struct DLoadInstruction: LoadInstruction {
		DLoadInstruction(uint16_t index): LoadInstruction(index) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new DLoadOperation(context, index); }
	};

	struct ALoadInstruction: LoadInstruction {
		ALoadInstruction(uint16_t index): LoadInstruction(index) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new ALoadOperation(context, index); }
	};
}

#endif
