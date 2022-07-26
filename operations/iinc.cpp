#ifndef JDECOMPILER_IINC_OPERATION_CPP
#define JDECOMPILER_IINC_OPERATION_CPP

namespace jdecompiler::operations {

	struct IIncOperation: Operation {
		public:
			const Variable& variable;
			const int16_t value;

		protected:
			const Type* returnType;
			bool isShortInc, isPostInc = false;

		public:
			IIncOperation(const DecompilationContext&, uint16_t index, int16_t value);

			virtual string toString(const StringifyContext& context) const override {
				if(isShortInc) {
					const char* inc = value == 1 ? "++" : "--";
					return isPostInc || returnType == VOID ? context.getCurrentScope()->getNameFor(variable) + inc :
							inc + context.getCurrentScope()->getNameFor(variable);
				}
				return context.getCurrentScope()->getNameFor(variable) + (value < 0 ? " -" : " +") + "= " + to_string(abs(value));
			}

			virtual const Type* getReturnType() const override {
				return returnType;
			}

			virtual Priority getPriority() const override {
				return isShortInc ? (isPostInc ? Priority::POST_INCREMENT : Priority::PRE_INCREMENT) : Priority::ASSIGNMENT;
			}

			virtual bool isIncrement() const override {
				return true;
			}
	};
}

#endif
