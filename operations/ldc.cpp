#ifndef JDECOMPILER_LDC_OPERATION_CPP
#define JDECOMPILER_LDC_OPERATION_CPP

namespace jdecompiler::operations {

	template<TypeSize size, class CT, typename RT>
	struct LdcOperation: ReturnableOperation<> {
		static_assert(is_base_of<ConstValueConstant, CT>::value,
				"template type CT of struct LdcOperation is not subclass of class ConstValueConstant");

		public:
			const uint16_t index;
			const CT* const value;

			LdcOperation(uint16_t index, const CT* value): ReturnableOperation(typeByBuiltinType<RT>()), index(index), value(value) {
				if(returnType->getSize() != size)
					throw TypeSizeMismatchException(TypeSize_nameOf(size), TypeSize_nameOf(returnType->getSize()), returnType->toString());
			}

			LdcOperation(const DecompilationContext& context, uint16_t index): LdcOperation(index, context.constPool.get<CT>(index)) {}

			LdcOperation(const CT* value): LdcOperation(0, value) {}

			virtual string toString(const StringifyContext& context) const override {
				return value->toString(context.classinfo);
			}
	};


	using ClassConstOperation = LdcOperation<TypeSize::FOUR_BYTES, ClassConstant, BuiltinTypes::Class>;
	using MethodTypeConstOperation = LdcOperation<TypeSize::FOUR_BYTES, MethodTypeConstant, BuiltinTypes::MethodType>;
	using MethodHandleConstOperation = LdcOperation<TypeSize::FOUR_BYTES, MethodHandleConstant, BuiltinTypes::MethodHandle>;

}

#endif
