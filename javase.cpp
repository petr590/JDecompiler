#ifndef JDECOMPILER_JAVASE_CPP
#define JDECOMPILER_JAVASE_CPP

namespace jdecompiler {

	namespace javaLang {
		static const ClassType
				&Void = ClassType("java/lang/Void"),
				&Byte = ClassType("java/lang/Byte"),
				&Character = ClassType("java/lang/Character"),
				&Short = ClassType("java/lang/Short"),
				&Integer = ClassType("java/lang/Integer"),
				&Long = ClassType("java/lang/Long"),
				&Float = ClassType("java/lang/Float"),
				&Double = ClassType("java/lang/Double"),
				&Boolean = ClassType("java/lang/Boolean"),

				&Object = *OBJECT,
				&String = *STRING,
				&Class = *CLASS,
				&Enum = *ENUM,
				&Throwable = *THROWABLE,
				&Exception = *EXCEPTION,
				&MethodType = *METHOD_TYPE,
				&MethodHandle = *METHOD_HANDLE;
	}

	namespace javaLangAnnotation {
		static const ClassType& Annotation = ClassType("java/lang/annotation/Annotation");
	}
}

#endif
