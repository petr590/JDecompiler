#ifndef JDECOMPILER_INFINITE_LOOP_BLOCKS_CPP
#define JDECOMPILER_INFINITE_LOOP_BLOCKS_CPP

namespace jdecompiler {

	struct EmptyInfiniteLoopBlock: Block {
		EmptyInfiniteLoopBlock(const DisassemblerContext& context):
				Block(context.index, context.index, context) {}

		virtual const Scope* toScope(const DecompilationContext& context) const override {
			return new EmptyInfiniteLoopScope(context);
		}
	};

	struct InfiniteLoopBlock: Block {
		InfiniteLoopBlock(const DisassemblerContext& context, index_t index):
				Block(index, context.index, context) {}


		virtual const Scope* toScope(const DecompilationContext& context) const override {
			return new InfiniteLoopScope(context, startIndex, endIndex);
		}
	};
}

#endif
