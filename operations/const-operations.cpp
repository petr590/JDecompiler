#ifndef JDECOMPILER_CONST_OPERATIONS_CPP
#define JDECOMPILER_CONST_OPERATIONS_CPP

namespace jdecompiler::operations {

	template<typename T>
	struct ConstantTypeOf {
		using type = NumberConstant<T>;
	};

	template<>
	struct ConstantTypeOf<string> {
		using type = StringConstant;
	};


	template<typename T>
	using constantTypeOf = typename ConstantTypeOf<T>::type;



	struct AbstractConstOperation: Operation {
		protected:
			mutable const Type* returnType;

		public:
			AbstractConstOperation(const Type* returnType): returnType(returnType) {}

			virtual const Type* getReturnType() const override {
				return returnType;
			}

		protected:
			virtual void onCastReturnType(const Type* newType) const override {
				returnType = newType;
			}
	};


	template<typename T>
	struct ConstOperation: AbstractConstOperation {

		public:
			const T value;

			static const Type* const TYPE;
			static const ClassType& WRAPPER_CLASS;

		protected:
			static const Operation* valueOf(const T&, const ConstantDecompilationContext);

			static bool canUseConstant(const FieldInfo* fieldinfo, const ClassType& clazz, const FieldDescriptor& descriptor) {
				return fieldinfo == nullptr || fieldinfo->clazz != clazz || fieldinfo->descriptor != descriptor;
			};

			friend struct IConstOperation;
			friend struct LConstOperation;

			ConstOperation(const Type* returnType, const T& value): AbstractConstOperation(returnType), value(value) {}

			ConstOperation(const T& value): ConstOperation(TYPE, value) {}

		public:
			virtual string toString(const StringifyContext& context) const override {
				return primitiveToString(value);
			}
	};

	template<typename T>
	const Type* const ConstOperation<T>::TYPE = exactTypeByBuiltinType<T>();

	template<> const ClassType& ConstOperation<int32_t>::WRAPPER_CLASS = javaLang::Integer;
	template<> const ClassType& ConstOperation<int64_t>::WRAPPER_CLASS = javaLang::Long;
	template<> const ClassType& ConstOperation<float>  ::WRAPPER_CLASS = javaLang::Float;
	template<> const ClassType& ConstOperation<double> ::WRAPPER_CLASS = javaLang::Double;


	struct IConstOperation: ConstOperation<int32_t> {
		private:
			static inline const Type* getTypeByValue(int32_t value) {
				if((bool)value == value)     return ANY_INT_OR_BOOLEAN;
				if((int8_t)value == value)   return ANY_INT;
				if((char16_t)value == value) return (int16_t)value == value ? CHAR_OR_SHORT_OR_INT : CHAR_OR_INT;
				if((int16_t)value == value)  return SHORT_OR_INT;
				return INT;
			}

		protected:
			IConstOperation(int32_t value): ConstOperation(getTypeByValue(value), value) {}

		public:
			static const Operation* valueOf(int32_t value, const ConstantDecompilationContext context) {
				const Operation* result = ConstOperation<int32_t>::valueOf(value, context);
				return result == nullptr ? new IConstOperation(value) : result;
			}

			virtual string toString(const StringifyContext& context) const override {
				if(returnType->isStrictSubtypeOf(INT))     return primitiveToString(value);
				if(returnType->isStrictSubtypeOf(SHORT))   return primitiveToString((int16_t)value);
				if(returnType->isStrictSubtypeOf(CHAR))    return primitiveToString((char16_t)value);
				if(returnType->isStrictSubtypeOf(BYTE))    return primitiveToString((int8_t)value);
				if(returnType->isStrictSubtypeOf(BOOLEAN)) return primitiveToString((bool)value);
				throw IllegalStateException("Illegal type of iconst operation: " + returnType->toString());
			}

			virtual void onCastReturnType(const Type* newType) const override {
				returnType = newType;
			}
	};


	struct LConstOperation: ConstOperation<int64_t> {
		protected:
			LConstOperation(int64_t value): ConstOperation(value) {}

		public:
			static const Operation* valueOf(int64_t, const ConstantDecompilationContext);
	};


	const Operation* LConstOperation::valueOf(int64_t value, const ConstantDecompilationContext context) {
		const Operation* result = ConstOperation<int64_t>::valueOf(value, context);
		return result == nullptr ? new LConstOperation(value) : result;
	}



	template<typename T>
	struct FPConstOperation: ConstOperation<T> {
		static_assert(is_floating_point<T>(), "Only float or double allowed");

		public:
			static const Operation* valueOf(T, const ConstantDecompilationContext);

			FPConstOperation(T value): ConstOperation<T>(value) {}
	};

	using FConstOperation = FPConstOperation<float>;
	using DConstOperation = FPConstOperation<double>;




	struct StringConstOperation: ConstOperation<string> {
		public:
			StringConstOperation(const string& value): ConstOperation(value) {}

			StringConstOperation(const StringConstant* value): StringConstOperation(value->value) {}

			static const Operation* valueOf(const StringConstant* value, const ConstantDecompilationContext context) {
				return valueOf(value->value, context);
			}

			static const Operation* valueOf(const string& value, const ConstantDecompilationContext context) {
				const Operation* operation = ConstOperation<string>::valueOf(value, context);
				return operation == nullptr ? new StringConstOperation(value) : operation;
			}

			virtual string toString(const StringifyContext& context) const override {
				if(JDecompiler::getInstance().multilineStringAllowed() && count(value.begin(), value.end(), '\n') > 1) {
					string result;

					context.classinfo.increaseIndent(2);

					const vector<string> lines = splitAndAddDelimiter(value, '\n');

					auto it = lines.begin();
					while(true) {
						result += primitiveToString(*it);
						if(++it == lines.end())
							break;
						result += (string)" +\n" + context.classinfo.getIndent();
					}

					context.classinfo.reduceIndent(2);

					return result;
				}
				return primitiveToString(value);
			}
	};


	struct EmptyStringConstOperation: Operation {
		private:
			EmptyStringConstOperation() {}

		public:
			virtual string toString(const StringifyContext& context) const override {
				return "\"\"";
			}

			virtual const Type* getReturnType() const override {
				return STRING;
			}

			static const EmptyStringConstOperation* getInstance() {
				static const EmptyStringConstOperation instance;
				return &instance;
			}
	};
}

#endif
