#ifndef JDECOMPILER_BOOLEA_TYPE_CPP
#define JDECOMPILER_BOOLEA_TYPE_CPP

namespace jdecompiler {

	struct BooleanType final: PrimitiveType {
		BooleanType(): PrimitiveType("Z", "boolean", "bool") {}

		virtual TypeSize getSize() const override final { return TypeSize::FOUR_BYTES; }

		virtual const ClassType& getWrapperType() const override;

		static const BooleanType& getInstance() {
			static const BooleanType instance;
			return instance;
		}
	};

}

#endif