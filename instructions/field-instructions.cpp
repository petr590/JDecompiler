#ifndef JDECOMPILER_FIELD_INSTRUCTIONS_CPP
#define JDECOMPILER_FIELD_INSTRUCTIONS_CPP

namespace jdecompiler {

	struct FieldInstruction: Instruction {
		const ClassType clazz;
		const FieldDescriptor descriptor;

		FieldInstruction(const FieldrefConstant* fieldref):
			clazz(fieldref->clazz), descriptor(fieldref->nameAndType) {}

		FieldInstruction(const DisassemblerContext& context, uint16_t index):
			FieldInstruction(context.constPool.get<FieldrefConstant>(index)) {}
	};


	struct GetStaticFieldInstruction: FieldInstruction {
		GetStaticFieldInstruction(const DisassemblerContext& context, uint16_t index): FieldInstruction(context, index) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override {
			return new GetStaticFieldOperation(clazz, descriptor);
		}
	};


	struct PutStaticFieldInstruction: FieldInstruction {
		PutStaticFieldInstruction(const DisassemblerContext& context, uint16_t index): FieldInstruction(context, index) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override {
			return new PutStaticFieldOperation(context, clazz, descriptor);
		}
	};


	struct GetInstanceFieldInstruction: FieldInstruction {
		GetInstanceFieldInstruction(const DisassemblerContext& context, uint16_t index): FieldInstruction(context, index) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override {
			return new GetInstanceFieldOperation(context, clazz, descriptor);
		}
	};


	struct PutInstanceFieldInstruction: FieldInstruction {
		PutInstanceFieldInstruction(const DisassemblerContext& context, uint16_t index): FieldInstruction(context, index) {}

		virtual const Operation* toOperation(const DecompilationContext& context) const override {
			return new PutInstanceFieldOperation(context, clazz, descriptor);
		}
	};
}

#endif
