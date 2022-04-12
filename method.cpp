#ifndef JDECOMPILER_METHOD_CPP
#define JDECOMPILER_METHOD_CPP

#include "code.cpp"
#include "attributes.cpp"

namespace jdecompiler {
	struct MethodDescriptor {
		public:
			const ReferenceType& clazz;
			const string name;
			const Type* returnType;
			vector<const Type*> arguments;

			enum class MethodType {
				CONSTRUCTOR, STATIC_INITIALIZER, PLAIN
			};

			const MethodType type;

		private: static MethodType typeForName(const string& name) {
			if(name == "<init>") return MethodType::CONSTRUCTOR;
			if(name == "<clinit>") return MethodType::STATIC_INITIALIZER;
			return MethodType::PLAIN;
		}

		public:
			MethodDescriptor(const ReferenceConstant* referenceConstant): MethodDescriptor(*referenceConstant->clazz->name,
					*referenceConstant->nameAndType->name, *referenceConstant->nameAndType->descriptor) {}

			MethodDescriptor(const string& className, const string& name, const string& descriptor):
					MethodDescriptor(*parseReferenceType(className), name, descriptor) {}

			MethodDescriptor(const ReferenceType& clazz, const string& name, const string& descriptor):
					clazz(clazz), name(name), type(typeForName(name)) {
				if(descriptor[0] != '(')
					throw IllegalMethodDescriptorException(descriptor);

				const uint32_t descriptorLength = descriptor.size();

				for(uint32_t i = 1; i < descriptorLength;) {
					const Type* argument;
					switch(descriptor[i]) {
						case 'B': argument = BYTE; break;
						case 'C': argument = CHAR; break;
						case 'S': argument = SHORT; break;
						case 'I': argument = INT; break;
						case 'J': argument = LONG; break;
						case 'F': argument = FLOAT; break;
						case 'D': argument = DOUBLE; break;
						case 'Z': argument = BOOLEAN; break;
						case 'L':
							argument = new ClassType(&descriptor[i + 1]);
							i += argument->getEncodedName().size() + 1;
							goto PushArgument;
						case '[':
							argument = new ArrayType(&descriptor[i]);
							i += argument->getEncodedName().size();
							if(((const ArrayType*)argument)->memberType->isPrimitive())
								goto PushArgument;
							break;
						case ')':
							returnType = parseReturnType(&descriptor[i + 1]);
							goto End;
						default:
							throw InvalidTypeNameException(descriptor);
					}
					i++;
					PushArgument:
					arguments.push_back(argument);
				}

				End:;
			}

			MethodDescriptor(const ReferenceType& clazz, const string& name, const Type& returnType, const initializer_list<const Type*> arguments):
					MethodDescriptor(clazz, name, &returnType, arguments) {}

			MethodDescriptor(const ReferenceType& clazz, const string& name, const Type* returnType, const initializer_list<const Type*> arguments):
					MethodDescriptor(clazz, name, returnType, vector<const Type*>(arguments)) {}

			MethodDescriptor(const ReferenceType& clazz, const string& name, const Type* returnType):
					MethodDescriptor(clazz, name, returnType, vector<const Type*>()) {}

			MethodDescriptor(const ReferenceType& clazz, const string& name, const Type* returnType, const vector<const Type*>& arguments):
					clazz(clazz), name(name), returnType(returnType), arguments(arguments), type(typeForName(name)) {}


			string toString(const StringifyContext& context) const {
				const bool isNonStatic = !(context.modifiers & ACC_STATIC);

				string str = type == MethodType::CONSTRUCTOR ? context.classinfo.thisType.simpleName :
						returnType->toString(context.classinfo) + ' ' + name;

				uint32_t offset = (uint32_t)isNonStatic;

				const auto getVarName = [&context, &offset] (const Type* type, size_t i) {
					return context.getCurrentScope()->getNameFor(
							context.methodScope.getVariable(i + (type->getSize() != TypeSize::EIGHT_BYTES ? offset : offset++), false));
				};


				function<string(const Type*, size_t)> concater;

				if(context.modifiers & ACC_VARARGS) {
					size_t varargsIndex = -1;
					for(size_t i = arguments.size(); i > 0; ) {
						if(instanceof<const ArrayType*>(arguments[--i])) {
							varargsIndex = i;
							break;
						}
					}

					if(varargsIndex == -1) {
						throw IllegalMethodHeaderException("Varargs method " + this->toString() + " must have at least one array argument");
					}

					concater = [&context, &getVarName, varargsIndex] (const Type* type, size_t i) {
						return (i == varargsIndex ? safe_cast<const ArrayType*>(type)->elementType->toString(context.classinfo) + "..." :
							type->toString(context.classinfo)) + ' ' + getVarName(type, i);
					};
				} else {
					concater = [&context, &getVarName] (const Type* type, size_t i) {
						return type->toString(context.classinfo) + ' ' + getVarName(type, i);
					};
				}


				return str + '(' + join<const Type*>(arguments, concater) + ')';
			}

			string toString() const {
				return clazz.getName() + '.' + name + '(' + join<const Type*>(arguments, [] (auto arg) { return arg->getName(); }) + ')';
			}


			bool operator==(const MethodDescriptor& other) const {
				return   this == &other || (this->name == other.name && this->clazz == other.clazz &&
						*this->returnType == *other.returnType &&
						 this->arguments.size() == other.arguments.size() &&
						 equal(this->arguments.begin(), this->arguments.end(), other.arguments.begin(),
								[] (auto arg1, auto arg2) { return *arg1 == *arg2; }));
			}
	};


	string MethodTypeConstant::toString(const ClassInfo& classinfo) const {
		const MethodDescriptor descriptor(classinfo.thisType, EMPTY_STRING, *this->descriptor);

		return METHOD_TYPE->toString(classinfo) + ".methodType(" + descriptor.returnType->toString(classinfo) + ".class" +
				join<const Type*>(descriptor.arguments, [&classinfo] (auto type) { return ", " + type->toString(classinfo) + ".class"; }, EMPTY_STRING) + ')';
	}


	struct Method: ClassElement {
		public:
			const uint16_t modifiers;
			const MethodDescriptor& descriptor;
			const Attributes& attributes;
			const CodeAttribute* const codeAttribute;
			const StringifyContext& context;

		protected:
			MethodScope& scope;

		public:
			typedef MethodDescriptor::MethodType MethodType;

			Method(uint16_t modifiers, const MethodDescriptor& descriptor, const Attributes& attributes, const ClassInfo& classinfo):
					modifiers(modifiers), descriptor(descriptor), attributes(attributes), codeAttribute(attributes.get<CodeAttribute>()),
					context(decompileCode(classinfo)), scope(context.methodScope) {

				const bool hasCodeAttribute = codeAttribute != nullptr;

				if(modifiers & ACC_ABSTRACT && hasCodeAttribute)
					throw IllegalMethodHeaderException(descriptor.toString() + ": Abstract method cannot have Code attribute");
				if(modifiers & ACC_NATIVE && hasCodeAttribute)
					throw IllegalMethodHeaderException(descriptor.toString() + ": Native method cannot have Code attribute");
				if(!(modifiers & (ACC_ABSTRACT | ACC_NATIVE)) && !hasCodeAttribute)
					throw IllegalMethodHeaderException(descriptor.toString() + ": Non-abstract and non-native method must have Code attribute");
			}

			const StringifyContext& decompileCode(const ClassInfo& classinfo);

			virtual string toString(const ClassInfo& classinfo) const override {
				string str;

				if(const AnnotationsAttribute* annotationsAttribute = attributes.get<AnnotationsAttribute>())
					str += annotationsAttribute->toString(classinfo);

				str += classinfo.getIndent();

				if(descriptor.type == MethodType::STATIC_INITIALIZER) {
					if(modifiers != ACC_STATIC)
						throw IllegalModifiersException(hexWithPrefix<4>(modifiers) + ": static initializer must have only static modifier");
					if(attributes.has<ExceptionsAttribute>())
						throw IllegalAttributeException("static initializer cannot have Exceptions attribute");
					str += "static";
				} else {
					str += modifiersToString(modifiers, classinfo) + descriptor.toString(context);

					if(const ExceptionsAttribute* exceptionsAttr = attributes.get<ExceptionsAttribute>())
						str += " throws " + join<const ClassConstant*>(exceptionsAttr->exceptions,
								[&classinfo] (auto clazz) { return (new ClassType(*clazz->name))->toString(classinfo); });

					if(const AnnotationDefaultAttribute* annotationDefaultAttr = attributes.get<AnnotationDefaultAttribute>())
						str += " default " + annotationDefaultAttr->toString(context.classinfo);
				}

				return str + (codeAttribute == nullptr ? ";" : ' ' + scope.toString(context));
						//&context.classinfo == &classinfo ? context : DecompilationContext(context, classinfo))); // For anonymous classes
			}

			virtual bool canStringify(const ClassInfo& classinfo) const override {
				return !((modifiers & (ACC_SYNTHETIC | ACC_BRIDGE)) ||
						(descriptor.type == MethodType::STATIC_INITIALIZER && scope.isEmpty()) || // empty static {}

						(descriptor.type == MethodType::CONSTRUCTOR &&
							(scope.isEmpty() && ((modifiers & ACC_PUBLIC) == ACC_PUBLIC ||
								(modifiers & ACC_PUBLIC) == (classinfo.modifiers & ACC_PUBLIC)) // constructor by default
							|| classinfo.thisType.isAnonymous)) // anonymous class constructor
						||

						(classinfo.modifiers & ACC_ENUM && (
							descriptor == MethodDescriptor(classinfo.thisType, "valueOf", &classinfo.thisType, {STRING}) || // Enum valueOf(String name)
							descriptor == MethodDescriptor(classinfo.thisType, "values", new ArrayType(classinfo.thisType), {}) || // Enum[] values()
							(modifiers & ACC_PRIVATE && descriptor == MethodDescriptor(classinfo.thisType, "<init>", VOID, {})) // enum constructor by default
						)));
			}

		private:
			static FormatString modifiersToString(uint16_t modifiers, const ClassInfo& classinfo) {
				FormatString str;

				switch(modifiers & (ACC_VISIBLE | ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED)) {
					case ACC_VISIBLE: break;
					case ACC_PUBLIC: str += "public"; break;
					case ACC_PRIVATE: str += "private"; break;
					case ACC_PROTECTED: str += "protected"; break;
					default: throw IllegalModifiersException(modifiers);
				}

				if(modifiers & ACC_STATIC) str += "static";

				if(modifiers & ACC_ABSTRACT) {
					if(modifiers & (ACC_STATIC | ACC_FINAL | ACC_SYNCHRONIZED | ACC_NATIVE | ACC_STRICT))
						throw IllegalModifiersException(modifiers);
					if(!(classinfo.modifiers & ACC_INTERFACE))
						str += "abstract";
				} else {
					if(classinfo.modifiers & ACC_INTERFACE)
						str += "default";
				}

				if(modifiers & ACC_FINAL) str += "final";
				if(modifiers & ACC_SYNCHRONIZED) str += "synchronized";
				if(modifiers & ACC_NATIVE && modifiers & ACC_STRICT) throw IllegalModifiersException(modifiers);
				if(modifiers & ACC_NATIVE) str += "native";
				if(modifiers & ACC_STRICT) str += "strictfp";

				return str;
			}
	};



	struct MethodDataHolder {
		public:
			const uint16_t modifiers;
			const MethodDescriptor& descriptor;
			const Attributes& attributes;

		#if 1
			MethodDataHolder(const ConstantPool& constPool, BinaryInputStream& instream, const ClassType& thisType):
					modifiers(instream.readUShort()), descriptor(
						*new MethodDescriptor(thisType, constPool.getUtf8Constant(instream.readUShort()), constPool.getUtf8Constant(instream.readUShort()))),
					attributes(*new Attributes(instream, constPool, instream.readUShort())) {}

			MethodDataHolder(uint16_t modifiers, const MethodDescriptor& descriptor, const Attributes& attributes):
					modifiers(modifiers), descriptor(descriptor), attributes(attributes) {}

			const Method* createMethod(const ClassInfo& classinfo) const {
				return new Method(modifiers, descriptor, attributes, classinfo);
			}

		#else // Когда я включаю эту чать блока, появляется странный баг при декомпиляции enum-ов

		private:
			function<const Method*(const ClassInfo&)> defaultMethodCreator() {
				return [this] (const ClassInfo& classinfo) {
					return new Method(modifiers, descriptor, attributes, classinfo);
				};
			}

		public:
			const function<const Method*(const ClassInfo&)> createMethod;

			MethodDataHolder(const ConstantPool& constPool, BinaryInputStream& instream, const ClassType& thisType):
					modifiers(instream.readUShort()), descriptor(
						*new MethodDescriptor(thisType, constPool.getUtf8Constant(instream.readUShort()), constPool.getUtf8Constant(instream.readUShort()))),
					attributes(*new Attributes(instream, constPool, instream.readUShort())), createMethod(defaultMethodCreator()) {}

			MethodDataHolder(uint16_t modifiers, const MethodDescriptor& descriptor, const Attributes& attributes,
					const function<const Method*(const ClassInfo&)>& methodCreator):
					modifiers(modifiers), descriptor(descriptor), attributes(attributes), createMethod(methodCreator) {}

			MethodDataHolder(uint16_t modifiers, const MethodDescriptor& descriptor, const Attributes& attributes):
					MethodDataHolder(modifiers, descriptor, attributes, defaultMethodCreator()) {}
		#endif
	};
}

#endif
