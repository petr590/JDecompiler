#ifndef JDECOMPILER_BLOCK_H
#define JDECOMPILER_BLOCK_H

namespace jdecompiler {

	struct Block {
		public:
			mutable index_t startIndex, endIndex;
			mutable const Block* parentBlock;

		protected:
			mutable vector<const Block*> innerBlocks;
			mutable const Scope* scope = nullptr;

		public:
			Block(index_t, index_t) noexcept;

			Block(index_t, index_t, const Block*) noexcept;

			Block(index_t, index_t, const DisassemblerContext&) noexcept;

		protected:
			virtual const Scope* toScope(const DecompilationContext&) const = 0;

		public:
			inline const Scope* getScope(const DecompilationContext& context) const {
				if(scope == nullptr)
					scope = toScope(context);

				return scope;
			}

			inline const Scope* getExistsScope(const DecompilationContext& context) const {
				if(scope == nullptr)
					throw IllegalStateException("Scope of " + this->toDebugString() + " is not created");

				return scope;
			}

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
				return short_typenameof(*this) + " {" + to_string(startIndex) + ".." + to_string(endIndex) + '}';
			}

			inline friend ostream& operator<<(ostream& out, const Block* block) {
				return out << (block != nullptr ? block->toDebugString() : "null");
			}

			inline friend ostream& operator<<(ostream& out, const Block& block) {
				return out << &block;
			}
	};

	struct RootBlock: Block {
		RootBlock(index_t) noexcept;

		virtual const Scope* toScope(const DecompilationContext&) const override;
	};
}

#endif
