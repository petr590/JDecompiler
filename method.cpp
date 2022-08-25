#ifndef JDECOMPILER_METHOD_CPP
#define JDECOMPILER_METHOD_CPP

#include "method.h"
#include "code.cpp"

namespace jdecompiler {

	inline MethodDescriptor::MethodType MethodDescriptor::typeForName(const string& name) {
		return name == "<init>" ? MethodType::CONSTRUCTOR :
				name == "<clinit>" ? MethodType::STATIC_INITIALIZER : MethodType::PLAIN;
	}


	string MethodDescriptor::toString(const StringifyContext& context, const Attributes& attributes) const {
		const bool isNonStatic = !(context.modifiers & ACC_STATIC);

		const MethodSignature* signature = attributes.has<MethodSignatureAttribute>() ? &attributes.get<MethodSignatureAttribute>()->signature : nullptr;

		string str = (signature == nullptr || signature->genericParameters.empty() ?
				EMPTY_STRING : '<' + join<const GenericParameter*>(signature->genericParameters,
					[&context] (const GenericParameter* parameter) { return parameter->toString(context.classinfo); }) + "> ") +
				(isConstructor() ? context.classinfo.thisType.simpleName :
				(signature == nullptr ? returnType : signature->returnType)->toString(context.classinfo) + ' ' + name);


		uint32_t offset = static_cast<uint32_t>(isNonStatic);

		const auto getVarName = [&context, &offset] (const Type* type, size_t i) {
			return context.getCurrentScope()->getNameFor(
					context.methodScope.getVariable(i + (type->getSize() == TypeSize::EIGHT_BYTES ? offset++ : offset), false));
		};

		const vector<const Type*>& arguments = signature != nullptr ? signature->arguments : this->arguments;

		const ParameterAnnotationsAttribute* parameterAnnotations = attributes.get<ParameterAnnotationsAttribute>();
		const function<string(size_t)> parameterAnnotationToString = parameterAnnotations != nullptr ?
			[parameterAnnotations, &context] (size_t i) { return parameterAnnotations->parameterAnnotationsToString(i, context.classinfo); } :
			static_cast<function<string(size_t)>>([] (size_t) { return EMPTY_STRING; });


		function<string(const Type*, size_t)> concater;

		if(context.modifiers & ACC_VARARGS) {
			size_t varargsIndex = arguments.size() - 1;

			if(arguments.size() == 0 || !instanceof<const ArrayType*>(arguments.back())) {
				throw IllegalMethodHeaderException("Varargs method " + this->toString() + " must have last argument as array");
			}

			concater = [&context, &parameterAnnotationToString, &getVarName, varargsIndex] (const Type* type, size_t i) {
				return parameterAnnotationToString(i) + (i == varargsIndex ?
					safe_cast<const ArrayType*>(type)->elementType->toString(context.classinfo) +
					"... " + getVarName(type, i) : variableDeclarationToString(type, context.classinfo, getVarName(type, i)));
			};

		} else {
			concater = [&context, &parameterAnnotationToString, &getVarName] (const Type* type, size_t i) {
				return parameterAnnotationToString(i) + variableDeclarationToString(type, context.classinfo, getVarName(type, i));
			};
		}


		return str + '(' + join<const Type*>(arguments, concater) + ')';
	}


	string MethodTypeConstant::toString(const ClassInfo& classinfo) const {
		const MethodDescriptor descriptor(classinfo.thisType, EMPTY_STRING, this->descriptor);

		return METHOD_TYPE->toString(classinfo) + ".methodType(" + descriptor.returnType->toString(classinfo) + ".class" +
				join<const Type*>(descriptor.arguments, [&classinfo] (auto type) { return ", " + type->toString(classinfo) + ".class"; }, EMPTY_STRING) + ')';
	}


	inline bool Method::isAutogenerated(const ClassInfo& classinfo) const {
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

	Method::Method(modifiers_t modifiers, const MethodDescriptor& descriptor, const Attributes& attributes, const ClassInfo& classinfo):
			ClassElement(modifiers), descriptor(descriptor), attributes(attributes), codeAttribute(attributes.get<CodeAttribute>()),
			context(decompileCode(classinfo)), scope(context.methodScope) {

		if(descriptor.isStaticInitializer()) {
			if(modifiers != ACC_STATIC)
				throw IllegalModifiersException(hexWithPrefix<4>(modifiers) + ": static initializer must have only static modifier");
			if(attributes.has<ExceptionsAttribute>())
				throw IllegalAttributeException("Static initializer cannot have Exceptions attribute");
		}
	}

	string Method::toString(const ClassInfo& classinfo) const {

		if(codeAttribute != nullptr) {
			if(modifiers & ACC_ABSTRACT)
				throw IllegalMethodHeaderException(descriptor.toString() + ": Abstract method cannot have Code attribute");
			if(modifiers & ACC_NATIVE)
				throw IllegalMethodHeaderException(descriptor.toString() + ": Native method cannot have Code attribute");
		} else {
			if(!(modifiers & (ACC_ABSTRACT | ACC_NATIVE)))
				throw IllegalMethodHeaderException(descriptor.toString() + ": Non-abstract and non-native method must have Code attribute");
		}


		string str;

		if(JDecompiler::getInstance().useOverrideAnnotation()) {
			static const ClassType OVERRIDE_ANNOTATION("java/lang/Override");

			function<bool(const ClassInfo&)> checkOverride;

			const function<bool(const ClassType&)> checkClassHasMethod = [this, &classinfo, &str, &checkOverride] (const ClassType& classType) {
				const ClassInfo* otherClassInfo = JDecompiler::getInstance().getClassInfo(classType.getClassEncodedName());
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
			str += annotationsAttribute->toString(classinfo) + '\n';

		if(!errorMessage.empty()) {
			str += classinfo.getIndent() + (string)"// Exception while decompiling method: " + errorMessage + '\n';
		}

		str += classinfo.getIndent();

		if(descriptor.isStaticInitializer()) {
			str += "static";
		} else {
			str += modifiersToString(classinfo) + descriptor.toString(context, attributes);

			if(const ExceptionsAttribute* exceptionsAttr = attributes.get<ExceptionsAttribute>())
				str += " throws " + join<const ClassConstant*>(exceptionsAttr->exceptions,
						[&classinfo] (auto clazz) { return (new ClassType(clazz->name))->toString(classinfo); });

			if(const AnnotationDefaultAttribute* annotationDefaultAttr = attributes.get<AnnotationDefaultAttribute>())
				str += " default " + annotationDefaultAttr->toString(context.classinfo);
		}

		format_string comment;

		if(modifiers & ACC_SYNTHETIC)
			comment += "synthetic";

		if(modifiers & ACC_BRIDGE)
			comment += "bridge";

		if(isAutogenerated(classinfo))
			comment += "autogenerated";

		if(!comment.empty())
			comment += "method";

		return str + (codeAttribute == nullptr || !errorMessage.empty() ? (comment.empty() ? ";" : "; // " + (string)comment) :
				(comment.empty() ? " " : " /* " + (string)comment + " */ ") + scope.toString(context));
				//&context.classinfo == &classinfo ? context : DecompilationContext(context, classinfo))); // For anonymous classes
	}

	bool Method::canStringify(const ClassInfo& classinfo) const {

		return !((!JDecompiler::getInstance().showAutogenerated() && isAutogenerated(classinfo)) ||
				(descriptor.isStaticInitializer() && scope.isEmpty())) && // empty static {}

				(!(modifiers & (ACC_SYNTHETIC | ACC_BRIDGE)) ||
				(modifiers & ACC_SYNTHETIC && JDecompiler::getInstance().showSynthetic()) ||
				(modifiers & ACC_BRIDGE && JDecompiler::getInstance().showBridge()));
	}

	format_string Method::modifiersToString(const ClassInfo& classinfo) const {
		format_string str;

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
}

#endif
