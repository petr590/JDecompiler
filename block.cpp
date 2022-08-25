#ifndef JDECOMPILER_BLOCK_CPP
#define JDECOMPILER_BLOCK_CPP

#include "block.h"

namespace jdecompiler {


	Block::Block(index_t startIndex, index_t endIndex) noexcept:
			Block(startIndex, endIndex, nullptr) {}

	Block::Block(index_t startIndex, index_t endIndex, const Block* parentBlock) noexcept:
			startIndex(startIndex), endIndex(endIndex), parentBlock(parentBlock) {}

	Block::Block(index_t startIndex, index_t endIndex, const DisassemblerContext& context) noexcept:
			Block(startIndex, endIndex, context.getCurrentBlock()) {}


	RootBlock::RootBlock(index_t endIndex) noexcept: Block(0, endIndex) {}

	const Scope* RootBlock::toScope(const DecompilationContext&) const {
		return nullptr;
	}
}

#endif
