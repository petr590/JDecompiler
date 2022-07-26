#ifndef JDECOMPILER_BLOCK_H
#define JDECOMPILER_BLOCK_H

namespace jdecompiler {

	struct Block {
		public:
			mutable index_t startIndex, endIndex;
			const Block* parentBlock;

		protected:
			mutable vector<const Block*> innerBlocks;

		public:
			Block(index_t, index_t, const Block*) noexcept;

			Block(index_t, index_t, const DisassemblerContext&) noexcept;

			Block(index_t, index_t) noexcept;

			virtual const Operation* toOperation(const DecompilationContext& context) const = 0;

			inline index_t start() const {
				return startIndex;
			}

			inline index_t end() const {
				return endIndex;
			}

			inline void addInnerBlock(const Block* block) const {
				innerBlocks.push_back(block);
			}

			inline string toDebugString() const {
				return typenameof(*this) + " {" + to_string(startIndex) + ".." + to_string(endIndex) + '}';
			}
	};

	struct RootBlock: Block {
		RootBlock(index_t) noexcept;

		virtual const Operation* toOperation(const DecompilationContext&) const override;
	};
}

#endif
