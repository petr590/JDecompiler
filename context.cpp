#ifndef JDECOMPILER_CONTEXT_CPP
#define JDECOMPILER_CONTEXT_CPP

namespace jdecompiler {

	struct Context {
		public:
			pos_t pos = 0;

			inline constexpr Context() noexcept {}


		protected:
			template<typename Arg, typename... Args>
			inline void print(ostream& out, Arg arg, Args... args) const {
				if constexpr(sizeof...(Args) > 0)
					print(out << arg, args...);
				else
					out << arg << endl;
			}
	};
}

#endif
