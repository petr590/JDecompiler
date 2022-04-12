#ifndef JDECOMPILER_MAIN_CPP
#define JDECOMPILER_MAIN_CPP

#include "function-definitions.cpp"

int main(int argc, const char* args[]) {
	using namespace jdecompiler;

	cout << boolalpha;
	cerr << boolalpha;

	if(!JDecompiler::init(argc, args))
		return 0;

	JDecompiler::getInstance().readClassFiles();

	for(const auto& clazz : JDecompiler::getInstance().getClasses()) {
		if(clazz.second->canStringify())
			cout << clazz.second->toString() << endl;
	}

	return 0;
}

#undef inline

#endif
