#ifndef JDECOMPILER_UTIL_CPP
#define JDECOMPILER_UTIL_CPP

#include "util.h"

namespace jdecompiler {

	string toLowerCamelCase(const string& str) {
		string result;
		const size_t strlength = str.size();

		size_t i = 0;
		for(char c = str[0]; i < strlength && c >= 'A' && c <= 'Z'; c = str[++i])
			result += (char)tolower(c);
		while(i < strlength)
			result += str[i++];

		return result;
	}


	const char* repeatString(const char* str, uint32_t count) {
		const size_t
				strlength = strlen(str),
				resultlength = strlength * count;

		char* result = strncpy(new char[resultlength + 1], str, resultlength + 1);

		for(uint32_t i = 0; i < resultlength; i++)
			result[i] = str[i % strlength];
		result[resultlength] = '\0';

		return result;
	}


	string unescapeString(const char* str) {
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


	string repeat(const string& str, size_t times) {
		string result;
		result.reserve(times);

		for(size_t i = 0; i < times; i++)
			result += str;

		return result;
	}

	char* str_concat(const char* str1, const char* str2) {
		const size_t
				len1 = strlen(str1),
				len = len1 + strlen(str2);

		char* result = new char[len + 1];
		strcpy(result, str1);
		strcpy(result + len1, str2);
		result[len] = '\0';

		return result;
	}

	char* str_concat(const char* str, char c) {
		const size_t len = strlen(str) + 1;

		char* result = new char[len + 1];
		strcpy(result, str);
		result[len - 1] = c;
		result[len] = '\0';

		return result;
	}


	const char* char32ToString(char32_t ch) {
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


	string encodeUtf8(char32_t c) {
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
	string escapeUtf16(char32_t ch) {
		assert(ch <= 0x10FFFF);

		if(ch > 0xFFFF) {
			ch -= 0x10000;
			return "\\u" + hex<4>(((ch >> 10) & 0x3FF) | 0xD800) + "\\u" + hex<4>((ch & 0x3FF) | 0xDC00);
		}

		return "\\u" + hex<4>(ch);
	}

	bool isNotDisplayedChar(char32_t ch) {
		return ch < 0x20 || (ch >= 0x7F && ch < 0xA0) // Control characters
			|| (ch >= 0xFFEF && ch < 0x10000)         // Unknown characters
			|| (ch >= 0xD800 && ch < 0xE000);         // Surrogate pairs in UTF-16
	}


	template<char quote>
	string charToString(char32_t ch) {
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

	template string charToString<'"'>(char32_t);
	template string charToString<'\''>(char32_t);

	string stringToLiteral(const string& str) {
		#define check(condition, message) if(!(condition)) throw DecompilationException(message)
		#define checkLength(n) check(bytes + n < end, "Unexpected end of the string: " + to_string(bytes - str.c_str()) + ", " + to_string(n) + ", " + to_string(end - str.c_str()))
		#define checkEncoding(condition) check(condition, "Invalid string encoding")

		const char* bytes = str.c_str();

		string result("\"");

		bytes = str.c_str();

		for(const char* end = bytes + strlen(bytes); bytes < end; bytes++) {
			char32_t ch = *bytes & 0xFF;

			if((ch & 0xE0) == 0xC0) {
				checkLength(1);
				checkEncoding((bytes[1] & 0xC0) == 0x80);

				ch = (ch & 0x1F) << 6 | (bytes[1] & 0x3F);

				bytes++;

			} else if((ch & 0xF0) == 0xE0) {

				if(ch == 0xED && bytes + 5 < end &&
						(bytes[1] & 0xF0) == 0xA0 && (bytes[2] & 0xC0) == 0x80 && (bytes[3] & 0xFF) == 0xED
					 && (bytes[4] & 0xF0) == 0xB0 && (bytes[5] & 0xC0) == 0x80) {

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

			check(ch <= 0x10FFFF, "Invalid string: char code U+" + hex(ch) + " is out of range");

			result += charToString<'"'>(ch);
		}
		return result + '"';

		#undef checkEncoding
		#undef checkLength
		#undef check
	}


	vector<string> splitAndAddDelimiter(const string& str, char delimiter) {
		vector<string> result;

		size_t oldPos = 0;

		for(size_t pos = 0; (pos = str.find(delimiter, oldPos)) != string::npos; oldPos = pos) {
			pos++;
			result.push_back(str.substr(oldPos, pos - oldPos));
		}

		if(oldPos < str.size())
			result.push_back(str.substr(oldPos));

		return result;
	}


	bool stringStartsWith(const string& str, const string& search) {
		if(search.size() > str.size())
			return false;

		const char* cstr = str.c_str();
		for(char c : search) {
			if(*(cstr++) != c)
				return false;
		}

		return true;
	}


	bool stringEndsWith(const string& str, const string& search) {
		if(search.size() > str.size())
			return false;

		const char* cstr = str.c_str() + (str.size() - search.size());
		for(char c : search) {
			if(*(cstr++) != c)
				return false;
		}

		return true;
	}
}

#endif
