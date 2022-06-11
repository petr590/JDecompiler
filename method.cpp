#ifndef JDECOMPILER_METHOD_CPP
#define JDECOMPILER_METHOD_CPP

#include "code.cpp"
#include "attributes.cpp"

namespace jdecompiler {
	struct MethodDescriptor {
		public:
			const ReferenceType& clazz;
			const string name;
			const Type* const returnType;
			vector<const Type*> arguments;

		private:
			enum class MethodType {
				CONSTRUCTOR, STATIC_INITIALIZER, PLAIN
			};

			const MethodType type;

			static MethodType typeForName(const string& name) {
				if(name == "<init>") return MethodType::CONSTRUCTOR;
				if(name == "<clinit>") return MethodType::STATIC_INITIALIZER;
				return MethodType::PLAIN;
			}

		public:
			inline bool isConstructor() const {
				return type == MethodType::CONSTRUCTOR;
			}

			inline bool isStaticInitializer() const {
				return type == MethodType::STATIC_INITIALIZER;
			}

		public:
			MethodDescriptor(const ReferenceConstant* referenceConstant):
					MethodDescriptor(referenceConstant->clazz->name, referenceConstant->nameAndType) {}

			MethodDescriptor(const string& className, const NameAndTypeConstant* nameAndType):
					MethodDescriptor(className, nameAndType->name, nameAndType->descriptor) {}

			MethodDescriptor(const string& className, const string& name, const string& descriptor):
					MethodDescriptor(*parseReferenceType(className), name, descriptor) {}

			MethodDescriptor(const ReferenceType& clazz, const NameAndTypeConstant* nameAndType):
					MethodDescriptor(clazz, nameAndType->name, nameAndType->descriptor) {}

			MethodDescriptor(const ReferenceType& clazz, const string& name, const string& descriptor):
					clazz(clazz), name(name), returnType(nullptr), type(typeForName(name)) {
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
							*const_cast<const Type**>(&returnType) = parseReturnType(&descriptor[i + 1]);
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

			MethodDescriptor(const ReferenceType& clazz, const string& name, const Type& returnType):
					MethodDescriptor(clazz, name, &returnType) {}

			MethodDescriptor(const ReferenceType& clazz, const string& name, const Type* returnType):
					clazz(clazz), name(name), returnType(returnType), arguments(), type(typeForName(name)) {}

			MethodDescriptor(const ReferenceType& clazz, const string& name, const Type* returnType, const vector<const Type*>& arguments):
					clazz(clazz), name(name), returnType(returnType), arguments(arguments), type(typeForName(name)) {}


		protected:
			string toString(const StringifyContext& context, size_t argsStart) const {
				const bool isNonStatic = !(context.modifiers & ACC_STATIC);

				string str = isConstructor() ? context.classinfo.thisType.simpleName :
						returnType->toString(context.classinfo) + ' ' + name;

				uint32_t offset = argsStart + (uint32_t)isNonStatic;

				const auto getVarName = [&context, &offset] (const Type* type, size_t i) {
					return context.getCurrentScope()->getNameFor(
							context.methodScope.getVariable(i + (type->getSize() != TypeSize::EIGHT_BYTES ? offset : offset++), false));
				};


				function<string(const Type*, size_t)> concater;

				if(context.modifiers & ACC_VARARGS) {
					size_t varargsIndex = -1U;
					for(size_t i = arguments.size(); i > argsStart; ) {
						if(instanceof<const ArrayType*>(arguments[--i])) {
							varargsIndex = i;
							break;
						}
					}

					if(varargsIndex == -1U) {
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


				return str + '(' + join<const Type*>(argsStart == 0 ? arguments : vector(arguments.begin() + argsStart, arguments.end()), concater) + ')';
			}

		public:
			virtual string toString(const StringifyContext& context) const {
				return this->toString(context, 0);
			}

			string toString() const {
				return clazz.getName() + '.' + name + '(' + join<const Type*>(arguments, [] (auto arg) { return arg->getName(); }) + ')';
			}


			inline bool equalsIgnoreClass(const MethodDescriptor& other) const {
				return  this == &other || (this->name == other.name &&
						*this->returnType == *other.returnType &&
						typesEquals(this->arguments, other.arguments));
			}


			inline friend bool operator==(const MethodDescriptor& descriptor1, const MethodDescriptor& descriptor2) {
				return  &descriptor1 == &descriptor2 || (descriptor1.name == descriptor2.name && descriptor1.clazz == descriptor2.clazz &&
						*descriptor1.returnType == *descriptor2.returnType &&
						typesEquals(descriptor1.arguments, descriptor2.arguments));
			}


			inline friend bool operator!=(const MethodDescriptor& descriptor1, const MethodDescriptor& descriptor2) {
				return !(descriptor1 == descriptor2);
			}


			inline friend ostream& operator<<(ostream& out, const MethodDescriptor& descriptor) {
				return out << descriptor.toString();
			}
	};


	string MethodTypeConstant::toString(const ClassInfo& classinfo) const {
		const MethodDescriptor descriptor(classinfo.thisType, EMPTY_STRING, this->descriptor);

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

		private:
			inline bool isAutogenerated(const ClassInfo& classinfo) const {
				const function<bool()> hasNoOtherConstructors = [this, &classinfo] () {
					return !has_if<const Method*>(classinfo.getMethods(), [this] (const Method* method) {
						return method != this && method->descriptor.isConstructor();
					});
				};

				return (descriptor == MethodDescriptor(classinfo.thisType, "<init>", VOID, {}) && hasNoOtherConstructors() &&
							((scope.isEmpty() && (modifiers & ACC_ACCESS_FLAGS) == (classinfo.modifiers & ACC_ACCESS_FLAGS)) // constructor by default
							|| classinfo.thisType.isAnonymous)) // anonymous class constructor
						||

						(classinfo.modifiers & ACC_ENUM && (
							(scope.isEmpty() && modifiers & ACC_PRIVATE &&
								descriptor == MethodDescriptor(classinfo.thisType, "<init>", VOID, {STRING, INT}) // enum constructor by default
								&& hasNoOtherConstructors()) ||
							descriptor == MethodDescriptor(classinfo.thisType, "valueOf", &classinfo.thisType, {STRING}) || // Enum valueOf(String name)
							descriptor == MethodDescriptor(classinfo.thisType, "values", new ArrayType(classinfo.thisType), {}) // Enum[] values()
						));
			}

		public:
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

				if(JDecompiler::getInstance().useOverrideAnnotation()) {
					static const ClassType OVERRIDE_ANNOTATION("java/lang/Override");

					function<bool(const ClassInfo&)> checkOverride;

					const function<bool(const ClassType&)> checkClassHasMethod = [this, &classinfo, &str, &checkOverride] (const ClassType& classType) {
						const ClassInfo* otherClassInfo = JDecompiler::getInstance().getClassInfo(classType.getEncodedName());
						if(otherClassInfo != nullptr) {
							if(otherClassInfo->hasMethod(descriptor)) {
								str += classinfo.getIndent() + (string)"@" + OVERRIDE_ANNOTATION.toString(classinfo) + '\n';
								return true;
							} else {
								return checkOverride(*otherClassInfo);
							}
						}

						return false;
					};

					checkOverride = [&str, checkClassHasMethod] (const ClassInfo& otherClassInfo) {
						if(otherClassInfo.superType != nullptr) {
							if(checkClassHasMethod(*otherClassInfo.superType))
								return true;
						}

						for(const ClassType* interface : otherClassInfo.interfaces)
							if(checkClassHasMethod(*interface))
								return true;

						return false;
					};

					checkOverride(classinfo);
				}

				if(const AnnotationsAttribute* annotationsAttribute = attributes.get<AnnotationsAttribute>())
					str += annotationsAttribute->toString(classinfo);

				str += classinfo.getIndent();

				if(descriptor.isStaticInitializer()) {
					if(modifiers != ACC_STATIC)
						throw IllegalModifiersException(hexWithPrefix<4>(modifiers) + ": static initializer must have only static modifier");
					if(attributes.has<ExceptionsAttribute>())
						throw IllegalAttributeException("static initializer cannot have Exceptions attribute");
					str += "static";
				} else {
					str += modifiersToString(classinfo) + descriptor.toString(context);

					if(const ExceptionsAttribute* exceptionsAttr = attributes.get<ExceptionsAttribute>())
						str += " throws " + join<const ClassConstant*>(exceptionsAttr->exceptions,
								[&classinfo] (auto clazz) { return (new ClassType(clazz->name))->toString(classinfo); });

					if(const AnnotationDefaultAttribute* annotationDefaultAttr = attributes.get<AnnotationDefaultAttribute>())
						str += " default " + annotationDefaultAttr->toString(context.classinfo);
				}

				FormatString comment;

				if(modifiers & ACC_SYNTHETIC)
					comment += "synthetic";

				if(modifiers & ACC_BRIDGE)
					comment += "bridge";

				if(isAutogenerated(classinfo))
					comment += "autogenerated";

				if(!comment.empty())
					comment += "method";

				return str + (codeAttribute == nullptr ? ";" + (comment.empty() ? EMPTY_STRING : " // " + (string)comment) :
						(comment.empty() ? " " : " /* " + (string)comment + " */ ") + scope.toString(context));
						//&context.classinfo == &classinfo ? context : DecompilationContext(context, classinfo))); // For anonymous classes
			}

			virtual bool canStringify(const ClassInfo& classinfo) const override {

				return !((!JDecompiler::getInstance().showAutogenerated() && isAutogenerated(classinfo)) ||
						(descriptor.isStaticInitializer() && scope.isEmpty())) && // empty static {}

						(!(modifiers & (ACC_SYNTHETIC | ACC_BRIDGE)) ||
						(modifiers & ACC_SYNTHETIC && JDecompiler::getInstance().showSynthetic()) ||
						(modifiers & ACC_BRIDGE && JDecompiler::getInstance().showBridge()));
			}

			inline bool isStatic() const {
				return modifiers & ACC_STATIC;
			}

		private:
			FormatString modifiersToString(const ClassInfo& classinfo) const {
				FormatString str;

				switch(modifiers & ACC_ACCESS_FLAGS) {
					case ACC_VISIBLE: break;
					case ACC_PUBLIC: // All non-static interface methods have public modifier by default so we don't have to print public in this case
						//if(!(classinfo.modifiers & ACC_INTERFACE && !(modifiers & ACC_STATIC)))
							str += "public";
						break;
					case ACC_PRIVATE: // Enum constructors have private modifier by default so we don't have to print private in this case
						if(!(classinfo.modifiers & ACC_ENUM && descriptor.clazz == classinfo.thisType && descriptor.isConstructor()))
							str += "private";
						break;
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
					if(classinfo.modifiers & ACC_INTERFACE && !(modifiers & ACC_STATIC) && !(modifiers & ACC_PRIVATE))
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
			MethodDataHolder(const ConstantPool& constPool, ClassInputStream& instream, const ClassType& thisType):
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

			MethodDataHolder(const ConstantPool& constPool, ClassInputStream& instream, const ClassType& thisType):
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
