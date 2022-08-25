#ifndef JDECOMPILER_ENUM_CLASS_CPP
#define JDECOMPILER_ENUM_CLASS_CPP

#include "enum-class.h"

namespace jdecompiler {

	EnumClass::EnumField::EnumField(const Field& field, const vector<const Operation*>& arguments):
			Field(field), arguments(arguments.begin(), arguments.end() - 2) {}

	string EnumClass::EnumField::toString(const StringifyContext& context) const {
		string str;

		if(const AnnotationsAttribute* annotationsAttribute = attributes.get<AnnotationsAttribute>())
			str += annotationsAttribute->toString(context.classinfo);

		return str + descriptor.name + (arguments.empty() ? EMPTY_STRING :
				'(' + rjoin<const Operation*>(arguments, [&context] (auto arg) { return arg->toString(context); }) + ')');
	}


	// EnumClass constructor definition moved into function-definitions.cpp


	string EnumClass::bodyToString(const ClassInfo& classinfo) const {
		string str = Class::bodyToString(classinfo);
		return !str.empty() && enumFields.empty() ? (string)"\n" + classinfo.getIndent() + "/* No enum constants */;\n" + str : str;
	}

	string EnumClass::fieldsToString(const ClassInfo& classinfo) const {
		string str;

		if(enumFields.size() > 0)
			str += (string)"\n" + classinfo.getIndent() +
					join<const EnumField*>(enumFields, [&classinfo, this] (auto field) { return field->toString(fieldStringifyContext); }) + ";\n";

		bool anyFieldStringified = false;
		for(const Field* field : otherFields)
			if(field->canStringify(classinfo)) {
				str += '\n' + field->toString(fieldStringifyContext) + ';';
				anyFieldStringified = true;
			}
		if(anyFieldStringified)
			str += '\n';

		return str;
	}
}

#endif
