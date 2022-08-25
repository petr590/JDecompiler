#ifndef UTIL_STACK_INDEX_OUT_OF_BOUNDS_EXCEPTION_CPP
#define UTIL_STACK_INDEX_OUT_OF_BOUNDS_EXCEPTION_CPP

#include "index-out-of-bounds-exception.cpp"

namespace util {

	struct StackIndexOutOfBoundsException: IndexOutOfBoundsException {
		StackIndexOutOfBoundsException(size_t index, size_t length): IndexOutOfBoundsException(index, length, "stack") {}
	};
}

#endif
