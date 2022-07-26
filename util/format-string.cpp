#ifndef FORMAT_STRING_CPP
#define FORMAT_STRING_CPP

namespace jdecompiler {

	class FormatString {
		private: string value;

		public:
			FormatString() noexcept: value() {}
			FormatString(const string& value): value(value) {}


			friend FormatString operator+(const FormatString& formatStr, const char* str) {
				return FormatString(formatStr.value + (formatStr.empty() || *str == '\0' ? str : ' ' + (string)str));
			}

			friend FormatString operator+(const FormatString& formatStr, const string& str) {
				return FormatString(formatStr.value + (formatStr.empty() || str.empty() ? str : ' ' + str));
			}

			friend FormatString operator+(const char* str, const FormatString& formatStr) {
				return FormatString(str + (formatStr.empty() || *str == '\0' ? (string)formatStr : ' ' + (string)formatStr));
			}

			friend FormatString operator+(const string& str, const FormatString& formatStr) {
				return FormatString(str + (formatStr.empty() || str.empty() ? (string)formatStr : ' ' + (string)formatStr));
			}


			friend FormatString& operator+=(FormatString& formatStr, const char* str) {
				formatStr.value += (formatStr.empty() || *str == '\0' ? str : ' ' + (string)str);
				return formatStr;
			}

			friend FormatString& operator+=(FormatString& formatStr, const string& str) {
				formatStr.value += (formatStr.empty() || str.empty() ? str : ' ' + str);
				return formatStr;
			}


			inline bool empty() const {
				return value.empty();
			}


			inline operator string() const {
				return value;
			}
	};
}

#endif
