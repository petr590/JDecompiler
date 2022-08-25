#ifndef JDECOMPILER_TYPES_CPP
#define JDECOMPILER_TYPES_CPP

#include "type.cpp"
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

namespace jdecompiler {

	Type::status_t PrimitiveType::implicitCastStatus(const Type* other) const {
		return *other == *OBJECT ? OBJECT_AUTOBOXING_STATUS : *other == getWrapperType() ? AUTOBOXING_STATUS : Type::implicitCastStatus(other);
	}

	const ClassType& VoidType::getWrapperType() const {
		return javaLang::Void;
	}

	const ClassType& BooleanType::getWrapperType() const {
		return javaLang::Boolean;
	}

	const ClassType& ByteType::getWrapperType() const {
		return javaLang::Byte;
	}

	const ClassType& CharType::getWrapperType() const {
		return javaLang::Character;
	}

	const ClassType& ShortType::getWrapperType() const {
		return javaLang::Short;
	}

	const ClassType& IntType::getWrapperType() const {
		return javaLang::Integer;
	}

	const ClassType& LongType::getWrapperType() const {
		return javaLang::Long;
	}

	const ClassType& FloatType::getWrapperType() const {
		return javaLang::Float;
	}

	const ClassType& DoubleType::getWrapperType() const {
		return javaLang::Double;
	}


	Type::status_t ClassType::implicitCastStatus(const Type* other) const {
		return other->isPrimitive() && *this == safe_cast<const PrimitiveType*>(other)->getWrapperType() ?
						AUTOBOXING_STATUS : Type::implicitCastStatus(other);
	}
}


#include "types/array.cpp"
#include "types/parameter.cpp"
#include "types/generics.cpp"
#include "types/variable-capacity-integral.cpp"
#include "types/excluding-boolean.cpp"
#include "types/any.cpp"
#include "types/any-object.cpp"
#include "types/generic-parameter.cpp"

namespace jdecompiler {

	const BasicType* parseType(const char*& restrict str) {
		switch(str[0]) {
			case 'B': ++str; return BYTE;
			case 'C': ++str; return CHAR;
			case 'S': ++str; return SHORT;
			case 'I': ++str; return INT;
			case 'J': ++str; return LONG;
			case 'F': ++str; return FLOAT;
			case 'D': ++str; return DOUBLE;
			case 'Z': ++str; return BOOLEAN;
			case 'L': return new ClassType(str += 1);
			case '[': return new ArrayType(str);
			case 'T': return new ParameterType(str += 1);
			default:
				throw InvalidTypeNameException(str);
		}
	}

	/* Крч, тесты показали, что если указать компилятору restrict, то он сам введет доп. переменную для значения указателя.
	   Поэтому можно спокойно забить на оптимизацию вручную и просто указывать restrict где попало 😆️ */
	vector<const Type*> parseMethodArguments(const char*& restrict str) {
		if(str[0] != '(')
			throw IllegalMethodDescriptorException(str);
		++str;

		vector<const Type*> arguments;

		while(true) {
			if(*str == ')') {
				++str;
				return arguments;
			}

			arguments.push_back(parseType(str));
		}
	}


	const BasicType* parseReturnType(const char* str) {
		return str[0] == 'V' ? VOID : parseType(str);
	}


	const ReferenceType* parseReferenceType(const char* str) {
		return str[0] == '[' ? (const ReferenceType*)new ArrayType(str) : (const ReferenceType*)new ClassType(str);
	}


	const ReferenceType* parseParameter(const char*& restrict str) {
		switch(str[0]) {
			case 'L': return new ClassType(str += 1);
			case '[': return new ArrayType(str);
			case 'T': return new ParameterType(str += 1);
			default:
				throw InvalidTypeNameException(str);
		}
	}


	vector<const ReferenceType*> parseParameters(const char*& restrict str) {
		if(str[0] != '<')
			throw InvalidTypeNameException(str, 0);

		vector<const ReferenceType*> parameters;

		for(const char* const srcStr = str++; ;) {
			switch(*str) {
				case '>':
					++str;
					return parameters;

				case '+':
					parameters.push_back(new ExtendingGenericType(str += 1));
					break;
				case '-':
					parameters.push_back(new SuperGenericType(str += 1));
					break;
				case '*':
					++str;
					parameters.push_back(AnyGenericType::getInstance());
					break;

				case '\0':
					throw InvalidTypeNameException(srcStr, str - srcStr);

				default:
					parameters.push_back(parseParameter(str));
			}
		}
	}


	vector<const GenericParameter*> parseGeneric(const char*& restrict str) {
		if(str[0] != '<')
			return vector<const GenericParameter*>();

		vector<const GenericParameter*> genericParameters;

		for(const char* const srcStr = str++; ;) {
			switch(*str) {
				case '>':
					++str;
					return genericParameters;

				default:
					genericParameters.push_back(new GenericParameter(str));
					break;

				case '\0':
					throw InvalidTypeNameException(srcStr, str - srcStr);
			}
		}

		return genericParameters;
	}


	IncopatibleTypesException::IncopatibleTypesException(const Type* type1, const Type* type2):
			DecompilationException("Incopatible types: " + type1->toString() + " and " + type2->toString()) {}

	string ClassConstant::toString(const ClassInfo& classinfo) const {
		return parseReferenceType(name)->toString(classinfo) + ".class";
	}
}

#endif
