#ifndef JDECOMPILER_OPERATION_CPP
#define JDECOMPILER_OPERATION_CPP

#include "operation.h"

namespace jdecompiler {

	template<class T>
	const T* Operation::getReturnTypeAs(const T* type) const {
		const T* newType = getReturnType()->castTo(type);
		onCastReturnType(newType);
		return newType;
	}

	template<class T>
	const T* Operation::getReturnTypeAsWidest(const T* type) const {
		const T* newType = getReturnType()->castToWidest(type);
		onCastReturnType(newType);
		return newType;
	}

	const Operation* Operation::getOriginalOperation() const {
		return this;
	}

	template<class D, class... Ds>
	bool Operation::checkDup(const DecompilationContext& context, const Operation* operation) {
		if(instanceof<const D*>(operation)) {
			if(static_cast<const D*>(operation)->operation != context.stack.pop())
				throw DecompilationException("Illegal stack state after dup operation");
			return true;
		}

		if constexpr(sizeof...(Ds) > 0)
			return checkDup<Ds...>(context, operation);
		else
			return false;
	}


	Priority Operation::getPriority() const {
		return Priority::DEFAULT_PRIORITY;
	}

	string Operation::toStringPriority(const Operation* operation, const StringifyContext& context, const Associativity associativity) const {
		const Priority thisPriority = this->getPriority(),
		               otherPriority = operation->getPriority();

		if(otherPriority < thisPriority || (otherPriority == thisPriority && getAssociativityByPriority(otherPriority) != associativity))
			return '(' + operation->toString(context) + ')';
		return operation->toString(context);
	}


	bool Operation::canAddToCode() const {
		return true;
	}

	inline void Operation::remove(const DecompilationContext& context) const {
		context.methodScope.removeOperation(this, context);
	}

	bool Operation::canStringify() const {
		return true;
	}
}

#endif
