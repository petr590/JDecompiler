#ifndef JDECOMPILER_FINISH_CPP
#define JDECOMPILER_FINISH_CPP

#include <csignal>

namespace jdecompiler {

	void finish() {

		log("finish()");

		if(JDecompiler::jvm != nullptr) {
			delete JDecompiler::jvm;
		}

		if(JDecompiler::libJvm != nullptr) {
			dlclose(JDecompiler::libJvm);
		}

		if(JDecompiler::systemLibJvm != nullptr) {
			dlclose(JDecompiler::systemLibJvm);
		}
	}

	void finishSigSegvHandler(int signum) {
		finish();
		signal(signum, SIG_DFL);
	}
}

#endif
