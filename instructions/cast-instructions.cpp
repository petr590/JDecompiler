#ifndef JDECOMPILER_CAST_INSTRUCTIONS_CPP
#define JDECOMPILER_CAST_INSTRUCTIONS_CPP

namespace jdecompiler {

	struct CastInstruction: Instruction {
		protected:
			const Type *const requiredType, *const type;
			const bool required;

		public:
			CastInstruction(const Type* requiredType, const Type* type, bool required):
					requiredType(requiredType), type(type), required(required) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return new CastOperation(context, requiredType, type, required);
			}
	};


	struct CheckCastInstruction: InstructionWithIndex {
		CheckCastInstruction(uint16_t index): InstructionWithIndex(index) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new CheckCastOperation(context, index); }
	};
}

#endif
