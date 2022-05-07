#ifndef JDECOMPILER_TYPENAME_OF_CPP
#define JDECOMPILER_TYPENAME_OF_CPP

#undef inline
#include <regex>
#define inline INLINE

namespace jdecompiler {
	using std::regex;
	using std::regex_replace;


	static inline string undecorateTypeName(const char* name) {
		#ifdef __GNUC__
			static const regex
					namespaceStartRegex("^N\\d+"),
					namespaceEndRegex("E$"),
					namespaceRegex("\\d+"),

					startRegex("^\\d+");

			if(name[0] == 'N' && isdigit(name[1]))
				return regex_replace(regex_replace(regex_replace(name, namespaceStartRegex, ""), namespaceEndRegex, ""), namespaceRegex, "::");
			else
				return regex_replace(name, startRegex, "");
		#else
			return name;
		#endif
	}



	template<typename T>
	static inline constexpr string typeNameOf() {
		if constexpr(is_pointer<T>())
			return typeNameOf<remove_pointer_t<T>>() + '*';
		else if constexpr(is_const<T>())
			return "const " + typeNameOf<remove_const_t<T>>();
		else if constexpr(is_volatile<T>())
			return "volatile " + typeNameOf<remove_volatile_t<T>>();
		else
			return undecorateTypeName(typeid(T).name());
	}


	template<> inline string typeNameOf<void>() {
		return "void";
	}

	template<> inline string typeNameOf<char>() {
		return "char";
	}

	template<> inline string typeNameOf<short>() {
		return "short";
	}

	template<> inline string typeNameOf<int>() {
		return "int";
	}

	template<> inline string typeNameOf<long>() {
		return "long";
	}

	template<> inline string typeNameOf<long long>() {
		return "long long";
	}


	template<> inline string typeNameOf<signed char>() {
		return "signed char";
	}


	template<> inline string typeNameOf<unsigned char>() {
		return "unsigned char";
	}

	template<> inline string typeNameOf<unsigned short>() {
		return "unsigned short";
	}

	template<> inline string typeNameOf<unsigned int>() {
		return "unsigned int";
	}

	template<> inline string typeNameOf<unsigned long>() {
		return "unsigned long";
	}

	template<> inline string typeNameOf<unsigned long long>() {
		return "unsigned long long";
	}


	template<> inline string typeNameOf<float>() {
		return "float";
	}

	template<> inline string typeNameOf<double>() {
		return "double";
	}

	template<> inline string typeNameOf<long double>() {
		return "long double";
	}


	template<> inline string typeNameOf<void*>() {
		return "void*";
	}

	template<> inline string typeNameOf<const void*>() {
		return "const void*";
	}



	template<class T>
	static inline string typeNameOf(T& value) {
		if constexpr(is_fundamental<T>() || is_same<remove_cv_t<remove_pointer_t<T>>, void>() /* const or volatile void* */)
			return typeNameOf<T>();
		else if constexpr(is_pointer<T>())
			return typeNameOf<remove_pointer_t<T>>(*value) + '*';
		else if constexpr(is_const<T>())
			return "const " + typeNameOf<remove_const_t<T>>(const_cast<remove_const_t<T>&>(value));
		else if constexpr(is_volatile<T>())
			return "volatile " + typeNameOf<remove_volatile_t<T>>(const_cast<remove_volatile_t<T>&>(value));
		else
			return undecorateTypeName(typeid(value).name());
	}
}

#endif
