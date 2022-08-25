#ifndef JDECOMPILER_DECOMPILATION_CONTEXT_CPP
#define JDECOMPILER_DECOMPILATION_CONTEXT_CPP

#include "decompilation-and-stringify-context.cpp"

namespace jdecompiler {

	struct DecompilationContext final: DecompilationAndStringifyContext {
			friend struct StringifyContext;

		public:
			CodeStack& stack;

			index_t index = 0, exprStartIndex = 0;
			map<uint32_t, index_t> exprIndexTable;

			DecompilationContext(const DisassemblerContext&, const ClassInfo&,
					MethodScope*, modifiers_t, const MethodDescriptor&, const Attributes&, uint16_t maxLocals);


			void updateScopes();

			~DecompilationContext() {
				delete &stack;
			}
	};
}

#endif
