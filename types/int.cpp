#ifndef JDECOMPILER_INT_TYPE_CPP
#define JDECOMPILER_INT_TYPE_CPP

namespace jdecompiler {

	struct IntType final: IntegralType {
		IntType(): IntegralType("I", "int", "n") {}

		virtual TypeSize getSize() const override final { return TypeSize::FOUR_BYTES; }
		virtual uint8_t getCapacity() const override { return 4; }

		virtual const ClassType& getWrapperType() const override;

		static const IntType& getInstance() {
			static const IntType instance;
			return instance;
		}
	};

}

#endif