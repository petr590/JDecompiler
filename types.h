#ifndef JDECOMPILER_TYPES_H
#define JDECOMPILER_TYPES_H

#include "type.h"
#include "types/basic.cpp"
#include "types/special.cpp"
#include "types/primitive.cpp"
#include "types/integral.cpp"
#include "types/void.cpp"
#include "types/boolean.cpp"
#include "types/byte.cpp"
#include "types/char.cpp"
#include "types/short.cpp"
#include "types/int.cpp"
#include "types/long.cpp"
#include "types/float.cpp"
#include "types/double.cpp"
#include "types/reference.cpp"
#include "types/class.cpp"

#include "javase.cpp"

#include "types/array.cpp"
#include "types/parameter.cpp"
#include "types/generics.cpp"
#include "types/variable-capacity-integral.cpp"
#include "types/excluding-boolean.cpp"
#include "types/any.cpp"
#include "types/any-object.cpp"
#include "types/generic-parameter.cpp"

namespace jdecompiler {

	extern const BasicType* parseType(const char*&);

	static inline const BasicType* parseType(const char*&& str) {
		return parseType(static_cast<const char*&>(str));
	}

	static inline const BasicType* parseType(const string& str) {
		return parseType(str.c_str());
	}

	extern vector<const Type*> parseMethodArguments(const char*&);


	extern const BasicType* parseReturnType(const char*);

	static inline const BasicType* parseReturnType(const string& str) {
		return parseReturnType(str.c_str());
	}


	extern const ReferenceType* parseReferenceType(const char*);

	static inline const ReferenceType* parseReferenceType(const string& str) {
		return parseReferenceType(str.c_str());
	}


	static inline const ClassType* parseClassType(const char*& restrict str) {
		return str[0] == 'L' ? new ClassType(str += 1) : throw InvalidSignatureException(str, 0);
	}


	extern const ReferenceType* parseParameter(const char*&);


	extern vector<const ReferenceType*> parseParameters(const char*&);


	extern vector<const GenericParameter*> parseGeneric(const char*&);
}

#endif
