#ifndef JDECOMPILER_DISASSEMBLER_CONTEXT_CPP
#define JDECOMPILER_DISASSEMBLER_CONTEXT_CPP

namespace jdecompiler {

	struct DisassemblerContext {
		public:
			const ConstantPool& constPool;
			const uint32_t length;
			const uint8_t* const bytes;

			pos_t pos = 0;
			index_t index = 0, exprStartIndex = 0;

		private:
			vector<Instruction*> instructions;
			mutable vector<const Block*> blocks, inactiveBlocks;
			const Block* currentBlock = nullptr;
			map<pos_t, index_t> indexMap;
			map<index_t, pos_t> posMap;

		public:
			inline const Block* getCurrentBlock() const {
				return currentBlock;
			}

			DisassemblerContext(const ConstantPool& constPool, uint32_t length, const uint8_t* bytes): constPool(constPool), length(length), bytes(bytes) {
				if(length == 0)
					return;

				int32_t stackContent = 0, exprIndex = 0;

				vector<index_t> exprStartIndexTable;

				while(available()) {
					indexMap[pos] = index;
					posMap[index] = pos;
					index++;

					auto [takeFromStack, putOnStack, instruction] = nextInstruction();

					stackContent -= takeFromStack;
					assert(stackContent >= 0);
					stackContent += putOnStack;
					assert(stackContent >= 0);

					exprStartIndexTable.push_back(exprStartIndex);

					if(stackContent == 0)
						exprStartIndex = index;

					instructions.push_back(instruction);

					next();
				}

				const index_t lastIndex = index;
				assert(lastIndex == instructions.size());

				currentBlock = new RootBlock(lastIndex);

				index = 0;

				for(const Instruction* instruction = instructions[0]; index < lastIndex; instruction = instructions[++index]) {
					pos = posMap[index];
					exprStartIndex = exprStartIndexTable[index];

					if(instruction != nullptr) {

						const Block* block = instruction->toBlock(*this);
						if(block != nullptr)
							addBlock(block);
					}

					updateBlocks();
				}
			}

			inline const vector<Instruction*>& getInstructions() const {
				return instructions;
			}

			inline const vector<const Block*>& getBlocks() const {
				return blocks;
			}

			const Instruction* getInstruction(index_t index) const {
				if(index >= instructions.size())
					throw IndexOutOfBoundsException(index, instructions.size());
				return instructions[index];
			}

			const Instruction* getInstructionNoexcept(index_t index) const noexcept {
				if(index >= instructions.size())
					return nullptr;
				return instructions[index];
			}

			inline void addBlock(const Block* block) const {
				blocks.push_back(block);
				inactiveBlocks.push_back(block);
			}

		protected:
			void updateBlocks() {

				while(index >= currentBlock->end()) {
					if(currentBlock->parentBlock == nullptr)
						throw DecompilationException("Unexpected end of global function block " + currentBlock->toDebugString());
					log("End of", currentBlock->toDebugString());
					currentBlock = currentBlock->parentBlock;
				}

				for(auto i = inactiveBlocks.begin(); i != inactiveBlocks.end(); ) {
					const Block* block = *i;
					if(block->start() <= index) {
						if(block->end() > currentBlock->end()) {
							throw DecompilationException("Block " + block->toDebugString() +
									" is out of bounds of the parent block " + currentBlock->toDebugString());
						}

						log("Start of", block->toDebugString());

						currentBlock->addInnerBlock(block);
						currentBlock = block;
						inactiveBlocks.erase(i);
					} else
						++i;
				}
			}

			inline uint8_t next() {
				return (uint8_t)bytes[++pos];
			}


			inline int8_t nextByte() {
				return (int8_t)next();
			}

			inline uint8_t nextUByte() {
				return (uint8_t)next();
			}

			inline int16_t nextShort() {
				return (int16_t)(next() << 8 | next());
			}

			inline uint16_t nextUShort() {
				return (uint16_t)(next() << 8 | next());
			}

			inline int32_t nextInt() {
				return (int32_t)(next() << 24 | next() << 16 | next() << 8 | next());
			}

			inline uint32_t nextUInt() {
				return (uint32_t)(next() << 24 | next() << 16 | next() << 8 | next());
			}

			inline uint8_t current() const {
				return bytes[pos];
			}

			inline bool available() const {
				return length - pos > 0;
			}

			tuple<int32_t, int32_t, Instruction*> nextInstruction();

		public:
			inline pos_t getPos() const {
				return pos;
			}

			inline index_t getIndex() const {
				return index;
			}

			inline void skip(int32_t count) {
				pos += (uint32_t)count;
			}

			index_t posToIndex(pos_t pos) const {
				const auto foundPos = indexMap.find(pos);
				if(foundPos != indexMap.end())
					return foundPos->second;

				throw BytecodePosOutOfBoundsException(pos, length);
			}

			pos_t indexToPos(index_t index) const {
				const auto foundIndex = posMap.find(index);
				if(foundIndex != posMap.end())
					return foundIndex->second;

				throw BytecodeIndexOutOfBoundsException(index, length);
			}
	};
}

#endif
