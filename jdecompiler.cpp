#include "jdecompiler.h"
#include "jdecompiler-util.cpp"
#include "jdecompiler-const-pool.cpp"
#include "jdecompiler-main.cpp"
#include "jdecompiler-method-code.cpp"

using namespace std;
using namespace JDecompiler;

int main(int argc, char* args[]) {
	BinaryInputStream* instream = nullptr;

	for(int i = 1; i < argc; i++) {
		const char* arg = args[i];
		instream = new BinaryInputStream(arg);
	}

	if(instream == nullptr) {
		cerr << "Input file is not specified" << endl;
		return 1;
	}

	Class* c = new Class(instream);
	//try {
		cout << c->toString() << endl;
	/*
	}catch(exception ex) {
		cout << "Exception: " << typeid(ex).name() << ": " << ex.what() << endl;
	}*/

	return 0;
}
