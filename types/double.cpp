#ifndef JDECOMPILER_DOUBLE_TYPE_CPP
#define JDECOMPILER_DOUBLE_TYPE_CPP

namespace jdecompiler {

	struct DoubleType final: PrimitiveType {
		DoubleType(): PrimitiveType("D", "double", "d") {}

		virtual TypeSize getSize() const override final { return TypeSize::EIGHT_BYTES; }

		virtual const ClassType& getWrapperType() const override;

		static const DoubleType& getInstance() {
			static const DoubleType instance;
			return instance;
		}
	};


	static const VoidType *const VOID = &VoidType::getInstance();
	static const BooleanType *const BOOLEAN = &BooleanType::getInstance();
	static const ByteType *const BYTE = &ByteType::getInstance();
	static const CharType *const CHAR = &CharType::getInstance();
	static const ShortType *const SHORT = &ShortType::getInstance();
	static const IntType *const INT = &IntType::getInstance();
	static const LongType *const LONG = &LongType::getInstance();
	static const FloatType *const FLOAT = &FloatType::getInstance();
	static const DoubleType *const DOUBLE = &DoubleType::getInstance();


	bool ByteType::isSubtypeOfImpl(const Type* other) const {
		return other == BYTE || other == SHORT || other == INT;
	}

	bool CharType::isSubtypeOfImpl(const Type* other) const {
		return other == CHAR || other == INT;
	}

	bool ShortType::isSubtypeOfImpl(const Type* other) const {
		return other == SHORT || other == INT;
	}


}

#endif