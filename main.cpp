#ifndef JDECOMPILER_MAIN_CPP
#define JDECOMPILER_MAIN_CPP

#include "function-definitions.cpp"
#include "finish.cpp"

int main(int argc, const char* args[]) {
	using namespace jdecompiler;
	using std::boolalpha;

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
			log("stringify of", nameAndClass.first);

			try {
				if(JDecompiler::getInstance().writeToConsole()) {
					cout << clazz->toString() << endl;

				} else {
					BinaryOutputStream* outfile = new FileBinaryOutputStream(clazz.outputPath);
					outfile->writeString(clazz->toString());
					delete outfile;
				}

			} catch(const Exception& ex) {
				cerr << "Exception while decompiling class " << nameAndClass.second->thisType.getClassEncodedName() << ": " << ex.toString() << endl;
			} catch(const exception& ex) {
				const char* errorMessage = ex.what();
				cerr << "Exception while decompiling class " << nameAndClass.second->thisType.getClassEncodedName() << ": " <<
						typenameof(ex) << (*errorMessage == '\0' ? "" : ": ") << errorMessage << endl;
			}
		}
	}

	return 0;
}

#endif
