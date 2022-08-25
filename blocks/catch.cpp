#ifndef JDECOMPILER_CATCH_BLOCK_CPP
#define JDECOMPILER_CATCH_BLOCK_CPP

namespace jdecompiler {

	struct CatchBlock: Block {
		public:
			mutable vector<const ClassType*> catchTypes;
			mutable bool hasNext = false;

			CatchBlock(index_t startIndex, const ClassType* catchType):
					Block(startIndex, static_cast<index_t>(-1)), catchTypes({catchType}) {}

			virtual const Scope* toScope(const DecompilationContext& context) const override {
				return new CatchScope(context, startIndex, endIndex, catchTypes, hasNext);
			}
	};
}

#endif
