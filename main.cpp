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

	for(const auto& nameAndClass : JDecompiler::getInstance().getClasses()) {
		if(nameAndClass.second->canStringify()) {
			/*if(!JDecompiler::getInstance().isFailOnError())

			try {*/
				cout << nameAndClass.second->toString() << endl;
			/*} catch(const exception& ex) {
				cerr << "Exception while decompiling class " << nameAndClass.first << ": " << typeNameOf(ex) << ": " << ex.what() << endl;
				throw;
			}*/
		}
	}

	return 0;
}

#undef inline

#endif
