#ifndef JDECOMPILER_MAIN_CPP
#define JDECOMPILER_MAIN_CPP

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include "jdecompiler.h"
#include "jdecompiler-util.cpp"
#include "jdecompiler-const-pool.cpp"

#undef LOG_PREFIX
#define LOG_PREFIX "[ jdecompiler-main.cpp ]"

#define CLASS_SIGNATURE 0xCAFEBABE

#define ACC_VISIBLE      0x0000 // class, field, method
#define ACC_PUBLIC       0x0001 // class, field, method
#define ACC_PRIVATE      0x0002 // class, field, method
#define ACC_PROTECTED    0x0004 // class, field, method
#define ACC_STATIC       0x0008 // nested class, field, method
#define ACC_FINAL        0x0010 // class, field, method
#define ACC_SYNCHRONIZED 0x0020 // method, block
#define ACC_SUPER        0x0020 // class (deprecated)
#define ACC_VOLATILE     0x0040 // field
#define ACC_BRIDGE       0x0040 // method
#define ACC_TRANSIENT    0x0080 // field
#define ACC_VARARGS      0x0080 // args
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
		public: virtual string toString(const ClassInfo& classinfo) const = 0;
	};


	struct ClassInfo {
		public:
			const ClassType * const type, * const superType;
			const ConstantPool& constPool;
			const Attributes& attributes;
			const uint16_t modifiers;

			set<string>& imports;

			ClassInfo(const ClassType* type, const ClassType* superType, const ConstantPool& constPool, const Attributes& attributes, const uint16_t modifiers, const char* baseIndent): type(type), superType(superType), constPool(constPool), attributes(attributes), modifiers(modifiers), imports(*new set<string>()), baseIndent(baseIndent) {}

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

	#include "jdecompiler-types.cpp"
	#include "jdecompiler-javase.cpp"
	#include "jdecompiler-attributes.cpp"


	string ClassConstant::toString(const ClassInfo& classinfo) const {
		return ClassType(*name).toString(classinfo);
	}


	struct Variable {
		const Type* type;
		string name;

		Variable(const Type* type, string name): type(type), name(name) {}
	};


	enum class Associativity { LEFT, RIGHT };


	struct Operation {
		const uint16_t priority;

		Operation(uint16_t priority = 15): priority(priority) {}

		virtual string toString(const CodeEnvironment& environment) const = 0;

		virtual const Type* getReturnType() const = 0;

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

			const vector<Instruction*>& getInstructions() const {
				return instructions;
			}

			const vector<uint32_t>& getPosMap() const {
				return posMap;
			}

			inline int16_t nextByte() {
				return bytes[++pos] & 0xFF;
			}

			inline uint16_t nextUByte() {
				return bytes[++pos] & 0xFF;
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

			inline Instruction* getInstruction(uint32_t index) const {
				return instructions[index];
			}

			uint32_t posToIndex(uint32_t pos) const {
				uint32_t i = 0, index = 0;
				const uint32_t max = posMap.size();
				while(index != pos && i < max)
					index = posMap[++i];
				if(index != pos) {
					LOG(join<uint32_t>(posMap, [] (uint32_t index) { return to_string(index); }));
					throw IndexOutOfBoundsException(to_string(pos));
				}
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

		public:
			void push(const Operation* operation) {
				firstEntry = new Entry(operation, firstEntry);
				length++;
			}

			void push(initializer_list<const Operation*> operations) {
				for(const Operation* operation : operations)
					push(operation);
			}

			const Operation* pop() {
				const Entry copiedEntry = *firstEntry;
				delete firstEntry;
				firstEntry = copiedEntry.next;
				length--;
				return copiedEntry.value;
			}

			template<class T>
			const T* pop() {
				const Operation* operation = pop();
				if(const T* t = dynamic_cast<const T*>(operation))
					return t;
				throw DecompilationException((string)"Illegal operation type " + typeid(T).name() + " for operation " + typeid(*operation).name());
			}

			const Operation* top() const {
				return firstEntry->value;
			}

			const Operation* lookup(uint16_t index) const {
				if(index < 0) throw IndexOutOfBoundsException(to_string(index));
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
			//const Method* method; // @Unused
			const ConstantPool& constPool;
			Stack& stack;
			Scope* const scope, *currentScope;
			const Attributes& attributes;
			const ClassInfo& classinfo;
			uint32_t pos, index, exprStartIndex;

		private: vector<Scope*> scopes;

		public:
			CodeEnvironment(const Bytecode& bytecode, const ConstantPool& constPool, Scope* scope, const Attributes& attributes, uint32_t codeLength, uint16_t maxLocals, const ClassInfo& classinfo);

			void checkCurrentScope();

			inline Scope* getCurrentScope() const;

			void addScope(Scope* scope);

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

			Variable* getVariable(uint32_t index) {
				return index >= variables.size() && parentScope != nullptr ? parentScope->getVariable(index) : variables[index];
			}

			bool hasVariable(string name) {
				for(Variable* var : variables)
					if(name == var->name)
						return true;
				return parentScope == nullptr ? false : parentScope->hasVariable(name);
			}

			Variable* addVariable(const Type* type, string name) {
				int n = 1;
				const string basename = name;

				while(hasVariable(name))
					name = basename + to_string(++n);

				Variable* var = new Variable(type, name);
				variables.push_back(var);
				return var;
			}

			virtual string toString(const CodeEnvironment& environment) const override {
				environment.classinfo.increaseIndent();

				string str = getHeader(environment) + "{\n";
				const size_t baseSize = str.size();

				for(auto i = code.begin(); i != code.end(); ++i) {
					if(printNextOperation(this, i)) {
						const Operation* operation = *i;
						str += operation->getFrontSeparator(environment.classinfo) + operation->toString(environment) +
								operation->getBackSeparator(environment.classinfo);
					}
				}

				environment.classinfo.reduceIndent();

				if(str.size() == baseSize) {
					str[baseSize - 1] = '}';
					return str;
				}

				return str + environment.classinfo.getIndent() + "}";
			}

		protected:
			virtual inline bool printNextOperation(const Scope* currentScope, const vector<const Operation*>::const_iterator i) const { return true; }

			virtual inline string getBackSeparator(const ClassInfo& classinfo) const override {
				return "\n";
			}

			virtual string getHeader(const CodeEnvironment& environment) const {
				return EMPTY_STRING;
			}

		public:
			int getVariablesCount() const {
				return variables.size();
			}

			void add(const Operation* operation) {
				code.push_back(operation);
			}

			virtual const Type* getReturnType() const { return VOID; }
	};


	static string getRawNameByType(const Type* const type, bool& unchecked) {
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
			return toLowerCamelCase(arrayType->memberType->name) + "Array";
		}
		return "o";
	}


	static string getNameByType(const Type* const type) {
		bool unchecked = false;

		const string name = getRawNameByType(type, unchecked);
		if(unchecked) {
			static map<const char*, const char*> keywords {
				{"boolean", "bool"}, {"byte", "b"}, {"char", "ch"}, {"short", "sh"}, {"int", "n"}, {"long", "l"},
				{"float", "f"}, {"double", "d"}, {"void", "v"},
				{"public", "pub"}, {"protected", "prot"}, {"private", "priv"}, {"static", "stat"}, {"final", "f"}, {"abstract", "abs"},
				{"transient", "trans"}, {"volatile", "vol"}, {"native", "nat"}, {"synchronized", "sync"},
				{"class", "clazz"}, {"interface", "interf"}, {"enum", "en"}, {"this", "t"}, {"super", "s"}, {"extends", "ext"}, {"implements", "impl"},
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


	// --------------------------------------------------


	struct Field: Stringified {
		const uint16_t modifiers;
		const string name;
		const Type& descriptor;
		const Attributes& attributes;

		public:
			Field(const ConstantPool& constPool, BinaryInputStream& instream): modifiers(instream.readShort()),
					name(constPool.getUtf8Constant(instream.readShort())),
					descriptor(*parseType(constPool.getUtf8Constant(instream.readShort()))), attributes(*new Attributes(instream, constPool, instream.readShort())) {}

			virtual string toString(const ClassInfo& classinfo) const override {
				return modifiersToString(modifiers) + " " + descriptor.toString(classinfo) + " " + name;
			}

		private:
			static string modifiersToString(uint16_t modifiers) {
				FormatString str;

				switch(modifiers & (ACC_VISIBLE | ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED)) {
					case ACC_VISIBLE: break;
					case ACC_PUBLIC: str += "public"; break;
					case ACC_PRIVATE: str += "private"; break;
					case ACC_PROTECTED: str += "protected"; break;
					default: throw IllegalModifersException(modifiers);
				}

				if(modifiers & ACC_STATIC) str += "static";
				if(modifiers & ACC_FINAL && modifiers & ACC_VOLATILE) throw IllegalModifersException(modifiers);
				if(modifiers & ACC_FINAL) str += "final";
				if(modifiers & ACC_TRANSIENT) str += "transient";
				if(modifiers & ACC_VOLATILE) str += "volatile";

				return str;
			}
	};



	struct MethodDescriptor {
		const string name;
		const Type* returnType;
		vector<const Type*> arguments;

		MethodDescriptor(const NameAndTypeConstant* nameAndType): MethodDescriptor(*nameAndType->name, *nameAndType->descriptor) {}

		MethodDescriptor(const string& name, const string descriptor): name(name) {
			if(descriptor[0] != '(')
				throw IllegalMethodDescriptorException(descriptor);

			const int descriptorLength = descriptor.size();

			int i = 1;

			while(i < descriptorLength) {
				const Type* argument;
				switch(descriptor[i]) {
					case 'B': argument = BYTE; break;
					case 'C': argument = CHAR; break;
					case 'S': argument = SHORT; break;
					case 'I': argument = INT; break;
					case 'J': argument = LONG; break;
					case 'D': argument = DOUBLE; break;
					case 'F': argument = FLOAT; break;
					case 'Z': argument = BOOLEAN; break;
					case 'L':
						argument = new ClassType(&descriptor[i + 1]);
						i += argument->encodedName.size() + 1;
						goto PushArgument;
					case '[':
						argument = new ArrayType(&descriptor[i]);
						i += argument->encodedName.size();
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
	};


	struct Method: Stringified {
		const uint16_t modifiers;
		const MethodDescriptor& descriptor;
		const Attributes& attributes;
		const CodeAttribute* const codeAttribute;

		protected:
			Scope* scope;

		public:
			Method(const ConstantPool& constPool, BinaryInputStream& instream, const Type* thisType): modifiers(instream.readShort()), descriptor(*new MethodDescriptor(constPool.getUtf8Constant(instream.readShort()), constPool.getUtf8Constant(instream.readShort()))), attributes(*new Attributes(instream, constPool, instream.readShort())), codeAttribute(attributes.get<CodeAttribute>()) {
				const bool hasCodeAttribute = codeAttribute != nullptr;

				if(modifiers & ACC_ABSTRACT && hasCodeAttribute)
					throw IllegalStateException("In method " + descriptor.name + ":\n" + "Abstract method mustn't have Code attribute");
				if(!(modifiers & ACC_ABSTRACT) && !hasCodeAttribute)
					throw IllegalStateException("In method " + descriptor.name + ":\n" + "Non-abstract method must have Code attribute");

				scope = new Scope(0, hasCodeAttribute ? codeAttribute->codeLength : 0, hasCodeAttribute ? 0 : descriptor.arguments.size());
				if(!(modifiers & ACC_STATIC))
					scope->addVariable(thisType, "this");

				const int argumentsCount = descriptor.arguments.size();
				for(int i = 0; i < argumentsCount; i++) {
					scope->addVariable(descriptor.arguments[i], getNameByType(descriptor.arguments[i]));
				}
			}

			virtual string toString(const ClassInfo& classinfo) const override {
				string str;

				const AnnotationsAttribute* annotationsAttribute = attributes.get<const AnnotationsAttribute>();
				if(annotationsAttribute != nullptr)
					str += annotationsAttribute->toString(classinfo);

				const bool isNonStatic = !(modifiers & ACC_STATIC);

				str += modifiersToString(modifiers, classinfo) + (descriptor.name == "<init>" ? classinfo.type->simpleName : descriptor.name == "<clinit>" ? EMPTY_STRING : descriptor.returnType->toString(classinfo) + " " + descriptor.name);

				str += "(" + join<const Type*>(descriptor.arguments, [this, &classinfo, isNonStatic] (const Type* type, uint32_t i) -> string {
					return type->toString(classinfo) + " " + scope->getVariable(i + isNonStatic)->name;
				}) + ")";

				if(attributes.has<const ExceptionsAttribute>())
					str += " throws " + join<const ClassConstant*>(attributes.get<const ExceptionsAttribute>()->exceptions, [&classinfo] (const ClassConstant* clazz) { return ClassType(*clazz->name).toString(classinfo); });

				str += (codeAttribute == nullptr ? ";" : " " + decompileCode(classinfo.constPool, attributes, codeAttribute, scope, classinfo));
				return str;
			}

		private:
			static FormatString modifiersToString(uint16_t modifiers, const ClassInfo& classinfo) {
				FormatString str;

				switch(modifiers & (ACC_VISIBLE | ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED)) {
					case ACC_VISIBLE: break;
					case ACC_PUBLIC: str += "public"; break;
					case ACC_PRIVATE: str += "private"; break;
					case ACC_PROTECTED: str += "protected"; break;
					default: throw IllegalModifersException(modifiers);
				}

				if(modifiers & ACC_STATIC) str += "static";

				if(modifiers & ACC_ABSTRACT) {
					if(modifiers & (ACC_STATIC | ACC_FINAL | ACC_SYNCHRONIZED | ACC_NATIVE | ACC_STRICT))
						throw IllegalModifersException(modifiers);
					if(!(classinfo.modifiers & ACC_INTERFACE))
						str += "abstract";
				} else {
					if(classinfo.modifiers & ACC_INTERFACE)
						str += "default";
				}

				if(modifiers & ACC_FINAL) str += "final";
				if(modifiers & ACC_SYNCHRONIZED) str += "synchronized";
				// ACC_BRIDGE, ACC_VARARGS
				if(modifiers & ACC_NATIVE && modifiers & ACC_STRICT) throw IllegalModifersException(modifiers);
				if(modifiers & ACC_NATIVE) str += "native";
				if(modifiers & ACC_STRICT) str += "strictfp";

				return str;
			}
	};


	string MethodTypeConstant::toString(const ClassInfo& classinfo) const {
		const MethodDescriptor descriptor(EMPTY_STRING, *this->descriptor);

		return METHOD_TYPE->toString(classinfo) + ".methodType(" + descriptor.returnType->toString(classinfo) + ".class" +
				join<const Type*>(descriptor.arguments, [&classinfo] (auto type) { return ", " + type->toString(classinfo) + ".class"; }, EMPTY_STRING) + ")";
	}


	struct Class: Stringified {
		public:
			const ClassType *thisType, *superType;
			uint16_t modifiers;
			uint16_t interfacesCount;
			vector<const ClassType*> interfaces;
			vector<Field*> fields;
			vector<Method*> methods;
			const Attributes* const attributes = 0;

		private: const ConstantPool* constPool;

		public:
			Class(BinaryInputStream& instream) {
				if(instream.readInt() != CLASS_SIGNATURE)
					throw ClassFormatException("Wrong class signature");

				const uint16_t
						majorVersion = instream.readShort(),
						minorVersion = instream.readShort();

				cout << "/* Java version: " << majorVersion << "." << minorVersion << " */" << endl;

				const uint16_t constPoolSize = instream.readShort();

				const ConstantPool& constPool = *(this->constPool = new ConstantPool(constPoolSize));

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
							throw ClassFormatException("Illegal constant type 0x" + hex<2>(constType) + " at pos 0x" + hex((int32_t)instream.getPos()));
					};
				}

				for(uint16_t i = 1; i < constPoolSize; i++) {
					Constant* constant = constPool[i];
					if(constant != nullptr)
						constant->init(constPool);
				}

				modifiers = instream.readShort();

				thisType = new ClassType(*constPool.get<ClassConstant>(instream.readShort())->name);
				superType = new ClassType(*constPool.get<ClassConstant>(instream.readShort())->name);

				interfacesCount = instream.readShort();
				interfaces.reserve(interfacesCount);
				for(uint16_t i = 0; i < interfacesCount; i++) {
					string name = *constPool.get<ClassConstant>(instream.readShort())->name;
					if(modifiers & ACC_ANNOTATION && name == "java/lang/annotation/Annotation") continue;
					interfaces[i] = new ClassType(name);
				}
				interfacesCount = interfaces.size();


				const uint16_t fieldsCount = instream.readShort();
				fields.reserve(fieldsCount);

				for(uint16_t i = 0; i < fieldsCount; i++) {
					fields.push_back(new Field(constPool, instream));
				}


				const uint16_t methodsCount = instream.readShort();
				methods.reserve(methodsCount);

				for(uint16_t i = 0; i < methodsCount; i++) {
					methods.push_back(new Method(constPool, instream, thisType));
				}

				*((const Attributes**)&attributes) = new Attributes(instream, constPool, instream.readShort());
			}


			virtual string toString(const ClassInfo& classinfo) const override {
				classinfo.increaseIndent();

				string str = modifiersToString(modifiers) + " " + thisType->simpleName + (superType->name == (string)"java.lang.Object" ? "" : (string)" extends " + superType->toString(classinfo));

				if(interfacesCount > 0) {
					str += " implements ";
					for(int i = 0; true; ) {
						str += interfaces[i]->toString(classinfo);
						if(++i == interfacesCount) break;
						str += ", ";
					}
				}

				str += " {";
				size_t baseSize = str.size();

				for(size_t i = 0; i < fields.size(); i++)
					str += (string)"\n" + classinfo.getIndent() + fields[i]->toString(classinfo) + ";";

				//LOG(methods.size() << endl);
				for(size_t i = 0; i < methods.size(); i++) {
					//LOG(methods[i] << endl);
					str += (string)"\n\n" + classinfo.getIndent() + methods[i]->toString(classinfo);
				}


				classinfo.reduceIndent();

				str += (baseSize == str.size() ? EMPTY_STRING : (string)"\n") + classinfo.getIndent() + "}";


				string headers = EMPTY_STRING;

				if(thisType->packageName.size() != 0)
					headers += (string)"package " + thisType->packageName + ";\n\n";

				for(auto imp : classinfo.imports)
					headers += "import " + imp + ";\n";
				if(classinfo.imports.size() > 0) headers += "\n";

				return headers + classinfo.getIndent() + str;
			}

			string toString() {
				const ClassInfo classinfo(thisType, superType, *constPool, *attributes, modifiers, "    ");
				return toString(classinfo);
			}



			static string modifiersToString(uint16_t modifiers) {
				FormatString str;

				switch(modifiers & (ACC_VISIBLE | ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED)) {
					case ACC_VISIBLE: break;
					case ACC_PUBLIC: str += "public"; break;
					case ACC_PRIVATE: str += "private"; break;
					case ACC_PROTECTED: str += "protected"; break;
					default: throw IllegalModifersException(modifiers);
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
					case ACC_ENUM: case ACC_ABSTRACT | ACC_ENUM:
						str += "enum"; break;
					default:
						throw IllegalModifersException(modifiers);
				}

				return str;
			}
	};

	// --------------------------------------------------

	CodeEnvironment::CodeEnvironment(const Bytecode& bytecode, const ConstantPool& constPool, Scope* scope, const Attributes& attributes, uint32_t codeLength, uint16_t maxLocals, const ClassInfo& classinfo): bytecode(bytecode), constPool(constPool), stack(*new Stack()), scope(scope), currentScope(scope), attributes(attributes), classinfo(classinfo) {
		for(int i = scope->getVariablesCount(); i < maxLocals; i++)
			scope->addVariable(ANY_OBJECT /*TODO*/, "x");

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
				throw DecompilationException("Unexpected end of global function scope");
			currentScope = currentScope->parentScope;
		}

		for(auto i = scopes.begin(); i != scopes.end(); ) {
			Scope* scope = *i;
			if(scope->from <= index) {
				if(scope->to > currentScope->to)
					throw DecompilationException("Scope is out of bounds of the parent scope: " +
						to_string(scope->from) + " - " + to_string(scope->to) + ", " + to_string(currentScope->from) + " - " + to_string(currentScope->to));
				currentScope->add(scope);
				currentScope = scope;
				scopes.erase(i);
			} else
				++i;
		}
	}

	inline Scope* CodeEnvironment::getCurrentScope() const {
		return currentScope;
	}

	void CodeEnvironment::addScope(Scope* scope) {
		scopes.push_back(scope);
	}
}

#endif
