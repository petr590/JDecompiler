#ifndef JDECOMPILER_MAIN_CPP
#define JDECOMPILER_MAIN_CPP

#undef inline
#include <iostream>
#include <vector>
#include <set>
#define inline FORCE_INLINE

#include "instructions.cpp"

namespace jdecompiler {

	EnumClass::EnumClass(const Version& version, const ClassType& thisType, const ClassType& superType, const ConstantPool& constPool, uint16_t modifiers,
			const vector<const ClassType*>& interfaces, const Attributes& attributes,
			const vector<const Field*>& fields, vector<MethodDataHolder>& methodDataHolders):
			Class(version, thisType, superType, constPool, modifiers, interfaces, attributes, fields, processMethodData(methodDataHolders)) {

		using namespace operations;

		for(const Field* field : fields) {
			const InvokespecialOperation* invokespecialOperation;
			const DupOperation<TypeSize::FOUR_BYTES>* dupOperation;
			const NewOperation* newOperation;
			if(field->modifiers == (ACC_PUBLIC | ACC_STATIC | ACC_FINAL) && field->descriptor.type == thisType &&
					field->hasInitializer() &&
					(invokespecialOperation = dynamic_cast<const InvokespecialOperation*>(field->getInitializer())) != nullptr &&
					(dupOperation = dynamic_cast<const DupOperation<TypeSize::FOUR_BYTES>*>(invokespecialOperation->object)) != nullptr &&
					(newOperation = dynamic_cast<const NewOperation*>(dupOperation->operation)) != nullptr) {
				if(invokespecialOperation->arguments.size() < 2)
					throw DecompilationException("enum constant initializer should have at least two arguments, got " +
							to_string(invokespecialOperation->arguments.size()));
				enumFields.push_back(new EnumField(*field, invokespecialOperation->arguments));
			} else {
				otherFields.push_back(field);
			}
		}
	}


	CodeEnvironment::CodeEnvironment(const Bytecode& bytecode, const ClassInfo& classinfo, MethodScope* methodScope, uint16_t modifiers,
			const MethodDescriptor& descriptor, const Attributes& attributes, uint16_t maxLocals):
			bytecode(bytecode), classinfo(classinfo), constPool(classinfo.constPool), stack(*new CodeStack()),
			methodScope(*methodScope), currentScope(methodScope), modifiers(modifiers), descriptor(descriptor), attributes(attributes) {
		for(uint32_t i = methodScope->getVariablesCount(); i < maxLocals; i++)
			methodScope->addVariable(new UnnamedVariable(AnyType::getInstance(), false));
	}


	void CodeEnvironment::updateScopes() {
		while(index == currentScope->end()) {
			if(currentScope->parentScope == nullptr)
				throw DecompilationException("Unexpected end of global function scope " + currentScope->toDebugString());
			currentScope->finalize(*this);
			LOG("End of " << currentScope->toDebugString());
			currentScope = currentScope->parentScope;
		}

		for(auto i = inactiveScopes.begin(); i != inactiveScopes.end(); ) {
			const Scope* scope = *i;
			if(scope->start() <= index) {
				if(scope->end() > currentScope->end())
					throw DecompilationException((string)"Scope " + scope->toDebugString() +
							" is out of bounds of the parent scope " + currentScope->toDebugString());
				if(index > scope->start())
					throw IllegalStateException("Scope " + currentScope->toDebugString() + " is added after it starts");

				LOG("Start of " << scope->toDebugString());
				currentScope->addOperation(scope, *this);
				currentScope = scope;
				//scope->initiate(*this);
				inactiveScopes.erase(i);
			} else
				++i;
		}
	}
}


#include "decompile.cpp"

namespace jdecompiler {
	string ClassInfo::importsToString() const {
		string str;

		for(const ClassType* clazz : imports) {
			if(clazz->packageName != "java.lang" && clazz->packageName != thisType.packageName) {
				str += (string)this->getIndent() + "import " + clazz->getName() + ";\n";
			}
		}

		return str.empty() ? str : str + '\n';
	}

	/* Returns true if we can write simple class name */
	bool ClassInfo::addImport(const ClassType* clazz) const {
		if(clazz->simpleName == thisType.simpleName && *clazz != thisType) {
			return false;
		}

		if(any_of(imports.begin(), imports.end(), [clazz] (const ClassType* imp) { return *clazz == *imp; })) { // find class
			return true;
		} else {
			if(all_of(imports.begin(), imports.end(), [clazz] (const ClassType* imp)
					{ return imp->simpleName != clazz->simpleName; })) { // check has no class with same name
				imports.insert(clazz);
				return true;
			}
			return false;
		}
	}

	string ClassType::toString(const ClassInfo& classinfo) const {
		return this->isAnonymous ? fullSimpleName : (classinfo.addImport(this) ? simpleName : name);
	}

	void JDecompiler::readClassFiles() const {
		for(BinaryInputStream* file : files) {
			#ifndef FAIL_ON_ERROR
			try {
			#endif
				const Class* clazz = Class::readClass(*file);
				classes[clazz->thisType.getEncodedName()] = clazz;
			#ifndef FAIL_ON_ERROR
			}
			  catch(const EOFException& ex) {
				JDecompiler::getInstance().error("unexpected end of file while reading ", file->path);
			} catch(const IOException& ex) {
				JDecompiler::getInstance().error(ex.getName(), ": ", ex.what());
			} catch(const DecompilationException& ex) {
				JDecompiler::getInstance().error(ex.getName(), ": ", ex.what());
			}
			#endif
		}
	}
}


int main(int argc, const char* args[]) {
	using namespace jdecompiler;

	cout << boolalpha;
	cerr << boolalpha;

	if(!JDecompiler::init(argc, args))
		return 0;

	//LOG("isFailOnError = " << JDecompiler::getInstance().isFailOnError());

	JDecompiler::getInstance().readClassFiles();

	for(const auto& clazz : JDecompiler::getInstance().getClasses()) {
		if(clazz.second->canStringify())
			cout << clazz.second->toString() << endl;
	}
}

#undef inline

#endif
