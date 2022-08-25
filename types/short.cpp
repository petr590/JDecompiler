#ifndef JDECOMPILER_SHORT_TYPE_CPP
#define JDECOMPILER_SHORT_TYPE_CPP

namespace jdecompiler {

	struct ShortType final: IntegralType {
		ShortType(): IntegralType("S", "short", "s") {}

		virtual TypeSize getSize() const override final { return TypeSize::FOUR_BYTES; }
		virtual uint8_t getCapacity() const override { return 2; }

		virtual bool isSubtypeOfImpl(const Type*) const override;
		virtual const Type* toVariableCapacityIntegralType() const override;

		virtual const ClassType& getWrapperType() const override;

		static const ShortType& getInstance() {
			static const ShortType instance;
			return instance;
		}
	};

}

#endif