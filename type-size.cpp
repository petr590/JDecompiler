#ifndef JDECOMPILER_TYPE_SIZE_CPP
#define JDECOMPILER_TYPE_SIZE_CPP

namespace jdecompiler {

	enum class TypeSize {
		ZERO_BYTES = 0, FOUR_BYTES = 1, EIGHT_BYTES = 2
	};


	static constexpr const char* TypeSize_nameOf(const TypeSize typeSize) {
		switch(typeSize) {
			case TypeSize::ZERO_BYTES: return "ZERO_BYTES";
			case TypeSize::FOUR_BYTES: return "FOUR_BYTES";
			case TypeSize::EIGHT_BYTES: return "EIGHT_BYTES";
			default: throw IllegalStateException("Illegal typeSize " + to_string(static_cast<unsigned int>(typeSize)));
		}
	}
}

#endif
