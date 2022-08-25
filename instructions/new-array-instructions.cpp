#ifndef JDECOMPILER_NEW_ARRAY_INSTRUCTIONS_CPP
#define JDECOMPILER_NEW_ARRAY_INSTRUCTIONS_CPP

namespace jdecompiler {

	static const Type* getArrayTypeByCode(uint8_t code) {
		switch(code) {
			case 0x4: return BOOLEAN;
			case 0x5: return CHAR;
			case 0x6: return FLOAT;
			case 0x7: return DOUBLE;
			case 0x8: return BYTE;
			case 0x9: return SHORT;
			case 0xA: return INT;
			case 0xB: return LONG;
			default: throw DecompilationException("Illegal array type code " + hexWithPrefix(code));
		}
	}


	struct NewArrayInstruction: Instruction {
		protected:
			const ArrayType *const arrayType;

		public:
			NewArrayInstruction(uint8_t code): arrayType(new ArrayType(getArrayTypeByCode(code))) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return new NewArrayOperation(context, arrayType);
			}
	};

	struct ANewArrayInstruction: InstructionWithIndex {
		ANewArrayInstruction(uint16_t index): InstructionWithIndex(index) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override { return new ANewArrayOperation(context, index); }
	};

	struct MultiANewArrayInstruction: InstructionWithIndex {
		protected: const uint16_t dimensions;

		public: MultiANewArrayInstruction(uint16_t index, uint16_t dimensions): InstructionWithIndex(index), dimensions(dimensions) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override {
			return new MultiANewArrayOperation(context, index, dimensions);
		}
	};
}

#endif
