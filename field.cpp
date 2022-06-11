#ifndef JDECOMPILER_FIELD_CPP
#define JDECOMPILER_FIELD_CPP

#include "code.cpp"
#include "attributes.cpp"

namespace jdecompiler {
	struct FieldDescriptor {
		const string name;
		const Type& type;

		FieldDescriptor(const string& name, const Type& type): name(name), type(type) {}

		FieldDescriptor(const string& name, const Type* type): FieldDescriptor(name, *type) {}

		FieldDescriptor(const string& name, const string& descriptor): FieldDescriptor(name, parseType(descriptor)) {}

		FieldDescriptor(const NameAndTypeConstant* nameAndType): FieldDescriptor(nameAndType->name, nameAndType->descriptor) {}

		string toString() const {
			return type.getName() + ' ' + name;
		}

		inline friend bool operator== (const FieldDescriptor& descriptor1, const FieldDescriptor& descriptor2) {
			return &descriptor1 == &descriptor2 || (descriptor1.name == descriptor2.name && descriptor1.type == descriptor2.type);
		}

		inline friend bool operator!= (const FieldDescriptor& descriptor1, const FieldDescriptor& descriptor2) {
			return !(descriptor1 == descriptor2);
		}
	};


	struct FieldInfo {
		const ClassType& clazz;
		const FieldDescriptor& descriptor;

		FieldInfo(const ClassType& clazz, const FieldDescriptor& descriptor):
				clazz(clazz), descriptor(descriptor) {}
	};


	struct Field: ClassElement {
		public:
			const uint16_t modifiers;
			const FieldDescriptor& descriptor;
			const Attributes& attributes;

			const ConstantValueAttribute* const constantValueAttribute;

		protected:
			mutable const Operation* initializer;
			mutable const StringifyContext* context;
			friend void StaticInitializerScope::addOperation(const Operation*, const StringifyContext&) const;

		public:
			Field(uint16_t modifiers, const FieldDescriptor& descriptor, const Attributes& attributes, const ClassInfo& classinfo): modifiers(modifiers),
					descriptor(descriptor), attributes(attributes), constantValueAttribute(attributes.get<ConstantValueAttribute>()),
					initializer(constantValueAttribute == nullptr ? nullptr :
							constantValueAttribute->getInitializer({classinfo, new FieldInfo(classinfo.thisType, descriptor)})),
					context(initializer == nullptr ? nullptr : classinfo.getFieldStringifyContext()) {
				if(initializer != nullptr)
					initializer->castReturnTypeTo(&descriptor.type);
			}

			Field(const ClassInfo& classinfo, ClassInputStream& instream): Field(instream.readUShort(),
					*new FieldDescriptor(classinfo.constPool.getUtf8Constant(instream.readUShort()),
							classinfo.constPool.getUtf8Constant(instream.readUShort())),
					*new Attributes(instream, classinfo.constPool, instream.readUShort()), classinfo) {}

			virtual string toString(const ClassInfo& classinfo) const override {
				string str;

				FormatString comment;
				if(modifiers & ACC_SYNTHETIC)
					comment += "synthetic field";

				if(const AnnotationsAttribute* annotationsAttribute = attributes.get<AnnotationsAttribute>())
					str += annotationsAttribute->toString(classinfo);

				return (str + classinfo.getIndent() + (string)(modifiersToString(modifiers) + descriptor.type.toString(classinfo)) + ' ' + descriptor.name +
						(initializer != nullptr ? " = " + (JDecompiler::getInstance().canUseShortArrayInitializing() ?
						initializer->toArrayInitString(*context) : initializer->toString(*context)) : EMPTY_STRING)) +
						(comment.empty() ? EMPTY_STRING : " // " + (string)comment);
			}

			virtual bool canStringify(const ClassInfo& classinfo) const override {
				return !(modifiers & ACC_SYNTHETIC && !JDecompiler::getInstance().showSynthetic());
			}

			inline bool hasInitializer() const {
				return initializer != nullptr;
			}

			inline const Operation* getInitializer() const {
				return initializer;
			}

			inline bool isConstant() const {
				return constantValueAttribute != nullptr;
			}

		private:
			static FormatString modifiersToString(uint16_t modifiers) {
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
	};


	struct FieldDataHolder {
		public:
			const uint16_t modifiers;
			const FieldDescriptor& descriptor;
			const Attributes& attributes;

			FieldDataHolder(const ConstantPool& constPool, ClassInputStream& instream): modifiers(instream.readUShort()),
					descriptor(*new FieldDescriptor(constPool.getUtf8Constant(instream.readUShort()), constPool.getUtf8Constant(instream.readUShort()))),
					attributes(*new Attributes(instream, constPool, instream.readUShort())) {}

			const Field* createField(const ClassInfo& classinfo) const {
				return new Field(modifiers, descriptor, attributes, classinfo);
			}
	};
}

#endif
