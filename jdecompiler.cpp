#ifndef JDECOMPILER_JDECOMPILER_CPP
#define JDECOMPILER_JDECOMPILER_CPP

#ifndef JDECOMPILER_MAIN_CPP
#error required file "jdecompiler/main.cpp" for correct compilation
#endif

#undef inline
#include <map>
#define inline FORCE_INLINE

namespace jdecompiler {

	using namespace std;


	struct JDecompiler {

		public:
			static const JDecompiler instance;

		protected:
			static bool isConfigParsed;

			vector<BinaryInputStream*> files;
			bool failOnError = false;

			mutable map<string, const Class*> classes;

		public:
			static bool parseConfig(int argc, const char* args[]) {
				if(isConfigParsed)
					throw IllegalStateException("Global config already parsed");

				JDecompiler& instance = const_cast<JDecompiler&>(JDecompiler::instance);

				if(argc <= 1) {
					cout << "Usage: " << args[0] << " [options] <class-files>" << endl;
					return false;
				}

				for(int i = 1; i < argc; i++) {
					const char* arg = args[i];
					const size_t length = strlen(arg);
					if(length > 1 && arg[0] == '-') {
						const string option(arg);
						if(option == "-h" || option == "--help" || option == "-?") {
							cout << "Usage: " << args[0] << " [options] <class-files>" << endl
								<< "  -h, --help, -?       show this message and exit" << endl
								<< "  -f, --fail-on-error  fail on error";
							return false;
						} else if(option == "-f" || option == "--fail-on-error") {
							instance.failOnError = true;
						} else {
							cerr << arg[0] << ": Unknown option " << arg << endl;
							cerr << "Use " << arg[0] << " for more information" << endl;
							return false;
						}
					} else {
						instance.files.push_back(new BinaryInputStream(arg));
					}
				}

				isConfigParsed = true;

				return true;
			}


			inline bool isFailOnError() const {
				return failOnError;
			}

			inline const vector<BinaryInputStream*>& getFiles() const {
				return files;
			}

			void readClassFiles() const;

			inline const map<string, const Class*>& getClasses() const {
				return classes;
			}

			inline const Class* getClass(const string& name) const {
				static int i = 0;
				if(i == 2)
					throw Exception();
				i++;
				const auto& classIterator = classes.find(name);
				return classIterator != classes.end() ? classIterator->second : nullptr;
			}
	};

	bool JDecompiler::isConfigParsed = false;

	const JDecompiler JDecompiler::instance{};
}

#endif
