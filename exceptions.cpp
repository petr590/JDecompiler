#ifndef JDECOMPILER_EXCEPTIONS_CPP
#define JDECOMPILER_EXCEPTIONS_CPP

#include "util/hex.cpp"
#include "util/exception.cpp"

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


	struct DecompilationException: Exception {
		DecompilationException(): Exception() {}
		DecompilationException(const string& message): Exception(message) {}
	};


	struct IndexOutOfBoundsException: DecompilationException {
		public:
			IndexOutOfBoundsException(const string& message): DecompilationException(message) {}

			IndexOutOfBoundsException(abstract_index_t<uint32_t> index, uint32_t length):
					DecompilationException("Index " + to_string(index) + " is out of bounds for length " + to_string(length)) {}

		protected:
			IndexOutOfBoundsException(abstract_index_t<uint32_t> index, uint32_t length, const string& name):
					DecompilationException(name + " index " + to_string(index) + " is out of bounds for length " + to_string(length)) {}
	};

	struct BytecodeIndexOutOfBoundsException: IndexOutOfBoundsException {
		BytecodeIndexOutOfBoundsException(abstract_index_t<uint32_t> index, uint32_t length): IndexOutOfBoundsException(index, length, "Bytecode") {}
	};

	struct BytecodePosOutOfBoundsException: IndexOutOfBoundsException {
		BytecodePosOutOfBoundsException(abstract_index_t<uint32_t> index, uint32_t length):
				IndexOutOfBoundsException("Bytecode pos " + to_string(index) + " is out of bounds for length " + to_string(length)) {}
	};

	struct StackIndexOutOfBoundsException: IndexOutOfBoundsException {
		StackIndexOutOfBoundsException(abstract_index_t<uint32_t> index, uint32_t length): IndexOutOfBoundsException(index, length, "Stack") {}
	};

	struct ConstantPoolIndexOutOfBoundsException: IndexOutOfBoundsException {
		ConstantPoolIndexOutOfBoundsException(abstract_index_t<uint32_t> index, uint32_t length): IndexOutOfBoundsException(index, length, "Constant pool") {}
	};


	struct InvalidConstantPoolReferenceException: DecompilationException {
		InvalidConstantPoolReferenceException(const string& message): DecompilationException(message) {}
	};


	struct InvalidTypeNameException: DecompilationException {
		InvalidTypeNameException(const string& message): DecompilationException(message) {}
	};

	struct InvalidClassNameException: InvalidTypeNameException {
		InvalidClassNameException(const string& message): InvalidTypeNameException(message) {}
	};

	struct InvalidSignatureException: InvalidTypeNameException {
		InvalidSignatureException(const string& message): InvalidTypeNameException(message) {}
	};

	struct IllegalModifiersException: DecompilationException {
		IllegalModifiersException(uint16_t modifiers): DecompilationException(hexWithPrefix<4>(modifiers)) {}
			IllegalModifiersException(const string& message): DecompilationException(message) {}
	};

	struct IllegalMethodDescriptorException: DecompilationException {
		IllegalMethodDescriptorException(const string& descriptor): DecompilationException(descriptor) {}
	};


	struct IllegalStackStateException: DecompilationException {
		IllegalStackStateException() {}
		IllegalStackStateException(const string& message): DecompilationException(message) {}
	};

	struct EmptyStackException: IllegalStackStateException {
		EmptyStackException() {}
		EmptyStackException(const string& message): IllegalStackStateException(message) {}
	};


	struct TypeSizeMismatchException: DecompilationException {
		TypeSizeMismatchException(const string& message): DecompilationException(message) {}
		TypeSizeMismatchException(const string& requiredSizeName, const string& sizeName, const string& typeName):
				DecompilationException("Required " + requiredSizeName + ", got " + sizeName + " of type " + typeName) {}
	};


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



	struct CastException: Exception {
		CastException(): Exception() {}
		CastException(const string& message): Exception(message) {}
	};
}

#endif
