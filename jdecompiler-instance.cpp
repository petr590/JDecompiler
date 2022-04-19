#ifndef JDECOMPILER_JDECOMPILER_INSTANCE_CPP
#define JDECOMPILER_JDECOMPILER_INSTANCE_CPP

#undef inline
#include <filesystem>
#define inline INLINE
#include "util.cpp"

namespace jdecompiler {

	struct JDecompiler {

		protected:
			static const JDecompiler* instance;


			const string progName;

			const vector<BinaryInputStream*> files;
			const bool atLeastOneFileSpecified;

			const char* const indent;

			const bool failOnError;

			enum class UseConstants { ALWAYS, MINIMAL, NEVER };

			const UseConstants useConstants;

			enum class UseHex { ALWAYS, AUTO, NEVER };

			const UseHex useHexNumbers;

			const bool useShortArrayInitializing;

			mutable map<string, const Class*> classes;

			JDecompiler(const string progName, const vector<BinaryInputStream*>& files, bool atLeastOneFileSpecified, const char* indent,
					bool failOnError, UseConstants useConstants, UseHex useHexNumbers, bool useShortArrayInitializing, map<string, const Class*>& classes):

					progName(progName), files(files), atLeastOneFileSpecified(atLeastOneFileSpecified), indent(indent),
					failOnError(failOnError), useConstants(useConstants), useHexNumbers(useHexNumbers),
					useShortArrayInitializing(useShortArrayInitializing), classes(classes) {}



		public:
			static const JDecompiler& getInstance() {
				if(instance == nullptr)
					throw IllegalStateException("JDecompiler yet not initialized");
				return *instance;
			}

			static bool init(int argc, const char* args[]) {
				if(instance != nullptr)
					throw IllegalStateException("JDecompiler already initialized");

				const string progPath = args[0];
				const string progName = progPath.substr(progPath.find_last_of(std::filesystem::path::preferred_separator) + 1);

				vector<BinaryInputStream*> files;

				int_fast32_t indentWidth = 4;
				const char* indent = "    ";

				bool failOnError = false;
				UseConstants useConstants = UseConstants::ALWAYS;
				UseHex useHexNumbers = UseHex::NEVER;

				bool useShortArrayInitializing = true;

				map<string, const Class*> classes;


				if(argc <= 1) {
					cout << "Usage: " << progName << " [options] <class-files>" << endl;
					return false;
				}

				#define printError(...) cerr << progName << ": error: " << __VA_ARGS__ << endl
				#define printErrorAndExit(...) { printError(__VA_ARGS__); return false; }

				#define requireValue() {\
					if(!hasValue) {\
						printErrorAndExit("option " << option << " required value");\
						return false;\
					}\
					i++;\
				}

				bool isIndentWidthSpecified = false;

				bool atLeastOneFileSpecified = false;

				for(int i = 1; i < argc; i++) {
					const string arg = args[i];
					const size_t length = arg.size();

					if(length > 1 && arg[0] == '-') {

						string option, value;
						bool hasValue = false, hasValueWeak = false;
						const auto splitpos = arg.find('=');

						if(splitpos != string::npos) {
							option = string(arg, 0, splitpos);
							value = string(arg.begin() + splitpos + 1, arg.end());
							hasValue = hasValueWeak = true;
						} else {
							option = arg;
							if(i + 1 < argc) {
								value = args[i + 1];
								hasValue = true;
							}
						}


						if(option == "-h" || option == "--help" || option == "-?") {
							cout << "Usage: " << progName << " [options] <class-files>\n"
								"\n"
								"  -h, --help, -?                      show this message and exit\n"
								"  -f, --fail-on-error                 fail on error\n"
								"  -w=<width>, --indent-width=<width>  set indent width (by default 4)\n"
								"  -i=<indent>, --indent=<indent>      set indent (by default four spaces)\n"
								"\n"
								"  --use-constants=auto|minimal|never  use constants:\n"
								"                                auto    use for all standart constants (MAX_VALUE, MIN_VALUE, etc.)\n"
								"                             minimal    use for only POSITIVE_INFINITY, NEGATIVE_INFINITY and NaN\n"
								"                               never    don't use constants at all\n"
								"  -mc, --use-min-constants            the same as --use-constants=minimal\n"
								"  -nc, --not-use-constants            the same as --use-constants=never\n"
								"  --hex[=always|auto|never]           use hex numbers:\n"
								"                              always    always use\n"
								"                                auto    only use for values like 0x7F, 0x80 and 0xFF\n"
								"                               never    do not use (by default)\n"
								"\n"
								"  --no-short-array-init               do not use short array initialization in the variable declaration" << endl;

							return false;

						} else if(option == "-f" || option == "--fail-on-error") {
							failOnError = true;

						} else if(option == "-w" || option == "--indent-width") {
							requireValue();
							try {
								indentWidth = stoi(value);
								if(indentWidth < 0)
									printErrorAndExit("indent width " << value << " cannot be negative");
								if(indentWidth > 16)
									printErrorAndExit("indent width " << value << " is out of range (valid values are from 0 to 16)");

								indent = repeatString(indent, indentWidth);
							} catch(const invalid_argument&) {
								printErrorAndExit("Invalid argument value: '" << value << '\'');
							} catch(const out_of_range&) {
								printErrorAndExit("Argument value '" << value << "' is out of range");
							}

							isIndentWidthSpecified = true;

						} else if(option == "-i" || option == "--indent") {
							requireValue();
							value = unescapeString(value);
							indent = repeatString(value, isIndentWidthSpecified || value != "\t" ? indentWidth : (indentWidth = 1));

						} else if(option == "--use-constants") {
							requireValue();

							if(value == "always")
								useConstants = UseConstants::ALWAYS;
							else if(value == "minimal" || value == "min")
								useConstants = UseConstants::MINIMAL;
							else if(value == "never")
								useConstants = UseConstants::NEVER;
							else
								printErrorAndExit("invalid value for option " << option << ": only valid always, minimal or never");

						} else if(option == "-mc" || option == "--use-min-constants") {
							useConstants = UseConstants::MINIMAL;

						} else if(option == "-nc" || option == "--not-use-constants") {
							useConstants = UseConstants::NEVER;

						} else if(option == "--hex") {
							if(hasValueWeak) {
								if(value == "always")     useHexNumbers = UseHex::ALWAYS;
								else if(value == "auto")  useHexNumbers = UseHex::AUTO;
								else if(value == "never") useHexNumbers = UseHex::NEVER;
								else printErrorAndExit("invalid value for option " << option << ": only valid always, auto or never");
							} else {
								useHexNumbers = UseHex::AUTO;
							}

						} else if(option == "--no-short-array-init") {
							useShortArrayInitializing = false;
						} else {
							printErrorAndExit("Unknown argument " << arg << "\n"
								"Use " << progName << " --help for more information");
						}
					} else {
						atLeastOneFileSpecified = true;
						try {
							files.push_back(new BinaryInputStream(arg));
						} catch(const IOException& ex) {
							printError(ex.what());
						}
					}
				}

				#undef requireValue
				#undef printErrorAndExit

				instance = new JDecompiler(progName, files, atLeastOneFileSpecified, indent,
						failOnError, useConstants, useHexNumbers, useShortArrayInitializing, classes);

				return true;
			}


			inline const char* getIndent() const {
				return indent;
			}

			inline bool isFailOnError() const {
				return failOnError;
			}

			inline bool canUseConstants() const {
				return useConstants == UseConstants::ALWAYS;
			}

			inline bool canUseNaNAndInfinity() const {
				return useConstants == UseConstants::ALWAYS || useConstants == UseConstants::MINIMAL;
			}

			inline bool canUseHexNumbers() const {
				return useHexNumbers == UseHex::AUTO || useHexNumbers == UseHex::ALWAYS;
			}

			inline bool useHexNumbersAlways() const {
				return useHexNumbers == UseHex::ALWAYS;
			}

			inline bool canUseShortArrayInitializing() const {
				return useShortArrayInitializing;
			}

			inline const vector<BinaryInputStream*>& getFiles() const {
				return files;
			}

			void readClassFiles() const;

			inline const map<string, const Class*>& getClasses() const {
				return classes;
			}

			inline const Class* getClass(const string& name) const {
				const auto& classIterator = classes.find(name);
				return classIterator != classes.end() ? classIterator->second : nullptr;
			}

		protected:
			template<typename Arg, typename... Args>
			static inline void print(ostream& out, Arg arg, Args... args) {
				if constexpr(sizeof...(Args) != 0)
					print(out << arg, args...);
				else
					out << arg << endl;
			}

		private:
			template<typename... Args>
			inline void error(Args... args) const {
				print(cerr << progName << ": error: ", args...);
			}
	};

	const JDecompiler* JDecompiler::instance = nullptr;
}

#endif
