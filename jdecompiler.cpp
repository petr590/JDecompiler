#include "jdecompiler.h"
#include "jdecompiler-util.cpp"
#include "jdecompiler-const-pool.cpp"
#include "jdecompiler-main.cpp"
#include "jdecompiler-method-code.cpp"

using namespace std;
using namespace JDecompiler;

int main(int argc, char* args[]) {
	if(argc <= 1) {
		cout << "Usage: " << args[0] << " [options] <class-files>" << endl;
		return 0;
	}

	vector<BinaryInputStream*> files;

	for(int i = 1; i < argc; i++) {
		const char* arg = args[i];
		const size_t length = strlen(arg);
		if(length > 1 && arg[0] == '-') {
			const string option(arg);
			if(option == "-h" || option == "--help" || option == "-?") {
				cout << "Usage: " << args[0] << " [options] <class-files>" << endl
					<< "  -h, --help, -?    show this message and exit" << endl;
				return 0;
			} else {
				cerr << arg[0] << ": Unknown option " << arg << endl;
				cerr << "Use " << arg[0] << " for more information" << endl;
				return 1;
			}
		} else
			files.push_back(new BinaryInputStream(arg));
	}

	for(BinaryInputStream* instream : files) {
		const Class& c = Class::readClass(*instream);
		cout << c.toString() << endl;
	}

	return 0;
}
