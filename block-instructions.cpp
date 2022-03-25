#ifndef JDECOMPILER_BLOCK_INSTRUCTIONS_CPP
#define JDECOMPILER_BLOCK_INSTRUCTIONS_CPP

namespace jdecompiler {
	namespace instructions {
		struct BlockInstruction: Instruction {
			protected:
				BlockInstruction() {}

			public:
				virtual const Block* toBlock(const Bytecode&) const = 0;

				virtual const Operation* toOperation(const CodeEnvironment&) const override {
					return nullptr;
				}
		};



		struct IfInstruction: BlockInstruction {
			const offset_t offset;

			IfInstruction(offset_t offset): offset(offset) {}

			virtual const Block* toBlock(const Bytecode& bytecode) const override final {
				const Block* currentBlock = bytecode.getCurrentBlock();

				const index_t index = bytecode.posToIndex(bytecode.getPos() + offset) - 1;

				if(const IfBlock* ifBlock = dynamic_cast<const IfBlock*>(currentBlock)) {
					if(offset > 0) {
						//LOG(environment.index << ' ' << index << ' ' << ifBlock->end());

						if(index - 1 == ifBlock->end()) {
							return nullptr;
						}

						if(bytecode.getIndex() == ifBlock->end()) {
							ifBlock->endIndex = index;
							return nullptr;
						}
					}
				}

				return createBlock(bytecode);
			}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override final {
				const Scope* currentScope = environment.getCurrentScope();

				const index_t index = environment.bytecode.posToIndex(environment.pos + offset);

				if(const IfScope* ifScope = dynamic_cast<const IfScope*>(currentScope)) {
					if(offset > 0) {
						if(index - 1 == ifScope->end()) {
							//ifScope->condition = new AndOperation(ifScope->condition, getCondition(environment)->invert()); // TODO
						}

						if(environment.index == ifScope->end()) {
							//ifScope->condition = new OrOperation(ifScope->condition->invert(), getCondition(environment)->invert()); // TODO
						}
					}
				}

				return nullptr;
			}

			virtual const Block* createBlock(const Bytecode& bytecode) const = 0;
		};

		struct IfCmpInstruction: IfInstruction {
			const CompareType& compareType;

			IfCmpInstruction(offset_t offset, const CompareType& compareType): IfInstruction(offset), compareType(compareType) {}

			virtual const Block* createBlock(const Bytecode& bytecode) const override {
				return new IfCmpBlock(bytecode, offset, compareType);
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

			virtual const Block* createBlock(const Bytecode& bytecode) const override {
				return new IfICmpBlock(bytecode, offset, compareType);
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
		};


		struct IfAEqInstruction: IfACmpInstruction {
			IfAEqInstruction(offset_t offset): IfACmpInstruction(offset, CompareType::EQUALS) {}
		};

		struct IfANotEqInstruction: IfACmpInstruction {
			IfANotEqInstruction(offset_t offset): IfACmpInstruction(offset, CompareType::NOT_EQUALS) {}
		};


		struct IfNullInstruction: IfInstruction {
			IfNullInstruction(offset_t offset): IfInstruction(offset) {}

			virtual const Block* createBlock(const Bytecode& bytecode) const override {
				return new IfNullBlock(bytecode, offset);
			}
		};

		struct IfNonNullInstruction: IfInstruction {
			IfNonNullInstruction(offset_t offset): IfInstruction(offset) {}

			virtual const Block* createBlock(const Bytecode& bytecode) const override {
				return new IfNonNullBlock(bytecode, offset);
			}
		};


		struct GotoInstruction: BlockInstruction {
			const offset_t offset;

			GotoInstruction(offset_t offset): offset(offset) {}


			virtual const Block* toBlock(const Bytecode& bytecode) const override {
				if(offset == 0) return new EmptyInfiniteLoopBlock(bytecode);

				const index_t index = bytecode.posToIndex(bytecode.getPos() + offset);

				const Block* const currentBlock = bytecode.getCurrentBlock();


				if(instanceof<const IfBlock*>(currentBlock)) {
					const IfBlock* ifBlock = static_cast<const IfBlock*>(currentBlock);

					// Here goto instruction creates else Block
					if(offset > 0 && !ifBlock->isLoop() && bytecode.getIndex() == ifBlock->end() /* check if goto instruction in the end of ifBlock */) {
						const Block* parentBlock = ifBlock->parentBlock;

						/* I don't remember why there is index minus 1 instead of index,
						   but since I wrote that, then it should be so :) */
						assert(parentBlock != nullptr);
						if(index - 1 <= parentBlock->end()) {
							ifBlock->addElseBlock(bytecode, index - 1);
							return nullptr;
						}

						const GotoInstruction* gotoInstruction = dynamic_cast<const GotoInstruction*>(bytecode.getInstruction(parentBlock->end()));
						if(gotoInstruction && bytecode.posToIndex(bytecode.indexToPos(parentBlock->end()) + gotoInstruction->offset) == index) {
							ifBlock->addElseBlock(bytecode, parentBlock->end() - 1);
							return nullptr;
						}
					} else {
						// Here goto creates operator continue
						do {
							//LOG(index << ' ' << ifBlock->start());
							if(index == ifBlock->start()) {
								ifBlock->makeItLoop(bytecode);
								return nullptr;
								//return new ContinueOperation(bytecode, ifBlock); // TODO
							}
							ifBlock = dynamic_cast<const IfBlock*>(ifBlock->parentBlock);
						} while(ifBlock != nullptr);
					}
				} else if(instanceof<const TryBlock*>(currentBlock)) {
					const TryBlock* tryBlock = static_cast<const TryBlock*>(currentBlock);

					if(bytecode.getIndex() == tryBlock->end()) return nullptr;
						//return new CatchBlock(bytecode, tryBlock);
				}

				//cerr << "GOTO LOG!!! " << offset << ' ' << index << ' ' << currentBlock->start() << "  " << index << ' ' << currentBlock->end() << '\n' << typeNameOf(*currentBlock) << endl;

				if(offset > 0 && index >= currentBlock->start() && index - 1 <= currentBlock->end()) { // goto in the borders of current Block
					const_cast<Bytecode&>(bytecode).skip(offset);
					return nullptr;
				}

				throw DecompilationException("Illegal using of goto instruction: goto from " +
						to_string(bytecode.getPos()) + " to " + to_string(bytecode.getPos() + offset));
			}
		};
	}
}

#endif
