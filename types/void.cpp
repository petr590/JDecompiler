#ifndef JDECOMPILER_VOID_TYPE_CPP
#define JDECOMPILER_VOID_TYPE_CPP

namespace jdecompiler {

	struct VoidType final: PrimitiveType {
		VoidType(): PrimitiveType("V", "void", "v") {}

		virtual TypeSize getSize() const override final { return TypeSize::ZERO_BYTES; }

		virtual const ClassType& getWrapperType() const override;

		static const VoidType& getInstance() {
			static const VoidType instance;
			return instance;
		}
	};

}

#endif