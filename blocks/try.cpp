#ifndef JDECOMPILER_TRY_BLOCK_CPP
#define JDECOMPILER_TRY_BLOCK_CPP

namespace jdecompiler {

	struct TryBlock: Block {
		protected:
			mutable vector<CatchBlock*> handlers;
			friend const StringifyContext& Method::decompileCode(const ClassInfo&);
			friend struct GotoInstruction;

		public:
			TryBlock(index_t startIndex, index_t endIndex): Block(startIndex, endIndex) {}

			virtual const Scope* toScope(const DecompilationContext& context) const override {
				return new TryScope(context, startIndex, endIndex);
			}


			void linkCatchBlocks(const DisassemblerContext& context) const {
				/*assert(handlers.size() > 0);
				sort(handlers.begin(), handlers.end(),
						[] (const auto& handler1, const auto& handler2) { return handler1.startIndex > handler2.startIndex; });

				const CatchBlock* prevHandler = nullptr;

				for(const CatchBlock* handler : handlers) {
					prevHandler = new CatchBlock(context, handler,
							prevHandler == nullptr ? context.getCurrentScope()->end() : lastHandler->start(), lastHandler);
					context.addBlock(prevHandler);
				}

				prevHandler->initiate(context);*/
			}
	};
}

#endif
