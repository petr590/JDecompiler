#ifndef JDECOMPILER_TYPE_H
#define JDECOMPILER_TYPE_H

#include "type.h"

namespace jdecompiler {

	template<bool isNoexcept, bool widest>
	const Type* Type::cast0(const Type* type) const {

		// I love such constructions in C++ :)
		// It's a pointer to a member function
		static constexpr const Type* (Type::* castImplFunc)(const Type*) const = getCastImplFunction<widest>();
		static constexpr const Type* (Type::* reversedCastImplFunc)(const Type*) const = getReversedCastImplFunction<widest>();

		const Type* castedType;

		if((castedType = (this->*castImplFunc)(type)) != nullptr) {
			return castedType;
		}

		(type->*reversedCastImplFunc)(this);

		if(this->canReverseCast(type) && (castedType = (type->*reversedCastImplFunc)(this)) != nullptr) {
			return castedType;
		}

		if constexpr(isNoexcept)
			return nullptr;
		else
			throw IncopatibleTypesException(this, type);
	}

	template<class T>
	const T* Type::twoWayCastTo(const T* t) const {
		checkType<T>();

		const Type* castedType;

		if((castedType = this->castToWidestImpl(t)) != nullptr)
			return safe_cast<const T*>(castedType);

		if((castedType = ((const Type*)t)->castToWidestImpl(this)) != nullptr)
			return safe_cast<const T*>(castedType);

		if((castedType = this->reversedCastToWidestImpl(t)) != nullptr)
			return safe_cast<const T*>(castedType);

		if((castedType = ((const Type*)t)->reversedCastToWidestImpl(this)) != nullptr)
			return safe_cast<const T*>(castedType);

		throw IncopatibleTypesException(this, t);
	}
}

#endif
