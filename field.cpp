#ifndef JDECOMPILER_FIELD_CPP
#define JDECOMPILER_FIELD_CPP

#include "field.h"
#include "code.cpp"

namespace jdecompiler {

	Field::Field(modifiers_t modifiers, const FieldDescriptor& descriptor, const Attributes& attributes, const ClassInfo& classinfo)
		try:
			ClassElement(modifiers), descriptor(descriptor), attributes(attributes),
			constantValueAttribute(attributes.get<ConstantValueAttribute>()), genericType(getGenericType(attributes)),
			initializer(constantValueAttribute == nullptr ? nullptr :
					constantValueAttribute->getInitializer()) {

		if(initializer != nullptr)
			initializer->castReturnTypeTo(&descriptor.type);

		if(modifiers & ACC_ENUM && !(classinfo.modifiers & ACC_ENUM))
			throw IllegalModifiersException("Field " + descriptor.toString() + " cannot have enum flag in non-enum class");

	} catch(DecompilationException& ex) {
		cerr << "Exception while decompiling field " << descriptor.toString() << ": " << ex.toString() << endl;
	}


	Field::Field(const ClassInfo& classinfo, ClassInputStream& instream): Field(instream.readUShort(),
			*new FieldDescriptor(classinfo.constPool.getUtf8Constant(instream.readUShort()), classinfo.constPool.getUtf8Constant(instream.readUShort())),
			*new Attributes(instream, classinfo.constPool, instream.readUShort(), AttributesType::FIELD), classinfo) {}

	string Field::toString(const ClassInfo&) const {
		throw IllegalStateException("For Fields, the toString(const StringifyContext&) method must be called");
	}

	string Field::toString(const StringifyContext& context) const {
		string str;

		format_string comment;
		if(this->isSynthetic())
			comment += "synthetic field";

		if(const AnnotationsAttribute* annotationsAttribute = attributes.get<AnnotationsAttribute>())
			str += annotationsAttribute->toString(context.classinfo) + '\n';

		str += context.classinfo.getIndent() + (string)(modifiersToString(modifiers) +
				variableDeclarationToString(genericType == nullptr ? &descriptor.type : genericType, context.classinfo, descriptor.name));

		if(initializer != nullptr) {
			str += " = " + (
				initializer->isAbstractConstOperation() ?
					initializer->toString(context,
						ConstantDecompilationContext(context.classinfo, context.classinfo.thisType, descriptor)) :

				JDecompiler::getInstance().useShortArrayInitializing() ?
					initializer->toArrayInitString(context) :
					initializer->toString(context));
		}

		if(!comment.empty())
			str += " // " + (string)comment;

		return str;
	}

	bool Field::canStringify(const ClassInfo&) const {
		return !(this->isSynthetic() && !JDecompiler::getInstance().showSynthetic());
	}

	format_string Field::modifiersToString(modifiers_t modifiers) {
		format_string str;

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
