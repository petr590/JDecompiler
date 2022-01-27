#include "jdecompiler.h"
#include "jdecompiler-util.cpp"
#include "jdecompiler-const-pool.cpp"
#include "jdecompiler-main.cpp"
#include "jdecompiler-method-code.cpp"

using namespace std;
using namespace JDecompiler;

int main(int argc, char* args[]) {
	if(argc == 1) {
		cout << "Usage: " << args[0] << " [options] <class-files>" << endl;
		return 0;
	}

	vector<BinaryInputStream*> files;

	for(int i = 1; i < argc; i++) {
		const char* arg = args[i];
		files.push_back(new BinaryInputStream(arg));
	}

	for(BinaryInputStream* instream : files) {
		Class* c = new Class(*instream);
		cout << c->toString() << endl;
	}

	return 0;
}
