#ifndef JDECOMPILER_UTIL_H
#define JDECOMPILER_UTIL_H

#include <fstream>
#include <cstring>
#include <functional>
#include <sstream>
#define _USE_MATH_DEFINES // constants M_PI, M_E
#include <cmath>
#include <algorithm>
#include <cassert>

#include "jdecompiler-fwd.h"
#include "index-types.cpp"
#include "exceptions.cpp"
#include "util/typenameof.cpp"
#include "util/class-input-stream.cpp"
#include "util/format-string.cpp"
#include "util/file-binary-output-stream.cpp"
#include "util/type-traits.cpp"
#include "util/strutil.cpp"
#include "util/maputil.cpp"
#include "util/instanceof.cpp"
#include "util/safe-cast.cpp"

#define LOG_POINT "[ " __FILE__ " ]: "
#define log(...) logFunc<cout>(LOG_POINT, __VA_ARGS__)
#define logerr(...) logFunc<cerr>(LOG_POINT, __VA_ARGS__)
#define logf(pattern, ...) printf(LOG_POINT pattern "\n", __VA_ARGS__)

namespace jdecompiler {

	static bool isLastSpaceLogged = true;

	template<ostream& out, typename Arg1, typename... Args>
	inline void logFunc(const Arg1& arg1, const Args&... args) {
		if(!isLastSpaceLogged)
			out << ' ';

		out << arg1;

		if constexpr(is_same<Arg1, string>()) {
			if(!arg1.empty())
				isLastSpaceLogged = isspace(arg1.back());

		} else if constexpr(is_same<Arg1, const char*>()) {
			size_t len = strlen(arg1);
			if(len > 0)
				isLastSpaceLogged = isspace(arg1[len - 1]);

		} else if constexpr(is_array<Arg1>() && is_same<array_element_type<Arg1>, char>()) {
			if(array_size<Arg1> > 1)
				isLastSpaceLogged = isspace(arg1[array_size<Arg1> - 2]);

		} else if constexpr(is_same<Arg1, char>() || is_same<Arg1, wchar_t>()) {
			isLastSpaceLogged = isspace(arg1);

		} else {
			isLastSpaceLogged = false;
		}

		if constexpr(sizeof...(Args) > 0) {
			logFunc<out>(args...);
		} else {
			out << endl;
			isLastSpaceLogged = true;
		}
	}


	static inline const string EMPTY_STRING {};


	extern string toLowerCamelCase(const string&);


	extern const char* repeatString(const char*, uint32_t);

	static inline const char* repeatString(const string& str, uint32_t count) {
		return repeatString(str.c_str(), count);
	}


	extern string unescapeString(const char*);

	static inline string unescapeString(const string& str) {
		return unescapeString(str.c_str());
	}


	static inline bool isLetterOrDigit(char c) {
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
	}


	extern string repeat(const string&, size_t);

	/* Concatenates two c-strings, allocates new memory that should be freed */
	extern char* str_concat(const char*, const char*);

	extern char* str_concat(const char*, char);


	static inline constexpr size_t length_of(const char* str) {
		return strlen(str);
	}

	static constexpr size_t length_of(char) {
		return 1;
	}


	template<typename Arg1, typename... Args>
	static inline constexpr size_t total_length_of(Arg1 arg1, Args... args) {
		if constexpr(sizeof...(Args) > 0)
			return length_of(arg1) + total_length_of(args...);
		else
			return length_of(arg1);
	}


	static inline char* str_copy(char* dest, const char* str) {
		return strcpy(dest, str) + strlen(str);
	}

	static inline char* str_copy(char* dest, char c) {
		dest[0] = c;
		return dest + 1;
	}


	template<typename Arg1, typename... Args>
	static inline constexpr char* strs_copy(char* dest, Arg1 arg1, Args... args) {
		dest = str_copy(dest, arg1);

		if constexpr(sizeof...(Args) > 0)
			return strs_copy(dest, args...);
		else
			return dest;
	}


	template<typename Arg1, typename Arg2, typename... Args>
	static char* str_concat(Arg1 arg1, Arg2 arg2, Args... args) {
		const size_t length = total_length_of(arg1, arg2, args...);
		char* result = new char[length + 1];
		result[length] = '\0';

		strs_copy(result, arg1, arg2, args...);

		return result;
	}


	struct IllegalLiteralException: Exception {
		IllegalLiteralException(): Exception() {}
		IllegalLiteralException(const char* message): Exception(message) {}
		IllegalLiteralException(const string& message): Exception(message) {}
	};


	constexpr char32_t operator"" _c32(const char* str, size_t length) {
		char32_t c = 0;

		if(length > 4)
			throw IllegalLiteralException((string)"char32_t literal '" + str + "' too long");

		for(size_t i = 0; i < length; i++) {
			c = (c << 8) | str[i];
		}

		return c;
	}


	extern const char* char32ToString(char32_t);


	extern string encodeUtf8(char32_t);


	extern string escapeUtf16(char32_t);

	extern bool isNotDisplayedChar(char32_t);


	template<char quote>
	extern string charToString(char32_t);

	extern template string charToString<'"'>(char32_t);
	extern template string charToString<'\''>(char32_t);

	extern string stringToLiteral(const string&);

	/*template<typename K, typename V>
	static inline bool has(const umap<K, V>& mp, const K& key) {
		return mp.find(key) != mp.end();
	}*/

	template<typename T>
	static inline bool has(const vector<T>& array, const T& element) {
		return find(array.begin(), array.end(), element) != array.end();
	}


	template<typename T>
	static inline bool has(const vector<T*>& array, const T* element) {
		return find(array.begin(), array.end(), element) != array.end();
	}

	template<typename T>
	static inline bool has_if(const vector<T>& array, const function<bool(T)>& predicate) {
		return find_if(array.begin(), array.end(), predicate) != array.end();
	}

	template<typename T>
	static vector<T> copy_if(const vector<T>& array, const function<bool(T)>& predicate) {
		vector<T> result;
		for(T value : array) {
			if(predicate(value))
				result.push_back(value);
		}
		return result;
	}

	template<typename T>
	static inline bool equal_values(const vector<T*>& v1, const vector<T*>& v2) {
		return v1.size() == v2.size() &&
				equal(v1.begin(), v1.end(), v2.begin(),
					[] (T* arg1, T* arg2) { return *arg1 == *arg2; });
	}

	template<typename T>
	static inline bool equal_values(const vector<T*>& v1, initializer_list<T*>& v2) {
		return equal_values(v1, vector<T*>(v2));
	}

	template<typename T>
	static inline bool equal_values(initializer_list<T*>& v1, const vector<T*>& v2) {
		return equal_values(vector<T*>(v1), v2);
	}

	template<typename T>
	static inline bool equal_values(initializer_list<T*>& v1, initializer_list<T*>& v2) {
		return equal_values(vector<T*>(v1), vector<T*>(v2));
	}

	/* This function works as usual, only a delimiter is added at the end of each line */
	extern vector<string> splitAndAddDelimiter(const string&, char);

	extern bool stringStartsWith(const string&, const string&);
	extern bool stringEndsWith(const string&, const string&);

	template<typename T>
	static inline enable_if_t<is_integral_v<T>, bool> isPowerOfTwo(T x) {
		return (x & (x - 1)) == 0;
	}

	template<class InputIterator, class OutputIterator, class UnaryPredicate, class UnaryOperator>
	OutputIterator transform_if(InputIterator first, InputIterator last, OutputIterator res, UnaryPredicate pred, UnaryOperator op) {

		while(first != last) {
			if(pred(*first))
				*res = op(*first);
			++first, ++res;
		}

		return res;
	}
}

#endif
