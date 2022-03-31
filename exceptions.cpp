#ifndef JDECOMPILER_EXCEPTIONS_CPP
#define JDECOMPILER_EXCEPTIONS_CPP

#include "hex.cpp"

namespace jdecompiler {

	#define DECLARE_EXCEPTION(thisClass, superClass, ...) struct thisClass: superClass {\
		virtual const char* getName() const override {\
			return #thisClass;\
		}\
		__VA_ARGS__\
	};

	struct Exception: exception {
		protected:
			const string message;

		public:
			Exception() {}
			Exception(const char* message): message(message) {}
			Exception(const string& message): message(message) {}

			virtual const char* what() const noexcept override {
				return message.c_str();
			}

			virtual const char* getName() const {
				return "Exception";
			}
	};


	DECLARE_EXCEPTION(IllegalArgumentException, Exception,
		IllegalArgumentException(const string& message): Exception(message) {}
	);

	DECLARE_EXCEPTION(IllegalStateException, Exception,
		IllegalStateException(const string& message): Exception(message) {}
	);

	DECLARE_EXCEPTION(AssertionError, Exception,
		AssertionError(const string& message): Exception(message) {}
	);


	DECLARE_EXCEPTION(DecompilationException, Exception,
		DecompilationException(): Exception() {}
		DecompilationException(const string& message): Exception(message) {}
	);


	DECLARE_EXCEPTION(IndexOutOfBoundsException, DecompilationException,
		public:
			IndexOutOfBoundsException(const string& message): DecompilationException(message) {}

			IndexOutOfBoundsException(abstract_index_t<uint32_t> index, uint32_t length):
					DecompilationException("Index " + to_string(index) + " is out of bounds for length " + to_string(length)) {}

		protected:
			IndexOutOfBoundsException(abstract_index_t<uint32_t> index, uint32_t length, const string& name):
					DecompilationException(name + " index " + to_string(index) + " is out of bounds for length " + to_string(length)) {}
	);

	DECLARE_EXCEPTION(BytecodeIndexOutOfBoundsException, IndexOutOfBoundsException,
		BytecodeIndexOutOfBoundsException(abstract_index_t<uint32_t> index, uint32_t length): IndexOutOfBoundsException(index, length, "Bytecode") {}
	);

	DECLARE_EXCEPTION(BytecodePosOutOfBoundsException, IndexOutOfBoundsException,
		BytecodePosOutOfBoundsException(abstract_index_t<uint32_t> index, uint32_t length):
				IndexOutOfBoundsException("Bytecode pos " + to_string(index) + " is out of bounds for length " + to_string(length)) {}
	);

	DECLARE_EXCEPTION(StackIndexOutOfBoundsException, IndexOutOfBoundsException,
		StackIndexOutOfBoundsException(abstract_index_t<uint32_t> index, uint32_t length): IndexOutOfBoundsException(index, length, "Stack") {}
	);

	DECLARE_EXCEPTION(ConstantPoolIndexOutOfBoundsException, IndexOutOfBoundsException,
		ConstantPoolIndexOutOfBoundsException(abstract_index_t<uint32_t> index, uint32_t length): IndexOutOfBoundsException(index, length, "Constant pool") {}
	);


	DECLARE_EXCEPTION(InvalidConstantPoolReferenceException, DecompilationException,
		InvalidConstantPoolReferenceException(const string& message): DecompilationException(message) {}
	);


	DECLARE_EXCEPTION(InvalidTypeNameException, DecompilationException,
		InvalidTypeNameException(const string& message): DecompilationException(message) {}
	);

	DECLARE_EXCEPTION(InvalidClassNameException, InvalidTypeNameException,
		InvalidClassNameException(const string& message): InvalidTypeNameException(message) {}
	);

	DECLARE_EXCEPTION(InvalidSignatureException, InvalidTypeNameException,
		InvalidSignatureException(const string& message): InvalidTypeNameException(message) {}
	);

	DECLARE_EXCEPTION(IllegalModifiersException, DecompilationException,
		IllegalModifiersException(uint16_t modifiers): DecompilationException(hexWithPrefix<4>(modifiers)) {}
			IllegalModifiersException(const string& message): DecompilationException(message) {}
	);

	DECLARE_EXCEPTION(IllegalMethodDescriptorException, DecompilationException,
		IllegalMethodDescriptorException(const string& descriptor): DecompilationException(descriptor) {}
	);


	DECLARE_EXCEPTION(IllegalStackStateException, DecompilationException,
		IllegalStackStateException() {}
		IllegalStackStateException(const string& message): DecompilationException(message) {}
	);

	DECLARE_EXCEPTION(EmptyStackException, IllegalStackStateException,
		EmptyStackException() {}
		EmptyStackException(const string& message): IllegalStackStateException(message) {}
	);


	DECLARE_EXCEPTION(TypeSizeMismatchException, DecompilationException,
		TypeSizeMismatchException(const string& message): DecompilationException(message) {}
		TypeSizeMismatchException(const string& requiredSizeName, const string& sizeName, const string& typeName):
				DecompilationException("Required " + requiredSizeName + ", got " + sizeName + " of type " + typeName) {}
	);


	DECLARE_EXCEPTION(ClassFormatError, Exception,
		ClassFormatError(const string& message): Exception(message) {}
	);

	DECLARE_EXCEPTION(IllegalOpcodeError, ClassFormatError,
		IllegalOpcodeError(const string& message): ClassFormatError(message) {}
	);

	DECLARE_EXCEPTION(InstructionFormatError, ClassFormatError,
		InstructionFormatError(const string& message): ClassFormatError(message) {}
	);

	DECLARE_EXCEPTION(IllegalAttributeException, ClassFormatError,
		IllegalAttributeException(const string& message): ClassFormatError(message) {}
	);

	DECLARE_EXCEPTION(AttributeNotFoundException, ClassFormatError,
		AttributeNotFoundException(const string& message): ClassFormatError(message) {}
	);


	DECLARE_EXCEPTION(IllegalMethodHeaderException, ClassFormatError,
		IllegalMethodHeaderException(const string& message): ClassFormatError(message) {}
	);


	DECLARE_EXCEPTION(IOException, Exception,
		IOException(): Exception() {}
		IOException(const string& message): Exception(message) {}
	);

	DECLARE_EXCEPTION(EOFException, IOException,
		EOFException(): IOException() {}
	);



	DECLARE_EXCEPTION(CastException, Exception,
		CastException(): Exception() {}
		CastException(const string& message): Exception(message) {}
	);

	#undef DECLARE_EXCEPTION
}

#endif
