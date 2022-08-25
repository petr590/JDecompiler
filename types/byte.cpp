#ifndef JDECOMPILER_BYTE_TYPE_CPP
#define JDECOMPILER_BYTE_TYPE_CPP

namespace jdecompiler {

	struct ByteType final: IntegralType {
		ByteType(): IntegralType("B", "byte", "b") {}

		virtual TypeSize getSize() const override final { return TypeSize::FOUR_BYTES; }
		virtual uint8_t getCapacity() const override { return 1; }

		virtual bool isSubtypeOfImpl(const Type*) const override;
		virtual const Type* toVariableCapacityIntegralType() const override;

		virtual const ClassType& getWrapperType() const override;

		static const ByteType& getInstance() {
			static const ByteType instance;
			return instance;
		}
	};

}

#endif