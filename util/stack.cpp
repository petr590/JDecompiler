#ifndef UTIL_STACK_CPP
#define UTIL_STACK_CPP

#include <stdint.h>
#include <ostream>
#include "stack-index-out-of-bounds-exception.cpp"
#include "empty-stack-exception.cpp"

namespace util {

	using std::ostream;


	template<typename T>
	struct stack {

		private:
			class entry {
				public:
					T value;
					entry* const next;

					entry(T value, entry* next): value(value), next(next) {}

					void deleteNext() const {
						if(next != nullptr) {
							next->deleteNext();
							delete next;
						}
					}
			};



			struct iterator {
				protected:
					entry* ent;
					friend struct stack;

					constexpr iterator(entry* ent): ent(ent) {}

				public:
					constexpr iterator(const iterator& other): ent(other.ent) {}

					T& operator*() {
						return ent->value;
					}

					const T& operator*() const {
						return ent->value;
					}

					iterator& operator++() {
						ent = ent->next;
						return *this;
					}

					iterator operator++(int) {
						iterator old(*this);
						ent = ent->next;
						return old;
					}

					iterator& operator=(const iterator& other) {
						ent = other.ent;
					}

					friend bool operator==(const iterator& iterator1, const iterator& iterator2) {
						return iterator1.ent == iterator2.ent;
					}

					friend bool operator!=(const iterator& iterator1, const iterator& iterator2) {
						return !(iterator1 == iterator2);
					}
			};


			entry* firstEntry;
			size_t length;

			iterator beginIterator;

		protected:
			inline void checkEmptyStack() const {
				if(firstEntry == nullptr)
					throw EmptyStackException();
			}

		public:

			stack(): firstEntry(nullptr), length(0), beginIterator(nullptr) {}

			stack(T value): firstEntry(new entry(value, nullptr)), length(1), beginIterator(firstEntry) {}

			void push(T value) {
				firstEntry = new entry(value, firstEntry);
				length++;
				beginIterator.ent = firstEntry;
			}

			inline void push(T value, T values...) {
				push(value);
				push(values);
			}

			T pop() {
				checkEmptyStack();

				const entry copiedEntry = *firstEntry;
				delete firstEntry;
				firstEntry = copiedEntry.next;
				length--;

				beginIterator.ent = firstEntry;

				return copiedEntry.value;
			}

			T top() const {
				checkEmptyStack();
				return firstEntry->value;
			}

			T lookup(size_t index) const {
				checkEmptyStack();

				if(index >= length)
					throw StackIndexOutOfBoundsException(index, length);

				const entry* currentEntry = firstEntry;
				for(size_t i = 0; i < index; i++)
					currentEntry = currentEntry->next;
				return currentEntry->value;
			}

			bool has(const T& value) const {
				for(const T& val : *this)
					if(val == value)
						return true;
				return false;
			}

			inline size_t size() const {
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
