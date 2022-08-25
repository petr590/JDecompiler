#ifndef JDECOMPILER_SWITCH_BLOCK_CPP
#define JDECOMPILER_SWITCH_BLOCK_CPP

namespace jdecompiler {

	struct SwitchBlock: Block {
		public:
			const offset_t defaultOffset;
			const map<jint, offset_t>& offsetTable;

		private:
			mutable bool isEndIndexFixed = false;

		public:
			SwitchBlock(const DisassemblerContext& context, offset_t defaultOffset, const map<jint, offset_t>& offsetTable):
					Block(context.index,
						context.posToIndex(context.pos + max(defaultOffset, max_element(offsetTable.begin(), offsetTable.end(),
							[] (const auto& e1, const auto& e2) { return e1.second < e2.second; })->second)) - 1,
						context),
					defaultOffset(defaultOffset), offsetTable(offsetTable) {}


			inline bool endIndexFixed() const {
				return isEndIndexFixed;
			}

			inline void fixEndIndex() const {
				isEndIndexFixed = true;
			}


			virtual const Scope* toScope(const DecompilationContext& context) const override {
				return new SwitchScope(context, startIndex, endIndex, defaultOffset, offsetTable);
			}
	};
}

#endif
