#ifndef JDECOMPILER_EXCEPTIONS_CPP
#define JDECOMPILER_EXCEPTIONS_CPP

#include "util/hex.cpp"
#include "util/exception.cpp"
#include "util/index-out-of-bounds-exception.cpp"

namespace jdecompiler {

	struct IllegalArgumentException: Exception {
		IllegalArgumentException(const string& message): Exception(message) {}
	};

	struct IllegalStateException: Exception {
		IllegalStateException(const string& message): Exception(message) {}
	};

	struct AssertionError: Exception {
		AssertionError(const string& message): Exception(message) {}
	};


	struct CastException: Exception {
		CastException(): Exception() {}
		CastException(const string& message): Exception(message) {}
	};


	struct BytecodeIndexOutOfBoundsException: IndexOutOfBoundsException {
		BytecodeIndexOutOfBoundsException(abstract_index_t<uint32_t> index, uint32_t length):
				IndexOutOfBoundsException(index, length, "Bytecode") {}
	};

	struct BytecodePosOutOfBoundsException: IndexOutOfBoundsException {
		BytecodePosOutOfBoundsException(abstract_index_t<uint32_t> index, uint32_t length):
				IndexOutOfBoundsException("Bytecode pos " + to_string(index) + " is out of bounds for length " + to_string(length)) {}
	};

	struct ConstantPoolIndexOutOfBoundsException: IndexOutOfBoundsException {
		ConstantPoolIndexOutOfBoundsException(abstract_index_t<uint32_t> index, uint32_t length): IndexOutOfBoundsException(index, length, "Constant pool") {}
	};

	// -------------------------------------------------- DecompilationException --------------------------------------------------

	struct DisassemblingException: Exception {
		DisassemblingException(): Exception() {}
		DisassemblingException(const string& message): Exception(message) {}
	};


	struct ConstantPoolInitializingException: DisassemblingException {
		ConstantPoolInitializingException(const string& message): DisassemblingException(message) {}
	};


	struct InvalidConstantPoolReferenceException: DisassemblingException {
		InvalidConstantPoolReferenceException(const string& message): DisassemblingException(message) {}
	};


	struct InvalidTypeNameException: DisassemblingException {
		InvalidTypeNameException(const string& message): DisassemblingException(message) {}
	};

	struct InvalidClassNameException: InvalidTypeNameException {
		InvalidClassNameException(const string& message): InvalidTypeNameException(message) {}
	};

	struct InvalidSignatureException: InvalidTypeNameException {
		InvalidSignatureException(const string& message): InvalidTypeNameException(message) {}
	};


	// -------------------------------------------------- DecompilationException --------------------------------------------------

	struct DecompilationException: Exception {
		DecompilationException(): Exception() {}
		DecompilationException(const string& message): Exception(message) {}
	};


	struct IllegalModifiersException: DecompilationException {
		IllegalModifiersException(uint16_t modifiers): DecompilationException(hexWithPrefix<4>(modifiers)) {}
			IllegalModifiersException(const string& message): DecompilationException(message) {}
	};

	struct IllegalMethodDescriptorException: DisassemblingException {
		IllegalMethodDescriptorException(const string& descriptor): DisassemblingException(descriptor) {}
	};


	struct IncopatibleTypesException: DecompilationException {
		IncopatibleTypesException(const string& message): DecompilationException(message) {}
		IncopatibleTypesException(const Type*, const Type*);
	};


	struct TypeSizeMismatchException: DecompilationException {
		TypeSizeMismatchException(const string& message): DecompilationException(message) {}
		TypeSizeMismatchException(const string& requiredSizeName, const string& sizeName, const string& typeName):
				DecompilationException("Required " + requiredSizeName + ", got " + sizeName + " of type " + typeName) {}
	};


	// -------------------------------------------------- ClassFormatError --------------------------------------------------

	struct ClassFormatError: Exception {
		ClassFormatError(const string& message): Exception(message) {}
	};

	struct IllegalOpcodeError: ClassFormatError {
		IllegalOpcodeError(const string& message): ClassFormatError(message) {}
	};

	struct InstructionFormatError: ClassFormatError {
		InstructionFormatError(const string& message): ClassFormatError(message) {}
	};

	struct IllegalAttributeException: ClassFormatError {
		IllegalAttributeException(const string& message): ClassFormatError(message) {}
	};

	struct AttributeNotFoundException: ClassFormatError {
		AttributeNotFoundException(const string& message): ClassFormatError(message) {}
	};


	struct IllegalMethodHeaderException: ClassFormatError {
		IllegalMethodHeaderException(const string& message): ClassFormatError(message) {}
	};
}

#endif
