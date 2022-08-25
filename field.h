#ifndef JDECOMPILER_FIELD_H
#define JDECOMPILER_FIELD_H

#include "code.h"
#include "class-element.cpp"

namespace jdecompiler {

	struct FieldDescriptor {
		const string name;
		const Type& type;

		inline FieldDescriptor(const string& name, const Type& type): name(name), type(type) {}

		inline FieldDescriptor(const string& name, const Type* type): FieldDescriptor(name, *type) {}

		inline FieldDescriptor(const string& name, const string& descriptor): FieldDescriptor(name, parseType(descriptor)) {}

		inline FieldDescriptor(const NameAndTypeConstant* nameAndType): FieldDescriptor(nameAndType->name, nameAndType->descriptor) {}

		inline string toString() const {
			return type.getName() + ' ' + name;
		}

		inline friend bool operator==(const FieldDescriptor& descriptor1, const FieldDescriptor& descriptor2) {
			return &descriptor1 == &descriptor2 || (descriptor1.name == descriptor2.name && descriptor1.type == descriptor2.type);
		}

		inline friend bool operator!=(const FieldDescriptor& descriptor1, const FieldDescriptor& descriptor2) {
			return !(descriptor1 == descriptor2);
		}
	};


	struct FieldInfo {
		const ClassType& clazz;
		const FieldDescriptor& descriptor;

		explicit FieldInfo(const ClassType& clazz, const FieldDescriptor& descriptor):
				clazz(clazz), descriptor(descriptor) {}
	};


	struct ConstantDecompilationContext {
		const ClassInfo& classinfo;
		const FieldInfo* const fieldinfo;

		explicit ConstantDecompilationContext(const ClassInfo& classinfo):
				classinfo(classinfo), fieldinfo(nullptr) {}

		explicit ConstantDecompilationContext(const ClassInfo& classinfo, const ClassType& clazz, const FieldDescriptor& descriptor):
				classinfo(classinfo), fieldinfo(new FieldInfo(clazz, descriptor)) {}

		~ConstantDecompilationContext() {
			if(fieldinfo != nullptr)
				delete fieldinfo;
		}
	};


	struct Field: ClassElement {
		public:
			const FieldDescriptor& descriptor;
			const Attributes& attributes;

			const ConstantValueAttribute* const constantValueAttribute;
			const Type* const genericType;

		private:
			static inline const Type* getGenericType(const Attributes& attributes) {
				const FieldSignatureAttribute* signatureAttribute = attributes.get<FieldSignatureAttribute>();
				return signatureAttribute == nullptr ? nullptr : &signatureAttribute->signature.type;
			}

		protected:
			mutable const Operation* initializer;
			friend void StaticInitializerScope::addOperation(const Operation*, const DecompilationContext&) const;

		public:
			Field(modifiers_t, const FieldDescriptor&, const Attributes&, const ClassInfo&);
			Field(const ClassInfo&, ClassInputStream&);

		private:
			virtual string toString(const ClassInfo&) const override final;

		public:
			virtual string toString(const StringifyContext&) const;

			virtual bool canStringify(const ClassInfo&) const override;

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
			static format_string modifiersToString(modifiers_t);

	};


	struct FieldDataHolder {
		public:
			const modifiers_t modifiers;
			const FieldDescriptor& descriptor;
			const Attributes& attributes;

			FieldDataHolder(const ConstantPool& constPool, ClassInputStream& instream): modifiers(instream.readUShort()),
					descriptor(*new FieldDescriptor(constPool.getUtf8Constant(instream.readUShort()), constPool.getUtf8Constant(instream.readUShort()))),
					attributes(*new Attributes(instream, constPool, instream.readUShort(), AttributesType::FIELD)) {}

			inline const Field* createField(const ClassInfo& classinfo) const {
				return new Field(modifiers, descriptor, attributes, classinfo);
			}
	};
}

#endif
