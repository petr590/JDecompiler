#ifndef JDECOMPILER_ACONST_NULL_INSTRUCTION_CPP
#define JDECOMPILER_ACONST_NULL_INSTRUCTION_CPP

namespace jdecompiler {

	struct AConstNull final: InstructionAndOperation {
		private:
			constexpr AConstNull() noexcept {}

		public:
			virtual string toString(const StringifyContext&) const override {
				return "null";
			}

			virtual const Type* getReturnType() const override {
				return AnyType::getInstance();
			}

			static inline AConstNull* getInstance() {
				static AConstNull instance {};
				return &instance;
			}
	};
}

#endif
