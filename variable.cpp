#ifndef JDECOMPILER_VARIABLE_CPP
#define JDECOMPILER_VARIABLE_CPP

#include "variable.h"

namespace jdecompiler {

	string Variable::getRawNameByType(const Type* type, bool* unchecked) {
		if(const ClassType* classType = dynamic_cast<const ClassType*>(type)) {
			if(classType->simpleName == "Object") return "obj";
			if(classType->simpleName == "Boolean") return "bool";
			if(classType->simpleName == "Byte") return "b";
			if(classType->simpleName == "Character") return "ch";
			if(classType->simpleName == "Short") return "sh";
			if(classType->simpleName == "Integer") return "n";
			if(classType->simpleName == "Long") return "l";
			if(classType->simpleName == "Float") return "f";
			if(classType->simpleName == "Double") return "d";
			if(classType->simpleName == "String") return "str";
			if(classType->simpleName == "StringBuilder") return "str";
			if(classType->simpleName == "BigInteger") return "bigint";
			if(classType->simpleName == "BigDemical") return "bigdem";
		}
		*unchecked = true;
		return type->getVarName();
	}

	string Variable::getNameByType(const Type* type) {
		bool unchecked = false;

		const string name = getRawNameByType(type, &unchecked);
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
				{"true", "tr"}, {"false", "fls"}, {"null", "nul"},
				{"strictfp", "strict"}, {"const", "cnst"}, {"goto", "gt"}
			};

			for(const auto& keyword : keywords)
				if(name == keyword.first)
					return keyword.second;
		}

		return name;
	}

	Variable::Variable(const Type* type, bool declared, bool isFixedType): type(type), declared(declared), isFixedType(isFixedType) {}


	const Type* Variable::setType(const Type* newType) const {
		if(isFixedType) {
			if(type->isSubtypeOf(newType))
				return this->type;
			else
				throw IncopatibleTypesException(type, newType);
		}

		if(typeSettingLocked) // To avoid infinite recursion
			return this->type;

		typeSettingLocked = true;

		for(const Operation* operation : bindedOperations) {
			newType = operation->getReturnTypeAsWidest(newType);
		}

		typeSettingLocked = false;

		return this->type = newType;//(instanceof<const PrimitiveType*>(newType) ? static_cast<const PrimitiveType*>(newType)->toVariableCapacityIntegralType() : newType);
	}


	NamedVariable::NamedVariable(const Type* type, bool declared, const string& name, bool isFixedType):
			Variable(type, declared, isFixedType), name(name) {}

	void NamedVariable::makeCounter() const {}

	bool NamedVariable::isCounter() const {
		return false;
	}

	string NamedVariable::getName() const {
		return name;
	}

	void NamedVariable::addName(const string&) const {}


	UnnamedVariable::UnnamedVariable(const Type* type, bool declared, bool isFixedType):
			Variable(type, declared, isFixedType) {}

	UnnamedVariable::UnnamedVariable(const Type* type, bool declared, const string& name, bool isFixedType):
			Variable(type, declared, isFixedType), names({name}) {}

	void UnnamedVariable::makeCounter() const {
		counter = true;
	}

	bool UnnamedVariable::isCounter() const {
		return counter;
	}

	string UnnamedVariable::getName() const {
		return names.size() == 1 ? *names.begin() : getNameByType(type);
	}

	void UnnamedVariable::addName(const string& name) const {
		names.insert(name);
	}
}

#endif
