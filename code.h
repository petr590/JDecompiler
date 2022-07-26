#ifndef JDECOMPILER_CODE_H
#define JDECOMPILER_CODE_H

#include "types.cpp"
#include "attributes.cpp"
#include "type-by-builtin-type.h"
#include "operation.h"
#include "variable.h"
#include "instruction.h"
#include "block.h"

#include "disassembler-context.cpp"
#include "code-stack.cpp"
#include "decompilation-context.cpp"
#include "stringify-context.cpp"

#include "scope.h"

namespace jdecompiler {

	static inline string variableDeclarationToString(const Type* type, const ClassInfo& classinfo, const string& name) {
		return JDecompiler::getInstance().useCStyleArrayDeclaration() && instanceof<const ArrayType*>(type) ?
				static_cast<const ArrayType*>(type)->memberType->toString(classinfo) + ' ' + name +
				static_cast<const ArrayType*>(type)->braces : type->toString(classinfo) + ' ' + name;
	}
}

#endif
