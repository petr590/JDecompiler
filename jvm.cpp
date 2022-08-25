#ifndef JDECOMPILER_JVM_CPP
#define JDECOMPILER_JVM_CPP

#include <jni.h>
#include <iostream>

//#include "class.cpp"
#include "jvm.h"

namespace jdecompiler {

	using std::cout;
	using std::endl;

	struct JVMImpl: JVM {

		public:
			JavaVM* const vm;
			JNIEnv* const env;
			const jclass class_Class, class_System;

			JVMImpl(JavaVM* vm, JNIEnv* env):
					vm(vm), env(env), class_Class(env->FindClass("java/lang/Class")), class_System(env->FindClass("java/lang/System")) {}

			JVMImpl(const JVMImpl&) = delete;

		private:
			void println(jobject obj) const {
				static jclass class_PrintStream = env->FindClass("java/io/PrintStream");
				static jmethodID method_println = env->GetMethodID(class_PrintStream, "println", "(Ljava/lang/Object;)V");
				static jfieldID field_out = env->GetStaticFieldID(class_System, "out", "Ljava/io/PrintStream;");
				static jobject out = env->GetStaticObjectField(class_System, field_out);

				env->CallVoidMethod(out, method_println, obj);
				env->ExceptionDescribe();
			}

		public:
			virtual const Class* loadClass(const char* name) const override {

				static const jclass class_NoClassDefFoundError = env->FindClass("java/lang/NoClassDefFoundError");
				static const jmethodID method_getDeclaredMethods = env->GetMethodID(class_Class, "getDeclaredMethods", "()[Ljava/lang/reflect/Method;");

				jclass clazz = env->FindClass(name/*"example/ExampleClass"*/);

				if(env->ExceptionCheck() && env->IsInstanceOf(env->ExceptionOccurred(), class_NoClassDefFoundError)) {
					env->ExceptionClear();
					return nullptr;
				}

				env->ExceptionDescribe();

				cout << "loadClass " << name << ' ' << clazz << endl;

				if(clazz == nullptr)
					return nullptr;

				jobjectArray methods = static_cast<jobjectArray>(env->CallObjectMethod(clazz, method_getDeclaredMethods));

				jsize length = env->GetArrayLength(methods);

				for(jsize i = 0; i < length; i++) {
					jobject method = env->GetObjectArrayElement(methods, i);

					println(method);
				}

				env->ExceptionDescribe();

				return nullptr;
			}

			virtual ~JVMImpl() {
				vm->DestroyJavaVM();
			}
	};
}

extern "C"
const jdecompiler::JVM* createJDecompilerJvm() {
	using std::cout;
	using std::cerr;
	using std::endl;
	using namespace jdecompiler;

	JavaVM* vm;
	JNIEnv* env;

	JavaVMInitArgs vmArgs { .version = JNI_VERSION_10, .nOptions = 0, .options = nullptr, .ignoreUnrecognized = true };

	jint res = JNI_CreateJavaVM(&vm, (void**)&env, &vmArgs);
	if(res != 0) {
		cerr << "Error: function JNI_CreateJavaVM returns " << res << endl;
		return nullptr;
	}

	return new JVMImpl(vm, env);
}

#endif
