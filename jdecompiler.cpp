#ifndef JDECOMPILER_JDECOMPILER_CPP
#define JDECOMPILER_JDECOMPILER_CPP

#undef inline
#include <filesystem>
#define inline FORCE_INLINE
#include "util.cpp"

namespace jdecompiler {

	struct JDecompiler {

		protected:
			static const JDecompiler* instance;


			const string progName;

			const vector<BinaryInputStream*> files;

			const uint16_t indentWidth = 4;
			const char* const indent = "    ";

			const bool failOnError;
			const bool useRawConstants;

			enum class UseHex { ALWAYS, AUTO, NEVER };

			const UseHex useHexNumbers;

			mutable map<string, const Class*> classes;

			JDecompiler(const string progName, const vector<BinaryInputStream*>& files, uint16_t indentWidth, const char* indent,
					bool failOnError, bool useRawConstants, UseHex useHexNumbers, map<string, const Class*>& classes):
					progName(progName), files(files), indentWidth(indentWidth), indent(indent),
					failOnError(failOnError), useRawConstants(useRawConstants), useHexNumbers(useHexNumbers), classes(classes) {}

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

				uint16_t indentWidth = 4;
				const char* indent = "    ";

				bool failOnError = false;
				bool useRawConstants = false;
				UseHex useHexNumbers = UseHex::NEVER;

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
								"  -h, --help, -?                      show this message and exit\n"
								"  -f, --fail-on-error                 fail on error\n"
								"  -w=<width>, --indent-width=<width>  set indent width (by default 4)\n"
								"  -i=<indent>, --indent=<indent>      set indent (by default four spaces)\n"
								"  -r, --use-raw-constants             do not use constants from classes Integer, Float,\n"
								"                                        Double etc., such as MAX_VALUE\n"
								"  --hex[=always|auto|never]           use hex numbers:\n"
								"                              always    always use\n"
								"                                auto    only use for values like 0x7F, 0x80 and 0xFF\n"
								"                               never    do not use (by default)" << endl;
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

						} else if(option == "-r" || option == "--use-raw-constants") {
							useRawConstants = true;

						} else if(option == "--hex") {
							if(hasValueWeak) {
								if(value == "always")     useHexNumbers = UseHex::ALWAYS;
								else if(value == "auto")  useHexNumbers = UseHex::AUTO;
								else if(value == "never") useHexNumbers = UseHex::NEVER;
								else printErrorAndExit("invalid value for option " << option << ": only valid always, auto or never");
							} else {
								useHexNumbers = UseHex::AUTO;
							}

						} else {
							printErrorAndExit("Unknown argument " << arg << "\n"
								"Use " << progName << " --help for more information");
						}
					} else {
						try {
							files.push_back(new BinaryInputStream(arg));
						} catch(const IOException& ex) {
							printError(ex.what());
						}
					}
				}

				#undef requireValue
				#undef printErrorAndExit

				instance = new JDecompiler(progName, files, indentWidth, indent, failOnError, useRawConstants, useHexNumbers, classes);

				return true;
			}


			inline bool isFailOnError() const {
				return failOnError;
			}

			inline bool canUseConstants() const {
				return !useRawConstants;
			}

			inline bool canUseHexNumbers() const {
				return useHexNumbers == UseHex::AUTO || useHexNumbers == UseHex::ALWAYS;
			}

			inline bool useHexNumbersAlways() const {
				return useHexNumbers == UseHex::ALWAYS;
			}

			inline const char* getIndent() const {
				return indent;
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


	struct Stringified {
		public:
			virtual string toString(const ClassInfo& classinfo) const = 0;

			virtual ~Stringified() {}
	};


	struct ClassElement: Stringified {
		public:
			virtual bool canStringify(const ClassInfo& classinfo) const = 0;
	};


	struct ClassInfo {
		public:
			const Class& clazz;

			const ClassType& thisType, & superType;
			const ConstantPool& constPool;
			const Attributes& attributes;
			const uint16_t modifiers;

			ClassInfo(const Class& clazz, const ClassType& thisType, const ClassType& superType, const ConstantPool& constPool,
					const Attributes& attributes, uint16_t modifiers):
					clazz(clazz), thisType(thisType), superType(superType), constPool(constPool), attributes(attributes),
					modifiers(modifiers) {}

		private:
			mutable set<const ClassType*> imports;

			mutable uint16_t indentWidth = 0;
			mutable const char* indent = new char[1] {'\0'};

		public:
			bool addImport(const ClassType* clazz) const;

			string importsToString() const;

			/*inline void setFormatting(uint16_t indentWidth, const char* indent, const set<const ClassType*>& imports) const {
				this->indentWidth = indentWidth;
				this->indent = repeatString(JDecompiler::getInstance().getIndent(), indentWidth);
				this->imports = imports;
			}*/

			inline void copyFormattingFrom(const ClassInfo& other) const {
				indentWidth = other.indentWidth;
				indent = repeatString(JDecompiler::getInstance().getIndent(), indentWidth);
				imports = other.imports;
			}

			inline void resetFormatting() const {
				indentWidth = 0;
				indent = new char[1] {'\0'};
				imports.clear();
			}

		public:
			inline const char* getIndent() const {
				return indent;
			}

			void increaseIndent() const {
				delete[] indent;
				indent = repeatString(JDecompiler::getInstance().getIndent(), ++indentWidth);
			}

			void increaseIndent(uint16_t count) const {
				delete[] indent;
				indent = repeatString(JDecompiler::getInstance().getIndent(), indentWidth += count);
			}

			void reduceIndent() const {
				delete[] indent;
				indent = repeatString(JDecompiler::getInstance().getIndent(), --indentWidth);
			}

			void reduceIndent(uint16_t count) const {
				delete[] indent;
				indent = repeatString(JDecompiler::getInstance().getIndent(), indentWidth -= count);
			}


			ClassInfo(const ClassInfo&) = delete;

			ClassInfo& operator=(const ClassInfo&) = delete;
	};
}

#endif
