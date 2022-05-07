#ifndef JDECOMPILER_UTIL_CPP
#define JDECOMPILER_UTIL_CPP

#undef inline
#include <fstream>
#include <cstring>
#include <functional>
#include <sstream>
#include <math.h>
#include <algorithm>
#define inline INLINE

#include "jdecompiler-fwd.h"
#include "typename-of.cpp"
#include "index-types.cpp"
#include "exceptions.cpp"
#include "util/binary-input-stream.cpp"
#include "util/stack.cpp"

#define LOG_POINT "[ jdecompiler/" __FILE__ " ]:"
#define log(...) logFunc<cout>(LOG_POINT, __VA_ARGS__)
#define logerr(...) logFunc<cerr>(LOG_POINT, __VA_ARGS__)
#define logf(pattern, ...) printf(LOG_POINT pattern, __VA_ARGS__)

namespace jdecompiler {

	template<ostream& out, typename Arg, typename... Args>
	static inline void logFunc(const Arg& arg, const Args&... args) {
		out << arg;
		if constexpr(sizeof...(Args) > 0) {
			out << ' ';
			logFunc<out>(args...);
		} else {
			out << endl;
		}
	}


	static const string EMPTY_STRING = string();


	template<typename T, typename O>
	static inline bool instanceof(O o) {
		return dynamic_cast<T>(o) != nullptr;
	}


	template<class T, class B>
	static T safe_cast(B o) {
		T t = dynamic_cast<T>(o);
		if(t == nullptr && o != nullptr)
			throw CastException((string)"cannot cast " + typeNameOf(o) + " to " + typeNameOf<T>());
		return t;
	}



	static string toLowerCamelCase(const string& str) {
		string result;
		const size_t strlength = str.size();

		size_t i = 0;
		for(char c = str[0]; i < strlength && c >= 'A' && c <= 'Z'; c = str[++i])
			result += (char)tolower(c);
		while(i < strlength)
			result += str[i++];

		return result;
	}


	static const char* repeatString(const char* str, uint32_t count) {
		const size_t
				strlength = strlen(str),
				resultlength = strlength * count;

		char* result = strncpy(new char[resultlength + 1], str, resultlength + 1);

		for(uint32_t i = 0; i < resultlength; i++)
			result[i] = str[i % strlength];
		result[resultlength] = '\0';

		return result;
	}

	static inline const char* repeatString(const string& str, uint32_t count) {
		return repeatString(str.c_str(), count);
	}


	static string unescapeString(const char* const str) {
		string result;

		for(const char* i = str; *i != '\0'; i++) {
			if(*i == '\\') {
				switch(*(++i)) {
					case 'b': result += '\b'; break;
					case 't': result += '\t'; break;
					case 'n': result += '\n'; break;
					case 'f': result += '\f'; break;
					case 'r': result += '\r'; break;
					default:
						result += '\\';
						result += *i;
				}
			} else {
				result += *i;
			}
		}

		return result;
	}

	static inline string unescapeString(const string& str) {
		return unescapeString(str.c_str());
	}


	static inline bool isLetterOrDigit(char c) {
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
	}


	template<typename T>
	static string join(const vector<T>& array, const function<string(T)> func, const string& separator = ", ") {
		string result;
		const size_t size = array.size();

		if(size > 0) {
			size_t i = 0;
			while(true) {
				result += func(array[i]);
				if(++i == size) break;
				result += separator;
			}
		}

		return result;
	}


	template<typename T>
	static string join(const vector<T>& array, const function<string(T, size_t)> func, const string& separator = ", ") {
		string result;
		const size_t size = array.size();

		if(size > 0) {
			size_t i = 0;
			while(true) {
				result += func(array[i], i);
				if(++i == size) break;
				result += separator;
			}
		}

		return result;
	}


	template<typename T>
	static string rjoin(const vector<T>& array, const function<string(T)>& func, const string& separator = ", ") {
		string result;
		size_t i = array.size();

		if(i > 0) {
			while(true) {
				i--;
				result += func(array[i]);
				if(i == 0) break;
				result += separator;
			}
		}

		return result;
	}

	struct IllegalLiteralException: Exception {
		IllegalLiteralException(): Exception() {}
		IllegalLiteralException(const char* message): Exception(message) {}
		IllegalLiteralException(const string& message): Exception(message) {}
	};



	class FormatString {
		private: string value;

		public:
			FormatString() noexcept {}
			FormatString(const string& value): value(value) {}


			friend FormatString operator+ (const FormatString& formatStr, const char* str) {
				return FormatString(formatStr.value + (formatStr.empty() || *str == '\0' ? str : " " + (string)str));
			}

			friend FormatString operator+ (const FormatString& formatStr, const string& str) {
				return FormatString(formatStr.value + (formatStr.empty() || str.empty() ? str : " " + str));
			}

			friend FormatString operator+ (const char* str, const FormatString& formatStr) {
				return FormatString(str + (formatStr.empty() || *str == '\0' ? (string)formatStr : " " + (string)formatStr));
			}

			friend FormatString operator+ (const string& str, const FormatString& formatStr) {
				return FormatString(str + (formatStr.empty() || str.empty() ? (string)formatStr : " " + (string)formatStr));
			}


			friend FormatString& operator+= (FormatString& formatStr, const char* str) {
				formatStr.value += (formatStr.empty() || *str == '\0' ? str : " " + (string)str);
				return formatStr;
			}

			friend FormatString& operator+= (FormatString& formatStr, const string& str) {
				formatStr.value += (formatStr.empty() || str.empty() ? str : " " + str);
				return formatStr;
			}


			inline bool empty() const {
				return value.empty();
			}


			inline operator string() const {
				return value;
			}
	};



	static constexpr char32_t operator"" _c32(const char* const str, size_t length) {
		char32_t c = 0;

		if(length > 4)
			throw IllegalLiteralException((string)"char32_t literal '" + str + "' too long");

		for(size_t i = 0; i < length; i++) {
			c = (c << 8) | str[i];
		}

		return c;
	}


	static const char* char32ToString(char32_t ch) {
		if(ch >> 24 != 0)
			return new char[5] { (char)(ch >> 24), (char)(ch >> 16), (char)(ch >> 8), (char)ch, '\0' };
		if(ch >> 16 != 0)
			return new char[4] { (char)(ch >> 16), (char)(ch >>  8), (char)ch, '\0' };
		if(ch >> 8 != 0)
			return new char[3] { (char)(ch >>  8), (char)ch, '\0' };
		if(ch != 0)
			return new char[2] { (char)ch, '\0' };
		return "";
	}


	static string encodeUtf8(char32_t c) {
		// 0xxxxxxx
		if(c < 0x80)       return { (char)c };
		// 110xxxxx 10xxxxxx
		if(c < 0x800)      return { (char)((c >>  6 & 0x1F) | 0xC0), (char)((c & 0x3F) | 0x80) };
		// 1110xxxx 10xxxxxx 10xxxxxx
		if(c < 0x10000)    return { (char)((c >> 12 &  0xF) | 0xE0), (char)((c >>  6 & 0x3F) | 0x80), (char)((c >>  0 & 0x3F) | 0x80) };
		// 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		if(c < 0x200000)   return { (char)((c >> 18 &  0x7) | 0xF0), (char)((c >> 12 & 0x3F) | 0x80), (char)((c >>  6 & 0x3F) | 0x80),
									(char)((c >>  0 & 0x3F) | 0x80) };
		 // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		if(c < 0x4000000)  return { (char)((c >> 24 &  0x3) | 0xF8), (char)((c >> 18 & 0x3F) | 0x80), (char)((c >> 12 & 0x3F) | 0x80),
									(char)((c >>  6 & 0x3F) | 0x80), (char)((c >>  0 & 0x3F) | 0x80) };
		// 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		if(c < 0x80000000) return { (char)((c >> 30 &  0x1) | 0xFC), (char)((c >> 24 & 0x3F) | 0x80), (char)((c >> 18 & 0x3F) | 0x80),
									(char)((c >> 12 & 0x3F) | 0x80), (char)((c >>  6 & 0x3F) | 0x80), (char)((c >>  0 & 0x3F) | 0x80) };

		throw Exception("Char code U+" + hex(c) + " is too large for encode");
	}


	// surrogate pairs in UTF-16: 0xD800-0xDFFF
	static inline string escapeUtf16(char32_t ch) {
		assert(ch <= 0x10FFFF);

		/*if(ch >= 0xD800 && ch < 0xE000)
			return ; // TODO*/

		if(ch > 0xFFFF) {
			ch -= 0x10000;
			return "\\u" + hex<4>(((ch >> 10) & 0x3FF) | 0xD800) + "\\u" + hex<4>((ch & 0x3FF) | 0xDC00);
		}

		return "\\u" + hex<4>(ch);
	}

	static inline bool isNotDisplayedChar(char32_t ch) {
		return ch < 0x20 || (ch >= 0x7F && ch < 0xA0) // Control characters
			|| (ch >= 0xD800 && ch < 0xE000);         // Surrogate pairs in UTF-16
	}


	template<char quote>
	static string charToString(char32_t ch) {
		static_assert(quote == '"' || quote == '\'', "Invalid quote");

		switch(ch) {
			case '\b': return "\\b";
			case '\t': return "\\t";
			case '\n': return "\\n";
			case '\f': return "\\f";
			case '\r': return "\\r";
			case '\\': return "\\\\";
			case quote:return (string)"\\" + quote;
			default:   return isNotDisplayedChar(ch) /*|| (JDecompiler::getInstance().escapeUnicodeChars() && ch >= 0x80)*/ ? escapeUtf16(ch) : encodeUtf8(ch);
		}
	}

	static string stringToLiteral(const string& str) {
		#define check(condition, message) if(!(condition)) throw DecompilationException(message)
		#define checkLength(n) check(bytes + n < end, "Unexpected end of the string")
		#define checkEncoding(condition) check(condition, "Invalid string")

		const char* bytes = str.c_str();

		string result("\"");

		for(const char* end = bytes + strlen(bytes); bytes < end; bytes++) {
			char32_t ch = *bytes & 0xFF;

			if((ch & 0xE0) == 0xC0) {
				checkLength(1);
				checkEncoding((bytes[1] & 0xC0) == 0x80);

				ch = (ch & 0x1F) << 6 | (bytes[1] & 0x3F);

				bytes++;

			} else if((ch & 0xF0) == 0xE0) {
				if(ch == 0xED) {
					checkLength(5);
					checkEncoding((bytes[1] & 0xF0) == 0xA0 && (bytes[2] & 0xC0) == 0x80 && (bytes[3] & 0xFF) == 0xED
					           && (bytes[4] & 0xF0) == 0xB0 && (bytes[5] & 0xC0) == 0x80);

					result += encodeUtf8((char32_t)(0x10000 | (bytes[1] & 0xF) << 16 |
							(bytes[2] & 0x3F) << 10 | (bytes[4] & 0xF) << 6 | (bytes[5] & 0x3F)));
					bytes += 5;
					continue;
				}

				checkLength(2);
				checkEncoding((bytes[1] & 0xC0) == 0x80 && (bytes[2] & 0xC0) == 0x80);

				ch = (ch & 0xF) << 12 | (bytes[1] & 0x3F) << 6 | (bytes[2] & 0x3F);

				bytes += 2;
			}

			check(ch <= 0x10FFFF && (ch < 0xD800 || ch >= 0xE000), "Invalid string: char code U+" + hex(ch) + " is out of range");

			result += charToString<'"'>(ch);
		}
		return result + '"';

		#undef checkEncoding
		#undef checkLength
		#undef check
	}

	static inline string primitiveToString(const string& str) {
		return stringToLiteral(str);
	}


	template<typename T>
	static string numberConstantToString(T value);


	static inline string primitiveToString(bool value) {
		return value ? "true" : "false";
	}

	static inline string primitiveToString(char16_t c) {
		return '\'' + charToString<'\''>(c) + '\'';
	}

	static inline string primitiveToString(int8_t num) { // byte
		return numberConstantToString(num);
	}

	static inline string primitiveToString(int16_t num) { // short
		return numberConstantToString(num);
	}

	static inline string primitiveToString(int32_t num) { // int
		return numberConstantToString(num);
	}

	static inline string primitiveToString(int64_t num) { // long
		return numberConstantToString(num) + 'l';
	}

	static string primitiveToString(float num) {
		if(isnan(num)) return "(0.0f / 0.0f)";
		if(!isfinite(num)) return num > 0 ? "(1.0f / 0.0f)" : "(-1.0f / 0.0f)";

		ostringstream out;
		out.precision(9);

		out << num;
		if(floor(num) == num)
			out << ".0";
		out << 'f';

		return out.str();
	}

	static string primitiveToString(double num) {
		if(isnan(num)) return "(0.0 / 0.0)";
		if(!isfinite(num)) return num > 0 ? "(1.0 / 0.0)" : "(-1.0 / 0.0)";

		ostringstream out;
		out.precision(17);

		out << num;
		if(floor(num) == num)
			out << ".0";

		return out.str();
	}


	template<typename T>
	inline bool has(const vector<T>& array, const T& element) {
		return find(array.begin(), array.end(), element) != array.end();
	}


	template<typename T>
	inline bool has(const vector<T*>& array, const T* element) {
		return find(array.begin(), array.end(), element) != array.end();
	}

	template<typename T>
	static inline bool has_if(const vector<T>& array, const function<bool(T)>& predicate) {
		return find_if(array.begin(), array.end(), predicate) != array.end();
	}

	template<typename T>
	static inline vector<T> copy_if(const vector<T>& array, const function<bool(T)>& predicate) {
		vector<T> result;
		for(T value : array) {
			if(predicate(value))
				result.push_back(value);
		}
		return result;
	}

	template<typename K, typename V>
	inline bool has(const map<K, V>& mp, const K& key) {
		return mp.find(key) != mp.end();
	}


	template<bool v>
	struct bool_type {
		static constexpr bool value = v;

		inline constexpr bool_type() noexcept {}

		inline constexpr operator bool () const noexcept { return v; }
	};

	struct false_type: bool_type<false> {};

	struct true_type: bool_type<true> {};


	template<typename...>
	struct is_one_of: false_type {};

	template<typename F, typename S, typename... T>
	struct is_one_of<F, S, T...>: bool_type<is_same<F, S>() || is_one_of<F, T...>()> {};


	template<typename T>
	struct is_float: false_type {};

	template<>
	struct is_float<float>: true_type {};



	static inline uint64_t toRawBits(float f) {
		return *(uint64_t*)&f;
	}

	static inline uint64_t toRawBits(double d) {
		return *(uint64_t*)&d;
	}
}
#endif
