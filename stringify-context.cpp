#ifndef JDECOMPILER_STRINGIFY_CONTEXT_CPP
#define JDECOMPILER_STRINGIFY_CONTEXT_CPP

namespace jdecompiler {

	struct StringifyContext: Context {
		private:
			const DisassemblerContext& disassemblerContext;

		public:
			const ClassInfo& classinfo;
			const ConstantPool& constPool;
			//CodeStack& stack;
			MethodScope& methodScope;

		private:
			mutable const Scope* currentScope;

		public:
			const uint16_t modifiers;
			const MethodDescriptor& descriptor;
			const Attributes& attributes;
			//index_t index = 0, exprStartIndex = 0;
			const map<uint32_t, index_t>& exprIndexTable;

		protected:
			mutable vector<const Scope*> inactiveScopes;

			StringifyContext(const DisassemblerContext& disassemblerContext, const ClassInfo& classinfo, MethodScope& methodScope,
					uint16_t modifiers, const MethodDescriptor& descriptor, const Attributes& attributes, const map<uint32_t, index_t>& exprIndexTable):
					disassemblerContext(disassemblerContext), classinfo(classinfo), constPool(classinfo.constPool),
					/*stack(*new CodeStack()),*/ methodScope(methodScope), currentScope((const Scope*)&methodScope),
					modifiers(modifiers), descriptor(descriptor), attributes(attributes), exprIndexTable(exprIndexTable) {}

		public:
			StringifyContext(const DisassemblerContext& disassemblerContext, const ClassInfo& classinfo, MethodScope* methodScope,
					uint16_t modifiers, const MethodDescriptor& descriptor, const Attributes& attributes):
					StringifyContext(disassemblerContext, classinfo, *methodScope, modifiers, descriptor, attributes, *new map<uint32_t, index_t>()) {}

			StringifyContext(const DecompilationContext& context):
					StringifyContext(context.disassemblerContext, context.classinfo, context.methodScope,
					context.modifiers, context.descriptor, context.attributes, context.exprIndexTable) {}

			StringifyContext(const StringifyContext&) = delete;

			StringifyContext& operator=(const StringifyContext&) = delete;

			/*bool addOperation(const Operation* operation) {
				if(operation == nullptr)
					return false;

				//log(typeNameOf(operation));

				bool status = false;

				if(operation->getReturnType() != VOID) {
					stack.push(operation);
				} else if(operation->canAddToCode() && !(index == instructions.size() - 1 && operation == &VReturn::getInstance())) {
					currentScope->addOperation(operation, *this);
					status = true;
				}

				if(instanceof<const Scope*>(operation))
					addScope(static_cast<const Scope*>(operation));

				return status;
			}*/


			void enterScope(const Scope* scope) const;

			void exitScope(const Scope* scope) const;

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

			~StringifyContext() {
				/*delete &stack;*/
			}


			template<typename... Args>
			inline void warning(Args... args) const {
				print(cerr << descriptor.toString() << ':' << pos << ": warning: ", args...);
			}
	};


	/*struct EmptyStringifyContext: StringifyContext {
		private:
			EmptyStringifyContext(const DisassemblerContext& disassemblerContext, const ClassInfo& classinfo, MethodScope* methodScope, uint16_t modifiers,
					const MethodDescriptor& descriptor, const Attributes& attributes, uint16_t maxLocals):
					StringifyContext()
	};*/
}

#endif
