#ifndef JDECOMPILER_MAIN_CPP
#define JDECOMPILER_MAIN_CPP

#undef inline
#include <iostream>
#include <vector>
#include <set>
#define inline FORCE_INLINE

#include "instructions.cpp"

namespace jdecompiler {

	EnumClass::EnumClass(const ClassType& thisType, const ClassType& superType, const ConstantPool& constPool, uint16_t modifiers,
			const vector<const ClassType*>& interfaces, const Attributes& attributes,
			const vector<const Field*>& fields, vector<MethodDataHolder>& methodDataHolders):
			Class(thisType, superType, constPool, modifiers, interfaces, attributes, fields, processMethodData(methodDataHolders)) {

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
					throw DecompilationException("enum constant initializer must have at least two arguments, got " +
							to_string(invokespecialOperation->arguments.size()));
				enumFields.push_back(new EnumField(*field, invokespecialOperation->arguments));
			} else
				otherFields.push_back(field);
		}
	}


	CodeEnvironment::CodeEnvironment(const Bytecode& bytecode, const ClassInfo& classinfo, MethodScope* methodScope, uint16_t modifiers,
			const MethodDescriptor& descriptor, const Attributes& attributes, uint16_t maxLocals):
			bytecode(bytecode), classinfo(classinfo), constPool(classinfo.constPool), stack(*new CodeStack()),
			methodScope(*methodScope), currentScopes(methodScope), modifiers(modifiers), descriptor(descriptor), attributes(attributes) {
		for(uint32_t i = methodScope->getVariablesCount(); i < maxLocals; i++)
			methodScope->addVariable(new UnnamedVariable(AnyType::getInstance()));
	}


	void CodeEnvironment::checkCurrentScope() {
		for(Scope* currentScope = currentScopes.top(); index >= currentScope->end(); currentScope = currentScopes.top()) {
			if(currentScope->parentScope == nullptr)
				throw DecompilationException("Unexpected end of global function scope {" +
						to_string(currentScope->start()) + ".." + to_string(currentScope->end()) + '}');
			currentScopes.pop();
			currentScope->finalize(*this);
		}

		Scope* currentScope = getCurrentScope();

		for(auto i = scopes.begin(); i != scopes.end(); ) {
			Scope* scope = *i;
			if(scope->start() <= index) {
				if(scope->end() > currentScope->end())
					throw DecompilationException("Scope is out of bounds of the parent scope: " +
						to_string(scope->start()) + ".." + to_string(scope->end()) + ", " + to_string(currentScope->start()) + ".." + to_string(currentScope->end()));
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
		if(imports.find(clazz) == imports.end()) {
			imports.insert(clazz);
			return true;
		} else {
			return all_of(imports.begin(), imports.end(), [clazz] (const ClassType* imp)
					{ return imp->simpleName != clazz->simpleName; }); // check has no class with same name
		}
	}

	string ClassType::toString(const ClassInfo& classinfo) const {
		return classinfo.addImport(this) ? simpleName : fullSimpleName;
	}

	void JDecompiler::readClassFiles() const {
		for(BinaryInputStream* file : files) {
			const Class* clazz = Class::readClass(*file);
			classes[clazz->thisType.getEncodedName()] = clazz;
		}
	}
}


int main(int argc, const char* args[]) {
	using namespace std;
	using namespace jdecompiler;

	if(!JDecompiler::parseConfig(argc, args))
		return 0;

	JDecompiler::instance.readClassFiles();

	for(const auto& clazz : JDecompiler::instance.getClasses()) {
		if(clazz.second->canStringify())
			cout << clazz.second->toString() << endl;
	}
}

#undef inline

#endif
