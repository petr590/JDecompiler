#ifndef JDECOMPILER_JAVASE_CPP
#define JDECOMPILER_JAVASE_CPP

namespace jdecompiler {

	namespace javaLang {
		static inline const ClassType
				Void("java/lang/Void"),
				Byte("java/lang/Byte"),
				Character("java/lang/Character"),
				Short("java/lang/Short"),
				Integer("java/lang/Integer"),
				Long("java/lang/Long"),
				Float("java/lang/Float"),
				Double("java/lang/Double"),
				Boolean("java/lang/Boolean"),

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
		static inline const ClassType Annotation("java/lang/annotation/Annotation");
	}
}

#endif
