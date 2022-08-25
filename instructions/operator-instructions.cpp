#ifndef JDECOMPILER_OPERATOR_INSTRUCTIONS_CPP
#define JDECOMPILER_OPERATOR_INSTRUCTIONS_CPP

namespace jdecompiler {

	template<char32_t operation, Priority priority, bool canUseBoolean = false>
	struct OperatorInstruction: Instruction {
		protected:
			const Type *const type;

			static inline const Type* getTypeByCode(uint16_t code) {
				switch(code) {
					case 0: return canUseBoolean ? ANY_INT_OR_BOOLEAN : ANY_INT;
					case 1: return LONG;
					case 2: return FLOAT;
					case 3: return DOUBLE;
					default: throw Exception("Illegal type code " + hexWithPrefix(code));
				}
			}

		public:
			OperatorInstruction(uint16_t typeCode): type(getTypeByCode(typeCode)) {}
	};


	template<char32_t operation, Priority priority, bool canUseBoolean = false>
	struct BinaryOperatorInstruction: OperatorInstruction<operation, priority, canUseBoolean> {
		BinaryOperatorInstruction(uint16_t typeCode): OperatorInstruction<operation, priority, canUseBoolean>(typeCode) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override {
			return new BinaryOperatorOperationImpl<operation, priority>(this->type, context);
		}
	};


	template<char32_t operation, Priority priority>
	struct ShiftOperatorInstruction: BinaryOperatorInstruction<operation, priority> {
		ShiftOperatorInstruction(uint16_t typeCode): BinaryOperatorInstruction<operation, priority>(typeCode) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override {
			return new BinaryOperatorOperationImpl<operation, priority>(this->type, INT, context);
		}
	};

	template<char32_t operation, Priority priority>
	using BooleanBinaryOperatorInstruction = BinaryOperatorInstruction<operation, priority, true>;



	template<char32_t operation, Priority priority>
	struct UnaryOperatorInstruction: OperatorInstruction<operation, priority> {
		UnaryOperatorInstruction(uint16_t typeCode): OperatorInstruction<operation, priority>(typeCode) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override {
			return new UnaryOperatorOperation<operation, priority>(this->type, context);
		}
	};

	using AddOperatorInstruction = BinaryOperatorInstruction<'+', Priority::PLUS>;
	using SubOperatorInstruction = BinaryOperatorInstruction<'-', Priority::MINUS>;
	using MulOperatorInstruction = BinaryOperatorInstruction<'*', Priority::MULTIPLE>;
	using DivOperatorInstruction = BinaryOperatorInstruction<'/', Priority::DIVISION>;
	using RemOperatorInstruction = BinaryOperatorInstruction<'%', Priority::REMAINDER>;

	using NegOperatorInstruction = UnaryOperatorInstruction<'-', Priority::UNARY_MINUS>;

	using ShiftLeftOperatorInstruction = ShiftOperatorInstruction<"<<"_c32, Priority::SHIFT>;
	using ShiftRightOperatorInstruction = ShiftOperatorInstruction<">>"_c32, Priority::SHIFT>;
	using UShiftRightOperatorInstruction = ShiftOperatorInstruction<">>>"_c32, Priority::SHIFT>;

	using AndOperatorInstruction = BooleanBinaryOperatorInstruction<'&', Priority::BIT_AND>;
	using OrOperatorInstruction  = BooleanBinaryOperatorInstruction<'|', Priority::BIT_OR>;
	using XorOperatorInstruction = BooleanBinaryOperatorInstruction<'^', Priority::BIT_XOR>;
}

#endif
