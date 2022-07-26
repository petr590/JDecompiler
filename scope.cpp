#ifndef JDECOMPILER_SCOPE_CPP
#define JDECOMPILER_SCOPE_CPP

#include "scope.h"

namespace jdecompiler {


	Scope::Scope(index_t startIndex, index_t endIndex, const Scope* parentScope, uint16_t variablesCount):
			startIndex(startIndex), endIndex(endIndex), parentScope(parentScope), variables(variablesCount) {}


	const Variable* Scope::findVariable(index_t index) const {
		const Variable* var = variables[index];
		return var == nullptr && parentScope != nullptr ? parentScope->findVariable(index) : var;
	}

	const Variable* Scope::findVariableAtInnerScopes(index_t index) const {
		const Variable* var = variables[index];

		if(var == nullptr) {
			for(const Scope* scope : innerScopes) {
				var = scope->findVariableAtInnerScopes(index);
				if(var != nullptr)
					return var;
			}
		} else {
			variables[index] = nullptr; // ???
		}

		return var;
	}

	const Variable& Scope::getVariable(index_t index, bool isDeclared) const {

		if(index >= variables.size()) {
			if(parentScope == nullptr)
				throw IndexOutOfBoundsException(index, variables.size());
			return parentScope->getVariable(index, isDeclared);
		}

		const Variable* var = findVariable(index);
		if(var == nullptr) {
			if(isDeclared) {
				var = this->findVariableAtInnerScopes(index);
				if(var != nullptr)
					return *var;

				throw DecompilationException("Variable #" + to_string(index) + " is not found");

			} else {
				var = variables[index] = new UnnamedVariable(AnyType::getInstance(), false);
				lastAddedVarIndex = variables.size();
			}
		}
		return *var;
	}

	void Scope::addVariable(Variable* var) {
		if(lastAddedVarIndex >= variables.size())
			throw IllegalStateException("Cannot add variable to " + this->toDebugString() + ": no free place");

		variables[lastAddedVarIndex] = var;
		switch(var->getType()->getSize()) {
			case TypeSize::FOUR_BYTES: lastAddedVarIndex += 1; break;
			case TypeSize::EIGHT_BYTES: lastAddedVarIndex += 2; break;
			default: throw IllegalStateException((string)"Illegal variable type size " + TypeSize_nameOf(var->getType()->getSize()));
		}
	}

	string Scope::getNameFor(const Variable* var) const {

		if(!has(variables, var)) {
			return parentScope != nullptr ? parentScope->getNameFor(var) :
					throw IllegalStateException("Variable of type " + var->getType()->toString() + " is not found");
		}

		/* // Not working correctly
		auto existsVar = varNames.find(var);
		if(existsVar != varNames.end()) {
			return existsVar->second;
		}

		const string baseName = var->getName();
		string name = baseName + "1";

		existsVar = find_by_value(varNames, baseName);
		if(existsVar != varNames.end()) {
			varNames[existsVar->first] = name;
		}

		if(find_by_value(varNames, name) != varNames.end()) {
			uint_fast16_t i = 1;

			while(find_if(varNames.begin(), varNames.end(), [&name] (const auto& it) { return it.second == name; }) != varNames.end()) {
				name = baseName + to_string(++i);
			}

			varNames[var] = name;

			return name;
		} else {
			varNames[var] = baseName;

			return baseName;
		}
		*/

		const auto varName = varNames.find(var);
		if(varName != varNames.end()) {
			return varName->second;
		}

		string name;

		if(var->isCounter()) {
			for(char c = 'i'; c < 'n'; c++) {
				name = string(1, c);
				if(!hasVariable(name))
					return varNames[var] = name;
			}
		}

		const string baseName = var->getName();
		name = baseName;
		uint_fast16_t n = 1;

		while(hasVariable(name)) {
			name = baseName + to_string(++n);
		}

		return varNames[var] = name;
	}

	void Scope::finalize(const DecompilationContext&) const {}

	string Scope::toString(const StringifyContext& context) const {

		context.enterScope(this);
		const string str = this->toStringImpl(context);
		context.exitScope(this);
		return str;
	}

	string Scope::toStringImpl(const StringifyContext& context) const {
		string str = (label.empty() ? EMPTY_STRING : label + ": ") + getHeader(context) + "{\n";
		const size_t baseSize = str.size();

		context.classinfo.increaseIndent();

		for(auto i = code.begin(); i != code.end(); ++i) {
			const Operation* operation = *i;

			if(operation->canStringify() && !operation->isRemoved() && canPrintNextOperation(i)) {
				assert(operation->getReturnType() == VOID);
				str += operation->getFrontSeparator(context.classinfo) + operation->toString(context) +
						operation->getBackSeparator(context.classinfo);
			}
		}

		context.classinfo.reduceIndent();

		if(str.size() == baseSize) {
			str.back() = '}';
			return str;
		}

		return str + context.classinfo.getIndent() + '}';
	}

	void Scope::addOperation(const Operation* operation, const DecompilationContext& context) const {
		this->update(context);

		if(context.stack.empty()) {
			code.push_back(operation);
			if(instanceof<const Scope*>(operation))
				innerScopes.push_back(static_cast<const Scope*>(operation));
		} else {
			tempCode.push_back(operation);
		}
	}

	void Scope::update(const DecompilationContext& context) const {
		if(context.stack.empty() && !tempCode.empty()) {
			code.insert(code.end(), tempCode.begin(), tempCode.end());
			tempCode.clear();
		}
	}

	const Type* Scope::getReturnType() const {
		return VOID;
	}

	bool Scope::canAddToCode() const {
		return false;
	}


	void Scope::makeLabel() const {
		assert(parentScope != nullptr);

		if(label.empty())
			label = defaultLabelName(); // TODO
	}


	bool Scope::isBreakable() const {
		return false;
	}

	bool Scope::isContinuable() const {
		return false;
	}


	MethodScope::MethodScope(index_t startIndex, index_t endIndex, uint16_t localsCount): Scope(startIndex, endIndex, localsCount) {
		variables.reserve(localsCount);
	}


	StaticInitializerScope::StaticInitializerScope(index_t startIndex, index_t endIndex, uint16_t localsCount):
			MethodScope(startIndex, endIndex, localsCount) {}
}

#endif
