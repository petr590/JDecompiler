#ifndef JDECOMPILER_BLOCKS_CPP
#define JDECOMPILER_BLOCKS_CPP

namespace jdecompiler {
	namespace instructions {

		struct CatchBlock: Block {
			public:
				mutable vector<const ClassType*> catchTypes;
				const bool hasNext;

				CatchBlock(index_t startIndex, index_t endIndex, bool hasNext): Block(startIndex, endIndex), catchTypes(), hasNext(hasNext) {}

				virtual const Operation* toOperation(const DecompilationContext& context) const override {
					return new CatchScope(context, startIndex, endIndex, catchTypes, hasNext);
				}
		};

		struct TryBlock: Block {
			protected:
				mutable vector<CatchBlock*> handlers;
				friend const StringifyContext& Method::decompileCode(const ClassInfo&);

			public:
				TryBlock(index_t startIndex, index_t endIndex): Block(startIndex, endIndex) {}

				virtual const Operation* toOperation(const DecompilationContext& context) const override {
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


		struct IfBlock: Block {

			struct ElseBlock: Block {
				const IfBlock* const ifBlock;

				const Operation* ternaryFalseOperation = nullptr;

				ElseBlock(const IfBlock* ifBlock, const DisassemblerContext& context, index_t endIndex):
						Block(ifBlock->end(), endIndex, ifBlock->parentBlock), ifBlock(ifBlock) {}

				virtual const Operation* toOperation(const DecompilationContext&) const override {
					return nullptr;
				}
			};


			mutable const ElseBlock* elseBlock = nullptr;

			const Operation* ternaryTrueOperation = nullptr;


			IfBlock(const DisassemblerContext& context, offset_t offset):
					Block(context.index, context.posToIndex(context.getPos() + offset) - 1, context) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override final {
				const ConditionOperation* condition = getCondition(context)->invert();

				const IfScope* ifScope = dynamic_cast<const IfScope*>(context.getCurrentScope());
				// TO DO $assertionsDisabled
				const Scope* ifParentScope = context.getCurrentScope();

				if(ifScope != nullptr) {
					if(ifScope->bodyStartIndex == context.exprStartIndex && ifScope->endIndex == endIndex) {
						ifScope->setCondition(new AndOperation(ifScope->getCondition(), condition));

						ifScope->bodyStartIndex = context.index + 1;

						return nullptr;
					}

					if(endIndex > ifScope->endIndex && context.index == ifScope->endIndex) {
						condition = new OrOperation(ifScope->condition->invert(), condition);
						ifParentScope = ifParentScope->parentScope;
						ifScope->remove();
					}

					/*if(ifScope->endIndex >= endIndex && endIndex <= ifScope->parentScope->endIndex) {

					}*/
				}

				return elseBlock != nullptr ?
						new IfScope(context, endIndex, condition, ifParentScope, elseBlock->endIndex) :
						new IfScope(context, endIndex, condition, ifParentScope);
			}

			virtual const ConditionOperation* getCondition(const DecompilationContext& context) const = 0;


			void addElseBlock(const DisassemblerContext& context, index_t endIndex) const {
				if(elseBlock != nullptr)
					throw IllegalStateException("Else block already added");

				//this->endIndex = endIndex;

				elseBlock = new ElseBlock(this, context, endIndex);
				context.addBlock(elseBlock);
			}
		};


		struct IfCmpBlock: IfBlock {
			const CompareType& compareType;

			IfCmpBlock(const DisassemblerContext& context, offset_t offset, const CompareType& compareType): IfBlock(context, offset), compareType(compareType) {}

			virtual const ConditionOperation* getCondition(const DecompilationContext& context) const override {
				const Operation* operation = context.stack.pop();
				if(const CmpOperation* cmpOperation = dynamic_cast<const CmpOperation*>(operation->getOriginalOperation()))
					return new CompareBinaryOperation(cmpOperation, compareType);
				return new CompareWithZeroOperation(operation, compareType);
			}
		};


		struct IfICmpBlock: IfCmpBlock {
			IfICmpBlock(const DisassemblerContext& context, offset_t offset, const CompareType& compareType): IfCmpBlock(context, offset, compareType) {}

			virtual const ConditionOperation* getCondition(const DecompilationContext& context) const override {
				return new CompareBinaryOperation(context, ANY_INT_OR_BOOLEAN, compareType);
			}
		};


		struct IfACmpBlock: IfCmpBlock {
			IfACmpBlock(const DisassemblerContext& context, offset_t offset, const EqualsCompareType& compareType): IfCmpBlock(context, offset, compareType) {}

			virtual const ConditionOperation* getCondition(const DecompilationContext& context) const override {
				return new CompareBinaryOperation(context, AnyObjectType::getInstance(), compareType);
			}
		};


		struct IfNullBlock: IfBlock {
			IfNullBlock(const DisassemblerContext& context, offset_t offset): IfBlock(context, offset) {}

			virtual const ConditionOperation* getCondition(const DecompilationContext& context) const override {
				return new CompareWithNullOperation(context, CompareType::EQUALS);
			}
		};

		struct IfNonNullBlock: IfBlock {
			IfNonNullBlock(const DisassemblerContext& context, offset_t offset): IfBlock(context, offset) {}

			virtual const ConditionOperation* getCondition(const DecompilationContext& context) const override {
				return new CompareWithNullOperation(context, CompareType::NOT_EQUALS);
			}
		};


		struct EmptyInfiniteLoopBlock: Block {
			EmptyInfiniteLoopBlock(const DisassemblerContext& context):
					Block(context.getIndex(), context.getIndex(), context) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return new EmptyInfiniteLoopScope(context);
			}
		};

		struct InfiniteLoopBlock: Block {
			InfiniteLoopBlock(const DisassemblerContext& context, index_t index):
					Block(index, context.getIndex(), context) {}


			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return new InfiniteLoopScope(context, startIndex, endIndex);
			}
		};
	}
}

#endif
