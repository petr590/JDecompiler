#ifndef JDECOMPILER_PRIMITIVE_TO_STRING_CPP
#define JDECOMPILER_PRIMITIVE_TO_STRING_CPP

namespace jdecompiler {

	template<typename T>
	static string numberConstantToString(T value) {
		static_assert(is_integral<T>(), "Type must be integral");

		if(JDecompiler::getInstance().useHexNumbersAlways()) {
			return hexWithPrefix(value);
		}

		if(JDecompiler::getInstance().canUseHexNumbers()) {
			if((value >= 16 || value <= -16) && (isPowerOfTwo(value) || isPowerOfTwo(value + 1)))
				return hexWithPrefix(value);
		}

		return to_string(value);
	}


	static inline string primitiveToString(jbool value) {
		return value ? "true" : "false";
	}

	static inline string primitiveToString(jchar c) {
		return '\'' + charToString<'\''>(c) + '\'';
	}

	static inline string primitiveToString(jbyte num) {
		return numberConstantToString(num);
	}

	static inline string primitiveToString(jshort num) {
		return numberConstantToString(num);
	}

	static inline string primitiveToString(jint num) {
		return numberConstantToString(num);
	}

	static inline string primitiveToString(jlong num) {
		return numberConstantToString(num) + JDecompiler::getInstance().getLongPostfix();
	}

	static string primitiveToString(jfloat num) {
		static const string integralFloatPostfix = (JDecompiler::getInstance().useTrailingZero() ? ".0" : EMPTY_STRING) +
				JDecompiler::getInstance().getFloatPostfix();
		// NaN and Infinity should be handled separately, but that's just in case
		if(isnan(num)) return "(0" + integralFloatPostfix + " / 0" + integralFloatPostfix + ')';
		if(isinf(num)) return '(' + (num > 0 ? "1" : "-1") + integralFloatPostfix + " / 0" + integralFloatPostfix + ')';

		ostringstream out;
		out.precision(9);

		out << num;
		if(floor(num) == num)
			out << integralFloatPostfix;
		else
			out << JDecompiler::getInstance().getFloatPostfix();

		return out.str();
	}

	static string primitiveToString(jdouble num) {
		static const string integralDoublePostfix = JDecompiler::getInstance().useDoublePostfix() ?
				(JDecompiler::getInstance().useTrailingZero() ? ".0" : EMPTY_STRING) + JDecompiler::getInstance().getDoublePostfix() : ".0";

		if(isnan(num)) return "(0" + integralDoublePostfix + " / 0" + integralDoublePostfix + ')';
		if(isinf(num)) return '(' + (num > 0 ? "1" : "-1") + integralDoublePostfix + " / 0" + integralDoublePostfix + ')';

		ostringstream out;
		out.precision(17);

		out << num;
		if(floor(num) == num)
			out << integralDoublePostfix;
		else if(JDecompiler::getInstance().useDoublePostfix())
			out << JDecompiler::getInstance().getDoublePostfix();

		return out.str();
	}


	static inline string primitiveToString(const string& str) {
		return stringToLiteral(str);
	}
}

#endif
