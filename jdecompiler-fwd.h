#ifndef JDECOMPILER_H
#define JDECOMPILER_H

#define LOG(s) cout << "[ jdecompiler/" __FILE__ " ]: " << s << endl

#ifdef NO_INLINE
#	define FORCE_INLINE
#elif !defined(NO_FORCE_INLINE)
#	ifdef __GNUC__ // For gcc
#		define FORCE_INLINE inline __attribute__((always_inline))
#	endif
#else
#	define FORCE_INLINE inline
#endif

#ifdef NO_STATIC_ASSERT
#define static_assert(...)
#endif

#include <stdint.h>
#include <string>
#include <vector>

#define inline FORCE_INLINE

namespace jdecompiler {

	using namespace std;


	static const uint32_t CLASS_SIGNATURE = 0xCAFEBABE;

	static const uint32_t
			ACC_VISIBLE      = 0x0000, // class, field, method
			ACC_PUBLIC       = 0x0001, // class, field, method
			ACC_PRIVATE      = 0x0002, // class, field, method
			ACC_PROTECTED    = 0x0004, // class, field, method
			ACC_STATIC       = 0x0008, // nested class, field, method
			ACC_FINAL        = 0x0010, // class, field, method
			ACC_SYNCHRONIZED = 0x0020, // method, block
			ACC_SUPER        = 0x0020, // class (deprecated)
			ACC_VOLATILE     = 0x0040, // field
			ACC_BRIDGE       = 0x0040, // method
			ACC_TRANSIENT    = 0x0080, // field
			ACC_VARARGS      = 0x0080, // args
			ACC_NATIVE       = 0x0100, // method
			ACC_INTERFACE    = 0x0200, // class
			ACC_ABSTRACT     = 0x0400, // class, method
			ACC_STRICT       = 0x0800, // class, non-abstract method
			ACC_SYNTHETIC    = 0x1000, // method
			ACC_ANNOTATION   = 0x2000, // class
			ACC_ENUM         = 0x4000; // class


	// jdecompiler-util

	struct Exception;

	struct BinaryInputStream;


	struct FormatString;

	// jdecompiler-const-pool

	struct Constant;

	struct ConstantPool;

	struct Utf8Constant;

	struct ClassConstant;

	struct StringConstant;

	struct NameAndTypeConstant;

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

	struct ConstantValueAttribute;

	struct CodeAttribute;

	struct ExceptionsAttribute;

	struct DeprecatedAttribute;

	struct AnnotationsAttribute;


	// jdecompiler-main

	struct Stringified;

	struct Variable;

	struct Operation;

	struct Instruction;

	struct Scope;

	struct MethodScope;

	struct ClassInfo;


	// jdecompiler-types

	enum class TypeSize;

	struct Type;

	struct BasicType;

	struct SpecialType;

	template<TypeSize>
	struct PrimitiveType;

	struct ReferenceType;

	struct ClassType;

	struct ArrayType;

	static const BasicType* parseType(const char* encodedName);

	static const BasicType* parseType(const string& encodedName);

	static const BasicType* parseReturnType(const char* encodedName);

	static const ReferenceType* parseReferenceType(const string& encodedName);

	static vector<const ReferenceType*> parseParameters(const char* str);

	// jdecompiler-main

	struct Field;

	struct MethodDescriptor;

	struct Method;

	struct Class;


	template<typename T>
	struct Stack;

	struct Bytecode;

	struct CodeEnvironment;

	// types.cpp

	enum class TypeSize;

	struct Type;


	// operations.cpp

	enum class Priority;

	namespace operations {
		template<class T> struct ReturnableOperation; // ReturnableOperation is an operation which returns specified type
		struct IntOperation;
		struct AnyIntOperation;
		struct BooleanOperation;
		struct VoidOperation;
		struct TransientReturnableOperation;

		template<TypeSize size> struct TypeSizeTemplatedOperation;
		template<TypeSize size> struct AbstractDupOperation;
		template<TypeSize size> struct DupOperation;
		struct DupX1Operation;
		struct DupX2Operation;
		struct Dup2X1Operation;
		struct Dup2X2Operation;

		template<typename T> struct ConstOperation;
		struct IConstOperation;

		template<TypeSize size, class CT, typename RT>
		struct LdcOperation;

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

		template<TypeSize size>
		struct PopOperation;

		template<char32_t operation, Priority priority> struct OperatorOperation;
		template<char32_t operation, Priority priority> struct BinaryOperatorOperation;
		template<char32_t operation, Priority priority> struct UnaryOperatorOperation;

		struct IIncOperation;

		template<bool required> struct CastOperation;

		struct CheckCastOperation;

		struct InstanceofOperation;


		struct SwitchScope;

		struct CatchScopeDataHolder;
		struct TryScope;
		struct CatchScope;

		struct ReturnOperation;
		struct IReturnOperation;
		struct LReturnOperation;
		struct FReturnOperation;
		struct DReturnOperation;
		struct AReturnOperation;

		struct FieldOperation;
		struct PutFieldOperation;
		struct PutStaticFieldOperation;
		struct PutInstanceFieldOperation;
		struct GetFieldOperation;
		struct GetStaticFieldOperation;
		struct GetInstanceFieldOperation;

		struct NewOperation;

		struct InvokeOperation;
		struct InvokeNonStaticOperation;
		struct InvokevirtualOperation;
		struct InvokespecialOperation;
		struct InvokestaticOperation;
		struct InvokeinterfaceOperation;

		struct ConcatStringsOperation;

		struct ArrayStoreOperation;
		struct NewArrayOperation;
		struct ANewArrayOperation;
		struct MultiANewArrayOperation;
		struct IAStoreOperation;
		struct LAStoreOperation;
		struct FAStoreOperation;
		struct DAStoreOperation;
		struct AAStoreOperation;
		struct BAStoreOperation;
		struct CAStoreOperation;
		struct SAStoreOperation;

		struct ArrayLengthOperation;

		struct AThrowOperation;

		// condition-operations.cpp
		struct CmpOperation;
		struct LCmpOperation;
		struct FCmpOperation;
		struct DCmpOperation;

		struct CompareType;
		struct EqualsCompareType;

		struct ConditionOperation;
		struct CompareBinaryOperation;
		struct CompareWithZeroOperation;
		struct CompareWithNullOperation;

		struct TernaryOperatorOperation;

		struct IfScope;
		struct ElseScope;
		struct ContinueOperation;
		struct EmptyInfiniteLoopScope;
	}


	// instructions.cpp

	namespace instructions {
		struct InstructionWithIndex;
		struct InstructionAndOperation;
		struct VoidInstructionAndOperation;

		struct AConstNull;
		template<typename T> struct NumberConstInstruction;
		struct IConstInstruction;
		struct LConstInstruction;
		struct FConstInstruction;
		struct DConstInstruction;

		template<typename T> struct IPushInstruction;
		template<TypeSize size> struct LdcInstruction;

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

		template<TypeSize size>
		struct PopInstruction;
		template<TypeSize size>
		struct DupInstruction;
		struct DupX1Instruction;
		struct DupX2Instruction;
		struct Dup2X1Instruction;
		struct Dup2X2Instruction;
		struct SwapInstruction;
		template<char32_t operation, Priority priority, bool canUseBoolean>
		struct OperatorInstruction;
		template<char32_t operation, Priority priority, bool canUseBoolean>
		struct BinaryOperatorInstruction;
		template<char32_t operation, Priority priority>
		struct ShiftOperatorInstruction;
		template<char32_t operation, Priority priority>
		struct UnaryOperatorInstruction;
		struct IIncInstruction;
		template<bool required>
		struct CastInstruction;
		struct LCmpInstruction;
		struct FCmpInstruction;
		struct DCmpInstruction;
		struct IfInstruction;
		struct IfCmpInstruction;
		struct IfEqInstruction;
		struct IfNotEqInstruction;
		struct IfGtInstruction;
		struct IfGeInstruction;
		struct IfLtInstruction;
		struct IfLeInstruction;
		struct IfICmpInstruction;
		struct IfIEqInstruction;
		struct IfINotEqInstruction;
		struct IfIGtInstruction;
		struct IfIGeInstruction;
		struct IfILtInstruction;
		struct IfILeInstruction;
		struct IfACmpInstruction;
		struct IfAEqInstruction;
		struct IfANotEqInstruction;
		struct IfNullInstruction;
		struct IfNonNullInstruction;
		struct GotoInstruction;
		struct SwitchInstruction;
		template<class ReturnOperation>
		struct ReturnInstruction;
		struct VReturn;
		struct GetStaticFieldInstruction;
		struct PutStaticFieldInstruction;
		struct GetInstanceFieldInstruction;
		struct PutInstanceFieldInstruction;
		struct InvokeInstruction;
		struct InvokevirtualInstruction;
		struct InvokespecialInstruction;
		struct InvokestaticInstruction;
		struct InvokeinterfaceInstruction;
		struct InvokedynamicInstruction;
		struct NewInstruction;
		struct NewArrayInstruction;
		struct ANewArrayInstruction;
		struct ArrayLengthInstruction;
		struct AThrowInstruction;
		struct CheckCastInstruction;
		struct InstanceofInstruction;
		struct MultiANewArrayInstruction;
	}

}

#undef inline

#endif
