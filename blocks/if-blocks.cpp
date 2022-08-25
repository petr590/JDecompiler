#ifndef JDECOMPILER_IF_BLOCKS_CPP
#define JDECOMPILER_IF_BLOCKS_CPP

namespace jdecompiler {

	struct IfBlock: Block {

		struct ElseBlock: Block {
			const IfBlock* const ifBlock;

			const Operation* ternaryFalseOperation = nullptr;

			ElseBlock(const IfBlock* ifBlock, const DisassemblerContext& context, index_t endIndex):
					Block(ifBlock->end(), endIndex, ifBlock->parentBlock), ifBlock(ifBlock) {}

			virtual const Scope* toScope(const DecompilationContext&) const override {
				return nullptr;
			}
		};


		mutable const ElseBlock* elseBlock = nullptr;

		const Operation* ternaryTrueOperation = nullptr;


		IfBlock(const DisassemblerContext& context, offset_t offset):
				Block(context.index, context.posToIndex(context.pos + offset) - 1, context) {}

		virtual const Scope* toScope(const DecompilationContext& context) const override final {
			const ConditionOperation* condition = getCondition(context)->invert();

			const IfScope* ifScope = dynamic_cast<const IfScope*>(context.getCurrentScope());
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
					ifScope->remove(context);
				}
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

		IfCmpBlock(const DisassemblerContext& context, offset_t offset, const CompareType& compareType):
				IfBlock(context, offset), compareType(compareType) {}

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
}

#endif
