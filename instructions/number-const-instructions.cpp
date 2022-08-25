#ifndef JDECOMPILER_NUMBER_CONST_INSTRUCTIONS_CPP
#define JDECOMPILER_NUMBER_CONST_INSTRUCTIONS_CPP

namespace jdecompiler {

	template<typename T>
	struct NumberConstInstruction: Instruction {
		static_assert(is_fundamental<T>(), "Type T of struct NumberConstInstruction must be primitive");

		const T value;

		NumberConstInstruction(const T value): value(value) {}
	};


	struct IConstInstruction: NumberConstInstruction<jint> {
		IConstInstruction(jint value): NumberConstInstruction(value) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override {
			return new IConstOperation(value);
		}
	};

	struct LConstInstruction: NumberConstInstruction<jlong> {
		LConstInstruction(jlong value): NumberConstInstruction(value) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override {
			return new LConstOperation(value);
		}
	};

	struct FConstInstruction: NumberConstInstruction<jfloat> {
		FConstInstruction(jfloat value): NumberConstInstruction(value) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override {
			return new FConstOperation(value);
		}
	};

	struct DConstInstruction: NumberConstInstruction<jdouble> {
		DConstInstruction(jdouble value): NumberConstInstruction(value) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override {
			return new DConstOperation(value);
		}
	};
}

#endif
