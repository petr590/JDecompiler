#ifndef JDECOMPILER_NEW_OPERATION_CPP
#define JDECOMPILER_NEW_OPERATION_CPP

namespace jdecompiler {
	struct NewOperation: Operation {
		public:
			const ClassType clazz;

			NewOperation(const DecompilationContext& context, const ClassConstant* classConstant):
					clazz(classConstant) {}

			NewOperation(const DecompilationContext& context, uint16_t classIndex):
					NewOperation(context, context.constPool.get<ClassConstant>(classIndex)) {}

			virtual string toString(const StringifyContext& context) const override {
				return "new " + clazz.toString(context.classinfo);
			}

			virtual const Type* getReturnType() const override {
				return &clazz;
			}
	};
}

#endif
