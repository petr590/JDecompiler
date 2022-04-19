#ifndef JDECOMPILER_BLOCK_INSTRUCTIONS_CPP
#define JDECOMPILER_BLOCK_INSTRUCTIONS_CPP

namespace jdecompiler {
	namespace instructions {
		struct BlockInstruction: Instruction {
			protected:
				BlockInstruction() {}

			public:
				virtual const Block* toBlock(const DisassemblerContext&) const = 0;

				virtual const Operation* toOperation(const DecompilationContext&) const override {
					return nullptr;
				}
		};



		struct IfInstruction: BlockInstruction {
			const offset_t offset;

			IfInstruction(offset_t offset): offset(offset) {}

			virtual const Block* toBlock(const DisassemblerContext& context) const override final {
				const Block* currentBlock = context.getCurrentBlock();

				const index_t index = context.posToIndex(context.getPos() + offset) - 1;

				if(instanceof<const IfBlock*>(currentBlock)) {
					const IfBlock* ifBlock = static_cast<const IfBlock*>(currentBlock);

					if(offset > 0) {
						//log(context.index, index, ifBlock->end());

						if(index - 1 == ifBlock->end()) {
							return nullptr;
						}

						if(context.getIndex() == ifBlock->end()) {
							ifBlock->endIndex = index;
							return nullptr;
						}
					}
				}

				return createBlock(context);
			}

			virtual const Operation* toOperation(const DecompilationContext& context) const override final {
				const Scope* currentScope = context.getCurrentScope();

				const index_t index = context.posToIndex(context.pos + offset);

				if(instanceof<const IfScope*>(currentScope)) {
					const IfScope* ifScope = static_cast<const IfScope*>(currentScope);

					if(offset > 0) {
						if(index - 1 == ifScope->end()) {
							//ifScope->condition = new AndOperation(ifScope->condition, getCondition(context)->invert()); // TODO
						}

						if(context.index == ifScope->end()) {
							//ifScope->condition = new OrOperation(ifScope->condition->invert(), getCondition(context)->invert()); // TODO
						}
					}
				}

				return nullptr;
			}

			virtual const Block* createBlock(const DisassemblerContext& context) const = 0;
		};

		struct IfCmpInstruction: IfInstruction {
			const CompareType& compareType;

			IfCmpInstruction(offset_t offset, const CompareType& compareType): IfInstruction(offset), compareType(compareType) {}

			virtual const Block* createBlock(const DisassemblerContext& context) const override {
				return new IfCmpBlock(context, offset, compareType);
			}
		};


		struct IfEqInstruction: IfCmpInstruction {
			IfEqInstruction(offset_t offset): IfCmpInstruction(offset, CompareType::EQUALS) {};
		};

		struct IfNotEqInstruction: IfCmpInstruction {
			IfNotEqInstruction(offset_t offset): IfCmpInstruction(offset, CompareType::NOT_EQUALS) {}
		};

		struct IfGtInstruction: IfCmpInstruction {
			IfGtInstruction(offset_t offset): IfCmpInstruction(offset, CompareType::GREATER) {}
		};

		struct IfGeInstruction: IfCmpInstruction {
			IfGeInstruction(offset_t offset): IfCmpInstruction(offset, CompareType::GREATER_OR_EQUALS) {}
		};

		struct IfLtInstruction: IfCmpInstruction {
			IfLtInstruction(offset_t offset): IfCmpInstruction(offset, CompareType::LESS) {}
		};

		struct IfLeInstruction: IfCmpInstruction {
			IfLeInstruction(offset_t offset): IfCmpInstruction(offset, CompareType::LESS_OR_EQUALS) {}
		};


		struct IfICmpInstruction: IfCmpInstruction {
			IfICmpInstruction(offset_t offset, const CompareType& compareType): IfCmpInstruction(offset, compareType) {}

			virtual const Block* createBlock(const DisassemblerContext& context) const override {
				return new IfICmpBlock(context, offset, compareType);
			}
		};


		struct IfIEqInstruction: IfICmpInstruction {
			IfIEqInstruction(offset_t offset): IfICmpInstruction(offset, CompareType::EQUALS) {}
		};

		struct IfINotEqInstruction: IfICmpInstruction {
			IfINotEqInstruction(offset_t offset): IfICmpInstruction(offset, CompareType::NOT_EQUALS) {}
		};

		struct IfIGtInstruction: IfICmpInstruction {
			IfIGtInstruction(offset_t offset): IfICmpInstruction(offset, CompareType::GREATER) {}
		};

		struct IfIGeInstruction: IfICmpInstruction {
			IfIGeInstruction(offset_t offset): IfICmpInstruction(offset, CompareType::GREATER_OR_EQUALS) {}
		};

		struct IfILtInstruction: IfICmpInstruction {
			IfILtInstruction(offset_t offset): IfICmpInstruction(offset, CompareType::LESS) {}
		};

		struct IfILeInstruction: IfICmpInstruction {
			IfILeInstruction(offset_t offset): IfICmpInstruction(offset, CompareType::LESS_OR_EQUALS) {}
		};


		struct IfACmpInstruction: IfCmpInstruction {
			IfACmpInstruction(offset_t offset, const EqualsCompareType& compareType): IfCmpInstruction(offset, compareType) {}

			virtual const Block* createBlock(const DisassemblerContext& context) const override {
				return new IfACmpBlock(context, offset, (const EqualsCompareType&)compareType);
			}
		};


		struct IfAEqInstruction: IfACmpInstruction {
			IfAEqInstruction(offset_t offset): IfACmpInstruction(offset, CompareType::EQUALS) {}
		};

		struct IfANotEqInstruction: IfACmpInstruction {
			IfANotEqInstruction(offset_t offset): IfACmpInstruction(offset, CompareType::NOT_EQUALS) {}
		};


		struct IfNullInstruction: IfInstruction {
			IfNullInstruction(offset_t offset): IfInstruction(offset) {}

			virtual const Block* createBlock(const DisassemblerContext& context) const override {
				return new IfNullBlock(context, offset);
			}
		};

		struct IfNonNullInstruction: IfInstruction {
			IfNonNullInstruction(offset_t offset): IfInstruction(offset) {}

			virtual const Block* createBlock(const DisassemblerContext& context) const override {
				return new IfNonNullBlock(context, offset);
			}
		};


		struct GotoInstruction: BlockInstruction {
			const offset_t offset;

			GotoInstruction(offset_t offset): offset(offset) {}


			virtual const Block* toBlock(const DisassemblerContext& context) const override {
				if(offset == 0) return new EmptyInfiniteLoopBlock(context);

				const index_t index = context.posToIndex(context.getPos() + offset);

				const Block* const currentBlock = context.getCurrentBlock();


				if(instanceof<const IfBlock*>(currentBlock)) {
					const IfBlock* ifBlock = static_cast<const IfBlock*>(currentBlock);

					// Here goto instruction creates else Block
					if(offset > 0 && context.index == ifBlock->end() /* check if goto instruction in the end of ifBlock */) {
						const Block* parentBlock = ifBlock->parentBlock;
						assert(parentBlock != nullptr);

						/* I don't remember why there is index minus 1 instead of index,
						   but since I wrote that, then it should be so :) */
						if(index - 1 <= parentBlock->end()) {
							ifBlock->addElseBlock(context, index - 1);
							return nullptr;
						}

						const GotoInstruction* gotoInstruction = dynamic_cast<const GotoInstruction*>(context.getInstruction(parentBlock->end()));
						if(gotoInstruction != nullptr && context.posToIndex(context.indexToPos(parentBlock->end()) + gotoInstruction->offset) == index) {
							ifBlock->addElseBlock(context, parentBlock->end() - 1);
							return nullptr;
						}
					}
				} else if(instanceof<const TryBlock*>(currentBlock)) {
					const TryBlock* tryBlock = static_cast<const TryBlock*>(currentBlock);

					if(context.getIndex() == tryBlock->end()) return nullptr;
						//return new CatchBlock(context, tryBlock);
				}

				if(offset < 0) {
					const Block* block = currentBlock;

					do {
						if(instanceof<const InfiniteLoopBlock*>(block)) {
							const InfiniteLoopBlock* loopBlock = static_cast<const InfiniteLoopBlock*>(block);

							//log(loopBlock->endIndex, context.index);

							loopBlock->endIndex = max(loopBlock->endIndex, context.index);
						}

						block = block->parentBlock;
					} while(block != nullptr);

					return new InfiniteLoopBlock(context, index);
				}

				if(offset > 0 && index >= currentBlock->start() && index - 1 <= currentBlock->end()) { // goto in the borders of current Block
					const_cast<DisassemblerContext&>(context).skip(offset);
					return nullptr;
				}

				throw DecompilationException("Illegal using of goto instruction: goto from " +
						to_string(context.getPos()) + " to " + to_string(context.getPos() + offset));
			}

			/*virtual const Operation* toOperation(const DecompilationContext& context) const override {}*/
		};
	}
}

#endif
