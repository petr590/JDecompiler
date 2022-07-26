#ifndef JDECOMPILER_H
#define JDECOMPILER_H

#ifdef NO_STATIC_ASSERT
	#define static_assert(...)
#endif

#ifndef NO_RESTRICT
	#if defined(__clang__)
		#define restrict __restrict__
	#elif defined(__GNUC__) || defined(_MSC_VER)
		#define restrict __restrict
	#else
		#define restrict
	#endif
#else
	#define restrict
#endif

#include <stdint.h>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <tuple>
#include <iostream>

namespace util {}

namespace jdecompiler {

	using std::cout;
	using std::cerr;
	using std::endl;
	using std::boolalpha;

	using std::string;
	using std::vector;
	using std::set;
	using std::map;
	template<typename T>
	using uset = std::unordered_set<T>;
	template<typename K, typename V>
	using umap = std::unordered_map<K, V>;

	using std::tuple;
	using std::pair;
	using std::get;
	using std::index_sequence;
	using std::index_sequence_for;

	using std::min;
	using std::max;
	using std::numeric_limits;

	using std::exception;
	using std::invalid_argument;
	using std::out_of_range;

	using std::function;

	using std::ostringstream;

	using std::to_string;
	using std::find;
	using std::find_if;
	using std::memcpy;

	using std::is_base_of;
	using std::is_arithmetic;
	using std::is_integral;
	using std::is_floating_point;
	using std::make_unsigned_t;

	using std::initializer_list;

	using std::isinf;
	using std::isnan;

	using std::nullptr_t;

	using namespace util;


	static inline const uint32_t CLASS_SIGNATURE = 0xCAFEBABE;

	static inline const uint32_t
			ACC_VISIBLE      = 0x0000, // class, field, method
			ACC_PUBLIC       = 0x0001, // class, field, method
			ACC_PRIVATE      = 0x0002, // nested class, field, method
			ACC_PROTECTED    = 0x0004, // nested class, field, method
			ACC_STATIC       = 0x0008, // nested class, field, method
			ACC_FINAL        = 0x0010, // class, field, method
			ACC_SYNCHRONIZED = 0x0020, // method, block
			ACC_SUPER        = 0x0020, // class (deprecated)
			ACC_VOLATILE     = 0x0040, // field
			ACC_TRANSIENT    = 0x0080, // field
			ACC_BRIDGE       = 0x0040, // method
			ACC_VARARGS      = 0x0080, // method
			ACC_NATIVE       = 0x0100, // method
			ACC_INTERFACE    = 0x0200, // class
			ACC_ABSTRACT     = 0x0400, // class, method
			ACC_STRICT       = 0x0800, // class, non-abstract method
			ACC_SYNTHETIC    = 0x1000, // class, field, method
			ACC_ANNOTATION   = 0x2000, // class
			ACC_ENUM         = 0x4000, // class, field
			ACC_ACCESS_FLAGS = ACC_VISIBLE | ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED;


	// util.cpp

	struct FormatString;

	// Maybe in far future...
	/*template<typename>
	struct const_ptr;*/

	// const-pool.cpp

	struct ConstantPool;

	struct Constant;
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


	// attributes.cpp

	struct Attributes;

	struct Attribute;
	struct UnknownAttribute;
	struct ConstantValueAttribute;
	struct CodeAttribute;
	struct ExceptionsAttribute;
	struct DeprecatedAttribute;
	struct AnnotationsAttribute;


	// code.cpp

	struct Stringified;

	struct Variable;

	struct Operation;

	struct Instruction;

	struct Scope;
	struct MethodScope;
	struct StaticInitializerScope;

	// classinfo.cpp
	struct ClassInfo;


	// types.cpp

	enum class TypeSize;

	struct Type;

	struct BasicType;
	struct SpecialType;

	struct PrimitiveType;

	struct ReferenceType;
	struct ClassType;
	struct ArrayType;
	struct ParameterType;

	struct VariableCapacityIntegralType;
	struct AnyType;
	struct AnyObjectType;

	struct GenericParameter;


	static const BasicType* parseType(const char*&);
	static const BasicType* parseType(const char*&&);
	static const BasicType* parseType(const string&);

	static vector<const Type*> parseMethodArguments(const char*&);

	static const BasicType* parseReturnType(const char*);
	static const BasicType* parseReturnType(const string&);

	static const ReferenceType* parseReferenceType(const char*);
	static const ReferenceType* parseReferenceType(const string&);

	static const ReferenceType* parseParameter(const char*&);

	static vector<const ReferenceType*> parseParameters(const char*&);
	static vector<const ReferenceType*> parseParameters(const char*&&);

	static vector<const GenericParameter*> parseGeneric(const char*&);

	// field.cpp

	struct FieldDescriptor;
	struct FieldInfo;
	struct Field;

	struct ConstantDecompilationContext {
		const ClassInfo& classinfo;
		const FieldInfo* fieldinfo;

		ConstantDecompilationContext(const ClassInfo& classinfo, const FieldInfo* fieldinfo = nullptr):
				classinfo(classinfo), fieldinfo(fieldinfo) {}
	};

	// method.cpp

	struct MethodDescriptor;
	struct Method;

	// class.spp
	struct Class;
	struct EnumClass;


	// contexts
	struct DisassemblerContext;

	struct DecompilationContext;

	struct StringifyContext;


	// operations.cpp

	enum class Priority;

	namespace operations {
		template<class> struct ReturnableOperation; // ReturnableOperation is an operation which returns specified type
		struct IntOperation;
		struct AnyIntOperation;
		struct BooleanOperation;
		struct VoidOperation;
		struct TransientReturnableOperation;

		template<TypeSize> struct TypeSizeTemplatedOperation;
		template<TypeSize> struct AbstractDupOperation;
		template<TypeSize> struct DupOperation;
		struct DupX1Operation;
		struct DupX2Operation;
		struct Dup2X1Operation;
		struct Dup2X2Operation;

		template<typename> struct ConstOperation;
		struct IConstOperation;

		template<TypeSize, class CT, typename RT>
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

		template<TypeSize>
		struct PopOperation;

		struct OperatorOperation;
		template<char32_t, Priority> struct BinaryOperatorOperationImpl;
		template<char32_t, Priority> struct UnaryOperatorOperation;

		struct IIncOperation;

		struct CastOperation;

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
		struct ContinueOperation;
		struct InfiniteLoopScope;
		struct EmptyInfiniteLoopScope;
	}


	// instructions.cpp

	namespace instructions {
		struct InstructionWithIndex;
		struct InstructionAndOperation;
		struct VoidInstructionAndOperation;

		struct AConstNull;
		template<typename> struct NumberConstInstruction;
		struct IConstInstruction;
		struct LConstInstruction;
		struct FConstInstruction;
		struct DConstInstruction;

		template<typename> struct IPushInstruction;
		template<TypeSize> struct LdcInstruction;

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

		template<TypeSize>
		struct PopInstruction;
		template<TypeSize>
		struct DupInstruction;
		struct DupX1Instruction;
		struct DupX2Instruction;
		struct Dup2X1Instruction;
		struct Dup2X2Instruction;
		struct SwapInstruction;
		template<char32_t, Priority, bool>
		struct OperatorInstruction;
		template<char32_t, Priority, bool>
		struct BinaryOperatorInstruction;
		template<char32_t, Priority>
		struct ShiftOperatorInstruction;
		template<char32_t, Priority>
		struct UnaryOperatorInstruction;
		struct IIncInstruction;
		struct CastInstruction;
		struct LCmpInstruction;
		struct FCmpInstruction;
		struct DCmpInstruction;
		struct IfBlock;
		struct IfCmpBlock;
		struct IfEqBlock;
		struct IfNotEqBlock;
		struct IfGtBlock;
		struct IfGeBlock;
		struct IfLtBlock;
		struct IfLeBlock;
		struct IfICmpBlock;
		struct IfIEqBlock;
		struct IfINotEqBlock;
		struct IfIGtBlock;
		struct IfIGeBlock;
		struct IfILtBlock;
		struct IfILeBlock;
		struct IfACmpBlock;
		struct IfAEqBlock;
		struct IfANotEqBlock;
		struct IfNullBlock;
		struct IfNonNullBlock;
		struct GotoInstruction;
		struct SwitchInstruction;
		template<class>
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


	struct Block;
	struct RootBlock;

	static void finish();
}

#undef inline

#endif
