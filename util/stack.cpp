#ifndef STACK_CPP
#define STACK_CPP

#include <stdint.h>
#include <ostream>
#include "index-out-of-bounds-exception.cpp"

namespace util {

	struct StackIndexOutOfBoundsException: IndexOutOfBoundsException {
		StackIndexOutOfBoundsException(uint32_t index, uint32_t length): IndexOutOfBoundsException(index, length, "stack") {}
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
	struct stack {

		private:
			class Entry {
				public:
					T value;
					Entry* const next;

					Entry(T value, Entry* next): value(value), next(next) {}

					void deleteNext() const {
						if(next != nullptr) {
							next->deleteNext();
							delete next;
						}
					}
			};



			struct iterator {
				protected:
					Entry* entry;
					friend struct stack;

					constexpr iterator(Entry* entry): entry(entry) {}

				public:
					T& operator*() {
						return entry->value;
					}

					const T& operator*() const {
						return entry->value;
					}

					iterator& operator++() {
						entry = entry->next;
						return *this;
					}

					iterator operator++(int) {
						iterator old(*this);
						entry = entry->next;
						return old;
					}

					iterator& operator=(const iterator& other) {
						entry = other.entry;
					}

					friend bool operator==(const iterator& iterator1, const iterator& iterator2) {
						return iterator1.entry == iterator2.entry;
					}

					friend bool operator!=(const iterator& iterator1, const iterator& iterator2) {
						return !(iterator1 == iterator2);
					}
			};


			Entry* firstEntry;
			uint16_t length;

			iterator beginIterator;

		protected:
			inline void checkEmptyStack() const {
				if(firstEntry == nullptr)
					throw EmptyStackException();
			}

		public:

			stack(): firstEntry(nullptr), length(0), beginIterator(nullptr) {}

			stack(T value): firstEntry(new Entry(value, nullptr)), length(1), beginIterator(firstEntry) {}

			void push(T value) {
				firstEntry = new Entry(value, firstEntry);
				length++;
				beginIterator.entry = firstEntry;
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

				beginIterator.entry = firstEntry;

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

			bool has(const T& value) const {
				for(const T& val : *this)
					if(val == value)
						return true;
				return false;
			}

			inline uint16_t size() const {
				return length;
			}

			inline bool empty() const {
				return length == 0;
			}


			friend ostream& operator<<(ostream& out, const stack& stack) {
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


			~stack() {
				if(firstEntry != nullptr) {
					firstEntry->deleteNext();
					delete firstEntry;
				}
			}



			iterator begin() const {
				return beginIterator;
			}


			iterator end() const {
				static const iterator endIterator(nullptr);
				return endIterator;
			}




			friend iterator begin(const stack& stck) {
				return stck.begin();
			}

			friend iterator end(const stack& stck) {
				return stck.end();
			}
	};
}

#endif
