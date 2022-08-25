#ifndef JDECOMPILER_FLOAT_TYPE_CPP
#define JDECOMPILER_FLOAT_TYPE_CPP

namespace jdecompiler {

	struct FloatType final: PrimitiveType {
		FloatType(): PrimitiveType("F", "float", "f") {}

		virtual TypeSize getSize() const override final { return TypeSize::FOUR_BYTES; }

		virtual const ClassType& getWrapperType() const override;

		static const FloatType& getInstance() {
			static const FloatType instance;
			return instance;
		}
	};

}

#endif