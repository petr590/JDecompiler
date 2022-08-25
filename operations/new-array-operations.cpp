#ifndef JDECOMPILER_NEW_ARRAY_OPERATIONS_CPP
#define JDECOMPILER_NEW_ARRAY_OPERATIONS_CPP

namespace jdecompiler {

	struct NewArrayOperation: Operation {
		protected:
			const ArrayType* const arrayType;
			vector<const Operation*> lengths;

			mutable vector<const Operation*> initializer;
			friend struct ArrayStoreOperation;

		public:
			NewArrayOperation(const DecompilationContext& context, const ArrayType* arrayType):
				NewArrayOperation(context, arrayType, 1) {}

		protected:
			NewArrayOperation(const DecompilationContext& context, const ArrayType* arrayType, uint16_t dimensions):
					arrayType(arrayType), lengths(arrayType->nestingLevel) {

				if(dimensions > arrayType->nestingLevel)
					throw DecompilationException("Instruction newarray (or another derivative of it)"
							"has too many dimensions (" + to_string(dimensions) + ") for its array type " + arrayType->toString());

				for(uint16_t i = dimensions; i > 0; )
					lengths[--i] = context.stack.popAs(INT);
			}


			virtual const Type* getReturnType() const override {
				return arrayType;
			}

		protected:
			bool canInitAsList() const {
				return !initializer.empty() || ((lengths.size() == 1 || (lengths.size() > 1 && lengths[1] == nullptr)) &&
						instanceof<const IConstOperation*>(lengths[0]) && static_cast<const IConstOperation*>(lengths[0])->value == 0);
			}

		public:
			virtual string toString(const StringifyContext& context) const override {
				return canInitAsList() ? "new " + arrayType->toString(context.classinfo) + ' ' + toArrayInitString(context) : toArrayInitString(context) ;
			}

			virtual string toArrayInitString(const StringifyContext& context) const override {

				if(canInitAsList()) {
					const bool useSpaces = arrayType->nestingLevel == 1 && !initializer.empty();
					return (useSpaces ? "{ " : "{") + join<const Operation*>(initializer,
							[&context] (const Operation* element) { return element->toArrayInitString(context); }) + (useSpaces ? " }" : "}");
				}

				return "new " + arrayType->memberType->toString(context.classinfo) + join<const Operation*>(lengths,
							[&context] (auto length) { return length == nullptr ? "[]" : '[' + length->toString(context) + ']'; }, EMPTY_STRING);
			}
	};

	struct ANewArrayOperation: NewArrayOperation {
		ANewArrayOperation(const DecompilationContext& context, uint16_t index):
				NewArrayOperation(context, new ArrayType(parseReferenceType(context.constPool.get<ClassConstant>(index)->name))) {}
	};


	struct MultiANewArrayOperation: NewArrayOperation {
		MultiANewArrayOperation(const DecompilationContext& context, uint16_t index, uint16_t dimensions):
				NewArrayOperation(context, safe_cast<const ArrayType*>(parseReferenceType(context.constPool.get<ClassConstant>(index)->name)),
				dimensions) {
			if(dimensions > arrayType->nestingLevel) {
				throw DecompilationException("The nesting level of the multianewarray instruction (" + to_string(dimensions) + ")"
						" greater than nesting level of the array type " + arrayType->toString());
			}
		}
	};
}

#endif
