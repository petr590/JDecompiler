#ifndef JDECOMPILER_CONFIG_CPP
#define JDECOMPILER_CONFIG_CPP

#define inline INLINE_ATTR

#undef LOG_PREFIX
#define LOG_PREFIX "[ jdecompiler-config.cpp ]"

namespace JDecompiler {

	using namespace std;


	struct Config {

		public:
			static const Config globalConfig;

		protected:
			static bool isParsed;

			vector<BinaryInputStream*> files;
			bool failOnError = false;

		public:
			static bool parseGlobalConfig(int argc, char* args[]) {
				if(isParsed)
					throw IllegalStateException("Global config already parsed");

				Config& config = const_cast<Config&>(Config::globalConfig);

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
								<< "  -h, --help, -?    show this message and exit" << endl;
							return false;
						} else if(option == "-f" || option == "--fail-on-error") {
							config.failOnError = true;
						} else {
							cerr << arg[0] << ": Unknown option " << arg << endl;
							cerr << "Use " << arg[0] << " for more information" << endl;
							return false;
						}
					} else
						config.files.push_back(new BinaryInputStream(arg));
				}

				isParsed = true;

				return true;
			}


			inline bool isFailOnError() const {
				return failOnError;
			}

			inline const vector<BinaryInputStream*>& getFiles() const {
				return files;
			}
	};

	bool Config::isParsed = false;

	const Config Config::globalConfig{};
}

#undef inline

#endif
