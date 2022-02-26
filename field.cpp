#ifndef JDECOMPILER_FIELD_CPP
#define JDECOMPILER_FIELD_CPP

#include "code.cpp"
#include "attributes.cpp"

namespace jdecompiler {
	struct FieldDescriptor {
		const string name;
		const Type& type;

		FieldDescriptor(const NameAndTypeConstant* nameAndType): FieldDescriptor(*nameAndType->name, *nameAndType->descriptor) {}

		FieldDescriptor(const string& name, const string& descriptor): name(name), type(*parseType(descriptor)) {}
	};


	struct Field: ClassElement {
		public:
			const uint16_t modifiers;
			const FieldDescriptor& descriptor;
			const Attributes& attributes;

		protected:
			const ConstantValueAttribute* constantValueAttribute;
			mutable const Operation* initializer = nullptr;
			mutable const CodeEnvironment* environment = nullptr;
			friend void StaticInitializerScope::add(const Operation*, const CodeEnvironment&);

		public:
			Field(const ConstantPool& constPool, BinaryInputStream& instream): modifiers(instream.readUShort()),
					descriptor(*new FieldDescriptor(constPool.getUtf8Constant(instream.readUShort()), constPool.getUtf8Constant(instream.readUShort()))),
					attributes(*new Attributes(instream, constPool, instream.readUShort())), constantValueAttribute(attributes.get<ConstantValueAttribute>()) {}

			virtual string toString(const ClassInfo& classinfo) const override {
				string str;

				if(const AnnotationsAttribute* annotationsAttribute = attributes.get<AnnotationsAttribute>())
					str += annotationsAttribute->toString(classinfo);

				return str + (string)(modifiersToString(modifiers) + descriptor.type.toString(classinfo)) + ' ' + descriptor.name +
						(constantValueAttribute != nullptr ? " = " + constantValueAttribute->toString(classinfo) :
						initializer != nullptr ? " = " + initializer->toString(*environment) : EMPTY_STRING);
			}

			virtual bool canStringify(const ClassInfo& classinfo) const override {
				return !(modifiers & ACC_SYNTHETIC);
			}

			inline bool hasInitializer() const {
				return initializer != nullptr;
			}

			inline const Operation* getInitializer() const {
				return initializer;
			}

		private:
			static FormatString modifiersToString(uint16_t modifiers) {
				FormatString str;

				switch(modifiers & (ACC_VISIBLE | ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED)) {
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
	};
}

#endif
