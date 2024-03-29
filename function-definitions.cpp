#ifndef FUNCTION_DEFINITIONS_CPP
#define FUNCTION_DEFINITIONS_CPP

#include "decompile.cpp"


namespace jdecompiler {

	EnumClass::EnumClass(const Version& version, const ClassType& thisType, const ClassType* superType, const ConstantPool& constPool, modifiers_t modifiers,
			const vector<const ClassType*>& interfaces, const Attributes& attributes, const vector<FieldDataHolder>& fieldsData,
			const vector<MethodDataHolder>& methodData, const vector<const GenericParameter*>& genericParameters):
			Class(version, thisType, superType, constPool, modifiers, interfaces, attributes, fieldsData, methodData, genericParameters) {

		for(const Field* field : fields) {
			const InvokespecialOperation* invokespecialOperation;
			const Dup1Operation* dupOperation;
			const NewOperation* newOperation;

			if(field->modifiers & ACC_ENUM && field->descriptor.type == thisType && field->hasInitializer() &&
					(invokespecialOperation = dynamic_cast<const InvokespecialOperation*>(field->getInitializer())) != nullptr &&
					(dupOperation = dynamic_cast<const Dup1Operation*>(invokespecialOperation->object)) != nullptr &&
					(newOperation = dynamic_cast<const NewOperation*>(dupOperation->operation)) != nullptr) {

				if(field->modifiers != (ACC_PUBLIC | ACC_STATIC | ACC_FINAL | ACC_ENUM))
					throw IllegalModifiersException("Enum constant must has public, static, final and enum flags, got " + hexWithPrefix<4>(field->modifiers));

				if(invokespecialOperation->arguments.size() < 2)
					throw DecompilationException("Enum constant initializer should have at least two arguments, got " +
							to_string(invokespecialOperation->arguments.size()));
				enumFields.push_back(new EnumField(*field, invokespecialOperation->arguments));
			} else {
				otherFields.push_back(field);
			}
		}
	}


	DecompilationContext::DecompilationContext(const DisassemblerContext& disassemblerContext, const ClassInfo& classinfo, MethodScope* methodScope,
			modifiers_t modifiers, const MethodDescriptor& descriptor, const Attributes& attributes, uint16_t maxLocals):
			DecompilationAndStringifyContext(disassemblerContext, classinfo, *methodScope, modifiers, descriptor, attributes),
			stack(*new CodeStack()) {

		for(uint32_t i = methodScope->getVariablesCount(); i < maxLocals; i++)
			methodScope->addVariable(new UnnamedVariable(AnyType::getInstance(), false));
	}


	void DecompilationContext::updateScopes() {
		while(index >= currentScope->end()) {
			if(currentScope->parentScope == nullptr)
				throw DecompilationException("Unexpected end of global function scope " + currentScope->toDebugString());
			currentScope->finalize(*this);
			log(index, "  end of ", currentScope->toDebugString());
			currentScope = currentScope->parentScope;
		}

		for(auto i = inactiveScopes.begin(); i != inactiveScopes.end(); ) {
			const Scope* scope = *i;
			if(scope->start() <= index) {
				/*if(scope->end() > currentScope->end())
					throw DecompilationException((string)"Scope " + scope->toDebugString() +
							" is out of bounds of the parent scope " + currentScope->toDebugString());*/
				/*if(index > scope->start()) {
					throw IllegalStateException("Scope " + scope->toDebugString() + " is added after it starts");
				}*/

				log(index, "start of ", scope->toDebugString());
				currentScope->addOperation(scope, *this);
				currentScope = scope;
				//scope->initiate(*this);
				inactiveScopes.erase(i);
			} else
				++i;
		}
	}


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
		if(clazz->simpleName == thisType.simpleName) {
			return *clazz == thisType;
		}

		if(any_of(imports.begin(), imports.end(), [clazz] (const ClassType* imp) { return *clazz == *imp; })) { // find class
			return true;
		} else {
			if(all_of(imports.begin(), imports.end(), [clazz] (const ClassType* imp)
					{ return imp->simpleName != clazz->simpleName; })) { // check has no class with same name
				imports.push_back(clazz);
				return true;
			}
			return false;
		}
	}

	string ClassType::toString(const ClassInfo& classinfo) const {
		return this->isAnonymous ? fullSimpleName : (classinfo.addImport(this) ? simpleName : name) + (parameters.empty() ? EMPTY_STRING :
				'<' + join<const ReferenceType*>(parameters, [&classinfo] (const ReferenceType* type) { return type->toString(classinfo); }) + '>');
	}

	void JDecompiler::readClassFiles() const {
		if(atLeastOneFileSpecified) {
			for(ClassInputStream* classFile : files) {
				try {
					Class::readClass(*classFile); /* Adding a class to JDecompiler::classes takes place in the class constructor */
				} catch(const EOFException& ex) {
					error("unexpected end of file while reading ", classFile->fileName);
				} catch(const Exception& ex) {
					error(ex.toString());
				}
			}

		} else {
			error("no input file specified");
		}
	}

	void StringifyContext::enterScope(const Scope* scope) const {
		currentScope = scope;
	}

	void StringifyContext::exitScope(const Scope* scope) const {
		/*if(scope != currentScope) {
			throw IllegalStateException("While stringify method " + descriptor.toString() + ": scope != currentScope (scope = " + scope->toDebugString() + "; currentScope = " + currentScope->toDebugString() + ')');
		}*/
		currentScope = scope->parentScope;
	}

	const DisassemblerContext& ClassInfo::getEmptyDisassemblerContext() const {
		if(emptyDisassemblerContext == nullptr)
			emptyDisassemblerContext = new DisassemblerContext(constPool, 0, (const uint8_t*)"");
		return *emptyDisassemblerContext;
	}
}

#endif
