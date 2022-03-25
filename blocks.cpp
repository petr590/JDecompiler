#ifndef JDECOMPILER_BLOCKS_CPP
#define JDECOMPILER_BLOCKS_CPP

namespace jdecompiler {
	namespace instructions {

		struct CatchBlock: Block {
			public:
				mutable vector<const ClassType*> catchTypes;
				const bool hasNext;

				CatchBlock(index_t startIndex, index_t endIndex, bool hasNext): Block(startIndex, endIndex), catchTypes(), hasNext(hasNext) {}

				virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
					return new CatchScope(environment, startIndex, endIndex, catchTypes, hasNext);
				}
		};

		struct TryBlock: Block {
			protected:
				mutable vector<CatchBlock*> handlers;
				friend const CodeEnvironment& Method::decompileCode(const ClassInfo&);

			public:
				TryBlock(index_t startIndex, index_t endIndex): Block(startIndex, endIndex) {}

				virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
					return new TryScope(environment, startIndex, endIndex);
				}


				void linkCatchBlocks(const Bytecode& bytecode) const {
					/*assert(handlers.size() > 0);
					sort(handlers.begin(), handlers.end(),
							[] (const auto& handler1, const auto& handler2) { return handler1.startIndex > handler2.startIndex; });

					const CatchBlock* prevHandler = nullptr;

					for(const CatchBlock* handler : handlers) {
						prevHandler = new CatchBlock(environment, handler,
								prevHandler == nullptr ? environment.getCurrentScope()->end() : lastHandler->start(), lastHandler);
						bytecode.addBlock(prevHandler);
					}

					prevHandler->initiate(bytecode);*/
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

				ElseBlock(const IfBlock* ifBlock, const Bytecode& bytecode, index_t endIndex):
						Block(ifBlock->end(), endIndex, bytecode), ifBlock(ifBlock) {}

				virtual const Operation* toOperation(const CodeEnvironment&) const override {
					return nullptr;
				}
			};


			mutable IfType type = IF;

			mutable const ElseBlock* elseBlock = nullptr;

			const Operation* ternaryTrueOperation = nullptr;


			IfBlock(const Bytecode& bytecode, offset_t offset):
					Block(bytecode.getIndex(), bytecode.posToIndex(bytecode.getPos() + offset) - 1, bytecode) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override final {
				const ConditionOperation* const condition = getCondition(environment)->invert();
				switch(type) {
					case IF:
						if(elseBlock != nullptr && /*code.empty() && !elseScope->code.empty() &&*/ !environment.stack.empty()) {
							const Operation* trueCase = environment.stack.pop();
							const Operation* falseCase = environment.stack.pop();
							return new TernaryOperatorOperation(condition, trueCase, falseCase);
						}

						return elseBlock != nullptr ? new IfScope(environment, startIndex, endIndex, condition, elseBlock->endIndex) :
								new IfScope(environment, startIndex, endIndex, condition);

					case WHILE: return new WhileScope(environment, startIndex, endIndex, condition);
					case FOR:   return new ForScope(environment, startIndex, endIndex, condition);

					default:
						throw IllegalStateException("Illegal type of IfBlock " + to_string(type));
				}
			}

			virtual const ConditionOperation* getCondition(const CodeEnvironment& environment) const = 0;

			inline bool isLoop() const {
				return type == WHILE || type == FOR;
			}

			inline bool isFor() const {
				return type == FOR;
			}

			inline bool isTernary() const {
				return type == TERNARY;
			}


			void addElseBlock(const Bytecode& bytecode, index_t endIndex) const {
				if(elseBlock != nullptr)
					throw IllegalStateException("Else block already added");

				if(type != IF)
					throw IllegalStateException("Cannot add else block to " + to_string(type));

				//this->endIndex = endIndex;

				elseBlock = new ElseBlock(this, bytecode, endIndex);
				bytecode.addBlock(elseBlock);
			}

			void makeItLoop(const Bytecode& bytecode) const {
				if(type != IF && !isLoop())
				/*const Operation* inital = startIndex == 0 ? nullptr : environment.getCurrentScope()->code[startIndex - 1];
					throw IllegalStateException("Cannot make loop from " + to_string(type));
				LOG("startIndex = " << startIndex);*/
				type = WHILE;
			}
		};


		struct IfCmpBlock: IfBlock {
			const CompareType& compareType;

			IfCmpBlock(const Bytecode& bytecode, offset_t offset, const CompareType& compareType): IfBlock(bytecode, offset), compareType(compareType) {}

			virtual const ConditionOperation* getCondition(const CodeEnvironment& environment) const override {
				const Operation* operation = environment.stack.pop();
				if(const CmpOperation* cmpOperation = Operation::castOperationTo<const CmpOperation*>(operation))
					return new CompareBinaryOperation(cmpOperation, compareType);
				return new CompareWithZeroOperation(operation, compareType);
			}
		};


		struct IfICmpBlock: IfCmpBlock {
			IfICmpBlock(const Bytecode& bytecode, offset_t offset, const CompareType& compareType): IfCmpBlock(bytecode, offset, compareType) {}

			virtual const ConditionOperation* getCondition(const CodeEnvironment& environment) const override {
				return new CompareBinaryOperation(environment, INT, compareType);
			}
		};


		struct IfACmpBlock: IfCmpBlock {
			IfACmpBlock(const Bytecode& bytecode, offset_t offset, const EqualsCompareType& compareType): IfCmpBlock(bytecode, offset, compareType) {}

			virtual const ConditionOperation* getCondition(const CodeEnvironment& environment) const override {
				return new CompareBinaryOperation(environment, AnyObjectType::getInstance(), compareType);
			}
		};


		struct IfNullBlock: IfBlock {
			IfNullBlock(const Bytecode& bytecode, offset_t offset): IfBlock(bytecode, offset) {}

			virtual const ConditionOperation* getCondition(const CodeEnvironment& environment) const override {
				return new CompareWithNullOperation(environment, CompareType::EQUALS);
			}
		};

		struct IfNonNullBlock: IfBlock {
			IfNonNullBlock(const Bytecode& bytecode, offset_t offset): IfBlock(bytecode, offset) {}

			virtual const ConditionOperation* getCondition(const CodeEnvironment& environment) const override {
				return new CompareWithNullOperation(environment, CompareType::NOT_EQUALS);
			}
		};

		struct EmptyInfiniteLoopBlock: Block {
			EmptyInfiniteLoopBlock(const Bytecode& bytecode):
					Block(bytecode.getIndex(), bytecode.getIndex(), bytecode) {}

			virtual const Operation* toOperation(const CodeEnvironment& environment) const override {
				return new EmptyInfiniteLoopScope(environment);
			}
		};
	}
}

#endif
