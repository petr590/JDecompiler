#ifndef JDECOMPILER_STRINGIFY_CONTEXT_CPP
#define JDECOMPILER_STRINGIFY_CONTEXT_CPP

#include "decompilation-and-stringify-context.cpp"

namespace jdecompiler {

	struct StringifyContext final: DecompilationAndStringifyContext {

		public:
			const map<uint32_t, index_t>& exprIndexTable;

		protected:
			StringifyContext(const DisassemblerContext& disassemblerContext, const ClassInfo& classinfo, MethodScope& methodScope,
					modifiers_t modifiers, const MethodDescriptor& descriptor, const Attributes& attributes, const map<uint32_t, index_t>& exprIndexTable):
					DecompilationAndStringifyContext(disassemblerContext, classinfo,
					methodScope, modifiers, descriptor, attributes), exprIndexTable(exprIndexTable) {}

		public:
			StringifyContext(const DisassemblerContext& disassemblerContext, const ClassInfo& classinfo, MethodScope* methodScope,
					modifiers_t modifiers, const MethodDescriptor& descriptor, const Attributes& attributes):
					StringifyContext(disassemblerContext, classinfo, *methodScope, modifiers, descriptor, attributes, *new map<uint32_t, index_t>()) {}

			StringifyContext(const DecompilationContext& context):
					StringifyContext(context.disassemblerContext, context.classinfo, context.methodScope,
					context.modifiers, context.descriptor, context.attributes, context.exprIndexTable) {}


			void enterScope(const Scope*) const;

			void exitScope(const Scope*) const;
	};
}

#endif
