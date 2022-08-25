#ifndef UTIL_FORMAT_STRING_CPP
#define UTIL_FORMAT_STRING_CPP

#include <string>

namespace util {

	using std::string;

	class format_string {
		private:
			string value;

		public:
			format_string() noexcept: value() {}
			format_string(const string& value): value(value) {}


			friend format_string operator+(const format_string& format_str, const char* str) {
				return format_string(format_str.value + (format_str.empty() || *str == '\0' ? str : ' ' + static_cast<string>(str)));
			}

			friend format_string operator+(const format_string& format_str, const string& str) {
				return format_string(format_str.value + (format_str.empty() || str.empty() ? str : ' ' + str));
			}

			friend format_string operator+(const char* str, const format_string& format_str) {
				return format_string(str + (format_str.empty() || *str == '\0' ? static_cast<string>(format_str) : ' ' + static_cast<string>(format_str)));
			}

			friend format_string operator+(const string& str, const format_string& format_str) {
				return format_string(str + (format_str.empty() || str.empty() ? static_cast<string>(format_str) : ' ' + static_cast<string>(format_str)));
			}


			format_string& operator+=(const char* str) {
				if(!value.empty() && *str != '\0')
					value += ' ';

				value += str;

				return *this;
			}

			format_string& operator+=(const string& str) {
				if(!value.empty() && !str.empty())
					value += ' ';

				value += str;

				return *this;
			}


			inline size_t size() const {
				return value.size();
			}

			inline bool empty() const {
				return value.empty();
			}

			inline void clear() {
				value.clear();
			}

			inline const char* c_str() const {
				return value.c_str();
			}


			inline operator string() const {
				return value;
			}
	};
}

#endif
