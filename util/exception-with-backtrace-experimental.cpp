#ifndef UTIL_EXCEPTION_CPP
#define UTIL_EXCEPTION_CPP

#include <exception>
#include <string>
#include <execinfo.h>

namespace util {

	using std::string;
	using std::exception;

	struct Exception: exception {
		public:
			static const int BACKTRACE_BUFFER_SIZE = 128;

		protected:
			const string message;
			const int backtraceLength;
			void* backtraceBuffer[BACKTRACE_BUFFER_SIZE];
			char** backtraceSymbols;

		public:
			Exception(const string& message):
				message(message),
				backtraceLength(backtrace(backtraceBuffer, BACKTRACE_BUFFER_SIZE)),
				backtraceSymbols(backtrace_symbols(backtraceBuffer, backtraceLength)) {}

			Exception(const char* message): Exception((string)message) {}
			Exception(): Exception(string()) {}

			virtual const char* what() const noexcept override {
				return message.c_str();
			}

			void printBacktrace() const noexcept {
				//cerr << "Exception" << endl;

				for(int i = 0; i < backtraceLength; ++i) {
					fprintf(stderr, "[%2d]: %s\n", i, backtraceSymbols[i]);
				}

				//backtrace_symbols_fd(backtraceBuffer, backtraceLength, 2);
			}

		virtual ~Exception() {
			delete[] backtraceSymbols;
		}
	};
}

#endif
