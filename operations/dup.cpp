#ifndef JDECOMPILER_DUP_OPERATION_CPP
#define JDECOMPILER_DUP_OPERATION_CPP

namespace jdecompiler {

	template<TypeSize size>
	struct AbstractDupOperation: Operation, TypeSizeTemplatedOperation<size> {
		const Operation* const operation;

		AbstractDupOperation(const DecompilationContext& context): operation(context.stack.top()) {
			TypeSizeTemplatedOperation<size>::checkTypeSize(operation->getReturnType());
		}

		virtual string toString(const StringifyContext& context) const override {
			return operation->toString(context);
		}

		virtual const Type* getReturnType() const override {
			return operation->getReturnType();
		}

		virtual Priority getPriority() const override {
			return operation->getPriority();
		}

		virtual const Operation* getOriginalOperation() const override {
			return operation->getOriginalOperation();
		}

		virtual void addVariableName(const string& name) const override {
			operation->addVariableName(name);
		}

		virtual void allowImplicitCast() const override {
			operation->allowImplicitCast();
		}
	};

	template<TypeSize size>
	struct DupOperation: AbstractDupOperation<size> {
		DupOperation(const DecompilationContext& context): AbstractDupOperation<size>(context) {}
	};

	using Dup1Operation = DupOperation<TypeSize::FOUR_BYTES>;
	using Dup2Operation = DupOperation<TypeSize::EIGHT_BYTES>;


	struct DupX1Operation: AbstractDupOperation<TypeSize::FOUR_BYTES> {
		DupX1Operation(const DecompilationContext& context): AbstractDupOperation<TypeSize::FOUR_BYTES>(context) {
			if(context.stack.size() < 2)
				throw IllegalStackStateException("Too less operations on stack for dup_x1: required 2, got " + context.stack.size());

			const Operation
				*operation1 = context.stack.pop(),
				*operation2 = context.stack.pop();
			TypeSizeTemplatedOperation<TypeSize::FOUR_BYTES>::checkTypeSize(operation2->getReturnType());
			context.stack.push(operation1, operation2);
		}
	};


	struct DupX2Operation: AbstractDupOperation<TypeSize::FOUR_BYTES> {
		DupX2Operation(const DecompilationContext& context): AbstractDupOperation<TypeSize::FOUR_BYTES>(context) {
			if(context.stack.size() < 3)
				throw IllegalStackStateException("Too less operations on stack for dup_x2: required 3, got " + context.stack.size());

			const Operation
				*operation1 = context.stack.pop(),
				*operation2 = context.stack.pop(),
				*operation3 = context.stack.pop();

			checkTypeSize(operation2->getReturnType());
			checkTypeSize(operation3->getReturnType());

			context.stack.push(operation1, operation3, operation2);
		}
	};


	struct Dup2X1Operation: AbstractDupOperation<TypeSize::EIGHT_BYTES> {
		Dup2X1Operation(const DecompilationContext& context): AbstractDupOperation<TypeSize::EIGHT_BYTES>(context) {
			if(context.stack.size() < 2)
				throw IllegalStackStateException("Too less operations on stack for dup2_x1: required 2, got " + context.stack.size());

			const Operation
				*operation1 = context.stack.pop(),
				*operation2 = context.stack.pop();

			checkTypeSize<TypeSize::FOUR_BYTES>(operation2->getReturnType());

			context.stack.push(operation1, operation2);
		}
	};


	struct Dup2X2Operation: AbstractDupOperation<TypeSize::EIGHT_BYTES> {
		Dup2X2Operation(const DecompilationContext& context): AbstractDupOperation<TypeSize::EIGHT_BYTES>(context) {
			if(context.stack.size() < 2)
				throw IllegalStackStateException("Too less operations on stack for dup2_x2: required 2, got " + context.stack.size());

			const Operation
				*operation1 = context.stack.pop(),
				*operation2 = context.stack.pop();

			checkTypeSize(operation2->getReturnType());

			context.stack.push(operation1, operation2);
		}
	};
}

#endif
