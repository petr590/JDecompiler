#ifndef JDECOMPILER_SPECIAL_TYPE_CPP
#define JDECOMPILER_SPECIAL_TYPE_CPP

namespace jdecompiler {


	struct SpecialType: Type {
		protected:
			constexpr SpecialType() noexcept {}

		public:
			virtual bool isBasic() const override final {
				return false;
			}
	};
}

#endif
