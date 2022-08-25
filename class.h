#ifndef JDECOMPILER_CLASS_H
#define JDECOMPILER_CLASS_H

#include "field.h"
#include "method.h"

namespace jdecompiler {

	struct Class: ClassElement {
		public:
			const Version version;
			const ClassType &thisType, *const superType;
			const ConstantPool& constPool;
			const vector<const ClassType*> interfaces;
			const Attributes& attributes;
			const ClassInfo& classinfo;
			const vector<const Field*> fields, constants;
			const vector<const Method*> methods;
			const vector<const GenericParameter*> genericParameters;

			const StringifyContext& fieldStringifyContext;

		private:
			const vector<const Field*> createFields(const vector<FieldDataHolder>&, const ClassInfo&) const;
			const vector<const Method*> createMethods(const vector<MethodDataHolder>&, const ClassInfo&) const;
			const StringifyContext& getFieldStringifyContext();

			static inline const vector<const Field*> filterConstants(const vector<const Field*>& fields) {
				return copy_if<const Field*>(fields, [] (const Field* field) { return field->isConstant(); });
			}


		protected:
			Class(const Version&, const ClassType&, const ClassType*, const ConstantPool&,
					modifiers_t, const vector<const ClassType*>&, const Attributes&,
					const vector<FieldDataHolder>&, const vector<MethodDataHolder>&,
					const vector<const GenericParameter*>&);


		public:
			static const Class* readClass(ClassInputStream&);

			template<class>
			static const Class* createClass(ClassInputStream&, const Version&, const ClassType&, const ClassType*, const ConstantPool&, modifiers_t,
					const vector<const ClassType*>&, const Attributes&, const vector<FieldDataHolder>&, const vector<MethodDataHolder>&,
					const vector<const GenericParameter*>&);


			const Field* getField(const string&) const;
			const Field* getField(const FieldDescriptor&) const;

			inline const vector<const Field*>& getFields() const {
				return fields;
			}


			const Method* getMethod(const MethodDescriptor&) const;

			inline bool hasMethod(const MethodDescriptor& descriptor) const {
				return getMethod(descriptor) != nullptr;
			}


			inline const vector<const Method*>& getMethods() const {
				return methods;
			}

			inline const vector<const Method*> getMethods(const function<bool(const Method*)>& predicate) const {
				return copy_if(methods, predicate);
			}

		protected:
			virtual string toString(const ClassInfo& classinfo) const override {
				return thisType.isPackageInfo ? packageInfoToString(classinfo) : toString0<false>(classinfo);
			}

			virtual string anonymousToString(const ClassInfo&) const;

		public:
			inline string toString() const {
				return toString(classinfo);
			}

			inline string anonymousToString() const {
				return anonymousToString(classinfo);
			}


		protected:
			template<bool>
			string toString0(const ClassInfo&) const;

			string annotationsToString(const ClassInfo&) const;


			string packageInfoToString(const ClassInfo&) const;


			virtual string headersToString(const ClassInfo&) const;

			string packageToString(const ClassInfo&) const;

			string versionCommentToString() const;


			virtual string anonymousDeclarationToString(const ClassInfo&) const;

			virtual string declarationToString(const ClassInfo&) const;

			string modifiersToString(modifiers_t) const;


			virtual string bodyToString(const ClassInfo& classinfo) const {
				return fieldsToString(classinfo) + methodsToString(classinfo) + innerClassesToString(classinfo);
			}


			virtual string fieldsToString(const ClassInfo&) const;

			virtual string methodsToString(const ClassInfo&) const;

			virtual string innerClassesToString(const ClassInfo&) const;


		public:
			inline bool canStringify() const {
				return !((modifiers & ACC_SYNTHETIC && !JDecompiler::getInstance().showSynthetic() && !thisType.isPackageInfo) ||
						(thisType.isNested && JDecompiler::getInstance().hasClass(thisType.enclosingClass->getEncodedName())));
			}

			virtual bool canStringify(const ClassInfo&) const override {
				return this->canStringify();
			}

		protected:
			template<typename T>
			inline void warning(const T& message) const {
				cerr << thisType.toString() << ": warning: " << message << endl;
			}
	};


	inline const vector<const Field*>& ClassInfo::getFields() const {
		return clazz.fields;
	}

	inline const vector<const Field*>& ClassInfo::getConstants() const {
		return clazz.constants;
	}

	inline const vector<const Method*>& ClassInfo::getMethods() const {
		return clazz.methods;
	}

	inline const vector<const Method*> ClassInfo::getMethods(const function<bool(const Method*)>& predicate) const {
		return clazz.getMethods(predicate);
	}

	inline const Method* ClassInfo::getMethod(const MethodDescriptor& descriptor) const {
		return clazz.getMethod(descriptor);
	}

	inline bool ClassInfo::hasMethod(const MethodDescriptor& descriptor) const {
		return clazz.hasMethod(descriptor);
	}


	inline const ClassInfo* JDecompiler::getClassInfo(const string& name) const {
		const Class* clazz = getClass(name);
		return clazz != nullptr ? &clazz->classinfo : nullptr;
	}

}

#endif
