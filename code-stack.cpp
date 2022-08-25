#ifndef JDECOMPILER_CODE_STACK_CPP
#define JDECOMPILER_CODE_STACK_CPP

namespace jdecompiler {

	struct CodeStack: stack<const Operation*> {
		inline const Operation* popAs(const Type* type) {
			const Operation* operation = this->pop();
			operation->castReturnTypeTo(type);
			return operation;
		}

		CodeStack() {}

		inline const Operation* pop() {
			try {
				return stack<const Operation*>::pop();
			} catch(const EmptyStackException& ex) {
				throw EmptyCodeStackException(ex);
			}
		}

		inline const Operation* top() {
			try {
				return stack<const Operation*>::top();
			} catch(const EmptyStackException& ex) {
				throw EmptyCodeStackException(ex);
			}
		}

		inline const Operation* lookup(size_t index) {
			try {
				return stack<const Operation*>::lookup(index);
			} catch(const EmptyStackException& ex) {
				throw EmptyCodeStackException(ex);
			}
		}

		template<class O>
		inline O pop() {
			checkEmptyStack();

			O operation = stack<const Operation*>::pop();
			if(const O* t = dynamic_cast<const O*>(operation))
				return t;
			throw DecompilationException("Illegal operation type " + typenameof<O>() + " for operation " + typenameof(*operation));
		}

		CodeStack(const CodeStack&) = delete;
		CodeStack& operator=(const CodeStack&) = delete;
	};
}

#endif
