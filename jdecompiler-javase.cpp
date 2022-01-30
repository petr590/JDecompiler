#ifndef JDECOMPILER_JAVASE_CPP
#define JDECOMPILER_JAVASE_CPP

#include "jdecompiler.h"

namespace JDecompiler {
	namespace JavaSE {
		namespace java {
			namespace lang {
				const ClassType
						&Byte = ClassType("java/lang/Byte"),
						&Character = ClassType("java/lang/Character"),
						&Short = ClassType("java/lang/Short"),
						&Integer = ClassType("java/lang/Integer"),
						&Long = ClassType("java/lang/Long"),
						&Float = ClassType("java/lang/Float"),
						&Double = ClassType("java/lang/Double"),
						&Boolean = ClassType("java/lang/Boolean");
			}
		}
	}
}

#endif
