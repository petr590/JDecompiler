#ifndef JDECOMPILER_CLASS_CPP
#define JDECOMPILER_CLASS_CPP

#include "class.h"
#include "field.cpp"
#include "method.cpp"
#include "version.cpp"
#include "enum-class.cpp"

namespace jdecompiler {

	const vector<const Field*> Class::createFields(const vector<FieldDataHolder>& fieldsData, const ClassInfo& classinfo) const {
		vector<const Field*> fields;
		fields.reserve(fieldsData.size());

		for(const FieldDataHolder& fieldData : fieldsData) {
			try {
				fields.push_back(fieldData.createField(classinfo));
			} catch(DecompilationException& ex) {
				cerr << "Exception while decompiling field " << fieldData.descriptor.toString() << ": " << ex.toString() << endl;
			}
		}

		return fields;
	}


	const vector<const Method*> Class::createMethods(const vector<MethodDataHolder>& methodsData, const ClassInfo& classinfo) const {
		vector<const Method*> methods;
		methods.reserve(methodsData.size());
		for(const MethodDataHolder& methodData : methodsData) {
			methods.push_back(methodData.createMethod(classinfo));
		}

		return methods;
	}

	const StringifyContext& Class::getFieldStringifyContext() {
		const MethodDescriptor& staticInitializerDescriptor = *new MethodDescriptor(thisType, "<clinit>", VOID);

		const Method* staticInitializer = getMethod(staticInitializerDescriptor);

		return staticInitializer != nullptr ? staticInitializer->context :
			*new StringifyContext(classinfo.getEmptyDisassemblerContext(), classinfo,
					new MethodScope(0, 0, 0), ACC_STATIC, staticInitializerDescriptor, Attributes::getEmptyInstance());
	}


	// Do not do it through constructor delegation, otherwise the fields are not initialized
	Class::Class(const Version& version, const ClassType& thisType, const ClassType* superType, const ConstantPool& constPool,
			uint16_t modifiers, const vector<const ClassType*>& interfaces, const Attributes& attributes,
			const vector<FieldDataHolder>& fieldsData, const vector<MethodDataHolder>& methodsData,
			const vector<const GenericParameter*>& genericParameters):
			ClassElement(modifiers), version(version), thisType(thisType), superType(superType),
			constPool(constPool), interfaces(interfaces), attributes(attributes),
			classinfo(*new ClassInfo(*this, thisType, superType, interfaces, constPool, attributes, modifiers, version)),
			fields(createFields(fieldsData, classinfo)), constants(filterConstants(fields)), methods(createMethods(methodsData, classinfo)),
			genericParameters(genericParameters), fieldStringifyContext(getFieldStringifyContext()) {

		if(thisType.isPackageInfo) {
			if(modifiers != (ACC_INTERFACE | ACC_ABSTRACT | ACC_SYNTHETIC))
				throw IllegalModifiersException("package-info must have only interface, abstract and synthetic modifiers");

			if(!fields.empty() || !methods.empty() || attributes.has<InnerClassesAttribute>())
				throw IllegalPackageInfoException("package-info must not have fields, methods or inner classes attribute");
		}
	}


	const Class* Class::readClass(ClassInputStream& instream) {
		if(instream.readUInt() != CLASS_SIGNATURE)
			throw ClassFormatError("Wrong class signature");

		const uint16_t
				minorVersion = instream.readUShort(),
				majorVersion = instream.readUShort();

		const Version version(majorVersion, minorVersion);

		const ConstantPool& constPool = *new ConstantPool(instream);

		const uint16_t modifiers = instream.readUShort();

		const ClassType& thisType = *new ClassType(constPool.get<ClassConstant>(instream.readUShort()));
		const ClassConstant* superClassConstant = constPool.getNullable<ClassConstant>(instream.readUShort());
		const ClassType* superType;

		if(superClassConstant == nullptr) {
			if(thisType != javaLang::Object)
				throw DecompilationException("Class " + thisType.getName() + " has no super class");
			superType = nullptr;
		} else {
			superType = new ClassType(superClassConstant);
		}

		const uint16_t interfacesCount = instream.readUShort();
		vector<const ClassType*> interfaces;
		interfaces.reserve(interfacesCount);

		for(uint16_t i = 0; i < interfacesCount; ++i)
			interfaces.push_back(new ClassType(constPool.get<ClassConstant>(instream.readUShort())->name));


		const uint16_t fieldsCount = instream.readUShort();
		vector<FieldDataHolder> fieldsData;
		fieldsData.reserve(fieldsCount);

		for(uint16_t i = 0; i < fieldsCount; ++i)
			fieldsData.push_back(FieldDataHolder(constPool, instream));


		const uint16_t methodsCount = instream.readUShort();
		vector<MethodDataHolder> methodsData;
		methodsData.reserve(methodsCount);

		for(uint16_t i = 0; i < methodsCount; ++i)
			methodsData.push_back(MethodDataHolder(constPool, instream, thisType));

		const Attributes& attributes = *new Attributes(instream, constPool, instream.readUShort(), AttributesType::CLASS);

		vector<const GenericParameter*> genericParameters;

		const ClassSignatureAttribute* signatureAttribute = attributes.get<ClassSignatureAttribute>();
		if(signatureAttribute != nullptr) {
			const ClassSignature& signature = signatureAttribute->signature;

			if(signature.superClass != *superType || !equal_values(interfaces, signature.interfaces))
				throw IncopatibleSignatureTypesException("In class " + thisType.getName());

			genericParameters = signature.genericParameters;

			superType = &signature.superClass;
			interfaces = signature.interfaces;

		}

		return modifiers & ACC_ENUM ?
			createClass<EnumClass>(instream, version, thisType, superType, constPool, modifiers, interfaces,
					attributes, fieldsData, methodsData, genericParameters) :
			createClass<Class>    (instream, version, thisType, superType, constPool, modifiers, interfaces,
					attributes, fieldsData, methodsData, genericParameters);
	}

	/* Hacking function: it is necessary to add this Class* to the JDecompiler::decompilationClasses BEFORE initializing fields and methods */
	template<class C>
	inline const Class* Class::createClass(ClassInputStream& instream, const Version& version, const ClassType& thisType, const ClassType* superType,
			const ConstantPool& constPool, uint16_t modifiers, const vector<const ClassType*>& interfaces,
			const Attributes& attributes, const vector<FieldDataHolder>& fieldsData, const vector<MethodDataHolder>& methodsData,
			const vector<const GenericParameter*>& genericParameters) {

		static_assert(is_base_of<Class, C>(), "Type must be subtype of type Class");

		C* clazz = static_cast<C*>(operator new(sizeof(C)));

		static const regex classExtension("\\.class$");
		static const JDecompiler& jdecompiler = JDecompiler::getInstance();

		jdecompiler.classes.emplace(thisType.getEncodedName(), clazz);
		jdecompiler.decompilationClasses.emplace(thisType.getEncodedName(),
				ClassHolder(regex_replace(instream.fileName, classExtension, "") + ".java", clazz));

		return new(clazz) C(version, thisType, superType, constPool, modifiers, interfaces, attributes, fieldsData, methodsData, genericParameters);
	}


	const Field* Class::getField(const string& name) const {
		for(const Field* field : fields)
			if(field->descriptor.name == name)
				return field;
		return nullptr;
	}

	const Field* Class::getField(const FieldDescriptor& descriptor) const {
		for(const Field* field : fields)
			if(field->descriptor == descriptor)
				return field;
		return nullptr;
	}

	const Method* Class::getMethod(const MethodDescriptor& descriptor) const {
		const auto& result = find_if(methods.begin(), methods.end(),
				[&descriptor] (const Method* method) { return method->descriptor.equalsIgnoreClass(descriptor); });
		return result == methods.end() ? nullptr : *result;
	}


	string Class::anonymousToString(const ClassInfo& classinfo) const {
		return thisType.isAnonymous ? toString0<true>(classinfo) : throw IllegalStateException("invokation of anonymousToString in a non-anonymous class");
	}



	template<bool isAnonymous>
	string Class::toString0(const ClassInfo& classinfo) const {
		string str;

		str += annotationsToString(classinfo);

		str += (isAnonymous ? anonymousDeclarationToString(classinfo) : declarationToString(classinfo)) + " {\n";

		const size_t baseSize = str.size();

		classinfo.increaseIndent();

		str += bodyToString(classinfo);

		classinfo.reduceIndent();

		if(str.size() == baseSize)
			str.back() = '}';
		else
			str += (str.back() == '\n' ? EMPTY_STRING : "\n") + classinfo.getIndent() + '}';


		return headersToString(classinfo) + str;
	}

	inline string Class::annotationsToString(const ClassInfo& classinfo) const {
		if(const AnnotationsAttribute* annotationsAttribute = attributes.get<AnnotationsAttribute>())
			return annotationsAttribute->toString(classinfo) + '\n';
		return EMPTY_STRING;
	}


	string Class::packageInfoToString(const ClassInfo& classinfo) const {
		string str;

		str += annotationsToString(classinfo);

		str += packageToString(classinfo);

		str += classinfo.importsToString();

		if(str.back() == '\n')
			str.pop_back();

		return str;
	}


	string Class::headersToString(const ClassInfo& classinfo) const {
		if(thisType.isNested)
			return EMPTY_STRING;

		string str;

		str += versionCommentToString();

		str += packageToString(classinfo);

		str += classinfo.importsToString();

		return str;
	}


	inline string Class::packageToString(const ClassInfo& classinfo) const {
		return thisType.packageName.empty() ? EMPTY_STRING : (string)classinfo.getIndent() + "package " + thisType.packageName + ";\n\n";
	}

	inline string Class::versionCommentToString() const {
		return JDecompiler::getInstance().printClassVersion() ? "/* Java version: " + to_string(version) + " */\n" : EMPTY_STRING;
	}


	string Class::anonymousDeclarationToString(const ClassInfo& classinfo) const {
		if(interfaces.size() > 1)
			throw DecompilationException("Anonymous class " + thisType.getName() + " cannot implement more than one interface");

		if(interfaces.size() == 1) {
			if(*superType != javaLang::Object)
				throw DecompilationException("Anonymous class " + thisType.getName() +
						" cannot implement an interface and simultaneously inherit from something other than java.lang.Object class");

			return interfaces[0]->toString(classinfo) + "()";
		}

		return superType->toString(classinfo) + "()";
	}


	string Class::declarationToString(const ClassInfo& classinfo) const {
		string str;

		if(thisType.isNested && !JDecompiler::getInstance().hasClass(thisType.enclosingClass->getEncodedName()))
			str += thisType.isAnonymous ? "/* anonymous class */\n" :
					"/* nested class of class " + thisType.enclosingClass->getName() + " */\n";

		if(modifiers & ACC_INTERFACE && superType != nullptr && *superType != javaLang::Object) {
			throw DecompilationException("interface " + thisType.getName() + " cannot inherit from other class than java.lang.Object");
		}

		str += classinfo.getIndent() + modifiersToString(modifiers) + ' ' + thisType.simpleName +
				(genericParameters.empty() ? EMPTY_STRING : '<' + join<const GenericParameter*>(genericParameters,
					[&classinfo] (const GenericParameter* parameter) { return parameter->toString(classinfo); }) + '>') +

				(superType == nullptr || *superType == javaLang::Object || (modifiers & ACC_ENUM && *superType == javaLang::Enum) ?
						EMPTY_STRING : " extends " + superType->toString(classinfo));

		vector<const ClassType*> interfacesToStringify;
		for(const ClassType* interface : interfaces) {
			if(modifiers & ACC_ANNOTATION && *interface == javaLangAnnotation::Annotation)
				continue;
			interfacesToStringify.push_back(interface);
		}

		if(!interfacesToStringify.empty())
			str += (modifiers & ACC_INTERFACE ? " extends " : " implements ") +
					join<const ClassType*>(interfacesToStringify, [&classinfo] (auto interface) { return interface->toString(classinfo); });

		return str;
	}


	string Class::modifiersToString(uint16_t modifiers) const {
		format_string str;

		bool isInnerProtected = false;

		if(thisType.isNested) {
			const InnerClassesAttribute* innerClasses = attributes.get<InnerClassesAttribute>();
			if(innerClasses != nullptr) {
				const InnerClass* innerClass = innerClasses->find(thisType);
				if(innerClass != nullptr) {
					const uint16_t innerClassModifiers = innerClass->modifiers;

					if(innerClassModifiers & ACC_PRIVATE) str += "private";
					if(innerClassModifiers & ACC_PROTECTED) {
						str += "protected";
						isInnerProtected = true;
					}

					if(innerClassModifiers & ACC_STATIC) str += "static";

					if((innerClassModifiers & ~(ACC_ACCESS_FLAGS | ACC_STATIC | ACC_SUPER)) != (modifiers & ~(ACC_ACCESS_FLAGS | ACC_SUPER)))
						warning("modifiers of class " + thisType.getName() + " are not matching to the modifiers in InnerClasses attribute:"
								+ hexWithPrefix<4>(innerClassModifiers) + ' ' + hexWithPrefix<4>(modifiers));
				}
			}
		}

		switch(modifiers & ACC_ACCESS_FLAGS) {
			case ACC_VISIBLE: break;
			case ACC_PUBLIC:
				if(!isInnerProtected)
					str += "public";
				break;

			default:
				throw IllegalModifiersException("in class " + thisType.getName() + ": " + hexWithPrefix<4>(modifiers));
		}


		if(modifiers & ACC_STRICT) str += "strictfp";

		switch(modifiers & (ACC_FINAL | ACC_ABSTRACT | ACC_INTERFACE | ACC_ANNOTATION | ACC_ENUM)) {
			case 0:
				str += "class"; break;
			case ACC_FINAL:
				str += "final class"; break;
			case ACC_ABSTRACT:
				str += "abstract class"; break;
			case ACC_ABSTRACT | ACC_INTERFACE:
				str += "interface"; break;
			case ACC_ABSTRACT | ACC_INTERFACE | ACC_ANNOTATION:
				str += "@interface"; break;
			case ACC_ENUM: case ACC_FINAL | ACC_ENUM: case ACC_ABSTRACT | ACC_ENUM:
				str += "enum"; break;
			default:
				throw IllegalModifiersException("in class " + thisType.getName() + ": " + hexWithPrefix<4>(modifiers));
		}

		return (string)str;
	}


	string Class::fieldsToString(const ClassInfo& classinfo) const {
		string str;

		bool anyFieldStringified = false;

		for(const Field* field : fields) {
			if(field->canStringify(classinfo)) {
				str += '\n' + field->toString(fieldStringifyContext) + ';';
				anyFieldStringified = true;
			}
		}

		if(anyFieldStringified)
			str += '\n';

		return str;
	}


	string Class::methodsToString(const ClassInfo& classinfo) const {
		string str;

		for(const Method* method : methods) {
			if(method->canStringify(classinfo)) {
				log("stringify of", method->descriptor.toString());
				str += '\n' + method->toString(classinfo) + '\n';
			}
		}

		return str;
	}

	string Class::innerClassesToString(const ClassInfo& classinfo) const {
		string str;

		const NestMembersAttribute* nestMembers = attributes.get<NestMembersAttribute>();

		if(nestMembers != nullptr) {

			for(const ClassType* nestMember : nestMembers->nestMembers) {
				if(nestMember->isAnonymous)
					continue;

				const Class* nestClass = JDecompiler::getInstance().getClass(nestMember->getClassEncodedName());

				if(nestClass != nullptr) {
					nestClass->classinfo.copyFormattingFrom(classinfo);
					str += '\n' + nestClass->toString() + '\n';
					nestClass->classinfo.resetFormatting();
				} else {
					warning("cannot load inner class " + nestMember->getName());
				}
			}
		}

		return str;
	}
}

#endif
