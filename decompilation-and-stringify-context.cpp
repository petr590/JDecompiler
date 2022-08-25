#ifndef JDECOMPILER_DECOMPILATION_AND_STRINGIFY_CONTEXT_CPP
#define JDECOMPILER_DECOMPILATION_AND_STRINGIFY_CONTEXT_CPP

#include "disassembler-context.cpp"

namespace jdecompiler {

	struct DecompilationAndStringifyContext: Context {
		protected:
			const DisassemblerContext& disassemblerContext;
			mutable const Scope* currentScope;

		public:
			const ClassInfo& classinfo;
			const ConstantPool& constPool;
			MethodScope& methodScope;

			const modifiers_t modifiers;
			const MethodDescriptor& descriptor;
			const Attributes& attributes;

		protected:
			mutable vector<const Scope*> inactiveScopes;

			DecompilationAndStringifyContext(const DisassemblerContext& disassemblerContext, const ClassInfo& classinfo,
					MethodScope& methodScope, modifiers_t modifiers, const MethodDescriptor& descriptor, const Attributes& attributes):
					disassemblerContext(disassemblerContext), currentScope((const Scope*)&methodScope), classinfo(classinfo), constPool(classinfo.constPool),
					methodScope(methodScope), modifiers(modifiers), descriptor(descriptor), attributes(attributes) {}

		public:
			inline const Scope* getCurrentScope() const {
				return currentScope;
			}

			inline void addScope(const Scope* scope) const {
				inactiveScopes.push_back(scope);
			}

			inline index_t posToIndex(pos_t pos) const {
				return disassemblerContext.posToIndex(pos);
			}

			inline pos_t indexToPos(index_t index) const {
				return disassemblerContext.indexToPos(index);
			}

			inline const Instruction* getInstruction(index_t index) const {
				return disassemblerContext.getInstruction(index);
			}

			inline const Instruction* getInstructionNoexcept(index_t index) const {
				return disassemblerContext.getInstructionNoexcept(index);
			}


			template<typename... Args>
			inline void warning(Args... args) const {
				print(cerr << descriptor.toString() << ':' << pos << ": warning: ", args...);
			}
	};
}

#endif
