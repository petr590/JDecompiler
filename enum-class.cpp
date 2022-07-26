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


	EnumClass::EnumConstructorDescriptor::EnumConstructorDescriptor(const MethodDescriptor& other):
			MethodDescriptor(other.clazz, other.name, other.returnType, other.arguments),
			factualArguments(other.arguments.begin() + 2, other.arguments.end()) {}

	string EnumClass::EnumConstructorDescriptor::toString(const StringifyContext& context, const Attributes& attributes) const {
		return MethodDescriptor::toString(context, attributes, 2);
	}


	vector<MethodDataHolder> EnumClass::processMethodData(const vector<MethodDataHolder>& methodsData) {
		vector<MethodDataHolder> newMethodDataHolders;
		newMethodDataHolders.reserve(methodsData.size());

		for(const MethodDataHolder& methodData : methodsData) {
			newMethodDataHolders.push_back(methodData.descriptor.isConstructor() ? MethodDataHolder(methodData.modifiers,
					*new EnumConstructorDescriptor(methodData.descriptor), methodData.attributes) : methodData);
		}

		return newMethodDataHolders;
	}


	// moved into function-definitions.cpp
	/*EnumClass::EnumClass(const Version& version, const ClassType& thisType, const ClassType* superType, const ConstantPool& constPool, uint16_t modifiers,
			const vector<const ClassType*>& interfaces, const Attributes& attributes,
			const vector<FieldDataHolder>& fieldsData, const vector<MethodDataHolder>& methodDataHolders):
			Class(version, thisType, superType, constPool, modifiers, interfaces, attributes, fieldsData, processMethodData(methodDataHolders)) {

		using namespace operations;

		for(const Field* field : fields) {
			const InvokespecialOperation* invokespecialOperation;
			const Dup1Operation* dupOperation;
			const NewOperation* newOperation;

			if(field->modifiers & ACC_ENUM && field->descriptor.type == thisType && field->hasInitializer() &&
					(invokespecialOperation = dynamic_cast<const InvokespecialOperation*>(field->getInitializer())) != nullptr &&
					(dupOperation = dynamic_cast<const Dup1Operation*>(invokespecialOperation->object)) != nullptr &&
					(newOperation = dynamic_cast<const NewOperation*>(dupOperation->operation)) != nullptr) {

				if(field->modifiers != (ACC_PUBLIC | ACC_STATIC | ACC_FINAL | ACC_ENUM))
					throw IllegalModifiersException("Enum constant must be public static final, got " + hexWithPrefix<4>(field->modifiers));

				if(invokespecialOperation->arguments.size() < 2)
					throw DecompilationException("enum constant initializer should have at least two arguments, got " +
							to_string(invokespecialOperation->arguments.size()));
				enumFields.push_back(new EnumField(*field, invokespecialOperation->arguments));
			} else {
				otherFields.push_back(field);
			}
		}
	}*/


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
