#ifndef UTIL_TYPE_TRAITS_CPP
#define UTIL_TYPE_TRAITS_CPP

#include <type_traits>

namespace util {

	using std::is_same;

	template<bool v>
	struct bool_type {
		static constexpr bool value = v;

		inline constexpr bool_type() noexcept {}

		inline constexpr operator bool () const noexcept { return v; }
	};

	typedef bool_type<false> false_type;
	typedef bool_type<true> true_type;


	template<typename...>
	struct is_one_of: false_type {};

	template<typename F, typename S, typename... T>
	struct is_one_of<F, S, T...>: bool_type<is_same<F, S>() || is_one_of<F, T...>()> {};


	template<typename>
	struct is_float: false_type {};

	template<>
	struct is_float<float>: true_type {};
}

#endif
