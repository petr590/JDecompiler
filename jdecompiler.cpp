#ifndef JDECOMPILER_JDECOMPILER_CPP
#define JDECOMPILER_JDECOMPILER_CPP

#undef inline
#include <map>
#include <set>
#include <filesystem>
#define inline FORCE_INLINE
#include "util.cpp"

namespace jdecompiler {

	using namespace std;


	struct JDecompiler {

		protected:
			static const JDecompiler* instance;


			const string progName;

			const vector<BinaryInputStream*> files;

			const uint16_t indentWidth = 4;
			const char* const indent = "    ";

			const bool failOnError;

			mutable map<string, const Class*> classes;

			JDecompiler(const string progName, const vector<BinaryInputStream*>& files, uint16_t indentWidth, const char* indent,
					bool failOnError, map<string, const Class*>& classes):
					progName(progName), files(files), indentWidth(indentWidth), indent(indent), failOnError(failOnError), classes(classes) {}

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
				const string progName = progPath.substr(progPath.find_last_of(filesystem::path::preferred_separator) + 1);

				vector<BinaryInputStream*> files;

				uint16_t indentWidth = 4;
				const char* indent = "    ";

				bool failOnError;

				map<string, const Class*> classes;


				if(argc <= 1) {
					cout << "Usage: " << progName << " [options] <class-files>" << endl;
					return false;
				}

				#define requireValue() if(!hasValue) {\
					cerr << progName << ": Option " << option << " required value" << endl;\
					return false;\
				}

				#define parseError(...) { cerr << __VA_ARGS__ << endl; return false; }

				bool isIndentWidthSpecified = false;

				for(int i = 1; i < argc; i++) {
					const string arg = args[i];
					const size_t length = arg.size();

					if(length > 1 && arg[0] == '-') {

						string option, value;
						bool hasValue = false;
						const auto splitpos = arg.find('=');

						if(splitpos != string::npos) {
							option = string(arg, 0, splitpos);
							value = string(arg.begin() + splitpos + 1, arg.end());
							hasValue = true;
						} else {
							option = arg;
							if(i + 1 < argc) {
								value = args[++i];
								hasValue = true;
							}
						}


						if(option == "-h" || option == "--help" || option == "-?") {
							cout << "Usage: " << progName << " [options] <class-files>\n"
								"  -h, --help, -?                     show this message and exit\n"
								"  -f, --fail-on-error                fail on error\n"
								"  -w=<width> --indent-width=<width>  set indent width (by default 4)\n"
								"  -i=<indent> --indent=<indent>      set indent (by default four spaces)" << endl;
							return false;
						} else if(option == "-f" || option == "--fail-on-error") {
							failOnError = true;

						} else if(option == "-w" || option == "--indent-width") {
							requireValue();
							try {
								indentWidth = stoi(value);
								if(indentWidth < 0)
									parseError("Argument value '" << value << "' cannot be negative");
								if(indentWidth < 0 || indentWidth > 16)
									parseError("Argument value '" << value << "' is out of range");

								indent = repeatString(indent, indentWidth);
							} catch(const invalid_argument&) {
								parseError("Invalid argument value: '" << value << '\'');
							} catch(const out_of_range&) {
								parseError("Argument value '" << value << "' is out of range");
							}

							isIndentWidthSpecified = true;

						} else if(option == "-i" || option == "--indent") {
							requireValue();
							value = unescapeString(value);
							indent = repeatString(value, isIndentWidthSpecified || value != "\t" ? indentWidth : (indentWidth = 1));

						} else {
							parseError(progName << ": Unknown argument " << arg << "\n"
								"Use " << progName << " --help for more information");
						}
					} else {
						files.push_back(new BinaryInputStream(arg));
					}
				}

				#undef requireValue
				#undef parseError

				instance = new JDecompiler(progName, files, indentWidth, indent, failOnError, classes);

				return true;
			}


			inline bool isFailOnError() const {
				return failOnError;
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
