#ifndef JDECOMPILER_STRINGIFY_CONTEXT_CPP
#define JDECOMPILER_STRINGIFY_CONTEXT_CPP

namespace jdecompiler {

	struct StringifyContext {
		public:
			const ClassInfo& classinfo;
			const ConstantPool& constPool;
			CodeStack& stack;
			MethodScope& methodScope;

		private:
			const DisassemblerContext& disassemblerContext;
			mutable const Scope* currentScope;

		public:
			const uint16_t modifiers;
			const MethodDescriptor& descriptor;
			const Attributes& attributes;
			pos_t pos = 0;
			index_t index = 0, exprStartIndex = 0;
			map<uint32_t, index_t> exprIndexTable;

		private:
			mutable vector<const Scope*> inactiveScopes;

		public:
			StringifyContext(const DisassemblerContext& disassemblerContext, const ClassInfo& classinfo, MethodScope* methodScope, uint16_t modifiers,
					const MethodDescriptor& descriptor, const Attributes& attributes, uint16_t maxLocals):
					disassemblerContext(disassemblerContext), classinfo(classinfo), constPool(classinfo.constPool), stack(*new CodeStack()),
					methodScope(*methodScope), currentScope((const Scope*)methodScope), modifiers(modifiers), descriptor(descriptor), attributes(attributes) {}

			StringifyContext(const StringifyContext&) = delete;

			StringifyContext& operator=(const StringifyContext&) = delete;

			/*bool addOperation(const Operation* operation) {
				if(operation == nullptr)
					return false;

				//LOG(typeNameOf(operation));

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
				delete &stack;
			}

		protected:
			template<typename Arg, typename... Args>
			inline void print(ostream& out, Arg arg, Args... args) const {
				if constexpr(sizeof...(Args) != 0)
					print(out << arg, args...);
				else
					out << arg << endl;
			}

		public:
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
