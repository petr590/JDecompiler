#ifndef FUNCTION_DEFINITIONS_CPP
#define FUNCTION_DEFINITIONS_CPP

#include "instructions.cpp"

namespace jdecompiler {

	template<typename T>
	static string numberConstantToString(T value) {
		static_assert(is_integral<T>(), "type must be integral");

		if(JDecompiler::getInstance().useHexNumbersAlways()) {
			return hexWithPrefix(value);
		}

		if(JDecompiler::getInstance().canUseHexNumbers()) {
			if(value == 0x0F) return "0x0F";
			if(value == 0x7F) return "0x7F";
			if constexpr((T)0x7FFF > 0) { // short, int, long
				if(value == 0x80)   return "0x80";
				if(value == 0xFF)   return "0xFF";
				if(value == 0x7FFF) return "0x7FFF";
			}
			if constexpr((T)0x7FFFFFFF > 0) { // int, long
				if(value == 0x8000)     return "0x8000";
				if(value == 0xFFFF)     return "0xFFFF";
				if(value == 0x7FFFFF)   return "0x7FFFFF";
				if(value == 0x800000)   return "0x800000";
				if(value == 0xFFFFFF)   return "0xFFFFFF";
				if(value == 0x7FFFFFFF) return "0x7FFFFFFF";
			}
			if constexpr((T)0x7FFFFFFFFFFFFFFF > 0) { // long
				if(value == 0x80000000ll)         return "0x80000000";
				if(value == 0xFFFFFFFFll)         return "0xFFFFFFFF";
				if(value == 0x7FFFFFFFFFll)       return "0x7FFFFFFFFF";
				if(value == 0x8000000000ll)       return "0x8000000000";
				if(value == 0xFFFFFFFFFFll)       return "0xFFFFFFFFFF";
				if(value == 0x7FFFFFFFFFFFll)     return "0x7FFFFFFFFFFF";
				if(value == 0x800000000000ll)     return "0x800000000000";
				if(value == 0xFFFFFFFFFFFFll)     return "0xFFFFFFFFFFFF";
				if(value == 0x7FFFFFFFFFFFFFll)   return "0x7FFFFFFFFFFFFF";
				if(value == 0x80000000000000ll)   return "0x80000000000000";
				if(value == 0xFFFFFFFFFFFFFFll)   return "0xFFFFFFFFFFFFFF";
				if(value == 0x7FFFFFFFFFFFFFFFll) return "0x7FFFFFFFFFFFFFFF";
			}
		}
		return to_string(value);
	}

	EnumClass::EnumClass(const Version& version, const ClassType& thisType, const ClassType& superType, const ConstantPool& constPool, uint16_t modifiers,
			const vector<const ClassType*>& interfaces, const Attributes& attributes,
			const vector<FieldDataHolder>& fieldsData, vector<MethodDataHolder>& methodDataHolders):
			Class(version, thisType, superType, constPool, modifiers, interfaces, attributes, fieldsData, processMethodData(methodDataHolders)) {

		using namespace operations;

		for(const Field* field : fields) {
			const InvokespecialOperation* invokespecialOperation;
			const Dup1Operation* dupOperation;
			const NewOperation* newOperation;

			if(field->modifiers == (ACC_PUBLIC | ACC_STATIC | ACC_FINAL | ACC_ENUM) &&
					field->descriptor.type == thisType && field->hasInitializer() &&

					(invokespecialOperation = dynamic_cast<const InvokespecialOperation*>(field->getInitializer())) != nullptr &&
					(dupOperation = dynamic_cast<const Dup1Operation*>(invokespecialOperation->object)) != nullptr &&
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


	DecompilationContext::DecompilationContext(const DisassemblerContext& disassemblerContext, const ClassInfo& classinfo, MethodScope* methodScope,
			uint16_t modifiers, const MethodDescriptor& descriptor, const Attributes& attributes, uint16_t maxLocals):
			disassemblerContext(disassemblerContext), classinfo(classinfo), constPool(classinfo.constPool), stack(*new CodeStack()),
			methodScope(*methodScope), currentScope(methodScope), modifiers(modifiers), descriptor(descriptor), attributes(attributes) {
		for(uint32_t i = methodScope->getVariablesCount(); i < maxLocals; i++)
			methodScope->addVariable(new UnnamedVariable(AnyType::getInstance(), false));
	}


	void DecompilationContext::updateScopes() {
		while(index == currentScope->end()) {
			if(currentScope->parentScope == nullptr)
				throw DecompilationException("Unexpected end of global function scope " + currentScope->toDebugString());
			currentScope->finalize(*this);
			log("End of", currentScope->toDebugString());
			currentScope = currentScope->parentScope;
		}

		for(auto i = inactiveScopes.begin(); i != inactiveScopes.end(); ) {
			const Scope* scope = *i;
			if(scope->start() <= index) {
				if(scope->end() > currentScope->end())
					throw DecompilationException((string)"Scope " + scope->toDebugString() +
							" is out of bounds of the parent scope " + currentScope->toDebugString());
				/*if(index > scope->start()) {
					throw IllegalStateException("Scope " + scope->toDebugString() + " is added after it starts");
				}*/

				log("Start of", scope->toDebugString());
				currentScope->addOperation(scope, *stringifyContext);
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
		if(atLeastOneFileSpecified) {
			for(BinaryInputStream* file : files) {
				if(!JDecompiler::getInstance().isFailOnError()) {
					try {
						const Class* clazz = Class::readClass(*file);
						classes[clazz->thisType.getEncodedName()] = clazz;
					}
					  catch(const EOFException& ex) {
						error("unexpected end of file while reading ", file->path);
					} catch(const IOException& ex) {
						error(typeNameOf(ex), ": ", ex.what());
					} catch(const DecompilationException& ex) {
						error(typeNameOf(ex), ": ", ex.what());
					}
				} else {
					const Class* clazz = Class::readClass(*file);
					classes[clazz->thisType.getEncodedName()] = clazz;
				}
			}
		} else {
			error("no input file specified");
		}
	}

	void StringifyContext::enterScope(const Scope* scope) const {
		//assert(scope->parentScope == currentScope);
		currentScope = scope;
	}

	void StringifyContext::exitScope(const Scope* scope) const {
		/*if(scope != currentScope) {
			throw AssertionError("While stringify method " + descriptor.toString() + ": scope != currentScope: scope = " + scope->toDebugString() + "; currentScope = " + currentScope->toDebugString());
		}*/
		currentScope = scope->parentScope;
	}

	const StringifyContext* ClassInfo::getFieldStringifyContext() const {
		if(fieldStringifyContext == nullptr)
			fieldStringifyContext = new StringifyContext(this->getEmptyDisassemblerContext(), *this,
					new MethodScope(0, 0, 0), ACC_STATIC, *new MethodDescriptor(thisType, "<init>", VOID), Attributes::getEmptyInstance());
		return fieldStringifyContext;
	}

	const DisassemblerContext& ClassInfo::getEmptyDisassemblerContext() const {
		if(emptyDisassemblerContext == nullptr)
			emptyDisassemblerContext = new DisassemblerContext(constPool, 0, (const uint8_t*)"");
		return *emptyDisassemblerContext;
	}
}

#endif
