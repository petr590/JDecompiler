#ifndef JDECOMPILER_MAIN_CPP
#define JDECOMPILER_MAIN_CPP

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <functional>
#include "jdecompiler.h"
#include "jdecompiler-util.cpp"
#include "jdecompiler-const-pool.cpp"

#undef LOG_PREFIX
#define LOG_PREFIX "[ jdecompiler-main.cpp ]"

#define CLASS_SIGNATURE 0xCAFEBABE

#define ACC_VISIBLE      0x0000 // class, field, method
#define ACC_PUBLIC       0x0001 // class, field, method
#define ACC_PRIVATE      0x0002 // nested class, field, method
#define ACC_PROTECTED    0x0004 // nested class, field, method
#define ACC_STATIC       0x0008 // nested class, field, method
#define ACC_FINAL        0x0010 // class, field, method
#define ACC_SYNCHRONIZED 0x0020 // method, scope
#define ACC_SUPER        0x0020 // class (deprecated)
#define ACC_VOLATILE     0x0040 // field
#define ACC_BRIDGE       0x0040 // method
#define ACC_TRANSIENT    0x0080 // field
#define ACC_VARARGS      0x0080 // method
#define ACC_NATIVE       0x0100 // method
#define ACC_INTERFACE    0x0200 // class
#define ACC_ABSTRACT     0x0400 // class, method
#define ACC_STRICT       0x0800 // class, non-abstract method
#define ACC_SYNTHETIC    0x1000 // method
#define ACC_ANNOTATION   0x2000 // class
#define ACC_ENUM         0x4000 // class

using namespace std;

namespace JDecompiler {

	struct Stringified {
		public:
			virtual string toString(const ClassInfo& classinfo) const = 0;

			virtual bool canStringify(const ClassInfo& classinfo) const {
				return true;
			}
	};


	struct ClassInfo {
		public:
			const Class& clazz;

			const ClassType& thisType, & superType;
			const ConstantPool& constPool;
			const Attributes& attributes;
			const uint16_t modifiers;

			set<string>& imports;

			ClassInfo(const Class& clazz, const ClassType& thisType, const ClassType& superType, const ConstantPool& constPool,
					const Attributes& attributes, uint16_t modifiers, const char* baseIndent): clazz(clazz),
					thisType(thisType), superType(superType), constPool(constPool), attributes(attributes), modifiers(modifiers),
					imports(*new set<string>()), baseIndent(baseIndent) {}

		private:
			const char* const baseIndent;
			mutable uint16_t indentWidth = 0;
			mutable const char* indent = new char[0];

		public:
			inline const char* getIndent() const {
				return indent;
			}

			void increaseIndent() const {
				delete[] indent;
				indent = repeatString(baseIndent, ++indentWidth);
			}

			void increaseIndent(uint16_t count) const {
				delete[] indent;
				indent = repeatString(baseIndent, indentWidth += count);
			}

			void reduceIndent() const {
				delete[] indent;
				indent = repeatString(baseIndent, --indentWidth);
			}

			void reduceIndent(uint16_t count) const {
				delete[] indent;
				indent = repeatString(baseIndent, indentWidth -= count);
			}

			~ClassInfo() {
				delete &imports;
			}

			ClassInfo(const ClassInfo&) = delete;

			ClassInfo& operator=(const ClassInfo&) = delete;
	};
}

#include "jdecompiler-types.cpp"
#include "jdecompiler-javase.cpp"
#include "jdecompiler-attributes.cpp"

namespace JDecompiler {

	string ClassConstant::toString(const ClassInfo& classinfo) const {
		return ClassType(*name).toString(classinfo);
	}


	struct Variable {
		public:
			mutable const Type* type;

		protected:
			static string getRawNameByType(const Type* type, bool& unchecked) {
				if(type->isPrimitive()) {
					if(type == BOOLEAN) return "bool";
					if(type == BYTE) return "b";
					if(type == CHAR) return "c";
					if(type == SHORT) return "s";
					if(type == INT) return "n";
					if(type == LONG) return "l";
					if(type == FLOAT) return "f";
					if(type == DOUBLE) return "d";
				}
				if(const ClassType* classType = dynamic_cast<const ClassType*>(type)) {
					if(classType->simpleName == "Boolean") return "bool";
					if(classType->simpleName == "Byte") return "b";
					if(classType->simpleName == "Character") return "ch";
					if(classType->simpleName == "Short") return "sh";
					if(classType->simpleName == "Integer") return "n";
					if(classType->simpleName == "Long") return "l";
					if(classType->simpleName == "Float") return "f";
					if(classType->simpleName == "Double") return "d";
					unchecked = true;
					return toLowerCamelCase(classType->simpleName);
				}
				if(const ArrayType* arrayType = dynamic_cast<const ArrayType*>(type)) {
					if(const ClassType* classMemberType = dynamic_cast<const ClassType*>(arrayType->memberType))
						return toLowerCamelCase(classMemberType->simpleName) + "Array";
					return toLowerCamelCase(arrayType->memberType->getName()) + "Array";
				}
				return "o";
			}

			static string getNameByType(const Type* type) {
				bool unchecked = false;

				const string name = getRawNameByType(type, unchecked);
				if(unchecked) {
					static const map<const char*, const char*> keywords {
						{"boolean", "bool"}, {"byte", "b"}, {"char", "ch"}, {"short", "sh"}, {"int", "n"}, {"long", "l"},
						{"float", "f"}, {"double", "d"}, {"void", "v"},
						{"public", "pub"}, {"protected", "prot"}, {"private", "priv"}, {"static", "stat"}, {"final", "f"}, {"abstract", "abs"},
						{"transient", "trans"}, {"volatile", "vol"}, {"native", "nat"}, {"synchronized", "sync"},
						{"class", "clazz"}, {"interface", "interf"}, {"enum", "en"}, {"this", "t"}, {"super", "sup"}, {"extends", "ext"}, {"implements", "impl"},
						{"import", "imp"}, {"package", "pack"}, {"instanceof", "inst"}, {"new", "n"},
						{"if", "cond"}, {"else", "el"}, {"while", "whl"}, {"do", "d"}, {"for", "f"}, {"switch", "sw"}, {"case", "cs"}, {"default", "def"},
						{"break", "brk"}, {"continue", "cont"}, {"return", "ret"},
						{"try", "tr"}, {"catch", "c"}, {"finally", "f"}, {"throw", "thr"}, {"throws", "thrs"}, {"assert", "assrt"},
						{"true", "tr"}, {"false", "fls"}, {"null", "nll"},
						{"strictfp", "strict"}, {"const", "cnst"}, {"goto", "gt"}
					};

					for(auto& keyword : keywords)
						if(name == keyword.first)
							return keyword.second;
				}

				return name;
			}

		public:
			Variable(const Type* type): type(type) {}

			virtual string getName() const = 0;

			virtual void addName(const string&) const = 0;

			inline bool operator==(const Variable& other) const {
				return this == &other;
			}
	};


	struct NamedVariable: Variable {
		protected:
			const string name;

		public:
			NamedVariable(const Type* type, const string& name): Variable(type), name(name) {}

			virtual string getName() const override {
				return name;
			}

			virtual void addName(const string& name) const override {}
	};


	struct UnnamedVariable: Variable {
		protected:
			mutable vector<string> names;

		public:
			UnnamedVariable(const Type* type): Variable(type) {}

			UnnamedVariable(const Type* type, const string& name): Variable(type), names({name}) {}

			virtual string getName() const override {
				return names.size() == 1 ? names[0] : getNameByType(type);
			}

			virtual void addName(const string& name) const override {
				names.push_back(name);
			}
	};


	enum class Associativity { LEFT, RIGHT };


	struct Operation {
		public:
			const uint16_t priority;

			Operation(uint16_t priority = 15): priority(priority) {}

			virtual string toString(const CodeEnvironment& environment) const = 0;

			virtual string toArrayInitString(const CodeEnvironment& environment) const {
				return toString(environment);
			}

			virtual const Type* getReturnType() const = 0;

			template<class T>
			const T* getReturnTypeAs(const T* type) const {
				const T* newType = getReturnType()->castTo(type);
				castReturnTypeTo(newType);
				return newType;
			}

		protected:
			virtual void castReturnTypeTo(const Type* newType) const {}

		public:
			string toString(const CodeEnvironment& environment, uint16_t priority, const Associativity& associativity) const {
				if(this->priority < priority || (this->priority == priority && getAssociativityByPriority(this->priority) != associativity))
					return '(' + this->toString(environment) + ')';
				return this->toString(environment);
			}

			virtual inline string getFrontSeparator(const ClassInfo& classinfo) const {
				return classinfo.getIndent();
			}

			virtual inline string getBackSeparator(const ClassInfo& classinfo) const {
				return ";\n";
			}


			virtual bool canAddToCode() const {
				return true;
			}

			private: static Associativity getAssociativityByPriority(uint16_t priority) {
				switch(priority) {
					case 1: case 2: case 13: return Associativity::RIGHT;
					default: return Associativity::LEFT;
				}
			}
	};

	struct Instruction {
		virtual const Operation* toOperation(const CodeEnvironment& environment) const = 0;
	};


	struct Bytecode {
		public:
			const uint32_t length;
			const char* const bytes;

		private:
			uint32_t pos = 0;
			vector<Instruction*> instructions;
			vector<uint32_t> posMap;

		public:
			Bytecode(const uint32_t length, const char* bytes): length(length), bytes(bytes) {
				instructions.reserve(length);
			}

			inline const vector<Instruction*>& getInstructions() const {
				return instructions;
			}

			const Instruction* getInstructionNoexcept(uint32_t index) const {
				if(index >= instructions.size())
					return nullptr;
				return instructions[index];
			}

			inline const vector<uint32_t>& getPosMap() const {
				return posMap;
			}

			inline int8_t nextByte() {
				return bytes[++pos];
			}

			inline uint8_t nextUByte() {
				return bytes[++pos];
			}

			inline int16_t nextShort() {
				return nextUByte() << 8 | nextUByte();
			}

			inline uint16_t nextUShort() {
				return nextUByte() << 8 | nextUByte();
			}

			inline int32_t nextInt() {
				return nextUByte() << 24 | nextUByte() << 16 | nextUByte() << 8 | nextUByte();
			}

			inline uint32_t nextUInt() {
				return nextUByte() << 24 | nextUByte() << 16 | nextUByte() << 8 | nextUByte();
			}

			inline uint16_t current() const {
				return bytes[pos] & 0xFF;
			}

			inline uint32_t getPos() const {
				return pos;
			}

			inline bool available() const {
				return length - pos > 0;
			}

			inline void skip(uint32_t count) {
				pos += count;
			}

			const Instruction* getInstruction(uint32_t index) const {
				if(index >= instructions.size())
					throw IndexOutOfBoundsException(index, instructions.size());
				return instructions[index];
			}

			uint32_t posToIndex(uint32_t pos) const {
				uint32_t i = 0, index = 0;
				const uint32_t max = posMap.size();
				while(index != pos && i < max)
					index = posMap[++i];
				if(index != pos)
					throw BytecodeIndexOutOfBoundsException(pos, length);
				return i;
			}

			inline uint32_t indexToPos(uint32_t index) const {
				return posMap[index];
			}

			inline Instruction* nextInstruction() {
				posMap.push_back(pos);

				Instruction* instruction = nextInstruction0();
				if(instruction != nullptr) {
					instructions.push_back(instruction);
				}
				return instruction;
			}

			private: Instruction* nextInstruction0();
	};

	struct Stack {
		private:
			class Entry {
				public:
					const Operation* const value;
					const Entry* const next;

					Entry(const Operation* value, const Entry* next): value(value), next(next) {}

					void deleteNext() const {
						if(next != nullptr) {
							next->deleteNext();
							delete next;
						}
					}
			};

			const Entry* firstEntry;
			uint16_t length;

			inline void checkEmptyStack() const {
				if(firstEntry == nullptr)
					throw EmptyStackException();
			}

		public:
			void push(const Operation* operation) {
				firstEntry = new Entry(operation, firstEntry);
				length++;
			}

			inline void push(const Operation* operation, const Operation* operations...) {
				push(operation);
				push(operations);
			}

			const Operation* pop() {
				checkEmptyStack();

				const Entry copiedEntry = *firstEntry;
				delete firstEntry;
				firstEntry = copiedEntry.next;
				length--;
				return copiedEntry.value;
			}

			template<class T>
			const T* pop() {
				checkEmptyStack();

				const Operation* operation = pop();
				if(const T* t = dynamic_cast<const T*>(operation))
					return t;
				throw DecompilationException((string)"Illegal operation type " + typeid(T).name() + " for operation " + typeid(*operation).name());
			}

			const Operation* popAs(const Type* type) {
				checkEmptyStack();

				const Operation* operation = pop();
				operation->getReturnTypeAs(type);
				return operation;
			}

			inline const Operation* popAs(const Type& type) {
				return popAs(&type);
			}

			const Operation* top() const {
				checkEmptyStack();
				return firstEntry->value;
			}

			const Operation* lookup(uint16_t index) const {
				checkEmptyStack();

				if(index >= length)
					throw StackIndexOutOfBoundsException(index, length);

				const Entry* currentEntry = firstEntry;
				for(uint16_t i = 0; i < index; i++)
					currentEntry = currentEntry->next;
				return currentEntry->value;
			}

			inline uint16_t size() const {
				return length;
			}

			inline bool empty() const {
				return length == 0;
			}

			~Stack() {
				if(firstEntry != nullptr) {
					firstEntry->deleteNext();
					delete firstEntry;
				}
			}
	};


	struct CodeEnvironment {
		public:
			const Bytecode& bytecode;
			const ClassInfo& classinfo;
			const ConstantPool& constPool;
			Stack& stack;
			Scope& scope;
			Scope* currentScope;
			const uint16_t modifiers;
			const MethodDescriptor& descriptor;
			const Attributes& attributes;
			uint32_t pos, index, exprStartIndex;
			map<uint32_t, uint32_t> exprIndexTable;

		private:
			mutable vector<Scope*> scopes;

		public:
			CodeEnvironment(const Bytecode& bytecode, const ClassInfo& classinfo, Scope* scope, uint16_t modifiers,
					const MethodDescriptor& descriptor, const Attributes& attributes, uint32_t codeLength, uint16_t maxLocals);

			void checkCurrentScope();

			inline Scope* getCurrentScope() const;

			void addScope(Scope* scope) const {
				scopes.push_back(scope);
			}

			~CodeEnvironment() {
				delete &stack;
			}

			CodeEnvironment(const CodeEnvironment& environment) = delete;

			CodeEnvironment& operator=(const CodeEnvironment& environment) = delete;
	};


	struct Scope: Operation {
		public:
			const uint32_t from, to;
			Scope *const parentScope;

		protected:
			vector<Variable*> variables;
			vector<const Operation*> code;

		public:
			vector<Scope*> innerScopes;

			Scope(uint32_t from, uint32_t to, Scope* parentScope): from(from), to(to), parentScope(parentScope) {}

		public:
			Scope(uint32_t from, uint32_t to, uint16_t localsCount): Scope(from, to, nullptr) {
				variables.reserve(localsCount);
			}

			virtual const Variable& getVariable(uint32_t index) const {
				if(index >= variables.size() && parentScope == nullptr)
					throw IndexOutOfBoundsException(index, variables.size());
				return index >= variables.size() ? parentScope->getVariable(index) : *variables[index];
			}

			bool hasVariable(const string& name) const {
				for(Variable* var : variables)
					if(name == var->getName())
						return true;
				return parentScope == nullptr ? false : parentScope->hasVariable(name);
			}

			void addVariable(Variable* var) {
				variables.push_back(var);
			}

			//virtual void initiate(const CodeEnvironment&) {}

			virtual void finalize(const CodeEnvironment&) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				environment.classinfo.increaseIndent();

				string str = getHeader(environment) + "{\n";
				const size_t baseSize = str.size();

				for(auto i = code.begin(); i != code.end(); ++i) {
					if(printNextOperation(i)) {
						const Operation* operation = *i;
						if(operation->getReturnType() == VOID)
							str += operation->getFrontSeparator(environment.classinfo) + operation->toString(environment) +
									operation->getBackSeparator(environment.classinfo);
					}
				}

				environment.classinfo.reduceIndent();

				if(str.size() == baseSize) {
					str[baseSize - 1] = '}';
					return str;
				}

				return str + environment.classinfo.getIndent() + '}';
			}

		protected:
			virtual inline bool printNextOperation(const vector<const Operation*>::const_iterator i) const { return true; }

			virtual inline string getBackSeparator(const ClassInfo& classinfo) const override {
				return "\n";
			}

			virtual string getHeader(const CodeEnvironment& environment) const {
				return EMPTY_STRING;
			}

		public:
			uint32_t getVariablesCount() const {
				return variables.size();
			}

			virtual void add(const Operation* operation, const CodeEnvironment& environment) {
				code.push_back(operation);
			}

			inline bool isEmpty() const {
				return code.empty();
			}

			virtual const Type* getReturnType() const override {
				return VOID;
			}

			virtual bool canAddToCode() const override {
				return false;
			}
	};


	// --------------------------------------------------


	struct StaticInitializerScope: Scope {
		private:
			bool fieldsInitialized = false;

		public:
			StaticInitializerScope(uint32_t from, uint32_t to, uint16_t localsCount):
					Scope(from, to, localsCount) {}

			virtual void add(const Operation* operation, const CodeEnvironment& environment) override;

			inline bool isFieldsInitialized() const {
				return fieldsInitialized;
			}
	};


	struct FieldDescriptor {
		const string name;
		const Type& type;

		FieldDescriptor(const NameAndTypeConstant* nameAndType): FieldDescriptor(*nameAndType->name, *nameAndType->descriptor) {}

		FieldDescriptor(const string& name, const string& descriptor): name(name), type(*parseType(descriptor)) {}
	};


	struct Field: Stringified {
		public:
			const uint16_t modifiers;
			const FieldDescriptor& descriptor;
			const Attributes& attributes;

		protected:
			const ConstantValueAttribute* constantValueAttribute;
			mutable const Operation* initializer = nullptr;
			mutable const CodeEnvironment* environment = nullptr;
			friend void StaticInitializerScope::add(const Operation*, const CodeEnvironment&);

		public:
			Field(const ConstantPool& constPool, BinaryInputStream& instream): modifiers(instream.readShort()),
					descriptor(*new FieldDescriptor(constPool.getUtf8Constant(instream.readShort()), constPool.getUtf8Constant(instream.readShort()))),
					attributes(*new Attributes(instream, constPool, instream.readShort())), constantValueAttribute(attributes.get<ConstantValueAttribute>()) {}

			virtual string toString(const ClassInfo& classinfo) const override {
				string str;

				if(const AnnotationsAttribute* annotationsAttribute = attributes.get<AnnotationsAttribute>())
					str += annotationsAttribute->toString(classinfo);

				return str + (string)(modifiersToString(modifiers) + descriptor.type.toString(classinfo)) + ' ' + descriptor.name +
						(constantValueAttribute != nullptr ? " = " + constantValueAttribute->toString(classinfo) :
						initializer != nullptr ? " = " + initializer->toString(*environment) : EMPTY_STRING);
			}

			virtual bool canStringify(const ClassInfo& classinfo) const {
				return !(modifiers & ACC_SYNTHETIC);
			}

			inline bool hasInitializer() const {
				return initializer != nullptr;
			}

			inline const Operation* getInitializer() const {
				return initializer;
			}

		private:
			static FormatString modifiersToString(uint16_t modifiers) {
				FormatString str;

				switch(modifiers & (ACC_VISIBLE | ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED)) {
					case ACC_VISIBLE: break;
					case ACC_PUBLIC: str += "public"; break;
					case ACC_PRIVATE: str += "private"; break;
					case ACC_PROTECTED: str += "protected"; break;
					default: throw IllegalModifiersException(modifiers);
				}

				if(modifiers & ACC_STATIC) str += "static";
				if(modifiers & ACC_FINAL && modifiers & ACC_VOLATILE) throw IllegalModifiersException(modifiers);
				if(modifiers & ACC_FINAL) str += "final";
				if(modifiers & ACC_TRANSIENT) str += "transient";
				if(modifiers & ACC_VOLATILE) str += "volatile";

				return str;
			}
	};



	struct MethodDescriptor {
		public:
			const ReferenceType& clazz;
			const string name;
			const Type* returnType;
			vector<const Type*> arguments;

			enum class MethodType {
				CONSTRUCTOR, STATIC_INITIALIZER, PLAIN
			};

			const MethodType type;

		private: static MethodType typeForName(const string& name) {
			if(name == "<init>") return MethodType::CONSTRUCTOR;
			if(name == "<clinit>") return MethodType::STATIC_INITIALIZER;
			return MethodType::PLAIN;
		}

		public:
			MethodDescriptor(const ReferenceConstant* referenceConstant): MethodDescriptor(*referenceConstant->clazz->name,
					*referenceConstant->nameAndType->name, *referenceConstant->nameAndType->descriptor) {}

			MethodDescriptor(const string& className, const string& name, const string& descriptor):
					MethodDescriptor(*parseReferenceType(className), name, descriptor) {}

			MethodDescriptor(const ReferenceType& clazz, const string& name, const string& descriptor):
					clazz(clazz), name(name), type(typeForName(name)) {
				if(descriptor[0] != '(')
					throw IllegalMethodDescriptorException(descriptor);

				const uint32_t descriptorLength = descriptor.size();

				for(uint32_t i = 1; i < descriptorLength;) {
					const Type* argument;
					switch(descriptor[i]) {
						case 'B': argument = BYTE; break;
						case 'C': argument = CHAR; break;
						case 'S': argument = SHORT; break;
						case 'I': argument = INT; break;
						case 'J': argument = LONG; break;
						case 'F': argument = FLOAT; break;
						case 'D': argument = DOUBLE; break;
						case 'Z': argument = BOOLEAN; break;
						case 'L':
							argument = new ClassType(&descriptor[i + 1]);
							i += argument->getEncodedName().size() + 1;
							goto PushArgument;
						case '[':
							argument = new ArrayType(&descriptor[i]);
							i += argument->getEncodedName().size();
							if(((const ArrayType*)argument)->memberType->isPrimitive())
								goto PushArgument;
							break;
						case ')':
							returnType = parseReturnType(&descriptor[i + 1]);
							goto End;
						default:
							throw IllegalTypeNameException(descriptor);
					}
					i++;
					PushArgument:
					arguments.push_back(argument);
				}

				End:;
			}

			MethodDescriptor(const ReferenceType& clazz, const string& name, const Type& returnType, const initializer_list<const Type*> arguments):
					MethodDescriptor(clazz, name, &returnType, arguments) {}

			MethodDescriptor(const ReferenceType& clazz, const string& name, const Type* returnType, const initializer_list<const Type*> arguments):
					MethodDescriptor(clazz, name, returnType, vector<const Type*>(arguments)) {}

			MethodDescriptor(const ReferenceType& clazz, const string& name, const Type* returnType, const vector<const Type*> arguments):
					clazz(clazz), name(name), returnType(returnType), arguments(arguments), type(typeForName(name)) {}


			virtual string toString(const ClassInfo& classinfo, uint16_t modifiers, const Scope& scope) const {
				const bool isNonStatic = !(modifiers & ACC_STATIC);

				string str = type == MethodType::CONSTRUCTOR ? classinfo.thisType.simpleName : returnType->toString(classinfo) + ' ' + name;

				function<string(const Type*, uint32_t)> concater = [&scope, &classinfo, isNonStatic] (const Type* type, uint32_t i) {
					return type->toString(classinfo) + ' ' + scope.getVariable(i + isNonStatic).getName();
				};

				if(modifiers & ACC_VARARGS) {
					uint32_t varargsIndex;
					for(uint32_t i = arguments.size(); i > 0; ) {
						if(dynamic_cast<const ArrayType*>(arguments[--i])) {
							varargsIndex = i;
							break;
						}
					}

					concater = [&scope, &classinfo, isNonStatic, varargsIndex] (const Type* type, uint32_t i) {
						return (i == varargsIndex ?
								safe_cast<const ArrayType*>(type)->elementType->toString(classinfo) + "..." : type->toString(classinfo)) +
								' ' + scope.getVariable(i + isNonStatic).getName();
					};
				}


				return str + '(' + join<const Type*>(arguments, concater) + ')';
			}


			bool operator==(const MethodDescriptor& other) const {
				return this == &other || this->name == other.name && this->clazz == other.clazz && *this->returnType == *other.returnType &&
						this->arguments.size() == other.arguments.size() &&
						equal(this->arguments.begin(), this->arguments.end(), other.arguments.begin(), [] (auto arg1, auto arg2) { return *arg1 == *arg2; });
			}
	};


	struct Method: Stringified {
		public:
			const uint16_t modifiers;
			const MethodDescriptor& descriptor;
			const Attributes& attributes;
			const CodeAttribute* const codeAttribute;
			const CodeEnvironment& environment;

		protected:
			Scope& scope;

		public:
			typedef MethodDescriptor::MethodType MethodType;

			Method(uint16_t modifiers, const MethodDescriptor& descriptor, const Attributes& attributes, const ClassInfo& classinfo):
					modifiers(modifiers), descriptor(descriptor), attributes(attributes), codeAttribute(attributes.get<CodeAttribute>()),
					environment(decompileCode(classinfo)), scope(environment.scope) {
				const bool hasCodeAttribute = codeAttribute != nullptr;

				if(modifiers & ACC_ABSTRACT && hasCodeAttribute)
					throw IllegalStateException("In method " + descriptor.name + ":\n" + "Abstract method cannot have Code attribute");
				if(modifiers & ACC_NATIVE && hasCodeAttribute)
					throw IllegalStateException("In method " + descriptor.name + ":\n" + "Native method cannot have Code attribute");
				if(!(modifiers & ACC_ABSTRACT) && !(modifiers & ACC_NATIVE) && !hasCodeAttribute)
					throw IllegalStateException("In method " + descriptor.name + ":\n" + "Non-abstract and non-native method must have Code attribute");
			}

			const CodeEnvironment& decompileCode(const ClassInfo& classinfo);

			virtual string toString(const ClassInfo& classinfo) const override {
				string str;

				if(const AnnotationsAttribute* annotationsAttribute = attributes.get<AnnotationsAttribute>())
					str += annotationsAttribute->toString(classinfo);

				if(descriptor.type == MethodType::STATIC_INITIALIZER) {
					if(modifiers != ACC_STATIC)
						throw IllegalModifiersException("0x" + hex<4>(modifiers) + ": static initializer must have only static modifier");
					if(attributes.has<ExceptionsAttribute>())
						throw IllegalAttributeException("static initializer cannot have Exceptions attribute");
					str += "static";
				} else {
					str += modifiersToString(modifiers, classinfo) + descriptor.toString(classinfo, modifiers, scope);

					if(const ExceptionsAttribute* exceptionsAttr = attributes.get<ExceptionsAttribute>())
						str += " throws " + join<const ClassConstant*>(exceptionsAttr->exceptions,
								[&classinfo] (auto clazz) { return ClassType(*clazz->name).toString(classinfo); });

					if(const AnnotationDefaultAttribute* annotationDefaultAttr = attributes.get<AnnotationDefaultAttribute>())
						str += " default " + annotationDefaultAttr->toString(environment.classinfo);
				}

				return str + (codeAttribute == nullptr ? ";" : ' ' + scope.toString(environment));
			}

			virtual bool canStringify(const ClassInfo& classinfo) const override {
				return !(modifiers & (ACC_SYNTHETIC | ACC_BRIDGE) ||
						descriptor.type == MethodType::STATIC_INITIALIZER && scope.isEmpty() || // empty static {}
						descriptor.type == MethodType::CONSTRUCTOR && (modifiers & ACC_PUBLIC ||
							(modifiers & ACC_PUBLIC) == ACC_VISIBLE && (classinfo.modifiers & ACC_PUBLIC) == ACC_VISIBLE) &&
								scope.isEmpty() || // constructor by default
						classinfo.modifiers & ACC_ENUM && (
							descriptor == MethodDescriptor(classinfo.thisType, "valueOf", &classinfo.thisType, {STRING}) || // Enum valueOf(String name)
							descriptor == MethodDescriptor(classinfo.thisType, "values", new ArrayType(classinfo.thisType), {}) || // Enum[] values()
							modifiers & ACC_PRIVATE && descriptor == MethodDescriptor(classinfo.thisType, "<init>", VOID, {}) // enum constructor by default
						));
			}

		private:
			static FormatString modifiersToString(uint16_t modifiers, const ClassInfo& classinfo) {
				FormatString str;

				switch(modifiers & (ACC_VISIBLE | ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED)) {
					case ACC_VISIBLE: break;
					case ACC_PUBLIC: str += "public"; break;
					case ACC_PRIVATE: str += "private"; break;
					case ACC_PROTECTED: str += "protected"; break;
					default: throw IllegalModifiersException(modifiers);
				}

				if(modifiers & ACC_STATIC) str += "static";

				if(modifiers & ACC_ABSTRACT) {
					if(modifiers & (ACC_STATIC | ACC_FINAL | ACC_SYNCHRONIZED | ACC_NATIVE | ACC_STRICT))
						throw IllegalModifiersException(modifiers);
					if(!(classinfo.modifiers & ACC_INTERFACE))
						str += "abstract";
				} else {
					if(classinfo.modifiers & ACC_INTERFACE)
						str += "default";
				}

				if(modifiers & ACC_FINAL) str += "final";
				if(modifiers & ACC_SYNCHRONIZED) str += "synchronized";
				if(modifiers & ACC_NATIVE && modifiers & ACC_STRICT) throw IllegalModifiersException(modifiers);
				if(modifiers & ACC_NATIVE) str += "native";
				if(modifiers & ACC_STRICT) str += "strictfp";

				return str;
			}
	};


	string MethodTypeConstant::toString(const ClassInfo& classinfo) const {
		const MethodDescriptor descriptor(classinfo.thisType, EMPTY_STRING, *this->descriptor);

		return METHOD_TYPE->toString(classinfo) + ".methodType(" + descriptor.returnType->toString(classinfo) + ".class" +
				join<const Type*>(descriptor.arguments, [&classinfo] (auto type) { return ", " + type->toString(classinfo) + ".class"; }, EMPTY_STRING) + ')';
	}



	struct MethodDataHolder {
		public:
			const uint16_t modifiers;
			const MethodDescriptor& descriptor;
			const Attributes& attributes;

			MethodDataHolder(const ConstantPool& constPool, BinaryInputStream& instream, const ClassType& thisType):
					modifiers(instream.readShort()), descriptor(
						*new MethodDescriptor(thisType, constPool.getUtf8Constant(instream.readShort()), constPool.getUtf8Constant(instream.readShort()))),
					attributes(*new Attributes(instream, constPool, instream.readShort())) {}

			MethodDataHolder(uint16_t modifiers, const MethodDescriptor& descriptor, const Attributes& attributes):
					modifiers(modifiers), descriptor(descriptor), attributes(attributes) {}

			const Method* createMethod(const ClassInfo& classinfo) const {
				return new Method(modifiers, descriptor, attributes, classinfo);
			}
	};



	struct Class: Stringified {
		public:
			const ClassType & thisType, & superType;
			const ConstantPool& constPool;
			const uint16_t modifiers;
			const vector<const ClassType*> interfaces;
			const Attributes& attributes;
			const ClassInfo& classinfo;
			const vector<const Field*> fields;
			const vector<const Method*> methods;

		protected:
			Class(const ClassType& thisType, const ClassType& superType, const ConstantPool& constPool, uint16_t modifiers,
					const vector<const ClassType*>& interfaces, const Attributes& attributes, const ClassInfo& classinfo,
					const vector<const Field*>& fields, const vector<const Method*>& methods):
					thisType(thisType), superType(superType), constPool(constPool), modifiers(modifiers),
					interfaces(interfaces), attributes(attributes), classinfo(classinfo),
					fields(fields), methods(methods) {}

			const ClassInfo& createClassInfo() const {
				return *new ClassInfo(*this, thisType, superType, constPool, attributes, modifiers, "    ");
			}

			const vector<const Method*> createMethodsFromMethodData(const vector<MethodDataHolder> methodDataHolders) const {
				vector<const Method*> methods;
				methods.reserve(methodDataHolders.size());
				for(const MethodDataHolder methodData : methodDataHolders) {
					//try {
						methods.push_back(methodData.createMethod(classinfo));
					/*} catch(DecompilationException& ex) {
						const char* message = ex.what();
						cerr << "Exception while decompiling method " + thisType.getName() + '.' + methodData.descriptor.name << ": "
								<< typeid(ex).name() << (*message == '\0' ? "" : (string)": " + message) << endl;
					}*/
				}
				return methods;
			}

			Class(const ClassType& thisType, const ClassType& superType, const ConstantPool& constPool, uint16_t modifiers,
					const vector<const ClassType*>& interfaces, const Attributes& attributes,
					const vector<const Field*>& fields, const vector<MethodDataHolder>& methodDataHolders):
					thisType(thisType), superType(superType), constPool(constPool), modifiers(modifiers),
					interfaces(interfaces), attributes(attributes), classinfo(createClassInfo()),
					fields(fields), methods(createMethodsFromMethodData(methodDataHolders)) {}


		public:
			static const Class& readClass(BinaryInputStream& instream);

			const Field* getField(const string& name) const {
				for(const Field* field : fields)
					if(field->descriptor.name == name)
						return field;
				return nullptr;
			}


			/*const Field* getMethod(const MethodDescriptor& descriptor, bool isStatic) const {
				for(const Method* method : methods)
					if(method->descriptor == descriptor && (bool)(method->modifiers & ACC_STATIC) == isStatic)
						return field;
				return nullptr;
			}*/


			virtual string toString(const ClassInfo& classinfo) const override {
				string str;

				if(const AnnotationsAttribute* annotationsAttribute = attributes.get<AnnotationsAttribute>())
					str += annotationsAttribute->toString(classinfo) + '\n';

				classinfo.increaseIndent();

				str += modifiersToString(modifiers) + ' ' + thisType.simpleName +
						(superType.getName() == "java.lang.Object" || (modifiers & ACC_ENUM && superType.getName() == "java.lang.Enum") ?
								EMPTY_STRING : " extends " + superType.toString(classinfo));

				vector<const ClassType*> interfacesToStringify;
				for(const ClassType* interface : interfaces) {
					if(modifiers & ACC_ANNOTATION && interface->getName() == "java.lang.annotation.Annotation")
						continue;
					interfacesToStringify.push_back(interface);
				}

				if(interfacesToStringify.size() > 0)
					str += " implements " +
							join<const ClassType*>(interfacesToStringify, [&classinfo] (auto interface) { return interface->toString(classinfo); });

				str += " {";

				str += fieldsToString(classinfo);

				str += methodsToString(classinfo);

				classinfo.reduceIndent();

				str += (str.back() == '\n' ? EMPTY_STRING : "\n") + classinfo.getIndent() + '}';


				string headers;

				if(thisType.packageName.size() != 0)
					headers += (string)"package " + thisType.packageName + ";\n\n";

				for(auto imp : classinfo.imports)
					headers += "import " + imp + ";\n";
				if(classinfo.imports.size() > 0) headers += "\n";

				return headers + classinfo.getIndent() + str;
			}

			string toString() const {
				return toString(classinfo);
			}


		protected:
			virtual string fieldsToString(const ClassInfo& classinfo) const {
				string str;

				bool anyFieldStringified = false;

				for(const Field* field : fields)
					if(field->canStringify(classinfo)) {
						str += (string)"\n" + classinfo.getIndent() + field->toString(classinfo) + ';';
						anyFieldStringified = true;
					}
				if(anyFieldStringified)
					str += '\n';

				return str;
			}

			virtual string methodsToString(const ClassInfo& classinfo) const {
				string str;

				for(const Method* method : methods)
					if(method->canStringify(classinfo))
						str += (string)"\n" + classinfo.getIndent() + method->toString(classinfo) + '\n';

				return str;
			}

			static string modifiersToString(uint16_t modifiers) {
				FormatString str;

				switch(modifiers & (ACC_VISIBLE | ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED)) {
					case ACC_VISIBLE: break;
					case ACC_PUBLIC: str += "public"; break;
					case ACC_PRIVATE: str += "private"; break;
					case ACC_PROTECTED: str += "protected"; break;
					default: throw IllegalModifiersException("in class: 0x" + hex<4>(modifiers));
				}

				if(modifiers & ACC_STRICT) str += "strictfp";

				switch(modifiers & (ACC_FINAL | ACC_ABSTRACT | ACC_INTERFACE | ACC_ANNOTATION | ACC_ENUM)) {
					case 0:
						str += "class"; break;
					case ACC_FINAL:
						str += "final class"; break;
					case ACC_ABSTRACT:
						str += "abstract class"; break;
					case ACC_ABSTRACT | ACC_INTERFACE:
						str += "interface"; break;
					case ACC_ABSTRACT | ACC_INTERFACE | ACC_ANNOTATION:
						str += "@interface"; break;
					case ACC_FINAL | ACC_ENUM: case ACC_ABSTRACT | ACC_ENUM:
						str += "enum"; break;
					default:
						throw IllegalModifiersException("in class: 0x" + hex<4>(modifiers));
				}

				return str;
			}
	};


	struct EnumClass final: Class {
		protected:
			struct EnumField: Field {
				const vector<const Operation*> arguments;

				EnumField(const Field& field, const vector<const Operation*>& arguments):
						Field(field), arguments(arguments.begin(), arguments.end() - 2) {}

				virtual string toString(const ClassInfo& classinfo) const override {
					string str;

					if(const AnnotationsAttribute* annotationsAttribute = attributes.get<AnnotationsAttribute>())
						str += annotationsAttribute->toString(classinfo);

					return str + descriptor.name + (arguments.empty() ? EMPTY_STRING :
							'(' + rjoin<const Operation*>(arguments, [this] (auto arg) { return arg->toString(*environment); }) + ')');
				}
			};


			struct EnumConstructorDescriptor: MethodDescriptor {
				EnumConstructorDescriptor(const MethodDescriptor& other):
						MethodDescriptor(other.clazz, other.name, other.returnType, vector<const Type*>(other.arguments.begin() + 2, other.arguments.end())) {}
			};

			vector<const EnumField*> enumFields;
			vector<const Field*> otherFields;

			static vector<MethodDataHolder> processMethodData(vector<MethodDataHolder>& methodDataHolders, const ClassType& thisType) {
				vector<MethodDataHolder> newMethodDataHolders;
				newMethodDataHolders.reserve(methodDataHolders.size());

				for(MethodDataHolder& methodData : methodDataHolders) {
					newMethodDataHolders.push_back(MethodDataHolder(methodData.modifiers,
							methodData.descriptor.type == MethodDescriptor::MethodType::CONSTRUCTOR ?
							*new EnumConstructorDescriptor(methodData.descriptor) : methodData.descriptor, methodData.attributes));
				}

				return newMethodDataHolders;
			}

		public:
			EnumClass(const ClassType& thisType, const ClassType& superType, const ConstantPool& constPool, uint16_t modifiers,
					const vector<const ClassType*>& interfaces, const Attributes& attributes,
					const vector<const Field*>& fields, vector<MethodDataHolder>& methodDataHolders);


		protected:
			virtual string fieldsToString(const ClassInfo& classinfo) const override {
				string str;

				if(enumFields.size() > 0)
					str += (string)"\n" + classinfo.getIndent() +
							join<const EnumField*>(enumFields, [&classinfo] (auto field) { return field->toString(classinfo); }) + ";\n";

				bool anyFieldStringified = false;
				for(const Field* field : otherFields)
					if(field->canStringify(classinfo)) {
						str += (string)"\n" + classinfo.getIndent() + field->toString(classinfo) + ';';
						anyFieldStringified = true;
					}
				if(anyFieldStringified)
					str += '\n';

				return str;
			}
	};




	const Class& Class::readClass(BinaryInputStream& instream) {
		if(instream.readInt() != CLASS_SIGNATURE)
			throw ClassFormatError("Wrong class signature");

		const uint16_t
				minorVersion = instream.readShort(),
				majorVersion = instream.readShort();

		cout << "/* Java version: " << majorVersion << '.' << minorVersion << " */" << endl;

		const uint16_t constPoolSize = instream.readShort();

		const ConstantPool& constPool = *new ConstantPool(constPoolSize);

		for(uint16_t i = 1; i < constPoolSize; i++) {
			uint8_t constType = instream.readByte();

			switch(constType) {
				case  1: {
					uint16_t size = instream.readShort();
					const char* bytes = instream.readBytes(size);
					constPool[i] = new Utf8Constant(bytes, size);
					delete[] bytes;
					break;
				}
				case  3:
					constPool[i] = new IntegerConstant(instream.readInt());
					break;
				case  4:
					constPool[i] = new FloatConstant(instream.readFloat());
					break;
				case  5:
					constPool[i] = new LongConstant(instream.readLong());
					i++; // Long and Double constants have historically held two positions in the pool
					break;
				case  6:
					constPool[i] = new DoubleConstant(instream.readDouble());
					i++;
					break;
				case  7:
					constPool[i] = new ClassConstant(instream.readShort());
					break;
				case  8:
					constPool[i] = new StringConstant(instream.readShort());
					break;
				case  9:
					constPool[i] = new FieldrefConstant(instream.readShort(), instream.readShort());
					break;
				case 10:
					constPool[i] = new MethodrefConstant(instream.readShort(), instream.readShort());
					break;
				case 11:
					constPool[i] = new InterfaceMethodrefConstant(instream.readShort(), instream.readShort());
					break;
				case 12:
					constPool[i] = new NameAndTypeConstant(instream.readShort(), instream.readShort());
					break;
				case 15:
					constPool[i] = new MethodHandleConstant(instream.readByte(), instream.readShort());
					break;
				case 16:
					constPool[i] = new MethodTypeConstant(instream.readShort());
					break;
				case 18:
					constPool[i] = new InvokeDynamicConstant(instream.readShort(), instream.readShort());
					break;
				default:
					throw ClassFormatError("Illegal constant type 0x" + hex<2>(constType) + " at index #" + to_string(i) +
							" at pos 0x" + hex((int32_t)instream.getPos()));
			};
		}

		for(uint16_t i = 1; i < constPoolSize; i++) {
			Constant* constant = constPool[i];
			if(constant != nullptr)
				constant->init(constPool);
		}

		const uint16_t modifiers = instream.readShort();

		const ClassType
				& thisType = *new ClassType(*constPool.get<ClassConstant>(instream.readShort())->name),
				& superType = *new ClassType(*constPool.get<ClassConstant>(instream.readShort())->name);

		const uint16_t interfacesCount = instream.readShort();
		vector<const ClassType*> interfaces;
		interfaces.reserve(interfacesCount);
		for(uint16_t i = 0; i < interfacesCount; i++)
			interfaces.push_back(new ClassType(*constPool.get<ClassConstant>(instream.readShort())->name));


		const uint16_t fieldsCount = instream.readShort();
		vector<const Field*> fields;
		fields.reserve(fieldsCount);

		for(uint16_t i = 0; i < fieldsCount; i++)
			fields.push_back(new Field(constPool, instream));


		const uint16_t methodsCount = instream.readShort();
		vector<MethodDataHolder> methodDataHolders;
		methodDataHolders.reserve(methodsCount);

		for(uint16_t i = 0; i < methodsCount; i++)
			methodDataHolders.push_back(MethodDataHolder(constPool, instream, thisType));

		return *(modifiers & ACC_ENUM ?
				new EnumClass(thisType, superType, constPool, modifiers, interfaces,
						*new Attributes(instream, constPool, instream.readShort()), fields, methodDataHolders) :
				new Class(thisType, superType, constPool, modifiers, interfaces,
						*new Attributes(instream, constPool, instream.readShort()), fields, methodDataHolders));
	}





	// --------------------------------------------------

	CodeEnvironment::CodeEnvironment(const Bytecode& bytecode, const ClassInfo& classinfo, Scope* scope, uint16_t modifiers,
			const MethodDescriptor& descriptor, const Attributes& attributes, uint32_t codeLength, uint16_t maxLocals):
			bytecode(bytecode), classinfo(classinfo), constPool(classinfo.constPool), stack(*new Stack()),
			scope(*scope), currentScope(scope), modifiers(modifiers), descriptor(descriptor), attributes(attributes) {
		for(uint32_t i = scope->getVariablesCount(); i < maxLocals; i++)
			scope->addVariable(new UnnamedVariable(&AnyType::getInstance()));

		//LOG(maxLocals << ' ' << scope->getVariablesCount());

		// DEBUG
		/*for(Attribute* attribute : *method->attributes)
			LOG("Attribute " << attribute->name);

		for(Attribute* attribute : *method->codeAttribute->attributes)
			LOG("Code attribute " << attribute->name);*/
	}


	void CodeEnvironment::checkCurrentScope() {
		while(index >= currentScope->to) {
			if(currentScope->parentScope == nullptr)
				throw DecompilationException("Unexpected end of global function scope {" +
						to_string(currentScope->from) + ".." + to_string(currentScope->to) + '}');
			Scope* finalizingCurrentScope = currentScope;
			currentScope = currentScope->parentScope;
			finalizingCurrentScope->finalize(*this);
		}

		for(auto i = scopes.begin(); i != scopes.end(); ) {
			Scope* scope = *i;
			if(scope->from <= index) {
				if(scope->to > currentScope->to)
					throw DecompilationException("Scope is out of bounds of the parent scope: " +
						to_string(scope->from) + ".." + to_string(scope->to) + ", " + to_string(currentScope->from) + ".." + to_string(currentScope->to));
				currentScope->add(scope, *this);
				//scope->initiate(*this);
				currentScope = scope;
				scopes.erase(i);
			} else
				++i;
		}
	}

	inline Scope* CodeEnvironment::getCurrentScope() const {
		return currentScope;
	}
}

#endif
