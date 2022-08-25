#ifndef JDECOMPILER_CONST_OPERATIONS_CPP
#define JDECOMPILER_CONST_OPERATIONS_CPP

namespace jdecompiler {

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

			virtual string toString(const StringifyContext& context) const override {
				return toString(context, ConstantDecompilationContext(context.classinfo));
			}

			virtual string toString(const StringifyContext&, const ConstantDecompilationContext&) const override = 0;

			virtual bool isAbstractConstOperation() const override final {
				return true;
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
			virtual const Operation* findConstant(const ConstantDecompilationContext&) const;

			static bool canUseConstant(const FieldInfo* fieldinfo, const ClassType& clazz, const FieldDescriptor& descriptor) {
				return fieldinfo == nullptr || fieldinfo->clazz != clazz || fieldinfo->descriptor != descriptor;
			};

			friend struct IConstOperation;
			friend struct LConstOperation;

			ConstOperation(const Type* returnType, const T& value): AbstractConstOperation(returnType), value(value) {}

			ConstOperation(const T& value): ConstOperation(TYPE, value) {}

		public:
			virtual string toString(const StringifyContext& context, const ConstantDecompilationContext& constantContext) const override {
				const Operation* operation = findConstant(constantContext);
				if(operation != nullptr)
					return operation->toString(context);

				return primitiveToString(value);
			}
	};

	template<typename T>
	const Type* const ConstOperation<T>::TYPE = typeByBuiltinJavaType<T>();

	template<> const ClassType& ConstOperation<jint>   ::WRAPPER_CLASS = javaLang::Integer;
	template<> const ClassType& ConstOperation<jlong>  ::WRAPPER_CLASS = javaLang::Long;
	template<> const ClassType& ConstOperation<jfloat> ::WRAPPER_CLASS = javaLang::Float;
	template<> const ClassType& ConstOperation<jdouble>::WRAPPER_CLASS = javaLang::Double;


	struct IConstOperation: ConstOperation<jint> {
		private:
			static inline const Type* getTypeByValue(jint value) {
				if((jbool)value == value)  return ANY_INT_OR_BOOLEAN;
				if((jbyte)value == value)  return value > 0 ? ANY_INT : ANY_SIGNED_INT;
				if((jchar)value == value)  return (jshort)value == value ? CHAR_OR_SHORT_OR_INT : CHAR_OR_INT;
				if((jshort)value == value) return SHORT_OR_INT;
				return INT;
			}

		public:
			IConstOperation(jint value): ConstOperation(getTypeByValue(value), value) {}

			virtual string toString(const StringifyContext& context, const ConstantDecompilationContext& constantContext) const override {
				if(const Operation* operation = findConstant(constantContext))
					return operation->toString(context);

				returnType = returnType->getReducedType();

				if(returnType->isStrictSubtypeOf(INT))     return primitiveToString(value);
				if(returnType->isStrictSubtypeOf(SHORT))   return primitiveToString((jshort)value);
				if(returnType->isStrictSubtypeOf(CHAR))    return primitiveToString((jchar)value);
				if(returnType->isStrictSubtypeOf(BYTE))    return primitiveToString((jbyte)value);
				if(returnType->isStrictSubtypeOf(BOOLEAN)) return primitiveToString((jbool)value);
				throw IllegalStateException("Illegal type of iconst operation: " + returnType->toString());
			}
	};


	template<typename T>
	struct IntConvertibleConstOperation: ConstOperation<T> {

		protected:
			mutable bool implicit = false;

			IntConvertibleConstOperation(const T& value): ConstOperation<T>(value) {}

		public:
			virtual const Type* getImplicitType() const override {
				return (jint)ConstOperation<T>::value == ConstOperation<T>::value ? INT : ConstOperation<T>::returnType;
			}

			virtual void allowImplicitCast() const override {
				implicit = (jint)ConstOperation<T>::value == ConstOperation<T>::value;
			}

			virtual string toString(const StringifyContext& context, const ConstantDecompilationContext& constantContext) const override {
				if(const Operation* operation = this->findConstant(constantContext))
					return operation->toString(context);

				return implicit ? primitiveToString((jint)ConstOperation<T>::value) : primitiveToString(ConstOperation<T>::value);
			}
	};



	struct LConstOperation: IntConvertibleConstOperation<jlong> {
		public:
			LConstOperation(jlong value): IntConvertibleConstOperation(value) {}
	};



	template<typename T>
	struct FPConstOperation: IntConvertibleConstOperation<T> {
		static_assert(is_floating_point<T>(), "Only float or double allowed");

		protected:
			virtual const Operation* findConstant(const ConstantDecompilationContext&) const override;

		public:
			FPConstOperation(T value): IntConvertibleConstOperation<T>(value) {}
	};

	using FConstOperation = FPConstOperation<jfloat>;
	using DConstOperation = FPConstOperation<jdouble>;




	struct StringConstOperation: ConstOperation<string> {
		public:
			StringConstOperation(const string& value): ConstOperation(value) {}

			StringConstOperation(const StringConstant* value): StringConstOperation(value->value) {}

		public:
			virtual string toString(const StringifyContext& context, const ConstantDecompilationContext& constantContext) const override {
				if(const Operation* operation = findConstant(constantContext))
					return operation->toString(context);

				if(JDecompiler::getInstance().multilineStringAllowed()) {
					size_t lnPos = value.find('\n');

					if(lnPos != string::npos && lnPos != value.size() - 1) {
						string result;

						context.classinfo.increaseIndent(2);

						const vector<string> lines = splitAndAddDelimiter(value, '\n');

						auto it = lines.begin();
						while(true) {
							result += (string)"\n" + context.classinfo.getIndent() + primitiveToString(*it);
							if(++it == lines.end())
								break;
							result += " +";
						}

						context.classinfo.reduceIndent(2);

						return result;
					}
				}
				return primitiveToString(value);
			}
	};


	struct EmptyStringConstOperation: Operation {
		private:
			constexpr EmptyStringConstOperation() noexcept {}

		public:
			virtual string toString(const StringifyContext&) const override {
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
