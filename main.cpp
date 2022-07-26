#ifndef JDECOMPILER_MAIN_CPP
#define JDECOMPILER_MAIN_CPP

#include "function-definitions.cpp"
#include "finish.cpp"

int main(int argc, const char* args[]) {
	using namespace jdecompiler;

	atexit(&finish);
	signal(SIGSEGV, &finishSigSegvHandler);

	cout << boolalpha;
	cerr << boolalpha;

	if(!JDecompiler::init(argc, args))
		return 0;

	JDecompiler::getInstance().readClassFiles();

	for(const auto& nameAndClass : JDecompiler::getInstance().getDecompilationClasses()) {
		const ClassHolder& clazz = nameAndClass.second;

		if(clazz->canStringify()) {
			/*if(!JDecompiler::getInstance().failOnError()) {
				try {
					cout << clazz->toString() << endl;
				} catch(const exception& ex) {
					cerr << "Exception while decompiling class " << nameAndClass.first << ": " << typenameof(ex) << ": " << ex.what() << endl;
				}
			} else {*/
				//clazz->getSourceFile();

				if(!JDecompiler::getInstance().writeToConsole()) {
					BinaryOutputStream& outfile = *new FileBinaryOutputStream(clazz.outputPath);
					outfile.writeString(clazz->toString());
					outfile.close();

				} else {
					cout << clazz->toString() << endl;
				}
			//}
		}
	}

	return 0;
}

#endif
