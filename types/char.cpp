#ifndef JDECOMPILER_CHAR_TYPE_CPP
#define JDECOMPILER_CHAR_TYPE_CPP

namespace jdecompiler {

	struct CharType final: PrimitiveType {
		CharType(): PrimitiveType("C", "char", "c") {}

		virtual TypeSize getSize() const override final { return TypeSize::FOUR_BYTES; }

		virtual bool isSubtypeOfImpl(const Type*) const override;
		virtual const Type* toVariableCapacityIntegralType() const override;

		virtual const ClassType& getWrapperType() const override;

		static const CharType& getInstance() {
			static const CharType instance;
			return instance;
		}
	};

}

#endif