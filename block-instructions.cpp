#ifndef JDECOMPILER_BLOCK_INSTRUCTIONS_CPP
#define JDECOMPILER_BLOCK_INSTRUCTIONS_CPP

namespace jdecompiler {

	struct BlockInstruction: Instruction {

		protected:
			BlockInstruction() noexcept {}

		public:
			virtual const Block* toBlock(const DisassemblerContext&) const = 0;

			virtual const Operation* toOperation(const DecompilationContext&) const override {
				return nullptr;
			}
	};


	struct JumpInstruction: BlockInstruction {
		protected:
			const offset_t offset;

			JumpInstruction(offset_t offset) noexcept: offset(offset) {}
	};



	struct IfInstruction: JumpInstruction {

		IfInstruction(offset_t offset) noexcept: JumpInstruction(offset) {}

		virtual const Block* toBlock(const DisassemblerContext& context) const override final {
			return createBlock(context);
		}

		virtual const Operation* toOperation(const DecompilationContext&) const override final {
			return nullptr;
		}

		virtual const Block* createBlock(const DisassemblerContext&) const = 0;
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


	struct GotoInstruction: JumpInstruction {
		public:
			GotoInstruction(offset_t offset) noexcept: JumpInstruction(offset) {}

		protected:
			mutable bool accepted = false;

			inline void setAccepted() const {
				accepted = true;
			}

			mutable const SwitchBlock* switchBlock = nullptr;

		public:
			virtual const Block* toBlock(const DisassemblerContext& context) const override {
				if(offset == 0) {
					setAccepted();
					return new EmptyInfiniteLoopBlock(context);
				}

				const index_t index = context.posToIndex(context.pos + offset);
				const index_t index_m1 = index - 1;

				const Block* const currentBlock = context.getCurrentBlock();


				if(instanceof<const IfBlock*>(currentBlock)) {
					const IfBlock* ifBlock = static_cast<const IfBlock*>(currentBlock);

					// Here goto instruction creates else Block
					if(offset > 0 && context.index == ifBlock->end() /* check if goto instruction in the end of ifBlock */) {
						const Block* parentBlock = ifBlock->parentBlock;
						assert(parentBlock != nullptr);

						//logerr(index_m1, ifBlock->end(), parentBlock->end());
						if(index_m1 <= parentBlock->end()) {
							ifBlock->addElseBlock(context, index_m1);
							setAccepted();
							return nullptr;
						}

						const IfBlock* current = ifBlock;
						for(const Block* parent = parentBlock;
								current->end() == parent->end() && parent->parentBlock != nullptr && instanceof<const IfBlock*>(parent);) {

							current = static_cast<const IfBlock*>(parent);
							parent = parent->parentBlock;

							if(index_m1 <= parent->end()) {
								current->addElseBlock(context, index_m1);
								setAccepted();
								return nullptr;
							}
						}

						const GotoInstruction* gotoInstruction = dynamic_cast<const GotoInstruction*>(context.getInstruction(parentBlock->end()));
						if(gotoInstruction != nullptr && gotoInstruction != this &&
								context.posToIndex(context.indexToPos(parentBlock->end()) + gotoInstruction->offset) == index) {

							ifBlock->addElseBlock(context, parentBlock->end() - 1);
							setAccepted();
							return nullptr;
						}
					}

				} else if(instanceof<const TryBlock*>(currentBlock)) {
					const TryBlock* tryBlock = static_cast<const TryBlock*>(currentBlock);

					if(context.index == tryBlock->end()) {
						setAccepted();
						tryBlock->handlers.back()->endIndex = index;
						return nullptr;//new CatchBlock(context, tryBlock);
					}

				}

				if(offset < 0) {
					const Block* block = currentBlock;

					do {
						if(instanceof<const InfiniteLoopBlock*>(block)) {
							const InfiniteLoopBlock* loopBlock = static_cast<const InfiniteLoopBlock*>(block);

							loopBlock->endIndex = max(loopBlock->endIndex, context.index);
						}

						block = block->parentBlock;
					} while(block != nullptr);

					setAccepted();
					return new InfiniteLoopBlock(context, index);

				} else {
					for(const Block* block = currentBlock; block != nullptr; block = block->parentBlock) {

						if(index_m1 >= block->endIndex && instanceof<const SwitchBlock*>(block)) {

							const SwitchBlock* switchBlock = static_cast<const SwitchBlock*>(block);

							if(index_m1 > switchBlock->endIndex) {
								if(!switchBlock->endIndexFixed())
									switchBlock->endIndex = index_m1;
								else
									continue;
							}

							switchBlock->fixEndIndex();

							this->switchBlock = switchBlock;

							return nullptr;
						}
					}
				}

				// UNSAFE BEHIAVOR
				/*if(offset > 0 && index >= currentBlock->start() && index_m1 <= currentBlock->end()) { // goto in the borders of current Block
					const_cast<DisassemblerContext&>(context).skip(offset);
					setAccepted();
					return nullptr;
				}*/

				return nullptr;
			}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {

				if(accepted)
					return nullptr;

				if(switchBlock != nullptr)
					return new BreakOperation(context, switchBlock->getExistsScope(context));

				const index_t index = context.posToIndex(context.pos + offset);

				for(const Scope* currentScope = context.getCurrentScope(); currentScope != nullptr; currentScope = currentScope->parentScope) {
					if(index - 1 == currentScope->endIndex && currentScope->isBreakable()) {
						setAccepted();
						return new BreakOperation(context, currentScope);
					}
				}

				if(!accepted)
					throw DecompilationException("Illegal using of goto instruction: goto from pos " +
							to_string(context.pos) + " to pos " + to_string(context.pos + offset));

				return nullptr;
			}
	};


	struct SwitchInstruction: BlockInstruction {
		protected:
			const offset_t defaultOffset;
			map<jint, offset_t> offsetTable;

		public:
			SwitchInstruction(offset_t defaultOffset, const map<jint, offset_t>& offsetTable):
					defaultOffset(defaultOffset), offsetTable(offsetTable) {}

			virtual const Block* toBlock(const DisassemblerContext& context) const override {
				return new SwitchBlock(context, defaultOffset, offsetTable);
			}
	};
}

#endif
