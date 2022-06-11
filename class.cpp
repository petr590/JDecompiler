#ifndef JDECOMPILER_CLASS_CPP
#define JDECOMPILER_CLASS_CPP

#include "field.cpp"
#include "method.cpp"

namespace jdecompiler {
	struct Version {
		const uint16_t majorVersion, minorVersion;

		constexpr Version(uint16_t majorVersion, uint16_t minorVersion): majorVersion(majorVersion), minorVersion(minorVersion) {}

		friend string to_string(const Version& version) {
			static const map<uint16_t, const string> versionTable {
					// I have not found an official indication of version JDK Beta, JDK 1.0 and JDK 1.1 numbers, and I'm too lazy to check it
					{43, "JDK Beta"}, {44, "JDK 1.0"}, {45, "JDK 1.1"}, {46, "Java 1.2"}, {47, "Java 1.3"}, {48, "Java 1.4"},
					{49, "Java 5"  }, {50, "Java 6" }, {51, "Java 7" }, {52, "Java 8"  }, {53, "Java 9"  }, {54, "Java 10"},
					{55, "Java 11" }, {56, "Java 12"}, {57, "Java 13"}, {58, "Java 14" }, {59, "Java 15" }, {60, "Java 16"},
					{61, "Java 17" }, {62, "Java 18"}, {63, "Java 19"}
			};
			return to_string(version.majorVersion) + '.' + to_string(version.minorVersion) +
					(has(versionTable, version.majorVersion) ? " (" + versionTable.at(version.majorVersion) + ')' : EMPTY_STRING);
		}
	};

	struct Class: Stringified {
		public:
			const Version version;
			const ClassType &thisType, *const superType;
			const ConstantPool& constPool;
			const uint16_t modifiers;
			const vector<const ClassType*> interfaces;
			const Attributes& attributes;
			const ClassInfo& classinfo;
			const vector<const Field*> fields, constants;
			const vector<const Method*> methods;

		protected:
			Class(const Version& version, const ClassType& thisType, const ClassType* superType, const ConstantPool& constPool, uint16_t modifiers,
					const vector<const ClassType*>& interfaces, const Attributes& attributes, const ClassInfo& classinfo,
					const vector<const Field*>& fields, const vector<const Method*>& methods):
					version(version), thisType(thisType), superType(superType), constPool(constPool), modifiers(modifiers),
					interfaces(interfaces), attributes(attributes), classinfo(classinfo),
					fields(fields), constants(getConstants(fields)), methods(methods) {}

		private:
			const vector<const Field*> createFields(const vector<FieldDataHolder>& fieldsData, const ClassInfo& classinfo) const {
				vector<const Field*> fields;
				fields.reserve(fieldsData.size());

				for(const FieldDataHolder& fieldData : fieldsData) {
					if(!JDecompiler::getInstance().isFailOnError()) {
						try {
							fields.push_back(fieldData.createField(classinfo));
						} catch(DecompilationException& ex) {
							const char* message = ex.what();
							cerr << "Exception while decompiling field " << fieldData.descriptor.toString() << ": "
									<< typenameof(ex) << (*message == '\0' ? EMPTY_STRING : (string)": " + message) << endl;
						}
					} else {
						fields.push_back(fieldData.createField(classinfo));
					}
				}

				return fields;
			}


			const vector<const Method*> createMethods(const vector<MethodDataHolder>& methodsData, const ClassInfo& classinfo) const {
				vector<const Method*> methods;
				methods.reserve(methodsData.size());
				for(const MethodDataHolder& methodData : methodsData) {
					if(!JDecompiler::getInstance().isFailOnError()) {
						try {
							methods.push_back(methodData.createMethod(classinfo));
						} catch(DecompilationException& ex) {
							const char* message = ex.what();
							cerr << "Exception while decompiling method " << methodData.descriptor.toString() << ": "
									<< typenameof(ex) << (*message == '\0' ? EMPTY_STRING : (string)": " + message) << endl;
						}
					} else {
						methods.push_back(methodData.createMethod(classinfo));
					}
				}
				return methods;
			}


		protected: // Do not do it through constructor delegation, otherwise the fields are not initialized
			Class(const Version& version, const ClassType& thisType, const ClassType* superType, const ConstantPool& constPool,
					uint16_t modifiers, const vector<const ClassType*>& interfaces, const Attributes& attributes,
					const vector<FieldDataHolder>& fieldsData, const vector<MethodDataHolder>& methodsData):
					version(version), thisType(thisType), superType(superType), constPool(constPool), modifiers(modifiers),
					interfaces(interfaces), attributes(attributes),
					classinfo(*new ClassInfo(*this, thisType, superType, interfaces, constPool, attributes, modifiers)),
					fields(createFields(fieldsData, classinfo)), constants(getConstants(fields)), methods(createMethods(methodsData, classinfo)) {}


		public:
			static const Class* readClass(ClassInputStream&);


			inline const vector<const Field*>& getFields() const {
				return fields;
			}

			const Field* getField(const string& name) const {
				for(const Field* field : fields)
					if(field->descriptor.name == name)
						return field;
				return nullptr;
			}

			static inline const vector<const Field*> getConstants(const vector<const Field*>& fields) {
				return copy_if<const Field*>(fields, [] (const Field* field) { return field->isConstant(); });
			}


			inline const vector<const Method*>& getMethods() const {
				return methods;
			}

			inline const vector<const Method*> getMethods(const function<bool(const Method*)>& predicate) const {
				return copy_if(methods, predicate);
			}

			const Method* getMethod(const MethodDescriptor& descriptor) const {
				const auto& result = find_if(methods.begin(), methods.end(),
						[&descriptor] (const Method* method) { return method->descriptor.equalsIgnoreClass(descriptor); });
				return result == methods.end() ? nullptr : *result;
			}

			inline bool hasMethod(const MethodDescriptor& descriptor) const {
				return getMethod(descriptor) != nullptr;
			}


			virtual string toString(const ClassInfo& classinfo) const override {
				return toString0<false>(classinfo);
			}

			inline string toString() const {
				return toString(classinfo);
			}

			virtual string anonymousToString(const ClassInfo& classinfo) const {
				if(thisType.isAnonymous)
					return toString0<true>(classinfo);
				throw IllegalStateException("invokation of anonymousToString in a non-anonymous class");
			}

			inline string anonymousToString() const {
				return anonymousToString(classinfo);
			}


		protected:
			template<bool isAnonymous>
			string toString0(const ClassInfo& classinfo) const {
				string str;

				if(const AnnotationsAttribute* annotationsAttribute = attributes.get<AnnotationsAttribute>())
					str += annotationsAttribute->toString(classinfo) + '\n';

				str += (isAnonymous ? anonymousDeclarationToString(classinfo) : declarationToString(classinfo)) + " {\n";

				const size_t baseSize = str.size();

				classinfo.increaseIndent();

				str += fieldsToString(classinfo);

				str += methodsToString(classinfo);

				str += innerClassesToString(classinfo);

				classinfo.reduceIndent();

				if(str.size() == baseSize)
					str.back() = '}';
				else
					str += (str.back() == '\n' ? EMPTY_STRING : "\n") + classinfo.getIndent() + '}';


				return headersToString(classinfo) + str;
			}


			template<typename T>
			inline void warning(const T& message) const {
				cerr << thisType.toString() << ": warning: " << message << endl;
			}


		public:
			inline bool canStringify() const {
				return !((modifiers & ACC_SYNTHETIC && !JDecompiler::getInstance().showSynthetic()) ||
						(thisType.isNested && JDecompiler::getInstance().hasClass(thisType.enclosingClass->getEncodedName())));
			}


		protected:
			virtual string anonymousDeclarationToString(const ClassInfo& classinfo) const {
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


			virtual string declarationToString(const ClassInfo& classinfo) const {
				string str;

				if(thisType.isNested && !JDecompiler::getInstance().hasClass(thisType.enclosingClass->getEncodedName()))
					str += thisType.isAnonymous ? "/* anonymous class */\n" :
							"/* nested class of class " + thisType.enclosingClass->getName() + " */\n";

				if(modifiers & ACC_INTERFACE && superType != nullptr && *superType != javaLang::Object) {
					throw DecompilationException("interface " + thisType.getName() + " cannot inherit from other class than java.lang.Object");
				}

				str += classinfo.getIndent() + modifiersToString(modifiers) + ' ' + thisType.simpleName +
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


			virtual string fieldsToString(const ClassInfo& classinfo) const {
				string str;

				bool anyFieldStringified = false;

				for(const Field* field : fields) {
					if(field->canStringify(classinfo)) {
						str += '\n' + field->toString(classinfo) + ';';
						anyFieldStringified = true;
					}
				}

				if(anyFieldStringified)
					str += '\n';

				return str;
			}


			virtual string methodsToString(const ClassInfo& classinfo) const {
				string str;

				for(const Method* method : methods) {
					log("stringify of", method->descriptor.toString());
					if(method->canStringify(classinfo))
						str += '\n' + method->toString(classinfo) + '\n';
				}

				return str;
			}

			virtual string innerClassesToString(const ClassInfo& classinfo) const {
				string str;

				const NestMembersAttribute* nestMembers = attributes.get<NestMembersAttribute>();

				if(nestMembers != nullptr) {

					for(const ClassType* nestMember : nestMembers->nestMembers) {
						if(nestMember->isAnonymous)
							continue;

						const Class* nestClass = JDecompiler::getInstance().getClass(nestMember->getEncodedName());

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

			string modifiersToString(uint16_t modifiers) const {
				FormatString str;

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
					case ACC_FINAL | ACC_ENUM: case ACC_ABSTRACT | ACC_ENUM:
						str += "enum"; break;
					default:
						throw IllegalModifiersException("in class " + thisType.getName() + ": " + hexWithPrefix<4>(modifiers));
				}

				return (string)str;
			}


			virtual string headersToString(const ClassInfo& classinfo) const {
				if(thisType.isNested)
					return EMPTY_STRING;

				string headers = "/* Java version: " + to_string(version) + " */\n";

				if(!thisType.packageName.empty())
					headers += (string)classinfo.getIndent() + "package " + thisType.packageName + ";\n\n";

				headers += classinfo.importsToString();

				return headers;
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


	struct EnumClass final: Class {
		protected:
			struct EnumField: Field {
				const vector<const Operation*> arguments;

				EnumField(const Field& field, const vector<const Operation*>& arguments):
						Field(field), arguments(arguments.begin(), arguments.end() - 2) {}

				virtual string toString(const ClassInfo& classinfo) const override {
					string str;

					if(const AnnotationsAttribute* annotationsAttribute = attributes.get<AnnotationsAttribute>())
						str += annotationsAttribute->toString(classinfo);

					return str + descriptor.name + (arguments.empty() ? EMPTY_STRING :
							'(' + rjoin<const Operation*>(arguments, [this] (auto arg) { return arg->toString(*context); }) + ')');
				}
			};


			struct EnumConstructorDescriptor: MethodDescriptor {
				const vector<const Type*> factualArguments;

				EnumConstructorDescriptor(const MethodDescriptor& other):
						MethodDescriptor(other.clazz, other.name, other.returnType, other.arguments),
						factualArguments(other.arguments.begin() + 2, other.arguments.end()) {}

				virtual string toString(const StringifyContext& context) const override {
					return MethodDescriptor::toString(context, 2);
				}
			};

			vector<const EnumField*> enumFields;
			vector<const Field*> otherFields;

			static vector<MethodDataHolder> processMethodData(const vector<MethodDataHolder>& methodsData) {
				vector<MethodDataHolder> newMethodDataHolders;
				newMethodDataHolders.reserve(methodsData.size());

				for(const MethodDataHolder& methodData : methodsData) {
					newMethodDataHolders.push_back(MethodDataHolder(methodData.modifiers,
							methodData.descriptor.isConstructor() ?
							*new EnumConstructorDescriptor(methodData.descriptor) : methodData.descriptor, methodData.attributes));
				}

				return newMethodDataHolders;
			}

		public:
			EnumClass(const Version& version, const ClassType& thisType, const ClassType* superType, const ConstantPool& constPool, uint16_t modifiers,
					const vector<const ClassType*>& interfaces, const Attributes& attributes,
					const vector<FieldDataHolder>& fieldsData, vector<MethodDataHolder>& methodsData);


		protected:
			virtual string fieldsToString(const ClassInfo& classinfo) const override {
				string str;

				if(enumFields.size() > 0)
					str += (string)"\n" + classinfo.getIndent() +
							join<const EnumField*>(enumFields, [&classinfo] (auto field) { return field->toString(classinfo); }) + ";\n";

				bool anyFieldStringified = false;
				for(const Field* field : otherFields)
					if(field->canStringify(classinfo)) {
						str += (string)"\n" + field->toString(classinfo) + ';';
						anyFieldStringified = true;
					}
				if(anyFieldStringified)
					str += '\n';

				return str;
			}
	};




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
		for(uint16_t i = 0; i < interfacesCount; i++)
			interfaces.push_back(new ClassType(constPool.get<ClassConstant>(instream.readUShort())->name));


		const uint16_t fieldsCount = instream.readUShort();
		vector<FieldDataHolder> fieldsData;
		fieldsData.reserve(fieldsCount);

		for(uint16_t i = 0; i < fieldsCount; i++)
			fieldsData.push_back(FieldDataHolder(constPool, instream));


		const uint16_t methodsCount = instream.readUShort();
		vector<MethodDataHolder> methodsData;
		methodsData.reserve(methodsCount);

		for(uint16_t i = 0; i < methodsCount; i++)
			methodsData.push_back(MethodDataHolder(constPool, instream, thisType));

		const Attributes& attributes = *new Attributes(instream, constPool, instream.readUShort());

		return modifiers & ACC_ENUM ? new EnumClass(version, thisType, superType, constPool, modifiers, interfaces, attributes, fieldsData, methodsData) :
		                                  new Class(version, thisType, superType, constPool, modifiers, interfaces, attributes, fieldsData, methodsData);
	}
}

#endif
