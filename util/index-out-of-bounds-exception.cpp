#ifndef UTIL_INDEX_OUT_OF_BOUNDS_EXCEPTION_CPP
#define UTIL_INDEX_OUT_OF_BOUNDS_EXCEPTION_CPP

#include "exception.cpp"

namespace util {

	using std::to_string;

	struct IndexOutOfBoundsException: Exception {
		public:
			IndexOutOfBoundsException(const string& message): Exception(message) {}

			IndexOutOfBoundsException(size_t index, size_t length):
					Exception("Index " + to_string(index) + " is out of bounds for length " + to_string(length)) {}

		protected:
			IndexOutOfBoundsException(size_t index, size_t length, const string& name):
					Exception(name + " index " + to_string(index) + " is out of bounds for length " + to_string(length)) {}
	};
}

#endif
