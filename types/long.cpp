#ifndef JDECOMPILER_LONG_TYPE_CPP
#define JDECOMPILER_LONG_TYPE_CPP

namespace jdecompiler {

	struct LongType final: PrimitiveType {
		LongType(): PrimitiveType("J", "long", "l") {}

		virtual TypeSize getSize() const override final { return TypeSize::EIGHT_BYTES; }

		virtual const ClassType& getWrapperType() const override;

		static const LongType& getInstance() {
			static const LongType instance;
			return instance;
		}
	};

}

#endif