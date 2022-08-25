#ifndef JDECOMPILER_JVM_H
#define JDECOMPILER_JVM_H

namespace jdecompiler {

	struct Class;

	struct JVM {

		virtual const Class* loadClass(const char*) const = 0;

		virtual ~JVM() {}
	};
}

#endif
