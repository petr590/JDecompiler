#ifndef UTIL_EMPTY_STACK_EXCEPTION_CPP
#define UTIL_EMPTY_STACK_EXCEPTION_CPP

#include "illegal-stack-state-exception.cpp"

namespace util {

	struct EmptyStackException: IllegalStackStateException {
		EmptyStackException() {}
		EmptyStackException(const string& message): IllegalStackStateException(message) {}
	};
}

#endif
