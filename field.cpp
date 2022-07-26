#ifndef JDECOMPILER_FIELD_CPP
#define JDECOMPILER_FIELD_CPP

#include "field.h"
#include "code.cpp"

namespace jdecompiler {

	Field::Field(uint16_t modifiers, const FieldDescriptor& descriptor, const Attributes& attributes, const ClassInfo& classinfo):
			ClassElement(modifiers), descriptor(descriptor), attributes(attributes),
			constantValueAttribute(attributes.get<ConstantValueAttribute>()), genericType(getGenericType(attributes)),
			initializer(constantValueAttribute == nullptr ? nullptr :
					constantValueAttribute->getInitializer({classinfo, new FieldInfo(classinfo.thisType, descriptor)})) {

		if(initializer != nullptr)
			initializer->castReturnTypeTo(&descriptor.type);

		if(modifiers & ACC_ENUM && !(classinfo.modifiers & ACC_ENUM))
			throw IllegalModifiersException("Field " + descriptor.toString() + " cannot have enum flag in non-enum class");
	}

	Field::Field(const ClassInfo& classinfo, ClassInputStream& instream): Field(instream.readUShort(),
			*new FieldDescriptor(classinfo.constPool.getUtf8Constant(instream.readUShort()), classinfo.constPool.getUtf8Constant(instream.readUShort())),
			*new Attributes(instream, classinfo.constPool, instream.readUShort(), AttributesType::FIELD), classinfo) {}

	string Field::toString(const ClassInfo&) const {
		throw IllegalStateException("For Fields, the toString(const StringifyContext&) method must be called");
	}

	string Field::toString(const StringifyContext& context) const {
		string str;

		FormatString comment;
		if(this->isSynthetic())
			comment += "synthetic field";

		if(const AnnotationsAttribute* annotationsAttribute = attributes.get<AnnotationsAttribute>())
			str += annotationsAttribute->toString(context.classinfo);

		return (str + context.classinfo.getIndent() + (string)(modifiersToString(modifiers) +
				variableDeclarationToString(genericType == nullptr ? &descriptor.type : genericType, context.classinfo, descriptor.name)) +
				(initializer != nullptr ? " = " + (JDecompiler::getInstance().useShortArrayInitializing() ?
				initializer->toArrayInitString(context) : initializer->toString(context)) : EMPTY_STRING)) +
				(comment.empty() ? EMPTY_STRING : " // " + (string)comment);
	}

	bool Field::canStringify(const ClassInfo& classinfo) const {
		return !(this->isSynthetic() && !JDecompiler::getInstance().showSynthetic());
	}

	FormatString Field::modifiersToString(uint16_t modifiers) {
		FormatString str;

		switch(modifiers & ACC_ACCESS_FLAGS) {
			case ACC_VISIBLE: break;
			case ACC_PUBLIC: str += "public"; break;
			case ACC_PRIVATE: str += "private"; break;
			case ACC_PROTECTED: str += "protected"; break;
			default: throw IllegalModifiersException(modifiers);
		}

		if(modifiers & ACC_STATIC) str += "static";
		if(modifiers & ACC_FINAL && modifiers & ACC_VOLATILE) throw IllegalModifiersException(modifiers);
		if(modifiers & ACC_FINAL) str += "final";
		if(modifiers & ACC_TRANSIENT) str += "transient";
		if(modifiers & ACC_VOLATILE) str += "volatile";

		return str;
	}

}

#endif
