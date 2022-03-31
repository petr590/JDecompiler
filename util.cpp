#ifndef JDECOMPILER_UTIL_CPP
#define JDECOMPILER_UTIL_CPP

#undef inline
#include <fstream>
#include <cstring>
#include <functional>
#include <sstream>
#include <math.h>
#include <algorithm>
#define inline FORCE_INLINE

#include "jdecompiler-fwd.h"
#include "typename-of.cpp"
#include "index-types.cpp"
#include "exceptions.cpp"

#define log(...) logFunc("[ jdecompiler/" __FILE__ " ]:", __VA_ARGS__)
#define logf(pattern, ...) printf("[ jdecompiler/" __FILE__ " ]: " pattern, __VA_ARGS__)

namespace jdecompiler {

	template<typename Arg, typename... Args>
	static inline void logFunc(Arg arg, Args... args) {
		cout << arg;
		if constexpr(sizeof...(Args) > 0) {
			cout << ' ';
			logFunc(args...);
		} else {
			cout << endl;
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
			throw CastException((string)"cannot cast " + typeNameOf<B>() + " to " + typeNameOf<T>());
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
	static string rjoin(const vector<T>& array, const function<string(T)> func, const string& separator = ", ") {
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
		virtual const char* getName() const override {
			return "IllegalLiteralException";
		}

		IllegalLiteralException(): Exception() {}
		IllegalLiteralException(const char* message): Exception(message) {}
		IllegalLiteralException(const string& message): Exception(message) {}
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
		if(ch >> 24)
			return new char[5] { (char)(ch >> 24), (char)(ch >> 16), (char)(ch >> 8), (char)ch, '\0' };
		if(ch >> 16)
			return new char[4] { (char)(ch >> 16), (char)(ch >>  8), (char)ch, '\0' };
		if(ch >> 8)
			return new char[3] { (char)(ch >>  8), (char)ch, '\0' };
		if(ch)
			return new char[2] { (char)ch, '\0' };
		return "";
	}


	static string encodeUtf8(char32_t c) {
		if(c < 0x80)       return { (char)c }; // 0xxxxxxx
		if(c < 0x800)      return { (char)((c >> 6 & 0x1F) | 0xC0), (char)((c & 0x3F) | 0x80) }; // 110xxxxx 10xxxxxx
		if(c < 0x10000)    return { (char)((c >> 12 & 0xF) | 0xE0), (char)((c >> 6  & 0x3F) | 0x80), (char)((c & 0x3F) | 0x80) }; // 1110xxxx 10xxxxxx 10xxxxxx
		if(c < 0x200000)   return { (char)((c >> 18 & 0x7) | 0xF0), (char)((c >> 12 & 0x3F) | 0x80), (char)((c >> 6  & 0x3F) | 0x80), (char)((c & 0x3F) | 0x80) }; // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		if(c < 0x4000000)  return { (char)((c >> 24 & 0x3) | 0xF8), (char)((c >> 18 & 0x3F) | 0x80), (char)((c >> 12 & 0x3F) | 0x80), (char)((c >> 6  & 0x3F) | 0x80), (char)((c & 0x3F) | 0x80) }; // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		if(c < 0x80000000) return { (char)((c >> 30 & 0x1) | 0xFC), (char)((c >> 24 & 0x3F) | 0x80), (char)((c >> 18 & 0x3F) | 0x80), (char)((c >> 12 & 0x3F) | 0x80), (char)((c >> 6 & 0x3F) | 0x80), (char)((c & 0x3F) | 0x80) }; // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		throw Exception("Invalid code point U+" + hex(c));
	}



	class BinaryInputStream {
		public:
			static const uint32_t BUFFER_SIZE = 4096;

			const string path;

		private:
			ifstream infile;
			char buffer[BUFFER_SIZE];
			streampos end;
			streampos pos = 0, max = 0;

			void check() {
				if(pos >= max) {
					max = min(infile.tellg() + (streampos)BUFFER_SIZE, end);
					infile.read(buffer, max % BUFFER_SIZE + BUFFER_SIZE);
				}
				if(pos >= end)
					throw EOFException();
			}

			uint8_t next() {
				check();
				pos += 1;
				return (uint8_t)buffer[(pos - (streampos)1) & (BUFFER_SIZE - 1)];
			}

		public:
			BinaryInputStream(const string& path): path(path), infile(path, ios::binary | ios::in) {
				if(!infile.good())
					throw IOException("Cannnot open the file '" + path + '\'');
				infile.seekg(0, ios::end);
				end = infile.tellg();
				infile.seekg(0, ios::beg);
			}

			inline const streampos& getPos() const {
				return pos;
			}

			inline void setPosTo(const streampos& pos) {
				this->pos = pos;
			}

			inline int8_t readByte() {
				return (int8_t)next();
			}

			inline uint8_t readUByte() {
				return (uint8_t)next();
			}

			inline int16_t readShort() {
				return (int16_t)readUShort();
			}

			inline uint16_t readUShort() {
				return (uint16_t)(next() << 8 | next());
			}

			inline int32_t readInt() {
				return (int32_t)readUInt();
			}

			inline uint32_t readUInt() {
				return (uint32_t)(next() << 24 | next() << 16 | next() << 8 | next());
			}

			inline int64_t readLong() {
				return (int64_t)readULong();
			}

			inline uint64_t readULong() {
				return (uint64_t)next() << 56 | (uint64_t)next() << 48 | (uint64_t)next() << 40 | (uint64_t)next() << 32 |
						(uint64_t)next() << 24 | (uint64_t)next() << 16 | (uint64_t)next() << 8 | (uint64_t)next();
			}

			float readFloat() {
				static_assert(sizeof(float) == sizeof(uint32_t));
				float f;
				uint32_t bytes = readUInt();
				memcpy(&f, &bytes, sizeof(float));
				return f;
			}

			double readDouble() {
				static_assert(sizeof(double) == sizeof(uint64_t));
				double d;
				uint64_t bytes = readULong();
				memcpy(&d, &bytes, sizeof(double));
				return d;
			}

			const uint8_t* readBytes(uint32_t size) {
				uint8_t* bytes = new uint8_t[size + 1];
				for(uint32_t i = 0; i < size; i++)
					bytes[i] = next();
				bytes[size] = '\0';

				return bytes;
			}

			inline const char* readString(uint32_t size) {
				return (const char*)readBytes(size);
			}
		#undef BUFFER_SIZE
	};


	class FormatString {
		private: string value;

		public:
			FormatString() noexcept {}
			FormatString(const string& value): value(value) {}


			FormatString operator+ (const char* str) const {
				return FormatString(value + (value.empty() || *str == '\0' ? str : " " + (string)str));
			}

			FormatString operator+ (const string& str) const {
				return FormatString(value + (value.empty() || str.empty() ? str : " " + str));
			}

			FormatString& operator+= (const char* str) {
				value += (value.empty() || *str == '\0' ? str : " " + (string)str);
				return *this;
			}

			FormatString& operator+= (const string& str) {
				value += (value.empty() || str.empty() ? str : " " + str);
				return *this;
			}


			operator string() const {
				return value;
			}
	};


	template<typename T>
	static string numberConstantToString(T value);


	static inline string primitiveToString(bool value) {
		return value ? "true" : "false";
	}

	static inline string primitiveToString(char16_t c) {
		return '\'' + (c < 0x20 ? "\\u" + hex<4>(c) : encodeUtf8(c)) + '\'';
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
		if(isnan(num)) return "Float.NaN";
		if(!isfinite(num)) return num > 0 ? "Float.POSITIVE_INFINITY" : "Float.NEGATIVE_INFINITY";

		ostringstream out;
		out.precision(9);

		out << num;
		if(floor(num) == num)
			out << ".0";
		out << 'f';

		return out.str();
	}

	static string primitiveToString(double num) {
		if(isnan(num)) return "Double.NaN";
		if(!isfinite(num)) return num > 0 ? "Double.POSITIVE_INFINITY" : "Double.NEGATIVE_INFINITY";

		ostringstream out;
		out.precision(17);

		out << num;
		if(floor(num) == num)
			out << ".0";

		return out.str();
	}

	static string stringToLiteral(const string& str) {
		#define checkLength(n) if(bytes + n >= end) throw DecompilationException("Unexpected end of the string")
		string result("\"");
		const char* bytes = str.c_str();
		for(const char* end = bytes + strlen(bytes); bytes < end; bytes++) {
			char32_t ch = *bytes & 0xFF;
			char32_t code = ch;
			switch(ch) {
				case '"': result += "\\\""; break;
				case '\b': result += "\\b"; break;
				case '\t': result += "\\t"; break;
				case '\n': result += "\\n"; break;
				case '\f': result += "\\f"; break;
				case '\r': result += "\\r"; break;
				case '\\': result += "\\\\"; break;
				default:
					if((ch & 0xE0) == 0xC0) {
						checkLength(1);
						ch = (ch << 8) | (*++bytes & 0xFF);
						code = (ch & 0x1F00) >> 2 | (ch & 0x3F);
					} else if((ch & 0xF0) == 0xE0) {
						if(ch == 0xED) {
							checkLength(5);
							result += encodeUtf8((char32_t)(0x10000 | (*++bytes & 0xF) << 16 |
									(*++bytes & 0x3F) << 10 | (*(bytes += 2) & 0xF) << 6 | (*++bytes & 0x3F)));
							continue;
						}
						checkLength(2);
						ch = (ch << 16) | (*++bytes & 0xFF) << 8 | (*++bytes & 0xFF);
						code = (ch & 0xF0000) >> 4 | (ch & 0x3F00) >> 2 | (ch & 0x3F);
					}
					result += code < 0x20 ? "\\u" + hex<4>(code) : char32ToString(ch);
			}
		}
		return result + '"';

		#undef checkLength
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



	template<typename T>
	struct Stack {
		private:
			class Entry {
				public:
					const T value;
					const Entry* const next;

					Entry(T value, const Entry* next): value(value), next(next) {}

					void deleteNext() const {
						if(next != nullptr) {
							next->deleteNext();
							delete next;
						}
					}
			};

			const Entry* firstEntry;
			uint16_t length;

		protected:
			inline void checkEmptyStack() const {
				if(firstEntry == nullptr)
					throw EmptyStackException();
			}

		public:
			Stack(): firstEntry(nullptr), length(0) {}

			Stack(T value): firstEntry(new Entry(value, nullptr)), length(1) {}

			void push(T value) {
				firstEntry = new Entry(value, firstEntry);
				length++;
			}

			inline void push(T value, T operations...) {
				push(value);
				push(operations);
			}

			T pop() {
				checkEmptyStack();

				const Entry copiedEntry = *firstEntry;
				delete firstEntry;
				firstEntry = copiedEntry.next;
				length--;
				return copiedEntry.value;
			}

			T top() const {
				checkEmptyStack();
				return firstEntry->value;
			}

			T lookup(uint16_t index) const {
				checkEmptyStack();

				if(index >= length)
					throw StackIndexOutOfBoundsException(index, length);

				const Entry* currentEntry = firstEntry;
				for(uint16_t i = 0; i < index; i++)
					currentEntry = currentEntry->next;
				return currentEntry->value;
			}

			inline uint16_t size() const {
				return length;
			}

			inline bool empty() const {
				return length == 0;
			}

			~Stack() {
				if(firstEntry != nullptr) {
					firstEntry->deleteNext();
					delete firstEntry;
				}
			}
	};
}
#endif
