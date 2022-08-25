#ifndef JDECOMPILER_VARIABLE_DECLARATION_TO_STRING_CPP
#define JDECOMPILER_VARIABLE_DECLARATION_TO_STRING_CPP

namespace jdecompiler {

	static inline string variableDeclarationToString(const Type* type, const ClassInfo& classinfo, const string& name) {
		return JDecompiler::getInstance().useCStyleArrayDeclaration() && instanceof<const ArrayType*>(type) ?
				static_cast<const ArrayType*>(type)->memberType->toString(classinfo) + ' ' + name +
				static_cast<const ArrayType*>(type)->braces : type->toString(classinfo) + ' ' + name;
	}
}

#endif
