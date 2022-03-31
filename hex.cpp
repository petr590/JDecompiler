#ifndef JDECOMPILER_HEX_CPP
#define JDECOMPILER_HEX_CPP

namespace jdecompiler {
	template<uint16_t length>
	static string hex(uint64_t n) {
		static_assert(length > 0, "length cannot be zero");

		static constexpr const char* const digits = "0123456789ABCDEF";

		char str[length + 1U];
		str[length] = '\0';

		for(uint16_t i = length; i-- > 0; ) {
			str[i] = digits[n & 0xF];
			n >>= 4;
		}

		return str;
	}

	template<uint16_t length>
	static inline string hexWithPrefix(uint64_t n) {
		return "0x" + hex<length>(n);
	}


	static string hex(uint64_t n) {
		static constexpr const char* digits = "0123456789ABCDEF";
		string str;

		do {
			str += digits[n & 0xF];
			n >>= 4;
		} while(n != 0);

		reverse(str.begin(), str.end());

		return str;
	}

	static inline string hexWithPrefix(uint64_t n) {
		return "0x" + hex(n);
	}
}

#endif
