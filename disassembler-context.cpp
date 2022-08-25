#ifndef JDECOMPILER_DISASSEMBLER_CONTEXT_CPP
#define JDECOMPILER_DISASSEMBLER_CONTEXT_CPP

#include "context.cpp"

namespace jdecompiler {

	struct DisassemblerContext final: Context {
		public:
			const ConstantPool& constPool;
			const uint32_t length;
			const uint8_t* const bytes;

			index_t index = 0;

		private:
			vector<Instruction*> instructions;
			mutable vector<const Block*> blocks, inactiveBlocks;
			const Block* currentBlock = nullptr;
			map<pos_t, index_t> indexMap;
			map<index_t, pos_t> posMap;

		public:
			DisassemblerContext(const ConstantPool& constPool, uint32_t length, const uint8_t bytes[]):
					constPool(constPool), length(length), bytes(bytes) {

				if(length == 0)
					return;

				while(available()) {
					indexMap[pos] = index;
					posMap[index] = pos;
					index++;

					instructions.push_back(nextInstruction());

					next();
				}
			}

		protected:
			friend struct Method;

			void decompile() {
				const index_t size = instructions.size();

				currentBlock = new RootBlock(size);

				index = 0;

				for(const Instruction* instruction = instructions[0]; index < size; instruction = instructions[++index]) {
					pos = posMap[index];

					if(instruction != nullptr) {

						const Block* block = instruction->toBlock(*this);
						if(block != nullptr)
							addBlock(block);
					}

					updateBlocks();
				}
			}


		public:
			inline const vector<Instruction*>& getInstructions() const {
				return instructions;
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


			inline const vector<const Block*>& getBlocks() const {
				return blocks;
			}

			inline const Block* getCurrentBlock() const {
				return currentBlock;
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
					log(index, "  end of ", currentBlock->toDebugString());
					currentBlock = currentBlock->parentBlock;
				}

				for(auto i = inactiveBlocks.begin(); i != inactiveBlocks.end(); ) {
					const Block* block = *i;
					if(index >= block->start()) {
						if(block->end() > currentBlock->end()) {
							if(block->end() != static_cast<index_t>(-1))
								throw DecompilationException("Block " + block->toDebugString() +
										" is out of bounds of the parent block " + currentBlock->toDebugString());

							block->endIndex = currentBlock->end();
						}

						log(index, "start of ", block->toDebugString());

						currentBlock->addInnerBlock(block);
						assert(block != currentBlock);

						if(block->parentBlock == nullptr) {
							block->parentBlock = currentBlock; // crutch for tryBlocks
						} else {
							assert(block->parentBlock == currentBlock);
						}

						currentBlock = block;
						inactiveBlocks.erase(i);
					} else
						++i;
				}
			}

			inline uint8_t next() {
				return static_cast<uint8_t>(bytes[++pos]);
			}


			inline int8_t nextByte() {
				return static_cast<int8_t>(next());
			}

			inline uint8_t nextUByte() {
				return static_cast<uint8_t>(next());
			}

			inline int16_t nextShort() {
				return static_cast<int16_t>(nextUShort());
			}

			inline uint16_t nextUShort() {
				return static_cast<uint16_t>(next() << 8 | next());
			}

			inline int32_t nextInt() {
				return static_cast<int32_t>(nextUInt());
			}

			inline uint32_t nextUInt() {
				return static_cast<uint32_t>(next() << 24 | next() << 16 | next() << 8 | next());
			}

			inline uint8_t current() const {
				return bytes[pos];
			}

			inline bool available() const {
				return length - pos > 0;
			}

			Instruction* nextInstruction();

		public:
			inline void skip(int32_t count) {
				pos += static_cast<uint32_t>(count);
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


			template<typename... Args>
			inline void warning(Args... args) const {
				print(cerr << "Disassembler warning: ", args...);
			}
	};
}

#endif
