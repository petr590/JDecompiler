#ifndef JDECOMPILER_UTIL_CPP
#define JDECOMPILER_UTIL_CPP

#include <fstream>
#include <cstring>
#include <vector>
#include <functional>
//#include <iostream> // DEBUG
#include <sstream>
#include <math.h>

using namespace std;

static const string EMPTY_STRING = string();

template<uint16_t length>
static string hex(uint64_t n) {
	static string digits = "0123456789ABCDEF";

	char str[length + 1];
	str[length] = '\0';

	for(int i = length; i-- > 0; ) {
		str[i] = digits[n & 0xF];
		n >>= 4;
	}

	return str;
}

static string hex(uint64_t n) {
	if(n == 0) return "0";

	static string digits = "0123456789ABCDEF";
	string str;

	while(n != 0) {
		str += digits[n & 0xF];
		n >>= 4;
	}

	return str;
}


template<class T, class B>
T safe_cast(B o) {
	T t = dynamic_cast<T>(o);
	if(t == nullptr && o != nullptr)
		throw bad_cast();
	return t;
}


class Exception: public exception {
	string message;

	public:
		Exception() {}
		Exception(const char* message): message(message) {}
		Exception(string message): message(message) {}

		virtual const char* what() const noexcept override {
			return message.c_str();
		}
};


class IndexOutOfBoundsException: public Exception {
	public:
		IndexOutOfBoundsException(const char* message): Exception(message) {}
		IndexOutOfBoundsException(string message): Exception(message) {}
};

class IllegalStateException: public Exception {
	public:
		IllegalStateException(const char* message): Exception(message) {}
		IllegalStateException(string message): Exception(message) {}
};

class DynamicCastException: public Exception {
	public:
		DynamicCastException(const char* message): Exception(message) {}
		DynamicCastException(string message): Exception(message) {}
};

class AssertionException: public Exception {
	public:
		AssertionException(const char* message): Exception(message) {}
		AssertionException(string message): Exception(message) {}
};


class DecompilationException: public Exception {
	public:
		DecompilationException(const char* message): Exception(message) {}
		DecompilationException(string message): Exception(message) {}
};


class IllegalTypeNameException: public DecompilationException {
	public:
		IllegalTypeNameException(const char* message): DecompilationException(message) {}
		IllegalTypeNameException(string message): DecompilationException(message) {}
};

class IllegalSignatureException: public IllegalTypeNameException {
	public:
		IllegalSignatureException(const char* message): IllegalTypeNameException(message) {}
		IllegalSignatureException(string message): IllegalTypeNameException(message) {}
};

class ClassFormatException: public DecompilationException {
	public:
		ClassFormatException(const char* message): DecompilationException(message) {}
		ClassFormatException(string message): DecompilationException(message) {}
};


class IllegalModifersException: public DecompilationException {
	public:
		IllegalModifersException(uint16_t modifiers) : DecompilationException("0x" + hex<4>(modifiers)) {}
};

class IllegalMethodDescriptorException: public DecompilationException {
	public:
		IllegalMethodDescriptorException(const char* descriptor): DecompilationException(descriptor) {}
		IllegalMethodDescriptorException(string descriptor): DecompilationException(descriptor) {}
};

class IllegalConstantPointerException: public DecompilationException {
	public:
		IllegalConstantPointerException(const char* message): DecompilationException(message) {}
		IllegalConstantPointerException(string message): DecompilationException(message) {}
};

class IllegalOpcodeException: public DecompilationException {
	public:
		IllegalOpcodeException(const char* message): DecompilationException(message) {}
		IllegalOpcodeException(string message): DecompilationException(message) {}
};

class IllegalAttributeException: public ClassFormatException {
	public:
		IllegalAttributeException(const char* message): ClassFormatException(message) {}
		IllegalAttributeException(string message): ClassFormatException(message) {}
};

class IOException: public Exception {
	public:
		IOException(const char* message): Exception(message) {}
		IOException(string message): Exception(message) {}
		IOException(): Exception() {}
};

class EOFException: public IOException {
	public:
		EOFException(): IOException() {}
};



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
	const uint16_t
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
static string join(const vector<T>& array, const function<string(T)> func, const string separator = ", ") {
	string result = EMPTY_STRING;
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
static string join(const vector<T>& array, const function<string(T, uint32_t)> func, const string separator = ", ") {
	string result = EMPTY_STRING;
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
static string rjoin(const vector<T>& array, const function<string(T)> func, const string separator = ", ") {
	string result = EMPTY_STRING;
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


/*
template<typename Base, typename T>
inline bool instanceof(const T*) {
	return is_base_of<Base, T>::value;
}
*/


class BinaryInputStream {
	private:
		ifstream infile;
		char buffer[4096];
		streampos end;
		streampos pos = 0, max = 0;

		void check() {
			if(pos >= max) {
				max = min((streampos)4096, (streampos)(end - infile.tellg()));
				infile.read(*&buffer, max);
			}
			if(pos >= end)
				throw EOFException();
		}

		unsigned short next() { // reads only one byte; returns not char because type expansion for unsigned is same as for signed types
			check();
			pos += 1;
			return buffer[pos - (streampos)1] & 0xFF;
		}

	public:
		BinaryInputStream(const string path): infile(path, ios::binary | ios::in) {
			if(!infile.good())
				throw IOException("Cannnot open the file '" + path + "'");
			infile.seekg(0, ios::end);
			end = infile.tellg();
			infile.seekg(0, ios::beg);
		}

		streampos getPos() const {
			return pos;
		}

		void setPosTo(streampos pos) {
			this->pos = pos;
		}

		uint8_t readByte() {
			return next();
		}

		char readChar() {
			return next();
		}

		uint16_t readShort() {
			return next() << 8 | next();
		}

		uint32_t readInt() {
			return next() << 24 | next() << 16 | next() << 8 | next();
		}

		uint64_t readLong() {
			return (uint64_t)next() << 56 | (uint64_t)next() << 48 | (uint64_t)next() << 40 | (uint64_t)next() << 32 | (uint64_t)next() << 24 | (uint64_t)next() << 16 | (uint64_t)next() << 8 | (uint64_t)next();
		}

		float readFloat() {
			float f;
			uint32_t bytes = readInt();
			memcpy(&f, &bytes, 4);
			return f;
		}

		double readDouble() {
			double d;
			uint32_t bytes = readLong();
			memcpy(&d, &bytes, 8);
			return d;
		}

		const char* readBytes(int size) {
			char* str = new char[size + 1];
			for(int i = 0; i < size; i++)
				str[i] = next();
			str[size] = '\0';
			return str;
		}
};


namespace JDecompiler {

	class FormatString {
		private:
			string str;

		public:
			FormatString() {}
			FormatString(string str): str(str) {}


			FormatString operator+(const char* str) const {
				return FormatString(this->str + (this->str.empty() || *str == '\0' ? str : " " + (string)str));
			}

			FormatString operator+(string str) const {
				return FormatString(this->str + (this->str.empty() || str.empty() ? str : " " + str));
			}

			FormatString* operator+=(const char* str) {
				this->str += (this->str.empty() || *str == '\0' ? str : " " + (string)str);
				return this;
			}

			FormatString* operator+=(string str) {
				this->str += (this->str.empty() || str.empty() ? str : " " + str);
				return this;
			}


			operator string() const {
				return str;
			}
	};

	static string primitiveToString(bool value) {
		return value ? "true" : "false";
	}

	static string primitiveToString(char16_t c) {
		return "'" + (c < 0x20 ? "\\u" + hex<4>(c) : encodeUtf8(c)) + "'";
	}

	static string primitiveToString(int8_t num) { // byte
		return to_string(num);
	}

	static string primitiveToString(int16_t num) { // short
		return to_string(num);
	}

	static string primitiveToString(int32_t num) { // int
		return to_string(num);
	}

	static string primitiveToString(int64_t num) { // long
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
}


#endif
