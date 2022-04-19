#ifndef STACK_CPP
#define STACK_CPP

#include <stdint.h>
#include <ostream>
#include "index-out-of-bounds-exception.cpp"

namespace util {

	struct StackIndexOutOfBoundsException: IndexOutOfBoundsException {
		StackIndexOutOfBoundsException(uint32_t index, uint32_t length): IndexOutOfBoundsException(index, length, "Stack") {}
	};

	struct IllegalStackStateException: Exception {
		IllegalStackStateException() {}
		IllegalStackStateException(const string& message): Exception(message) {}
	};

	struct EmptyStackException: IllegalStackStateException {
		EmptyStackException() {}
		EmptyStackException(const string& message): IllegalStackStateException(message) {}
	};


	template<typename T>
	struct Stack {
		private:
			class Entry {
				public:
					const T value;
					const Entry* const next;

					Entry(T value, const Entry* next): value(value), next(next) {}

					void deleteNext() const {
						if(next != nullptr) {
							next->deleteNext();
							delete next;
						}
					}
			};

			const Entry* firstEntry;
			uint16_t length;

		protected:
			inline void checkEmptyStack() const {
				if(firstEntry == nullptr)
					throw EmptyStackException();
			}

		public:
			Stack(): firstEntry(nullptr), length(0) {}

			Stack(T value): firstEntry(new Entry(value, nullptr)), length(1) {}

			void push(T value) {
				firstEntry = new Entry(value, firstEntry);
				length++;
			}

			inline void push(T value, T values...) {
				push(value);
				push(values);
			}

			T pop() {
				checkEmptyStack();

				const Entry copiedEntry = *firstEntry;
				delete firstEntry;
				firstEntry = copiedEntry.next;
				length--;

				return copiedEntry.value;
			}

			T top() const {
				checkEmptyStack();
				return firstEntry->value;
			}

			T lookup(uint16_t index) const {
				checkEmptyStack();

				if(index >= length)
					throw StackIndexOutOfBoundsException(index, length);

				const Entry* currentEntry = firstEntry;
				for(uint16_t i = 0; i < index; i++)
					currentEntry = currentEntry->next;
				return currentEntry->value;
			}

			inline uint16_t size() const {
				return length;
			}

			inline bool empty() const {
				return length == 0;
			}


			friend ostream& operator<< (ostream& out, const Stack& stack) {
				out << '[';
				if(!stack.empty()) {
					size_t i = 0;
					while(true) {
						out << stack.lookup(i);
						if(++i >= stack.size()) break;
						out << ", ";
					}
				}
				out << ']';

				return out;
			}


			~Stack() {
				if(firstEntry != nullptr) {
					firstEntry->deleteNext();
					delete firstEntry;
				}
			}
	};
}

#endif
