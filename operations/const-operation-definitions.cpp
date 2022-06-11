#ifndef JDECOMPILER_CONST_OPERATION_DEFINITIONS_CPP
#define JDECOMPILER_CONST_OPERATION_DEFINITIONS_CPP

namespace jdecompiler {
	namespace operations {

		template<typename T>
		const Operation* ConstOperation<T>::valueOf(const T& value, const ConstantDecompilationContext context) {

			if(JDecompiler::getInstance().canUseCustomConstants() && context.fieldinfo == nullptr) {
				const Field* foundConstant = nullptr;

				for(const Field* field : context.classinfo.getConstants()) {

					if(field->descriptor.type.isSubtypeOf(TYPE) && field->isConstant() &&
							instanceof<const constantTypeOf<T>*>(field->constantValueAttribute->value) &&
							static_cast<const constantTypeOf<T>*>(field->constantValueAttribute->value)->value == value) {

						if(foundConstant == nullptr) { // Found first value
							foundConstant = field;
						} else { // Found second value
							foundConstant = nullptr;
							break;
						}
					}
				}

				if(foundConstant != nullptr) {
					return new GetStaticFieldOperation(context.classinfo.thisType, foundConstant->descriptor);
				}
			}


			if constexpr(is_arithmetic<T>()) {

				if constexpr(is_floating_point<T>()) {
					static const FieldDescriptor
							NaNField("NaN", TYPE),
							PositiveInfinityField("POSITIVE_INFINITY", TYPE),
							NegativeInfinityField("NEGATIVE_INFINITY", TYPE);

					if(isnan(value)) {
						return JDecompiler::getInstance().canUseNaNAndInfinity() && canUseConstant(context.fieldinfo, WRAPPER_CLASS, NaNField) ?
								(const Operation*)new GetStaticFieldOperation(WRAPPER_CLASS, NaNField) :
								new DivOperatorOperation(TYPE, new ConstOperation<T>(0), new ConstOperation<T>(0)); // 0.0 / 0.0
					}

					if(value == numeric_limits<T>::infinity()) {
						return JDecompiler::getInstance().canUseNaNAndInfinity() && canUseConstant(context.fieldinfo, WRAPPER_CLASS, PositiveInfinityField) ?
							(const Operation*)new GetStaticFieldOperation(WRAPPER_CLASS, PositiveInfinityField) :
							new DivOperatorOperation(TYPE, new ConstOperation<T>(1), new ConstOperation<T>(0)); // 1.0 / 0.0
					}

					if(value == -numeric_limits<T>::infinity()) {
						return JDecompiler::getInstance().canUseNaNAndInfinity() && canUseConstant(context.fieldinfo, WRAPPER_CLASS, NegativeInfinityField) ?
							(const Operation*)new GetStaticFieldOperation(WRAPPER_CLASS, PositiveInfinityField) :
							new DivOperatorOperation(TYPE, new ConstOperation<T>(-1), new ConstOperation<T>(0)); // -1.0 / 0.0
					}

					if(JDecompiler::getInstance().canUseConstants()) {
						static const FieldDescriptor DenormMinValueField("MIN_VALUE", TYPE);

						if(value == numeric_limits<T>::denorm_min() && canUseConstant(context.fieldinfo, WRAPPER_CLASS, DenormMinValueField)) {
							return new GetStaticFieldOperation(WRAPPER_CLASS, DenormMinValueField);
						}

						if(value == -numeric_limits<T>::denorm_min() && canUseConstant(context.fieldinfo, WRAPPER_CLASS, DenormMinValueField)) {
							return new NegOperatorOperation(TYPE, new GetStaticFieldOperation(WRAPPER_CLASS, DenormMinValueField));
						}
					}
				}

				if(JDecompiler::getInstance().canUseConstants()) {
					static const FieldDescriptor
							MaxValueField("MAX_VALUE", TYPE),
							MinValueField(is_floating_point<T>() ? "MIN_NORMAL" : "MIN_VALUE", TYPE);

					if(value == numeric_limits<T>::max() && canUseConstant(context.fieldinfo, WRAPPER_CLASS, MaxValueField)) {
						return new GetStaticFieldOperation(WRAPPER_CLASS, MaxValueField);
					}

					if(value == -numeric_limits<T>::max() && canUseConstant(context.fieldinfo, WRAPPER_CLASS, MaxValueField)) {
						return new NegOperatorOperation(TYPE, new GetStaticFieldOperation(WRAPPER_CLASS, MaxValueField));
					}

					if(value == numeric_limits<T>::min() && canUseConstant(context.fieldinfo, WRAPPER_CLASS, MinValueField)) {
						return new GetStaticFieldOperation(WRAPPER_CLASS, MinValueField);
					}

					if constexpr(is_floating_point<T>()) { // For int and long MIN_VALUE == -MIN_VALUE
						if(value == -numeric_limits<T>::min() && canUseConstant(context.fieldinfo, WRAPPER_CLASS, MinValueField)) {
							return new NegOperatorOperation(TYPE, new GetStaticFieldOperation(WRAPPER_CLASS, MinValueField));
						}
					}
				}
			}

			return nullptr;
		}

		template<typename T>
		const Operation* FPConstOperation<T>::valueOf(T value, const ConstantDecompilationContext context) {
			if(JDecompiler::getInstance().canUseConstants()) {

				static const ClassType MathClass("java/lang/Math");

				static const FieldDescriptor PIField("PI", DOUBLE);

				const bool isNegative = value < 0;

				using operationCreator = function<const Operation*(const Operation*)>;

				const operationCreator createOperation = isNegative ?
						[] (const Operation* operation) { return new NegOperatorOperation(ConstOperation<T>::TYPE, operation); } :
						static_cast<operationCreator>([] (const Operation* operation) { return operation; });

				if(isNegative)
					value = -value;

				const function<const Operation*(const ClassType&, const FieldDescriptor&)> createGetFieldOperation =
						[&createOperation] (const ClassType& clazz, const FieldDescriptor& descriptor) {
							return createOperation(new GetStaticFieldOperation(clazz, descriptor));
						};

				if(value == (T)M_PI && ConstOperation<T>::canUseConstant(context.fieldinfo, MathClass, PIField)) {
					if constexpr(is_float<T>()) {
						return new CastOperation(createGetFieldOperation(MathClass, PIField), FLOAT, true); // cast Math.PI to float
					} else {
						return createGetFieldOperation(MathClass, PIField);
					}
				}

				static const FieldDescriptor EField("E", DOUBLE);

				if(value == (T)M_E && ConstOperation<T>::canUseConstant(context.fieldinfo, MathClass, EField)) {
					if constexpr(is_float<T>()) {
						return new CastOperation(createGetFieldOperation(MathClass, EField), FLOAT, true); // cast Math.E to float
					} else {
						return createGetFieldOperation(MathClass, EField);
					}
				}

				if(isNegative)
					value = -value;
			}

			const Operation* result = ConstOperation<T>::valueOf(value, context);
			return result == nullptr ? new FPConstOperation<T>(value) : result;
		}
	}


	const Operation* StringConstant::toOperation(const ConstantDecompilationContext context) const {
		return operations::StringConstOperation::valueOf(this, context);
	}

	const Operation* ClassConstant::toOperation(const ConstantDecompilationContext context) const {
		return new operations::ClassConstOperation(this);
	}

	const Operation* IntegerConstant::toOperation(const ConstantDecompilationContext context) const {
		return operations::IConstOperation::valueOf(value, context);
	}

	const Operation* FloatConstant::toOperation(const ConstantDecompilationContext context) const {
		return operations::FConstOperation::valueOf(value, context);
	}

	const Operation* LongConstant::toOperation(const ConstantDecompilationContext context) const {
		return operations::LConstOperation::valueOf(value, context);
	}

	const Operation* DoubleConstant::toOperation(const ConstantDecompilationContext context) const {
		return operations::DConstOperation::valueOf(value, context);
	}

	const Operation* MethodTypeConstant::toOperation(const ConstantDecompilationContext context) const {
		return new operations::MethodTypeConstOperation(this);
	}

	const Operation* MethodHandleConstant::toOperation(const ConstantDecompilationContext context) const {
		return new operations::MethodHandleConstOperation(this);
	}
}

#endif
