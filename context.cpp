#ifndef JDECOMPILER_CONTEXT_CPP
#define JDECOMPILER_CONTEXT_CPP

namespace jdecompiler {

	struct Context {
		public:
			pos_t pos = 0;

			constexpr Context() noexcept {}

			Context(const Context&) = delete;

			Context& operator=(const Context&) = delete;


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
