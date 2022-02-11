#ifndef JDECOMPILER_UTIL_CPP
#define JDECOMPILER_UTIL_CPP

#include <fstream>
#include <cstring>
#include <vector>
#include <functional>
#include <iostream> // DEBUG
#include <sstream>
#include <math.h>
#include <algorithm>
#include "jdecompiler.h"

#define inline INLINE_ATTR

#undef LOG_PREFIX
#define LOG_PREFIX "[ jdecompiler-util.cpp ]"

using namespace std;

static const string EMPTY_STRING = string();

template<uint16_t length>
static string hex(uint64_t n) {
	static const char* digits = "0123456789ABCDEF";

	char str[length + 1u];
	str[length] = '\0';

	for(uint16_t i = length; i-- > 0; ) {
		str[i] = digits[n & 0xF];
		n >>= 4;
	}

	return str;
}

static string hex(uint64_t n) {
	if(n == 0) return "0";

	static const char* digits = "0123456789ABCDEF";
	string str;

	while(n != 0) {
		str += digits[n & 0xF];
		n >>= 4;
	}

	reverse(str.begin(), str.end());

	return str;
}


struct Exception: exception {
	//#define EXCEPTION_NAME(name) virtual const char* getName() const noexcept override { return "name"; }

	protected:
		const string message;

	public:
		Exception() {}
		Exception(const char* message): message(message) {}
		Exception(const string& message): message(message) {}

		virtual const char* what() const noexcept override {
			return message.c_str();
		}
};


struct IndexOutOfBoundsException: Exception {
	IndexOutOfBoundsException(const string& message): Exception(message) {}
	IndexOutOfBoundsException(uint32_t index, uint32_t length): Exception("Index " + to_string(index) + " out of bounds for length " + to_string(length)) {}
};

struct BytecodeIndexOutOfBoundsException: IndexOutOfBoundsException {
	BytecodeIndexOutOfBoundsException(uint32_t index, uint32_t length): IndexOutOfBoundsException(index, length) {}
};

struct StackIndexOutOfBoundsException: IndexOutOfBoundsException {
	StackIndexOutOfBoundsException(uint32_t index, uint32_t length): IndexOutOfBoundsException(index, length) {}
};

struct ConstantPoolIndexOutOfBoundsException: IndexOutOfBoundsException {
	ConstantPoolIndexOutOfBoundsException(uint32_t index, uint32_t length): IndexOutOfBoundsException(index, length) {}
};


struct IllegalArgumentException: Exception {
	IllegalArgumentException(const string& message): Exception(message) {}
};

struct IllegalStateException: Exception {
	IllegalStateException(const string& message): Exception(message) {}
};

struct DynamicCastException: Exception {
	DynamicCastException(const string& message): Exception(message) {}
};

struct AssertionException: Exception {
	AssertionException(const string& message): Exception(message) {}
};


struct DecompilationException: Exception {
	DecompilationException(): Exception() {}
	DecompilationException(const string& message): Exception(message) {}
};


struct IllegalTypeNameException: DecompilationException {
	IllegalTypeNameException(const string& message): DecompilationException(message) {}
};

struct IllegalSignatureException: IllegalTypeNameException {
	IllegalSignatureException(const string& message): IllegalTypeNameException(message) {}
};

struct IllegalModifiersException: DecompilationException {
	IllegalModifiersException(uint16_t modifiers): DecompilationException("0x" + hex<4>(modifiers)) {}
		IllegalModifiersException(const string& message): DecompilationException(message) {}
};

struct IllegalMethodDescriptorException: DecompilationException {
	IllegalMethodDescriptorException(const string& descriptor): DecompilationException(descriptor) {}
};

struct IllegalConstantPointerException: DecompilationException {
	IllegalConstantPointerException(const string& message): DecompilationException(message) {}
};

struct IllegalStackStateException: DecompilationException {
	IllegalStackStateException() {}
	IllegalStackStateException(const string& message): DecompilationException(message) {}
};

struct EmptyStackException: IllegalStackStateException {
	EmptyStackException() {}
	EmptyStackException(const string& message): IllegalStackStateException(message) {}
};

struct TypeSizeMismatchException: DecompilationException {
	TypeSizeMismatchException(const string& message): DecompilationException(message) {}
	TypeSizeMismatchException(const string& requiredSizeName, const string& sizeName, const string& typeName):
			DecompilationException("Required " + requiredSizeName + ", got " + sizeName + " of type " + typeName) {}
};


struct ClassFormatError: Exception {
	ClassFormatError(const string& message): Exception(message) {}
};

struct IllegalOpcodeException: ClassFormatError {
	IllegalOpcodeException(const string& message): ClassFormatError(message) {}
};

struct InstructionFormatError: ClassFormatError {
	InstructionFormatError(const string& message): ClassFormatError(message) {}
};

struct IllegalAttributeException: ClassFormatError {
	IllegalAttributeException(const string& message): ClassFormatError(message) {}
};

struct AttributeNotFoundException: ClassFormatError {
	AttributeNotFoundException(const string& message): ClassFormatError(message) {}
};


struct IOException: Exception {
	IOException(): Exception() {}
	IOException(const string& message): Exception(message) {}
};

struct EOFException: IOException {
	EOFException(): IOException() {}
};



struct CastException: Exception {
	CastException(): Exception() {}
	CastException(const string& message): Exception(message) {}
};

template<class T, class B>
static T safe_cast(B o) {
	T t = dynamic_cast<T>(o);
	if(t == nullptr && o != nullptr)
		throw CastException((string)"cannot cast " + typeid(B).name() + " to " + typeid(T).name());
	return t;
}



static string toLowerCamelCase(const string& str) {
	string result;
	const size_t strlength = str.size();

	size_t i = 0;
	for(char c = str[0]; i < strlength && c >= 'A' && c <= 'Z'; c = str[++i])
		result += tolower(c);
	while(i < strlength)
		result += str[i++];

	return result;
}


static const char* repeatString(const char* str, uint16_t count) {
	const size_t
			strlength = strlen(str),
			resultlength = strlength * count;

	char* result = strncpy(new char[resultlength + 1], str, resultlength + 1);

	for(uint16_t i = 0; i < resultlength; i++)
		result[i] = str[i % strlength];
	result[resultlength] = '\0';

	return result;
}


static inline bool isLetterOrDigit(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}


template<typename T>
static string join(const vector<T>& array, const function<string(T)> func, const string& separator = ", ") {
	string result;
	const uint32_t size = array.size();

	if(size > 0) {
		uint32_t i = 0;
		while(true) {
			result += func(array[i]);
			if(++i == size) break;
			result += separator;
		}
	}

	return result;
}


template<typename T>
static string join(const vector<T>& array, const function<string(T, uint32_t)> func, const string& separator = ", ") {
	string result;
	const uint32_t size = array.size();

	if(size > 0) {
		uint32_t i = 0;
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
	uint32_t i = array.size();

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


static string char32ToString(char32_t ch) {
	if(ch >> 24)
		return { (char)(ch >> 24), (char)(ch >> 16), (char)(ch >> 8), (char)ch };
	if(ch >> 16)
		return { (char)(ch >> 16), (char)(ch >>  8), (char)ch };
	if(ch >> 8)
		return { (char)(ch >>  8), (char)ch };
	return { (char)ch };
}


static string encodeUtf8(char32_t c) {
	if(c < 0x80)       return { (char)c }; // 0xxxxxxx
	if(c < 0x800)      return { (char)((c >> 6 & 0x1F) | 0xC0), (char)((c & 0x3F) | 0x80) }; // 110xxxxx 10xxxxxx
	if(c < 0x10000)    return { (char)((c >> 12 & 0xF) | 0xE0), (char)((c >> 6 & 0x3F) | 0x80), (char)((c & 0x3F) | 0x80) }; // 1110xxxx 10xxxxxx 10xxxxxx
	if(c < 0x200000)   return { (char)((c >> 18 & 0x7) | 0xF0), (char)((c >> 12 & 0x3F) | 0x80), (char)((c >> 6 & 0x3F) | 0x80), (char)((c & 0x3F) | 0x80) }; // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
	if(c < 0x4000000)  return { (char)((c >> 24 & 0x3) | 0xF8), (char)((c >> 18 & 0x3F) | 0x80), (char)((c >> 12 & 0x3F) | 0x80), (char)((c >> 6 & 0x3F) | 0x80), (char)((c & 0x3F) | 0x80) }; // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
	if(c < 0x80000000) return { (char)((c >> 30 & 0x1) | 0xFC), (char)((c >> 24 & 0x3F) | 0x80), (char)((c >> 18 & 0x3F) | 0x80), (char)((c >> 12 & 0x3F) | 0x80), (char)((c >> 6 & 0x3F) | 0x80), (char)((c & 0x3F) | 0x80) }; // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
	throw Exception("Invalid code point U+" + hex(c));
}



class BinaryInputStream {
	#define bufferSize 4096

	private:
		ifstream infile;
		char buffer[bufferSize];
		streampos end;
		streampos pos = 0, max = 0;

		void check() {
			if(pos >= max) {
				max = min(infile.tellg() + (streampos)bufferSize, end);
				infile.read(buffer, max % bufferSize + bufferSize);
			}
			if(pos >= end)
				throw EOFException();
		}

		uint8_t next() {
			check();
			pos += 1;
			return (uint8_t)buffer[(pos - (streampos)1) & 0xFFF];
		}

	public:
		BinaryInputStream(const string& path): infile(path, ios::binary | ios::in) {
			if(!infile.good())
				throw IOException("Cannnot open the file '" + path + "'");
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

		inline uint8_t readByte() {
			return next();
		}

		inline char readChar() {
			return (char)next();
		}

		inline uint16_t readShort() {
			return (uint16_t)(next() << 8 | next());
		}

		inline uint32_t readInt() {
			return (uint32_t)(next() << 24 | next() << 16 | next() << 8 | next());
		}

		inline uint64_t readLong() {
			return (uint64_t)next() << 56 | (uint64_t)next() << 48 | (uint64_t)next() << 40 | (uint64_t)next() << 32 |
					(uint64_t)(next() << 24 | next() << 16 | next() << 8 | next());
		}

		float readFloat() {
			float f;
			uint32_t bytes = readInt();
			memcpy(&f, &bytes, sizeof(bytes));
			return f;
		}

		double readDouble() {
			double d;
			uint64_t bytes = readLong();
			memcpy(&d, &bytes, sizeof(bytes));
			return d;
		}

		const char* readBytes(uint32_t size) {
			char* bytes = new char[size + 1];
			for(uint32_t i = 0; i < size; i++)
				bytes[i] = next();
			bytes[size] = '\0';

			return bytes;
		}
};


namespace JDecompiler {

	class FormatString {
		private: string value;

		public:
			FormatString() {}
			FormatString(const string& value): value(value) {}


			FormatString operator+(const char* str) const {
				return FormatString(value + (value.empty() || *str == '\0' ? str : " " + (string)str));
			}

			FormatString operator+(const string& str) const {
				return FormatString(value + (value.empty() || str.empty() ? str : " " + str));
			}

			FormatString& operator+=(const char* str) {
				value += (value.empty() || *str == '\0' ? str : " " + (string)str);
				return *this;
			}

			FormatString& operator+=(const string& str) {
				value += (value.empty() || str.empty() ? str : " " + str);
				return *this;
			}


			operator string() const {
				return value;
			}
	};

	static inline string primitiveToString(bool value) {
		return value ? "true" : "false";
	}

	static inline string primitiveToString(char16_t c) {
		return '\'' + (c < 0x20 ? "\\u" + hex<4>(c) : encodeUtf8(c)) + '\'';
	}

	static inline string primitiveToString(int8_t num) { // byte
		return to_string(num);
	}

	static inline string primitiveToString(int16_t num) { // short
		return to_string(num);
	}

	static inline string primitiveToString(int32_t num) { // int
		return to_string(num);
	}

	static inline string primitiveToString(int64_t num) { // long
		return to_string(num) + "l";
	}

	static string primitiveToString(float num) {
		if(isnan(num)) return "Float.NaN";
		if(!isfinite(num)) return num > 0 ? "Float.POSITIVE_INFINITY" : "Float.NEGATIVE_INFINITY";
		ostringstream out;
		out << num << 'f';
		return out.str();
	}

	static string primitiveToString(double num) {
		if(isnan(num)) return "Double.NaN";
		if(!isfinite(num)) return num > 0 ? "Double.POSITIVE_INFINITY" : "Double.NEGATIVE_INFINITY";
		ostringstream out;
		out << num;
		return out.str();
	}

	static string stringToLiteral(const string& str) {
		#define checkLength(n) if(bytes + n >= end) throw DecompilationException("Unexpected end of the string")
		string result = "\"";
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
							result += encodeUtf8(0x10000 | (*++bytes & 0xF) << 16 | (*++bytes & 0x3F) << 10 | (*(bytes += 2) & 0xF) << 6 | (*++bytes & 0x3F));
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
	}
}

#undef inline

#endif
