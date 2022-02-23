#ifndef JDECOMPILER_MAIN_CPP
#define JDECOMPILER_MAIN_CPP

#undef inline
#include <iostream>
#include <vector>
#include <set>
#include "jdecompiler.h"
#include "util.cpp"
#include "jdecompiler.cpp"
#include "const-pool.cpp"
#define inline FORCE_INLINE

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

namespace jdecompiler {

	using namespace std;


	struct Stringified {
		public:
			virtual string toString(const ClassInfo& classinfo) const = 0;

			virtual bool canStringify(const ClassInfo& classinfo) const {
				return true;
			}

			virtual ~Stringified() {}
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

#include "types.cpp"
#include "attributes.cpp"

#include "code.cpp"

#include "field.cpp"
#include "method.cpp"
#include "class.cpp"

namespace jdecompiler {

	CodeEnvironment::CodeEnvironment(const Bytecode& bytecode, const ClassInfo& classinfo, MethodScope* methodScope, uint16_t modifiers,
			const MethodDescriptor& descriptor, const Attributes& attributes, uint16_t maxLocals):
			bytecode(bytecode), classinfo(classinfo), constPool(classinfo.constPool), stack(*new CodeStack()),
			methodScope(*methodScope), currentScopes(methodScope), modifiers(modifiers), descriptor(descriptor), attributes(attributes) {
		for(uint32_t i = methodScope->getVariablesCount(); i < maxLocals; i++)
			methodScope->addVariable(new UnnamedVariable(&AnyType::getInstance()));
	}


	void CodeEnvironment::checkCurrentScope() {
		for(Scope* currentScope = currentScopes.top(); index >= currentScope->to; currentScope = currentScopes.top()) {
			if(currentScope->parentScope == nullptr)
				throw DecompilationException("Unexpected end of global function scope {" +
						to_string(currentScope->from) + ".." + to_string(currentScope->to) + '}');
			currentScopes.pop();
			currentScope->finalize(*this);
		}

		Scope* currentScope = getCurrentScope();

		//LOG("scopes.size() = " << scopes.size())
		for(auto i = scopes.begin(); i != scopes.end(); ) {
			Scope* scope = *i;
			if(scope->from <= index) {
				if(scope->to > currentScope->to)
					throw DecompilationException("Scope is out of bounds of the parent scope: " +
						to_string(scope->from) + ".." + to_string(scope->to) + ", " + to_string(currentScope->from) + ".." + to_string(currentScope->to));
				currentScope->add(scope, *this);
				//scope->initiate(*this);
				currentScopes.push(currentScope = scope);
				scopes.erase(i);
			} else
				++i;
		}
	}
}


#include "bytecode-instructions.cpp"

namespace jdecompiler {
	string ClassType::toString(const ClassInfo& classinfo) const {
		if(packageName != "java.lang" && packageName != classinfo.thisType.packageName)
			classinfo.imports.insert(name);
		if(isAnonymous) {
			const Class* clazz = JDecompiler::instance.getClass(encodedName);
			return clazz != nullptr ? clazz->toString(classinfo) : fullSimpleName;
		}
		return simpleName;
	}

	void JDecompiler::readClassFiles() const {
		for(BinaryInputStream* file : files) {
			const Class* clazz = Class::readClass(*file);
			LOG(clazz->thisType.getEncodedName());
			classes[clazz->thisType.getEncodedName()] = clazz;
		}
	}
}


int main(int argc, const char* args[]) {
	using namespace std;
	using namespace jdecompiler;

	if(!JDecompiler::parseConfig(argc, args))
		return 0;

	LOG("PRE");
	JDecompiler::instance.readClassFiles();
	LOG("POST");

	for(const auto& clazz : JDecompiler::instance.getClasses()) {
		LOG(clazz.second->thisType.getEncodedName());
		cout << clazz.second->toString() << endl;
	}
}

#undef inline

#endif
