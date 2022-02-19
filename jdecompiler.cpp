#include "jdecompiler.h"
#include "jdecompiler-util.cpp"
#include "jdecompiler-config.cpp"
#include "jdecompiler-const-pool.cpp"
#include "jdecompiler-main.cpp"
#include "jdecompiler-method-code.cpp"

int main(int argc, char* args[]) {
	using namespace std;
	using namespace JDecompiler;

	if(!Config::parseGlobalConfig(argc, args))
		return 0;

	for(BinaryInputStream* instream : Config::globalConfig.getFiles()) {
		const Class& c = Class::readClass(*instream);
		cout << c.toString() << endl;
	}
}
