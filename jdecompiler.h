#ifndef JDECOMPILER_H
#define JDECOMPILER_H

#include <string>
#include <vector>
#include <stdint.h>

using namespace std;

// jdecompiler-util

class Exception;

class BinaryInputStream;

namespace JDecompiler {
	class FormatString;

	// jdecompiler-const-pool

	struct Constant;

	struct ConstantPool;

	struct Utf8Constant;

	struct ClassConstant;

	template<typename T>
	struct ConstValueConstant;

	struct StringConstant;

	struct NameAndTypeConstant;

	struct ReferenceConstant;

	struct FieldrefConstant;

	struct MethodrefConstant;

	struct InterfaceMethodrefConstant;

	struct MethodHandleConstant;

	struct MethodTypeConstant;

	struct InvokeDynamicConstant;


	// jdecompiler-attributes

	struct Attribute;

	struct Attributes;

	struct UnknownAttribute;

	template<typename T>
	struct ConstantValueAttribute;

	struct CodeAttribute;

	struct ExceptionsAttribute;

	struct DeprecatedAttribute;

	//struct Annotation;

	//struct AnnotationsAttribute;

	static Attributes* readAttributes(BinaryInputStream* instream, const ConstantPool* constPool, uint16_t attributeCount);


	// jdecompiler-main

	struct Stringified;

	struct Variable;

	struct Operation;

	struct Instruction;

	struct Scope;

	struct ClassInfo;

	struct Type;

	struct PrimitiveType;

	struct ReferenceType;

	struct ClassType;

	struct ArrayType;

	static const Type* parseType(const char* encodedName);

	static const Type* parseType(const string& encodedName);

	static const Type* parseReturnType(const char* encodedName);

	static const ReferenceType* parseReferenceType(const char* encodedName);

	static string getNameByType(const Type* const type);

	struct ClassEntry;

	struct Field;

	struct MethodDescriptor;

	struct Method;

	struct Class;


	// jdecompiler-method-code

	struct Stack;

	struct Bytecode;

	struct CodeEnvironment;

	static string decompileCode(const ConstantPool& constPool, const CodeAttribute* attribute, Scope* scope, const ClassInfo& classinfo);


	// jdecompiler-operations

	/*struct ReturnableOperation;

	struct VoidOperation;

	struct ConstOperation;

	struct NumberConstOperation;

	struct IConstOperation;

	struct LConstOperation;

	struct FConstOperation;

	struct DConstOperation;

	struct IPushOperation;

	struct BIPushOperation;

	struct SIPushOperation;

	struct AbstractLdcOperation;

	struct LdcOperation;

	struct Ldc2Operation;

	struct LoadOperation;

	struct ILoadOperation;

	struct LLoadOperation;

	struct FLoadOperation;

	struct DLoadOperation;

	struct ALoadOperation;

	struct ArrayLoadOperation;

	struct IALoadOperation;

	struct LALoadOperation;

	struct FALoadOperation;

	struct DALoadOperation;

	struct AALoadOperation;

	struct BALoadOperation;

	struct CALoadOperation;

	struct SALoadOperation;

	struct StoreOperation;

	struct IStoreOperation;

	struct LStoreOperation;

	struct FStoreOperation;

	struct DStoreOperation;

	struct AStoreOperation;

	struct ArrayStoreOperation;

	struct IAStoreOperation;

	struct LAStoreOperation;

	struct FAStoreOperation;

	struct DAStoreOperation;

	struct AAStoreOperation;

	struct BAStoreOperation;

	struct CAStoreOperation;

	struct SAStoreOperation;

	struct PopOperation;

	struct DupOperation;

	struct SwapOperation;

	struct BinaryOperatorOperation;

	struct UnaryOperatorOperation;

	struct IIncOperation;

	struct CastOperation;

	struct LCpmOperation;

	struct IfScope;

	struct IfCmpScope;

	struct IfIEqScope;

	struct IfINotEqScope;

	struct IfIGtScope;

	struct IfIGeScope;

	struct IfILtScope;

	struct IfILeScope;

	struct IfAEqScope;

	struct IfANotEqScope;

	struct ReturnOperation;

	struct GetstaticOperation;

	struct PutstaticOperation;

	struct GetfieldOperation;

	struct PutfieldOperation;

	struct InvokeOperation;

	struct InvokestaticOperation;

	struct NewOperation;

	struct NewArrayOperation;

	struct ANewArrayOperation;

	struct ArrayLengthOperation;

	struct AThrowOperation;

	struct CheckCastOperation;

	struct InstanceofOperation;

	struct MultiANewArrayOperation;*/


	// jdecompiler-instructions

	/*struct InstructionWithIndex;

	struct InstructionAndOperation;

	struct VoidInstructionAndOperation;

	struct Nop;

	struct AConstNull;

	struct NumberConstInstruction;

	struct IConstInstruction;

	struct LConstInstruction;

	struct FConstInstruction;

	struct DConstInstruction;

	struct IPushInstruction;

	struct BIPushInstruction;

	struct SIPushInstruction;

	struct AbstractLdcInstruction;

	struct LdcInstruction;

	struct Ldc2Instruction;

	struct LoadInstruction;

	struct ILoadInstruction;

	struct LLoadInstruction;

	struct FLoadInstruction;

	struct DLoadInstruction;

	struct ALoadInstruction;

	struct ArrayLoadInstruction;

	struct IALoadInstruction;

	struct LALoadInstruction;

	struct FALoadInstruction;

	struct DALoadInstruction;

	struct AALoadInstruction;

	struct BALoadInstruction;

	struct CALoadInstruction;

	struct SALoadInstruction;

	struct StoreInstruction;

	struct IStoreInstruction;

	struct LStoreInstruction;

	struct FStoreInstruction;

	struct DStoreInstruction;

	struct AStoreInstruction;

	struct ArrayStoreInstruction;

	struct IAStoreInstruction;

	struct LAStoreInstruction;

	struct FAStoreInstruction;

	struct DAStoreInstruction;

	struct AAStoreInstruction;

	struct BAStoreInstruction;

	struct CAStoreInstruction;

	struct SAStoreInstruction;

	struct PopInstruction;

	struct DupInstruction;

	struct SwapInstruction;

	static Type* getTypeByCode(uint16_t code);

	struct OperatorInstruction;

	struct BinaryOperatorInstruction;

	struct UnaryOperatorInstruction;

	struct IIncInstruction;

	struct CastInstruction;

	struct LCpmInstruction;

	struct IfInstruction;

	struct IfIEqInstruction;

	struct IfINotEqInstruction;

	struct IfIGtInstruction;

	struct IfIGeInstruction;

	struct IfILtInstruction;

	struct IfILeInstruction;

	struct IfAEqInstruction;

	struct IfANotEqInstruction;

	struct Goto;

	struct ReturnInstruction;

	struct VReturn;

	struct GetstaticInstruction;

	struct PutstaticInstruction;

	struct GetfieldInstruction;

	struct PutfieldInstruction;

	struct InvokeInstruction;

	struct InvokestaticInstruction;

	struct NewInstruction;

	static const PrimitiveType* getArrayTypeByCode(uint8_t code);

	struct NewArrayInstruction;

	struct ANewArrayInstruction;

	struct ArrayLengthInstruction;

	struct AThrowInstruction;

	struct CheckCastInstruction;

	struct InstanceofInstruction;

	struct MultiANewArrayInstruction;*/

}

#endif
