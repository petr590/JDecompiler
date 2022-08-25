#ifndef JDECOMPILER_CONST_OPERATION_DEFINITIONS_CPP
#define JDECOMPILER_CONST_OPERATION_DEFINITIONS_CPP

namespace jdecompiler {

	template<typename T>
	const Operation* ConstOperation<T>::findConstant(const ConstantDecompilationContext& context) const {

		T value = this->value;
		const Type* type = returnType;

		if(JDecompiler::getInstance().canUseCustomConstants() && context.fieldinfo == nullptr) {
			const Field* foundConstant = nullptr;

			for(const Field* field : context.classinfo.getConstants()) {

				if(field->descriptor.type.isSubtypeOf(type) && field->isConstant() &&
						instanceof<const constantTypeOf<T>*>(field->constantValueAttribute->value) &&
						static_cast<const constantTypeOf<T>*>(field->constantValueAttribute->value)->value == value) {

					if(foundConstant == nullptr) { // Found first constant
						foundConstant = field;
					} else { // Found second constant
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
						NaNField("NaN", type),
						PositiveInfinityField("POSITIVE_INFINITY", type),
						NegativeInfinityField("NEGATIVE_INFINITY", type);

				static const ConstOperation<T> ZERO(0), ONE(1), MINUS_ONE(-1);

				if(isnan(value)) {
					return JDecompiler::getInstance().canUseNaNAndInfinity() && canUseConstant(context.fieldinfo, WRAPPER_CLASS, NaNField) ?
							static_cast<const Operation*>(new GetStaticFieldOperation(WRAPPER_CLASS, NaNField)) :
							new DivOperatorOperation(type, &ZERO, &ZERO); // 0.0 / 0.0
				}

				if(value == numeric_limits<T>::infinity()) {
					return JDecompiler::getInstance().canUseNaNAndInfinity() && canUseConstant(context.fieldinfo, WRAPPER_CLASS, PositiveInfinityField) ?
							static_cast<const Operation*>(new GetStaticFieldOperation(WRAPPER_CLASS, PositiveInfinityField)) :
							new DivOperatorOperation(type, &ONE, &ZERO); // 1.0 / 0.0
				}

				if(value == -numeric_limits<T>::infinity()) {
					return JDecompiler::getInstance().canUseNaNAndInfinity() && canUseConstant(context.fieldinfo, WRAPPER_CLASS, NegativeInfinityField) ?
							static_cast<const Operation*>(new GetStaticFieldOperation(WRAPPER_CLASS, PositiveInfinityField)) :
							new DivOperatorOperation(type, &MINUS_ONE, &ZERO); // -1.0 / 0.0
				}

				if(JDecompiler::getInstance().canUseConstants()) {
					static const FieldDescriptor DenormMinValueField("MIN_VALUE", type);

					if(value == numeric_limits<T>::denorm_min() && canUseConstant(context.fieldinfo, WRAPPER_CLASS, DenormMinValueField)) {
						return new GetStaticFieldOperation(WRAPPER_CLASS, DenormMinValueField);
					}

					if(value == -numeric_limits<T>::denorm_min() && canUseConstant(context.fieldinfo, WRAPPER_CLASS, DenormMinValueField)) {
						return new NegOperatorOperation(type, new GetStaticFieldOperation(WRAPPER_CLASS, DenormMinValueField));
					}
				}
			}

			if(JDecompiler::getInstance().canUseConstants()) {
				static const FieldDescriptor
						MaxValueField("MAX_VALUE", type),
						MinValueField(is_floating_point<T>() ? "MIN_NORMAL" : "MIN_VALUE", type);

				if(value == numeric_limits<T>::max() && canUseConstant(context.fieldinfo, WRAPPER_CLASS, MaxValueField)) {
					return new GetStaticFieldOperation(WRAPPER_CLASS, MaxValueField);
				}

				if(value == -numeric_limits<T>::max() && canUseConstant(context.fieldinfo, WRAPPER_CLASS, MaxValueField)) {
					return new NegOperatorOperation(type, new GetStaticFieldOperation(WRAPPER_CLASS, MaxValueField));
				}

				if(value == numeric_limits<T>::min() && canUseConstant(context.fieldinfo, WRAPPER_CLASS, MinValueField)) {
					return new GetStaticFieldOperation(WRAPPER_CLASS, MinValueField);
				}

				if constexpr(is_floating_point<T>()) { // For int and long MIN_VALUE == -MIN_VALUE
					if(value == -numeric_limits<T>::min() && canUseConstant(context.fieldinfo, WRAPPER_CLASS, MinValueField)) {
						return new NegOperatorOperation(type, new GetStaticFieldOperation(WRAPPER_CLASS, MinValueField));
					}
				}
			}
		}

		return nullptr;
	}

	template<typename T>
	const Operation* FPConstOperation<T>::findConstant(const ConstantDecompilationContext& context) const {

		T value = this->value;

		if(JDecompiler::getInstance().canUseConstants()) {

			static const ClassType MathClass("java/lang/Math");

			static const FieldDescriptor PIField("PI", DOUBLE);

			const bool isNegative = value < 0;

			using OperationCreatorFunc = function<const Operation*(const Operation*)>;

			const OperationCreatorFunc createOperation = isNegative ?
					[this] (const Operation* operation) { return new NegOperatorOperation(ConstOperation<T>::returnType, operation); } :
					static_cast<OperationCreatorFunc>([] (const Operation* operation) { return operation; });

			if(isNegative)
				value = -value;

			const function<const Operation*(const ClassType&, const FieldDescriptor&)> createGetFieldOperation =
					[&createOperation] (const ClassType& clazz, const FieldDescriptor& descriptor) {
						return createOperation(new GetStaticFieldOperation(clazz, descriptor));
					};

			if(value == static_cast<T>(M_PI) && ConstOperation<T>::canUseConstant(context.fieldinfo, MathClass, PIField)) {
				if constexpr(is_float<T>()) {
					return new CastOperation(createGetFieldOperation(MathClass, PIField), FLOAT, true); // cast Math.PI to float
				} else {
					return createGetFieldOperation(MathClass, PIField);
				}
			}

			static const FieldDescriptor EField("E", DOUBLE);

			if(value == static_cast<T>(M_E) && ConstOperation<T>::canUseConstant(context.fieldinfo, MathClass, EField)) {
				if constexpr(is_float<T>()) {
					return new CastOperation(createGetFieldOperation(MathClass, EField), FLOAT, true); // cast Math.E to float
				} else {
					return createGetFieldOperation(MathClass, EField);
				}
			}

			if(isNegative)
				value = -value;
		}

		return ConstOperation<T>::findConstant(context);
	}


	const Operation* StringConstant::toOperation() const {
		return new StringConstOperation(this);
	}

	const Operation* ClassConstant::toOperation() const {
		return new ClassConstOperation(this);
	}

	const Operation* IntegerConstant::toOperation() const {
		return new IConstOperation(value);
	}

	const Operation* FloatConstant::toOperation() const {
		return new FConstOperation(value);
	}

	const Operation* LongConstant::toOperation() const {
		return new LConstOperation(value);
	}

	const Operation* DoubleConstant::toOperation() const {
		return new DConstOperation(value);
	}

	const Operation* MethodTypeConstant::toOperation() const {
		return new MethodTypeConstOperation(this);
	}

	const Operation* MethodHandleConstant::toOperation() const {
		return new MethodHandleConstOperation(this);
	}
}

#endif
