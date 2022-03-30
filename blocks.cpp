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
			enum IfType { IF, TERNARY, WHILE, FOR };

			static string to_string(const IfType type) {
				switch(type) {
					case IF:      return "if";
					case TERNARY: return "ternary operator";
					case WHILE:   return "while";
					case FOR:     return "for";
					default:      return "(ScopeType)" + std::to_string((uint32_t)type);
				}
			}


			struct ElseBlock: Block {
				const IfBlock* const ifBlock;

				const Operation* ternaryFalseOperation = nullptr;

				ElseBlock(const IfBlock* ifBlock, const DisassemblerContext& context, index_t endIndex):
						Block(ifBlock->end(), endIndex, ifBlock->parentBlock), ifBlock(ifBlock) {}

				virtual const Operation* toOperation(const DecompilationContext&) const override {
					return nullptr;
				}
			};


			mutable IfType type = IF;

			mutable const ElseBlock* elseBlock = nullptr;

			const Operation* ternaryTrueOperation = nullptr;


			IfBlock(const DisassemblerContext& context, offset_t offset):
					Block(context.getIndex(), context.posToIndex(context.getPos() + offset) - 1, context) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override final {
				const ConditionOperation* const condition = getCondition(context)->invert();
				switch(type) {
					case IF:
						if(elseBlock != nullptr && /*code.empty() && !elseScope->code.empty() &&*/ !context.stack.empty()) {
							const Operation* trueCase = context.stack.pop();
							const Operation* falseCase = context.stack.pop();
							return new TernaryOperatorOperation(condition, trueCase, falseCase);
						}

						return elseBlock != nullptr ? new IfScope(context, endIndex, condition, elseBlock->endIndex) :
								new IfScope(context, endIndex, condition);

					case WHILE: return new WhileScope(context, startIndex, endIndex, condition);
					case FOR:   return new ForScope(context, startIndex, endIndex, condition);

					default:
						throw IllegalStateException("Illegal type of IfBlock " + to_string(type));
				}
			}

			virtual const ConditionOperation* getCondition(const DecompilationContext& context) const = 0;

			inline bool isLoop() const {
				return type == WHILE || type == FOR;
			}

			inline bool isFor() const {
				return type == FOR;
			}

			inline bool isTernary() const {
				return type == TERNARY;
			}


			void addElseBlock(const DisassemblerContext& context, index_t endIndex) const {
				if(elseBlock != nullptr)
					throw IllegalStateException("Else block already added");

				if(type != IF)
					throw IllegalStateException("Cannot add else block to " + to_string(type));

				//this->endIndex = endIndex;

				elseBlock = new ElseBlock(this, context, endIndex);
				context.addBlock(elseBlock);
			}

			void makeItLoop(const DisassemblerContext& context) const {
				if(type != IF && !isLoop())
				/*const Operation* inital = startIndex == 0 ? nullptr : context.getCurrentScope()->code[startIndex - 1];
					throw IllegalStateException("Cannot make loop from " + to_string(type));
				LOG("startIndex = " << startIndex);*/
				type = WHILE;
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
				return new CompareBinaryOperation(context, INT, compareType);
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
	}
}

#endif
