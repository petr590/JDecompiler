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
#include "util/stack.cpp"
#include "util/type-traits.cpp"
#include "util/strutil.cpp"
#include "util/maputil.cpp"
#include "util/instanceof.cpp"

#define LOG_POINT "[ jdecompiler/" __FILE__ " ]:"
#define log(...) logFunc<cout>(LOG_POINT, __VA_ARGS__)
#define logerr(...) logFunc<cerr>(LOG_POINT, __VA_ARGS__)
#define logf(pattern, ...) printf(LOG_POINT pattern, __VA_ARGS__)

namespace jdecompiler {

	template<ostream& out, typename Arg, typename... Args>
	inline void logFunc(const Arg& arg, const Args&... args) {
		out << arg;
		if constexpr(sizeof...(Args) > 0) {
			out << ' ';
			logFunc<out>(args...);
		} else {
			out << endl;
		}
	}


	static inline const string EMPTY_STRING {};


	template<class T, class B>
	static inline T safe_cast(B o) {
		T t = dynamic_cast<T>(o);
		if(t == nullptr && o != nullptr)
			throw CastException((string)"cannot cast " + typenameof(o) + " to " + typenameof<T>());
		return t;
	}



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


	template<typename Arg, typename... Args>
	static inline constexpr size_t total_length_of(Arg arg, Args... args) {
		if constexpr(sizeof...(Args) > 0)
			return length_of(arg) + total_length_of(args...);
		else
			return length_of(arg);
	}


	static inline char* str_copy(char* dest, const char* str) {
		return strcpy(dest, str) + strlen(str);
	}

	static inline char* str_copy(char* dest, char c) {
		dest[0] = c;
		return dest + 1;
	}


	template<typename Arg, typename... Args>
	static inline constexpr char* strs_copy(char* dest, Arg arg, Args... args) {
		dest = str_copy(dest, arg);

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

	/* The function works as usual, only a delimiter is added at the end of each line */
	extern vector<string> splitAndAddDelimiter(const string&, char);

	extern bool stringStartsWith(const string&, const string&);

	template<typename T, typename = enable_if_t<is_integral_v<T>>>
	static inline bool isPowerOfTwo(T x) {
		return (x & (x - 1)) == 0;
	}
}

#endif
