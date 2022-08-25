#ifndef JDECOMPILER_CLASS_TYPE_CPP
#define JDECOMPILER_CLASS_TYPE_CPP

namespace jdecompiler {

	struct ClassType final: ReferenceType {
		public:
			string classEncodedName,
					simpleName,
					fullSimpleName, // fullSimpleName is a class name including enclosing class name
					packageName;

			vector<const ReferenceType*> parameters;

			const ClassType* enclosingClass;
			bool isNested = false, isAnonymous = false, isPackageInfo = false;

			ClassType(const ClassConstant* clazz): ClassType(clazz->name) {}

			ClassType(const string& str): ClassType(str.c_str()) {}

			ClassType(const char*&& str): ClassType(static_cast<const char*&>(str)) {}

			ClassType(const char*& restrict str) {

				const char* const srcStr = str;

				string encodedName = str;
				string name = encodedName;

				uint32_t nameStartPos = 0,
				         packageEndPos = 0,
				         enclosingClassNameEndPos = 0;

				if(stringEndsWith(encodedName, "/package-info"))
					isPackageInfo = true;

				for(uint32_t i = 0;;) {

					switch(*str) {
						case '/':
							packageEndPos = i;
							nameStartPos = i + 1;
							name[i] = '.';
							break;

						case '$':
							enclosingClassNameEndPos = i;
							nameStartPos = i + 1;
							name[i] = '.';
							break;

						case '<':
							parameters = parseParameters(str);

							switch(*str) {
								case ';':
									++str; // the falling through is done on purpose
								case '\0':
									name = name.substr(0, i);
									encodedName = encodedName.substr(0, i);
									goto BreakFor;

								default:
									throw InvalidClassNameException(srcStr, i);
							}

						case ';':
							++str; // the falling through is done on purpose
						case '\0':
							name = name.substr(0, i);
							encodedName = encodedName.substr(0, i);
							goto BreakFor;

						// invalid chars
						case '-':
							if(!isPackageInfo) // А ЧЁ, ТАК МОЖНО БЫЛО ЧТОЛИ??? ОФИГЕТЬ! РАБОТАЕТ!!! case ВНУТРИ блока!
						case '\t': case '\n': case '\v': case '\f': case '\r': case ' ': case '!':
						case '"': case '#': case '%': case '&': case '\'': case '(': case ')': case '*': case '+':
						case ',': case '.': case ':': case '=':  case '?': case '@': case '[': case '\\':
						case ']': case '^': case '`': case '{': case '|':  case '}': case '~': case '\x7F': // DEL
								throw InvalidClassNameException(srcStr, i);
					}

					++i, ++str;
				}
				BreakFor:

				this->name = name;

				this->encodedName = 'L' + encodedName + ';';
				this->classEncodedName = encodedName;

				simpleName = name.substr(nameStartPos);

				packageName = name.substr(0, packageEndPos);

				if(enclosingClassNameEndPos == 0) {
					enclosingClass = nullptr;
					fullSimpleName = simpleName;
				} else {
					isNested = true;
					isAnonymous = all_of(simpleName.begin(), simpleName.end(), [] (unsigned char c) { return isdigit(c); });
					enclosingClass = new ClassType(encodedName.substr(0, enclosingClassNameEndPos));

					fullSimpleName = enclosingClass->fullSimpleName + (isAnonymous ? '$' : '.') + simpleName;

					if(isAnonymous) {
						this->name[enclosingClassNameEndPos] = '$';
						simpleName = enclosingClass->simpleName + '$' + simpleName;
					}
				}
			}

			virtual string toString(const ClassInfo&) const override;

			virtual string toString() const override {
				return "class " + (parameters.empty() ? name : name +
						'<' + join<const ReferenceType*>(parameters, [] (const ReferenceType* type) { return type->toString(); }) + '>');
			}

			virtual string getVarName() const override final {
				return toLowerCamelCase(simpleName);
			}

			virtual status_t implicitCastStatus(const Type*) const override;

			virtual string getClassEncodedName() const override {
				return classEncodedName;
			}

		protected:
			virtual bool isSubtypeOfImpl(const Type* other) const override {
				return instanceof<const ClassType*>(other);
			}
	};


	static const ClassType
			*const OBJECT(new ClassType("java/lang/Object")),
			*const STRING(new ClassType("java/lang/String")),
			*const CLASS(new ClassType("java/lang/Class")),
			*const ENUM(new ClassType("java/lang/Enum")),
			*const THROWABLE(new ClassType("java/lang/Throwable")),
			*const EXCEPTION(new ClassType("java/lang/Exception")),
			*const METHOD_TYPE(new ClassType("java/lang/invoke/MethodType")),
			*const METHOD_HANDLE(new ClassType("java/lang/invoke/MethodHandle"));
}

#endif
