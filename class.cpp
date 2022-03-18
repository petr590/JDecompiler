#ifndef JDECOMPILER_CLASS_CPP
#define JDECOMPILER_CLASS_CPP

#include "field.cpp"
#include "method.cpp"

namespace jdecompiler {
	struct Version {
		const uint16_t majorVersion, minorVersion;

		inline Version(uint16_t majorVersion, uint16_t minorVersion): majorVersion(majorVersion), minorVersion(minorVersion) {}
	};

	struct Class: Stringified {
		public:
			const Version version;
			const ClassType & thisType, & superType;
			const ConstantPool& constPool;
			const uint16_t modifiers;
			const vector<const ClassType*> interfaces;
			const Attributes& attributes;
			const ClassInfo& classinfo;
			const vector<const Field*> fields;
			const vector<const Method*> methods;

		protected:
			Class(const Version& version, const ClassType& thisType, const ClassType& superType, const ConstantPool& constPool, uint16_t modifiers,
					const vector<const ClassType*>& interfaces, const Attributes& attributes, const ClassInfo& classinfo,
					const vector<const Field*>& fields, const vector<const Method*>& methods):
					version(version), thisType(thisType), superType(superType), constPool(constPool), modifiers(modifiers),
					interfaces(interfaces), attributes(attributes), classinfo(classinfo),
					fields(fields), methods(methods) {}

			const vector<const Method*> createMethodsFromMethodData(const vector<MethodDataHolder> methodDataHolders) const {
				vector<const Method*> methods;
				methods.reserve(methodDataHolders.size());
				for(const MethodDataHolder methodData : methodDataHolders) {
					try {
						methods.push_back(methodData.createMethod(classinfo));
					} catch(DecompilationException& ex) {
						const char* message = ex.what();
						cerr << "Exception while decompiling method " + methodData.descriptor.toString() << ": "
								<< typeid(ex).name() << (*message == '\0' ? EMPTY_STRING : (string)": " + message) << endl;
						if(JDecompiler::getInstance().isFailOnError())
							throw;
					}
				}
				return methods;
			}

			Class(const Version& version, const ClassType& thisType, const ClassType& superType, const ConstantPool& constPool, uint16_t modifiers,
					const vector<const ClassType*>& interfaces, const Attributes& attributes,
					const vector<const Field*>& fields, const vector<MethodDataHolder>& methodDataHolders):
					version(version), thisType(thisType), superType(superType), constPool(constPool), modifiers(modifiers),
					interfaces(interfaces), attributes(attributes),
					classinfo(*new ClassInfo(*this, thisType, superType, constPool, attributes, modifiers)),
					fields(fields), methods(createMethodsFromMethodData(methodDataHolders)) {}


		public:
			static const Class* readClass(BinaryInputStream& instream);

			const Field* getField(const string& name) const {
				for(const Field* field : fields)
					if(field->descriptor.name == name)
						return field;
				return nullptr;
			}


			/*const Method* getMethod(const MethodDescriptor& descriptor, bool isStatic) const {
				for(const Method* method : methods)
					if(method->descriptor == descriptor && (bool)(method->modifiers & ACC_STATIC) == isStatic)
						return field;
				return nullptr;
			}*/


			virtual string toString(const ClassInfo& classinfo) const override {
				string str;

				if(const AnnotationsAttribute* annotationsAttribute = attributes.get<AnnotationsAttribute>())
					str += annotationsAttribute->toString(classinfo) + '\n';

				str += declarationToString(classinfo) + " {";

				const size_t baseSize = str.size();

				classinfo.increaseIndent();

				str += fieldsToString(classinfo);

				str += methodsToString(classinfo);

				classinfo.reduceIndent();

				str += (str.size() == baseSize ? "}" : (str.back() == '\n' ? EMPTY_STRING : "\n") + classinfo.getIndent() + '}');


				return headersToString(classinfo) + str;
			}

			inline string toString() const {
				return toString(classinfo);
			}


			inline bool canStringify() const {
				return !thisType.isAnonymous;
			}


		protected:
			virtual string declarationToString(const ClassInfo& classinfo) const {
				string str = classinfo.getIndent() + modifiersToString(modifiers) + ' ' + thisType.simpleName +
						(superType.getName() == "java.lang.Object" || (modifiers & ACC_ENUM && superType.getName() == "java.lang.Enum") ?
								EMPTY_STRING : " extends " + superType.toString(classinfo));

				vector<const ClassType*> interfacesToStringify;
				for(const ClassType* interface : interfaces) {
					if(modifiers & ACC_ANNOTATION && interface->getName() == "java.lang.annotation.Annotation")
						continue;
					interfacesToStringify.push_back(interface);
				}

				if(!interfacesToStringify.empty())
					str += " implements " +
							join<const ClassType*>(interfacesToStringify, [&classinfo] (auto interface) { return interface->toString(classinfo); });

				return str;
			}

			virtual string fieldsToString(const ClassInfo& classinfo) const {
				string str;

				bool anyFieldStringified = false;

				for(const Field* field : fields) {
					if(field->canStringify(classinfo)) {
						str += (string)"\n" + field->toString(classinfo) + ';';
						anyFieldStringified = true;
					}
				}

				if(anyFieldStringified)
					str += '\n';

				return str;
			}

			virtual string methodsToString(const ClassInfo& classinfo) const {
				string str;

				for(const Method* method : methods)
					if(method->canStringify(classinfo))
						str += (string)"\n" + method->toString(classinfo) + '\n';

				return str;
			}

			string modifiersToString(uint16_t modifiers) const {
				FormatString str;

				switch(modifiers & (ACC_VISIBLE | ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED)) {
					case ACC_VISIBLE: break;
					case ACC_PUBLIC: str += "public"; break;
					case ACC_PRIVATE: str += "private"; break;
					case ACC_PROTECTED: str += "protected"; break;
					default: throw IllegalModifiersException("in class " + thisType.getName() + ": " + hexWithPrefix<4>(modifiers));
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
				string headers = "/* Java version: " + to_string(version.majorVersion) + '.' + to_string(version.minorVersion) + " */\n";

				if(!thisType.packageName.empty())
					headers += (string)classinfo.getIndent() + "package " + thisType.packageName + ";\n\n";

				headers += classinfo.importsToString();

				return headers;
			}
	};


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
							'(' + rjoin<const Operation*>(arguments, [this] (auto arg) { return arg->toString(*environment); }) + ')');
				}
			};


			struct EnumConstructorDescriptor: MethodDescriptor {
				EnumConstructorDescriptor(const MethodDescriptor& other):
						MethodDescriptor(other.clazz, other.name, other.returnType, vector<const Type*>(other.arguments.begin() + 2, other.arguments.end())) {}
			};

			vector<const EnumField*> enumFields;
			vector<const Field*> otherFields;

			static vector<MethodDataHolder> processMethodData(vector<MethodDataHolder>& methodDataHolders) {
				vector<MethodDataHolder> newMethodDataHolders;
				newMethodDataHolders.reserve(methodDataHolders.size());

				for(MethodDataHolder& methodData : methodDataHolders) {
					newMethodDataHolders.push_back(MethodDataHolder(methodData.modifiers,
							methodData.descriptor.type == MethodDescriptor::MethodType::CONSTRUCTOR ?
							*new EnumConstructorDescriptor(methodData.descriptor) : methodData.descriptor, methodData.attributes));
				}

				return newMethodDataHolders;
			}

		public:
			EnumClass(const Version& version, const ClassType& thisType, const ClassType& superType, const ConstantPool& constPool, uint16_t modifiers,
					const vector<const ClassType*>& interfaces, const Attributes& attributes,
					const vector<const Field*>& fields, vector<MethodDataHolder>& methodDataHolders);


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



	struct AnonymousClass: Class {
		public:
			AnonymousClass(const Version& version, const ClassType& thisType, const ClassType& superType, const ConstantPool& constPool, uint16_t modifiers,
					const vector<const ClassType*>& interfaces, const Attributes& attributes,
					const vector<const Field*>& fields, vector<MethodDataHolder>& methodDataHolders):
					Class(version, thisType, superType, constPool, modifiers, interfaces, attributes, fields, methodDataHolders) {}



			virtual string declarationToString(const ClassInfo& classinfo) const override {

				if(interfaces.size() > 1)
					throw DecompilationException("Anonymous class " + thisType.getName() + " cannot implement more than one interface");

				if(interfaces.size() == 1) {
					if(superType.getName() != "java.lang.Object")
						throw DecompilationException("Anonymous class " + thisType.getName() +
								" cannot implement an interface and simultaneously inherit from something other than java.lang.Object class");

					return interfaces[0]->toString(classinfo) + "()";
				}

				return superType.toString(classinfo) + "()";
			}

			virtual string headersToString(const ClassInfo& classinfo) const override {
				return EMPTY_STRING;
			}
	};




	const Class* Class::readClass(BinaryInputStream& instream) {
		if(instream.readUInt() != CLASS_SIGNATURE)
			throw ClassFormatError("Wrong class signature");

		const uint16_t
				minorVersion = instream.readUShort(),
				majorVersion = instream.readUShort();

		const Version version(majorVersion, minorVersion);

		const uint16_t constPoolSize = instream.readUShort();

		const ConstantPool& constPool = *new ConstantPool(constPoolSize);

		for(uint16_t i = 1; i < constPoolSize; i++) {
			uint8_t constType = instream.readUByte();

			switch(constType) {
				case  1: {
					uint16_t size = instream.readUShort();
					const char* bytes = instream.readString(size);
					constPool[i] = new Utf8Constant(bytes, size);
					delete[] bytes;
					break;
				}
				case  3:
					constPool[i] = new IntegerConstant(instream.readInt());
					break;
				case  4:
					constPool[i] = new FloatConstant(instream.readFloat());
					break;
				case  5:
					constPool[i] = new LongConstant(instream.readLong());
					i++; // Long and Double constants have historically held two positions in the pool
					break;
				case  6:
					constPool[i] = new DoubleConstant(instream.readDouble());
					i++;
					break;
				case  7:
					constPool[i] = new ClassConstant(instream.readUShort());
					break;
				case  8:
					constPool[i] = new StringConstant(instream.readUShort());
					break;
				case  9:
					constPool[i] = new FieldrefConstant(instream.readUShort(), instream.readUShort());
					break;
				case 10:
					constPool[i] = new MethodrefConstant(instream.readUShort(), instream.readUShort());
					break;
				case 11:
					constPool[i] = new InterfaceMethodrefConstant(instream.readUShort(), instream.readUShort());
					break;
				case 12:
					constPool[i] = new NameAndTypeConstant(instream.readUShort(), instream.readUShort());
					break;
				case 15:
					constPool[i] = new MethodHandleConstant(instream.readUByte(), instream.readUShort());
					break;
				case 16:
					constPool[i] = new MethodTypeConstant(instream.readUShort());
					break;
				case 18:
					constPool[i] = new InvokeDynamicConstant(instream.readUShort(), instream.readUShort());
					break;
				default:
					throw ClassFormatError("Illegal constant type " + hexWithPrefix<2>(constType) + " at index #" + to_string(i) +
							" at pos " + hexWithPrefix((uint32_t)instream.getPos()));
			};
		}

		for(uint16_t i = 1; i < constPoolSize; i++) {
			Constant* constant = constPool[i];
			if(constant != nullptr)
				constant->init(constPool);
		}

		const uint16_t modifiers = instream.readUShort();

		const ClassType
				& thisType = *new ClassType(*constPool.get<ClassConstant>(instream.readUShort())->name),
				& superType = *new ClassType(*constPool.get<ClassConstant>(instream.readUShort())->name);

		const uint16_t interfacesCount = instream.readUShort();
		vector<const ClassType*> interfaces;
		interfaces.reserve(interfacesCount);
		for(uint16_t i = 0; i < interfacesCount; i++)
			interfaces.push_back(new ClassType(*constPool.get<ClassConstant>(instream.readUShort())->name));


		const uint16_t fieldsCount = instream.readUShort();
		vector<const Field*> fields;
		fields.reserve(fieldsCount);

		for(uint16_t i = 0; i < fieldsCount; i++)
			fields.push_back(new Field(constPool, instream));


		const uint16_t methodsCount = instream.readUShort();
		vector<MethodDataHolder> methodDataHolders;
		methodDataHolders.reserve(methodsCount);

		for(uint16_t i = 0; i < methodsCount; i++)
			methodDataHolders.push_back(MethodDataHolder(constPool, instream, thisType));

		const Attributes& attributes = *new Attributes(instream, constPool, instream.readUShort());

		return modifiers & ACC_ENUM ?      new EnumClass(version, thisType, superType, constPool, modifiers, interfaces, attributes, fields, methodDataHolders) :
		       thisType.isAnonymous ? new AnonymousClass(version, thisType, superType, constPool, modifiers, interfaces, attributes, fields, methodDataHolders) :
		                                       new Class(version, thisType, superType, constPool, modifiers, interfaces, attributes, fields, methodDataHolders);
	}
}

#endif
