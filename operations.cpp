#ifndef JDECOMPILER_OPERATIONS_CPP
#define JDECOMPILER_OPERATIONS_CPP

#include "class.cpp"

namespace jdecompiler {

	namespace operations {

		template<class T = Type>
		struct ReturnableOperation: Operation { // ReturnableOperation is an operation which returns specified type
			static_assert(is_base_of<Type, T>::value, "template class T of struct ReturnableOperation is not subclass of class Type");

			protected:
				const T* const returnType;

			public:
				ReturnableOperation(const T* returnType): returnType(returnType) {}

				virtual const Type* getReturnType() const override { return returnType; }
		};

		struct IntOperation: Operation {
			inline IntOperation() {}

			virtual const Type* getReturnType() const override { return INT; }
		};

		struct AnyIntOperation: Operation {
			inline AnyIntOperation() {}

			virtual const Type* getReturnType() const override { return ANY_INT; }
		};

		struct BooleanOperation: Operation {
			inline BooleanOperation() {}

			virtual const Type* getReturnType() const override { return BOOLEAN; }
		};

		struct VoidOperation: Operation {
			VoidOperation() {}

			virtual const Type* getReturnType() const override { return VOID; }
		};


		struct TransientReturnableOperation: Operation {
			protected:
				mutable const Type* returnType;

				TransientReturnableOperation(const Type* returnType): returnType(returnType) {}
				TransientReturnableOperation(): returnType(nullptr) {}

				template<class D, class... Ds>
				inline void initReturnType(const DecompilationContext& context, const Operation* operation) {
					returnType = getDupReturnType<D, Ds...>(context, operation);
				}

				virtual const Type* getReturnType() const override {
					return returnType;
				}
		};


		// ----------------------------------------------------------------------------------------------------


		template<TypeSize size>
		struct TypeSizeTemplatedOperation {
			protected:
				template<TypeSize S = size>
				void checkTypeSize(const Type* type) const {
					if(type->getSize() != S)
						throw TypeSizeMismatchException(TypeSize_nameOf(S), TypeSize_nameOf(type->getSize()), type->toString());
				}
		};


		template<TypeSize size>
		struct AbstractDupOperation: Operation, TypeSizeTemplatedOperation<size> {
			const Operation* const operation;

			AbstractDupOperation(const DecompilationContext& context): operation(context.stack.top()) {
				TypeSizeTemplatedOperation<size>::checkTypeSize(operation->getReturnType());
			}

			virtual string toString(const StringifyContext& context) const override {
				return operation->toString(context);
			}

			virtual const Type* getReturnType() const override {
				return operation->getReturnType();
			}

			virtual Priority getPriority() const override {
				return operation->getPriority();
			}

			virtual const Operation* getOriginalOperation() const override {
				return operation->getOriginalOperation();
			}
		};


		template<TypeSize size>
		struct DupOperation: AbstractDupOperation<size> {
			DupOperation(const DecompilationContext& context): AbstractDupOperation<size>(context) {}
		};

		using Dup1Operation = DupOperation<TypeSize::FOUR_BYTES>;
		using Dup2Operation = DupOperation<TypeSize::EIGHT_BYTES>;


		struct DupX1Operation: AbstractDupOperation<TypeSize::FOUR_BYTES> {
			DupX1Operation(const DecompilationContext& context): AbstractDupOperation<TypeSize::FOUR_BYTES>(context) {
				if(context.stack.size() < 2)
					throw IllegalStackStateException("Too less operations on stack for dup_x1: required 2, got " + context.stack.size());

				const Operation
					*operation1 = context.stack.pop(),
					*operation2 = context.stack.pop();
				TypeSizeTemplatedOperation<TypeSize::FOUR_BYTES>::checkTypeSize(operation2->getReturnType());
				context.stack.push(operation1, operation2);
			}
		};


		struct DupX2Operation: AbstractDupOperation<TypeSize::FOUR_BYTES> {
			DupX2Operation(const DecompilationContext& context): AbstractDupOperation<TypeSize::FOUR_BYTES>(context) {
				if(context.stack.size() < 3)
					throw IllegalStackStateException("Too less operations on stack for dup_x2: required 3, got " + context.stack.size());

				const Operation
					*operation1 = context.stack.pop(),
					*operation2 = context.stack.pop(),
					*operation3 = context.stack.pop();

				checkTypeSize(operation2->getReturnType());
				checkTypeSize(operation3->getReturnType());

				context.stack.push(operation1, operation3, operation2);
			}
		};


		struct Dup2X1Operation: AbstractDupOperation<TypeSize::EIGHT_BYTES> {
			Dup2X1Operation(const DecompilationContext& context): AbstractDupOperation<TypeSize::EIGHT_BYTES>(context) {
				if(context.stack.size() < 2)
					throw IllegalStackStateException("Too less operations on stack for dup2_x1: required 2, got " + context.stack.size());

				const Operation
					*operation1 = context.stack.pop(),
					*operation2 = context.stack.pop();

				checkTypeSize<TypeSize::FOUR_BYTES>(operation2->getReturnType());

				context.stack.push(operation1, operation2);
			}
		};


		struct Dup2X2Operation: AbstractDupOperation<TypeSize::EIGHT_BYTES> {
			Dup2X2Operation(const DecompilationContext& context): AbstractDupOperation<TypeSize::EIGHT_BYTES>(context) {
				if(context.stack.size() < 2)
					throw IllegalStackStateException("Too less operations on stack for dup2_x2: required 2, got " + context.stack.size());

				const Operation
					*operation1 = context.stack.pop(),
					*operation2 = context.stack.pop();

				checkTypeSize(operation2->getReturnType());

				context.stack.push(operation1, operation2);
			}
		};


		template<TypeSize size>
		struct PopOperation: VoidOperation, TypeSizeTemplatedOperation<size> {
			const Operation* const operation;

			PopOperation(const DecompilationContext& context): operation(context.stack.pop()) {
				TypeSizeTemplatedOperation<size>::checkTypeSize(operation->getReturnType());
			}

			virtual string toString(const StringifyContext& context) const override {
				return operation->toString(context);
			}
		};


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
				static const Operation* valueOf(T, const FieldInfo* fieldinfo = nullptr);

				static bool canUseConstant(const FieldInfo* fieldinfo, const ClassType& clazz, const FieldDescriptor& descriptor) {
					return fieldinfo == nullptr || fieldinfo->clazz != clazz || fieldinfo->descriptor != descriptor;
				};

				friend struct IConstOperation;
				friend struct LConstOperation;

				ConstOperation(const Type* returnType, const T value): AbstractConstOperation(returnType), value(value) {}

				ConstOperation(const T value): ConstOperation(TYPE, value) {}

			public:
				virtual string toString(const StringifyContext& context) const override {
					return primitiveToString(value);
				}
		};

		template<typename T>
		const Type* const ConstOperation<T>::TYPE = typeByBuiltinType<T>();

		template<> const ClassType& ConstOperation<int32_t>::WRAPPER_CLASS = javase::java::lang::Integer;
		template<> const ClassType& ConstOperation<int64_t>::WRAPPER_CLASS = javase::java::lang::Long;
		template<> const ClassType& ConstOperation<float>  ::WRAPPER_CLASS = javase::java::lang::Float;
		template<> const ClassType& ConstOperation<double> ::WRAPPER_CLASS = javase::java::lang::Double;


		struct IConstOperation: ConstOperation<int32_t> {
			private:
				static inline const Type* getTypeByValue(int32_t value) {
					static const AmbigousType
							CHAR_OR_SHORT_OR_INT({CHAR, SHORT, INT}),
							CHAR_OR_INT({CHAR, INT}),
							SHORT_OR_INT({SHORT, INT});

					if((bool)value == value)     return ANY_INT_OR_BOOLEAN;
					if((int8_t)value == value)   return ANY_INT;
					if((char16_t)value == value) {
						if((int16_t)value == value)
							return &CHAR_OR_SHORT_OR_INT;
						return &CHAR_OR_INT;
					}
					if((int16_t)value == value)  return &SHORT_OR_INT;
					return INT;
				}

			protected:
				IConstOperation(int32_t value): ConstOperation(getTypeByValue(value), value) {}

			public:
				static const Operation* valueOf(int32_t value, const FieldInfo* fieldinfo = nullptr) {
					const Operation* result = ConstOperation<int32_t>::valueOf(value, fieldinfo);
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
				static const Operation* valueOf(int64_t, const FieldInfo* fieldinfo = nullptr);
		};


		const Operation* LConstOperation::valueOf(int64_t value, const FieldInfo* fieldinfo) {
			const Operation* result = ConstOperation<int64_t>::valueOf(value, fieldinfo);
			return result == nullptr ? new LConstOperation(value) : result;
		}



		template<typename T>
		struct FPConstOperation: ConstOperation<T> {
			static_assert(is_floating_point<T>::value, "Only float or double allowed");

			public:
				static const Operation* valueOf(T value, const FieldInfo* fieldinfo = nullptr);

				FPConstOperation(T value): ConstOperation<T>(value) {}
		};

		using FConstOperation = FPConstOperation<float>;
		using DConstOperation = FPConstOperation<double>;


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


		using StringConstOperation = LdcOperation<TypeSize::FOUR_BYTES, StringConstant, BuiltinTypes::String>;
		using ClassConstOperation = LdcOperation<TypeSize::FOUR_BYTES, ClassConstant, BuiltinTypes::Class>;
		using MethodTypeConstOperation = LdcOperation<TypeSize::FOUR_BYTES, MethodTypeConstant, BuiltinTypes::MethodType>;
		using MethodHandleConstOperation = LdcOperation<TypeSize::FOUR_BYTES, MethodHandleConstant, BuiltinTypes::MethodHandle>;


		struct LoadOperation: Operation {
			public:
				const uint16_t index;
				const Variable& variable;

				LoadOperation(const Type* requiredType, const DecompilationContext& context, uint16_t index):
						index(index), variable(context.getCurrentScope()->getVariable(index, true)) {
					variable.castTypeTo(requiredType);
				}

				virtual string toString(const StringifyContext& context) const override {
					return context.getCurrentScope()->getNameFor(variable);
				}

				virtual const Type* getReturnType() const override {
					return variable.getType();
				}

				virtual void onCastReturnType(const Type* newType) const override {
					variable.setType(newType);
				}
		};

		struct ILoadOperation: LoadOperation {
			ILoadOperation(const DecompilationContext& context, uint16_t index): LoadOperation(ANY_INT_OR_BOOLEAN, context, index) {}
		};

		struct LLoadOperation: LoadOperation {
			LLoadOperation(const DecompilationContext& context, uint16_t index): LoadOperation(LONG, context, index) {}
		};

		struct FLoadOperation: LoadOperation {
			FLoadOperation(const DecompilationContext& context, uint16_t index): LoadOperation(FLOAT, context, index) {}
		};

		struct DLoadOperation: LoadOperation {
			DLoadOperation(const DecompilationContext& context, uint16_t index): LoadOperation(DOUBLE, context, index) {}
		};

		struct ALoadOperation: LoadOperation {
			ALoadOperation(const DecompilationContext& context, uint16_t index): LoadOperation(AnyObjectType::getInstance(), context, index) {}
		};


		struct ArrayLoadOperation: Operation {
			public:
				const Operation *const index, *const array;
				const Type* const returnType;

			protected:
				ArrayLoadOperation(const Type* elementType, const DecompilationContext& context):
						index(context.stack.popAs(INT)), array(context.stack.pop()),
						returnType(array->getReturnTypeAs(new ArrayType(elementType))->elementType) {}

			public:
				virtual string toString(const StringifyContext& context) const override {
					return array->toString(context) + '[' + index->toString(context) + ']';
				}

				virtual const Type* getReturnType() const override {
					return returnType;
				}
		};

		struct IALoadOperation: ArrayLoadOperation {
			IALoadOperation(const DecompilationContext& context): ArrayLoadOperation(INT, context) {}
		};

		struct LALoadOperation: ArrayLoadOperation {
			LALoadOperation(const DecompilationContext& context): ArrayLoadOperation(LONG, context) {}
		};

		struct FALoadOperation: ArrayLoadOperation {
			FALoadOperation(const DecompilationContext& context): ArrayLoadOperation(FLOAT, context) {}
		};

		struct DALoadOperation: ArrayLoadOperation {
			DALoadOperation(const DecompilationContext& context): ArrayLoadOperation(DOUBLE, context) {}
		};

		struct AALoadOperation: ArrayLoadOperation {
			AALoadOperation(const DecompilationContext& context): ArrayLoadOperation(
					context.stack.lookup(1)->getReturnTypeAs(AnyType::getArrayTypeInstance())->elementType, context) {}
		};

		struct BALoadOperation: ArrayLoadOperation {
			BALoadOperation(const DecompilationContext& context): ArrayLoadOperation(BYTE_OR_BOOLEAN, context) {}
		};

		struct CALoadOperation: ArrayLoadOperation {
			CALoadOperation(const DecompilationContext& context): ArrayLoadOperation(CHAR, context) {}
		};

		struct SALoadOperation: ArrayLoadOperation {
			SALoadOperation(const DecompilationContext& context): ArrayLoadOperation(SHORT, context) {}
		};



		struct OperatorOperation: ReturnableOperation<> {
			public:
				const char* const stringOperator;

			private:
				const char* const oppositeOperator;

				const Priority priority;

				static const char* oppositeOperatorOf(char32_t charOperator) {
					switch(charOperator) {
						case '+': return "-";
						case '-': return "+";
						case '*': return "/";
						case '/': return "*";
						default: return "";
					}
				}

			public:
				OperatorOperation(char32_t charOperator, Priority priority, const Type* type):
						ReturnableOperation(type), stringOperator(char32ToString(charOperator)),
						oppositeOperator(oppositeOperatorOf(charOperator)), priority(priority) {}

				virtual Priority getPriority() const override {
					return priority;
				}

				const char* getOppositeOperator() const {
					if(*oppositeOperator != '\0')
						return oppositeOperator;
					throw IllegalStateException((string)"Cannot get opposite operator for operator '" + stringOperator + '\'');
				}
		};


		struct BinaryOperatorOperation: OperatorOperation {
			public:
				const Operation *const operand2, *const operand1;

			private:
				inline void castOperands(const Type* type) {
					const Type* requiredType = operand1->getReturnType()->twoWayCastTo(operand2->getReturnType())->castTo(type);
					operand1->twoWayCastReturnTypeTo(requiredType);
					operand2->twoWayCastReturnTypeTo(requiredType);
				}

			public:
				BinaryOperatorOperation(char32_t charOperator, Priority priority, const Type* type1, const Type* type2, const DecompilationContext& context):
							OperatorOperation(charOperator, priority, type1),
							operand2(context.stack.popAs(type2)), operand1(context.stack.popAs(type1)) {}

				BinaryOperatorOperation(char32_t charOperator, Priority priority, const Type* type, const DecompilationContext& context):
						OperatorOperation(charOperator, priority, type), operand2(context.stack.pop()), operand1(context.stack.pop()) {
					castOperands(type);
				}

				BinaryOperatorOperation(char32_t charOperator, Priority priority, const Type* type, const Operation* operand1, const Operation* operand2):
						OperatorOperation(charOperator, priority, type), operand2(operand2), operand1(operand1) {
					castOperands(type);
				}

				virtual string toString(const StringifyContext& context) const override {
					return toStringPriority(operand1, context, Associativity::LEFT) + ' ' + stringOperator + ' ' +
							toStringPriority(operand2, context, Associativity::RIGHT);
				}

				string toShortFormString(const StringifyContext& context, const Variable& variable) const {
					return context.getCurrentScope()->getNameFor(variable) + ' ' + stringOperator + "= " + operand2->toString(context);
				}
		};


		/* The template parameter named priority_parameter is named to avoid
		   naming conflicts with field OperatorOperation::priority */
		template<char32_t charOperator, Priority priority_parameter>
		struct BinaryOperatorOperationImpl: BinaryOperatorOperation {
			BinaryOperatorOperationImpl(const Type* type1, const Type* type2, const DecompilationContext& context):
					BinaryOperatorOperation(charOperator, priority_parameter, type1, type2, context) {}

			BinaryOperatorOperationImpl(const Type* type, const DecompilationContext& context):
					BinaryOperatorOperation(charOperator, priority_parameter, type, context) {}

			BinaryOperatorOperationImpl(const Type* type, const Operation* operand1, const Operation* operand2):
					BinaryOperatorOperation(charOperator, priority_parameter, type, operand1, operand2) {}
		};


		/* Operator bit not realized in java through operator xor with value -1 */
		template<Priority priority_parameter>
		struct BinaryOperatorOperationImpl<'^', priority_parameter>: BinaryOperatorOperation {
			const bool isBitNot;

			BinaryOperatorOperationImpl(const Type* type, const DecompilationContext& context):
						BinaryOperatorOperation('^', priority_parameter, type, context),
						isBitNot((instanceof<const IConstOperation*>(operand2) && static_cast<const IConstOperation*>(operand2)->value == -1)
							|| (instanceof<const LConstOperation*>(operand2) && static_cast<const LConstOperation*>(operand2)->value == -1)) {}

			virtual string toString(const StringifyContext& context) const override {
				return isBitNot ? '~' + this->toStringPriority(operand1, context, Associativity::RIGHT):
						this->toStringPriority(operand1, context, Associativity::LEFT) + ' ' + this->stringOperator + ' ' +
						this->toStringPriority(operand2, context, Associativity::RIGHT);
			}

			virtual Priority getPriority() const override {
				return isBitNot ? Priority::BIT_NOT : priority_parameter;
			}
		};

		template<char32_t charOperator, Priority priority_parameter>
		struct UnaryOperatorOperation: OperatorOperation {
			public:
				const Operation* const operand;

				UnaryOperatorOperation(const Type* type, const Operation* operand):
						OperatorOperation(charOperator, priority_parameter, type), operand(operand) {}

				UnaryOperatorOperation(const Type* type, const DecompilationContext& context):
						UnaryOperatorOperation(type, context.stack.popAs(type)) {}

				virtual string toString(const StringifyContext& context) const override {
					return this->stringOperator + this->toStringPriority(operand, context, Associativity::RIGHT);
				}
		};


		using AddOperatorOperation = BinaryOperatorOperationImpl<'+', Priority::PLUS>;
		using SubOperatorOperation = BinaryOperatorOperationImpl<'-', Priority::MINUS>;
		using MulOperatorOperation = BinaryOperatorOperationImpl<'*', Priority::MULTIPLE>;
		using DivOperatorOperation = BinaryOperatorOperationImpl<'/', Priority::DIVISION>;
		using RemOperatorOperation = BinaryOperatorOperationImpl<'%', Priority::REMAINDER>;

		using NegOperatorOperation = UnaryOperatorOperation<'-', Priority::UNARY_MINUS>;

		/*using ShiftLeftOperatorOperation = ShiftOperatorOperation<"<<"_c32, Priority::SHIFT>;
		using ShiftRightOperatorOperation = ShiftOperatorOperation<">>"_c32, Priority::SHIFT>;
		using UShiftRightOperatorOperation = ShiftOperatorOperation<">>>"_c32, Priority::SHIFT>;

		using AndOperatorOperation = BooleanBinaryOperatorOperation<'&', Priority::BIT_AND>;
		using OrOperatorOperation  = BooleanBinaryOperatorOperation<'|', Priority::BIT_OR>;
		using XorOperatorOperation = BooleanBinaryOperatorOperation<'^', Priority::BIT_XOR>;*/



		struct StoreOperation: TransientReturnableOperation {
			public:
				const Operation* const value;
				const uint16_t index;
				const Variable& variable;

			protected:
				int_fast32_t postInc = 0;
				const BinaryOperatorOperation* shortFormOperator = nullptr;
				const Operation* revokeIncrementOperation = nullptr;
				bool canDeclare = false;

			public:
				StoreOperation(const Type* requiredType, const DecompilationContext& context, uint16_t index):
						value(context.stack.popAs(requiredType)), index(index), variable(context.getCurrentScope()->getVariable(index, false)) {

					initReturnType<Dup1Operation, Dup2Operation>(context, value);
					value->castReturnTypeTo(requiredType);
					variable.bindTo(value);

					if(!variable.isDeclared() && returnType == VOID) {
						variable.setDeclared(true);
						canDeclare = true;
					} else if(const BinaryOperatorOperation* binaryOperator = dynamic_cast<const BinaryOperatorOperation*>(value->getOriginalOperation())) {
						if(const LoadOperation* loadOperation = dynamic_cast<const LoadOperation*>(binaryOperator->operand1->getOriginalOperation())) {
							if(loadOperation->index == index) {
								shortFormOperator = binaryOperator;

								const AbstractConstOperation* constOperation = dynamic_cast<const AbstractConstOperation*>(binaryOperator->operand2);

								if(returnType == VOID && constOperation != nullptr) { // post increment
									int_fast32_t incValue = instanceof<const AddOperatorOperation*>(binaryOperator) ? 1 :
												instanceof<const SubOperatorOperation*>(binaryOperator) ? -1 : 0;

									if(incValue != 0) {
										initReturnType<Dup1Operation, Dup2Operation>(context, binaryOperator->operand1);

										if(returnType != VOID) {
											bool isPostInc =
												instanceof<const IConstOperation*>(constOperation) ?
													static_cast<const IConstOperation*>(constOperation)->value == 1 :
												instanceof<const LConstOperation*>(constOperation) ?
													static_cast<const LConstOperation*>(constOperation)->value == 1 :
												instanceof<const FConstOperation*>(constOperation) ?
													static_cast<const FConstOperation*>(constOperation)->value == 1 :
												instanceof<const DConstOperation*>(constOperation) ?
													static_cast<const DConstOperation*>(constOperation)->value == 1 : false;

											if(isPostInc) {
												postInc = incValue;
											} else {
												revokeIncrementOperation = constOperation;
												context.warning("Cannot decompile bytecode exactly: it contains post-increment by a number, other than one or minus one");
											}
										}
									}
								}
							}
						}
					}
				}

				virtual string toString(const StringifyContext& context) const override {
					return (canDeclare ? variable.getType()->toString(context.classinfo) + ' ' : EMPTY_STRING) +
							// Increment
							(shortFormOperator != nullptr ?
								(postInc != 0 ? (context.getCurrentScope()->getNameFor(variable) + (postInc > 0 ? "++" : "--")) :
								revokeIncrementOperation != nullptr ?
								'(' + shortFormOperator->toShortFormString(context, variable) + ") " + // x++ is equivalent (x += 1) - 1
										shortFormOperator->getOppositeOperator() + ' ' + revokeIncrementOperation->toString(context) :
								shortFormOperator->toShortFormString(context, variable)) :

							context.getCurrentScope()->getNameFor(variable) + " = " +

							// Short form array
							(canDeclare && JDecompiler::getInstance().canUseShortArrayInitializing() ? value->toArrayInitString(context) : value->toString(context)));
				}

				virtual Priority getPriority() const override {
					return Priority::ASSIGNMENT;
				}
		};

		struct IStoreOperation: StoreOperation {
			IStoreOperation(const DecompilationContext& context, uint16_t index): StoreOperation(ANY_INT_OR_BOOLEAN, context, index) {}
		};

		struct LStoreOperation: StoreOperation {
			LStoreOperation(const DecompilationContext& context, uint16_t index): StoreOperation(LONG, context, index) {}
		};

		struct FStoreOperation: StoreOperation {
			FStoreOperation(const DecompilationContext& context, uint16_t index): StoreOperation(FLOAT, context, index) {}
		};

		struct DStoreOperation: StoreOperation {
			DStoreOperation(const DecompilationContext& context, uint16_t index): StoreOperation(DOUBLE, context, index) {}
		};

		struct AStoreOperation: StoreOperation {
			AStoreOperation(const DecompilationContext& context, uint16_t index): StoreOperation(AnyObjectType::getInstance(), context, index) {}
		};



		struct IIncOperation: Operation {
			public:
				const Variable& variable;
				const int16_t value;

			protected:
				const Type* returnType;
				bool isShortInc, isPostInc = false;

			public:
				IIncOperation(const DecompilationContext& context, uint16_t index, int16_t value);

				virtual string toString(const StringifyContext& context) const override {
					if(isShortInc) {
						const char* inc = value == 1 ? "++" : "--";
						return isPostInc || returnType == VOID ? context.getCurrentScope()->getNameFor(variable) + inc :
								inc + context.getCurrentScope()->getNameFor(variable);
					}
					return context.getCurrentScope()->getNameFor(variable) + (value < 0 ? " -" : " +") + "= " + to_string(abs(value));
				}

				virtual const Type* getReturnType() const override {
					return returnType;
				}

				virtual Priority getPriority() const override {
					return isShortInc ? (isPostInc ? Priority::POST_INCREMENT : Priority::PRE_INCREMENT) : Priority::ASSIGNMENT;
				}
		};


		template<bool required>
		struct CastOperation: Operation {
			public:
				const Operation* const value;
				const Type* const type;

			private:
				static inline const Operation* getValue(const Operation* value, const Type* type) {
					if((type == BYTE || type == SHORT || type == CHAR) && instanceof<const CastOperation<true>*>(value)) {
						const CastOperation<true>* castValue = static_cast<const CastOperation<true>*>(value);
						if(castValue->type == INT)
						return castValue->value;
					}

					return value;
				}

			public:
				CastOperation(const Operation* value, const Type* type):
						value(getValue(value, type)), type(type) {}

				CastOperation(const DecompilationContext& context, const Type* requiredType, const Type* type):
						CastOperation(context.stack.popAs(requiredType), type) {}

				virtual const Type* getReturnType() const override {
					return type;
				}

				virtual string toString(const StringifyContext& context) const override {
					return required ? '(' + type->toString(context.classinfo) + ')' + toStringPriority(value, context, Associativity::RIGHT) :
							value->toString(context);
				}

				virtual Priority getPriority() const override {
					return required ? Priority::CAST : value->getPriority();
				}
		};


		struct CheckCastOperation: CastOperation<true> {
			CheckCastOperation(const DecompilationContext& context, uint16_t index):
					CastOperation(context, AnyObjectType::getInstance(), parseReferenceType(*context.constPool.get<ClassConstant>(index)->name)) {}
		};


		struct InstanceofOperation: BooleanOperation {
			protected:
				const Type* const type;
				const Operation* const object;

			public:
				InstanceofOperation(const DecompilationContext& context, uint16_t index):
						type(parseReferenceType(*context.constPool.get<ClassConstant>(index)->name)), object(context.stack.pop()) {}

				virtual string toString(const StringifyContext& context) const override {
					return toStringPriority(object, context, Associativity::LEFT) + " instanceof " + type->toString(context.classinfo);
				}

				virtual Priority getPriority() const override {
					return Priority::INSTANCEOF;
				}
		};



		struct CatchBlockDataHolder {
			index_t startIndex;
			vector<const ClassType*> catchTypes;

			CatchBlockDataHolder(index_t startIndex, const ClassType* catchType): startIndex(startIndex), catchTypes{catchType} {}
		};


		struct TryScope: Scope {
			public:
				TryScope(const DecompilationContext& context, index_t startIndex, index_t endIndex):
						Scope(startIndex, endIndex, context) {}

				virtual string getHeader(const StringifyContext& context) const override {
					return "try ";
				}

				virtual string getBackSeparator(const ClassInfo& classinfo) const override {
					return EMPTY_STRING;
				}
		};


		struct CatchScope: Scope {
			public:
				const vector<const ClassType*> catchTypes;
				const ClassType* const catchType;

			protected:
				mutable vector<const Operation*> tmpStack;
				mutable Variable* exceptionVariable = nullptr;
				mutable uint16_t exceptionVariableIndex;
				const bool hasNext;

			public:
				CatchScope(const DecompilationContext& context, index_t startIndex, index_t endIndex,
						const vector<const ClassType*>& catchTypes, bool hasNext):
						Scope(startIndex, endIndex, context), catchTypes(catchTypes), catchType(catchTypes[0]), hasNext(hasNext) {}

				CatchScope(const DecompilationContext& context, index_t startIndex, index_t endIndex,
						const initializer_list<const ClassType*>& catchTypes, bool hasNext):
						CatchScope(context, startIndex, endIndex, vector<const ClassType*>(catchTypes), hasNext) {}


				virtual void addOperation(const Operation* operation, const StringifyContext& context) const override {
					if(exceptionVariable == nullptr) {
						if(instanceof<const StoreOperation*>(operation)) {
							exceptionVariableIndex = static_cast<const StoreOperation*>(operation)->index;
							exceptionVariable = new NamedVariable(catchType, true, "ex");
							return;
						} else {
							context.warning("first instruction in the catch or finally block should be `astore`");
						}
					}
					Scope::addOperation(operation, context);
				}

				virtual const Variable& getVariable(index_t index, bool isDeclared) const override {
					if(exceptionVariable != nullptr && index == exceptionVariableIndex)
						return *exceptionVariable;
					return Scope::getVariable(index, isDeclared);
				}


				virtual string getFrontSeparator(const ClassInfo& classinfo) const override {
					return " ";
				}

				virtual string getHeader(const StringifyContext& context) const override {
					return catchType == nullptr ? "finally" :
							"catch(" + join<const ClassType*>(catchTypes,
									[&context] (const ClassType* catchType) { return catchType->toString(context.classinfo); }, " | ") +
										' ' + context.getCurrentScope()->getNameFor(*exceptionVariable) + ") ";
				}

				virtual string getBackSeparator(const ClassInfo& classinfo) const override {
					return hasNext ? EMPTY_STRING : "\n";
				}

				void initiate(const DecompilationContext& context) {
					tmpStack.reserve(context.stack.size());
					while(!context.stack.empty())
						tmpStack.push_back(context.stack.pop());

					context.stack.push(new LoadCatchedExceptionOperation(catchTypes.empty() ? catchType : THROWABLE));
				}

			protected:
				struct LoadCatchedExceptionOperation: Operation {
					const ClassType* const catchType;

					LoadCatchedExceptionOperation(const ClassType* catchType): catchType(catchType) {}

					virtual const Type* getReturnType() const override {
						return catchType;
					}

					virtual string toString(const StringifyContext& context) const override {
						throw Exception("Illegal using of LoadCatchedExceptionOperation: toString()");
					}
				};

			public:
				virtual void finalize(const DecompilationContext& context) const override {
					reverse(tmpStack.begin(), tmpStack.end());
					for(const Operation* operation : tmpStack)
						context.stack.push(operation);
				}
		};



		struct FieldOperation: Operation {
			public:
				const ClassType clazz;
				const FieldDescriptor descriptor;

			protected:
				FieldOperation(const ClassType& clazz, const FieldDescriptor& descriptor):
						clazz(clazz), descriptor(descriptor) {}

				FieldOperation(const FieldrefConstant* fieldref):
						FieldOperation(fieldref->clazz, fieldref->nameAndType) {}

				FieldOperation(const DecompilationContext& context, uint16_t index):
						FieldOperation(context.constPool.get<FieldrefConstant>(index)) {}

				inline string staticFieldToString(const StringifyContext& context) const {
					return clazz == context.classinfo.thisType && !context.getCurrentScope()->hasVariable(descriptor.name) ?
							descriptor.name : clazz.toString(context.classinfo) + '.' + descriptor.name;
				}

				inline string instanceFieldToString(const StringifyContext& context, const Operation* object) const {
					return !(context.modifiers & ACC_STATIC) && instanceof<const ALoadOperation*>(object) &&
							static_cast<const ALoadOperation*>(object)->index == 0 && !context.getCurrentScope()->hasVariable(descriptor.name) ?
								descriptor.name : object->toString(context) + '.' + descriptor.name;
				}
		};


		struct GetFieldOperation: FieldOperation {
			protected:
				GetFieldOperation(const ClassType& clazz, const FieldDescriptor& descriptor):
						FieldOperation(clazz, descriptor) {}

			public:
				virtual const Type* getReturnType() const override {
					return &descriptor.type;
				}
		};

		struct GetStaticFieldOperation: GetFieldOperation {
			public:
				GetStaticFieldOperation(const ClassType& clazz, const FieldDescriptor& descriptor):
						GetFieldOperation(clazz, descriptor) {}

				GetStaticFieldOperation(const FieldrefConstant* fieldref):
						GetStaticFieldOperation(fieldref->clazz, fieldref->nameAndType) {}

				virtual string toString(const StringifyContext& context) const override {
					static const FieldDescriptor TypeField("TYPE", CLASS);

					if(descriptor == TypeField) {
						if(clazz == javase::java::lang::Void) return "void.class";
						if(clazz == javase::java::lang::Byte) return "byte.class";
						if(clazz == javase::java::lang::Short) return "short.class";
						if(clazz == javase::java::lang::Character) return "char.class";
						if(clazz == javase::java::lang::Integer) return "int.class";
						if(clazz == javase::java::lang::Long) return "long.class";
						if(clazz == javase::java::lang::Float) return "float.class";
						if(clazz == javase::java::lang::Double) return "double.class";
						if(clazz == javase::java::lang::Boolean) return "boolean.class";
					}

					return staticFieldToString(context);
				}
		};

		struct GetInstanceFieldOperation: GetFieldOperation {
			public:
				const Operation* const object;

				GetInstanceFieldOperation(const DecompilationContext& context, const ClassType& clazz, const FieldDescriptor& descriptor):
						GetFieldOperation(clazz, descriptor), object(context.stack.popAs(&clazz)) {}

				GetInstanceFieldOperation(const DecompilationContext& context, const FieldrefConstant* fieldref):
						GetInstanceFieldOperation(context, fieldref->clazz, fieldref->nameAndType) {}

				virtual string toString(const StringifyContext& context) const override {
					return instanceFieldToString(context, object);
				}
		};


		struct PutFieldOperation: FieldOperation {
			public:
				const Operation* const value;

			protected:
				const Type* returnType;

				PutFieldOperation(const DecompilationContext& context, const ClassType& clazz, const FieldDescriptor& descriptor):
						FieldOperation(clazz, descriptor), value(context.stack.popAs(&descriptor.type)) {
					if(const LoadOperation* loadOperation = dynamic_cast<const LoadOperation*>(value->getOriginalOperation()))
						loadOperation->variable.addName(descriptor.name);
				}

			public:
				virtual const Type* getReturnType() const override {
					return returnType;
				}

				virtual Priority getPriority() const override {
					return Priority::ASSIGNMENT;
				}
		};

		struct PutStaticFieldOperation: PutFieldOperation {
			public:
				PutStaticFieldOperation(const DecompilationContext& context, const ClassType& clazz, const FieldDescriptor& descriptor):
						PutFieldOperation(context, clazz, descriptor) {
					returnType = getDupReturnType<Dup1Operation, Dup2Operation>(context, value);
				}

				PutStaticFieldOperation(const DecompilationContext& context, const MethodrefConstant* methodref):
						PutStaticFieldOperation(context, methodref->clazz, methodref->nameAndType) {}

				virtual string toString(const StringifyContext& context) const override {
					return staticFieldToString(context) + " = " + value->toString(context);
				}
		};

		struct PutInstanceFieldOperation: PutFieldOperation {
			public:
				const Operation* const object;

				PutInstanceFieldOperation(const DecompilationContext& context, const ClassType& clazz, const FieldDescriptor& decriptor):
						PutFieldOperation(context, clazz, decriptor), object(context.stack.popAs(&clazz)) {
					returnType = getDupReturnType<DupX1Operation, Dup2X1Operation>(context, value);
				}

				PutInstanceFieldOperation(const DecompilationContext& context, const MethodrefConstant* methodref):
						PutInstanceFieldOperation(context, methodref->clazz, methodref->nameAndType) {}

				virtual string toString(const StringifyContext& context) const override {
					return instanceFieldToString(context, object) + " = " + value->toString(context);
				}
		};



		struct NewOperation: Operation {
			public:
				const ClassType clazz;

				NewOperation(const DecompilationContext& context, const ClassConstant* classConstant):
						clazz(classConstant) {}

				NewOperation(const DecompilationContext& context, uint16_t classIndex):
						NewOperation(context, context.constPool.get<ClassConstant>(classIndex)) {}

				virtual string toString(const StringifyContext& context) const override {
					return "new " + clazz.toString(context.classinfo);
				}

				virtual const Type* getReturnType() const override {
					return &clazz;
				}
		};



		struct InvokeOperation: Operation {
			public:
				const MethodDescriptor& descriptor;
				const vector<const Operation*> arguments;

			protected:
				const vector<const Operation*> popArguments(const DecompilationContext& context) const {
					const uint32_t size = descriptor.arguments.size();

					vector<const Operation*> arguments;
					arguments.reserve(size);

					for(uint32_t i = size; i > 0; ) {
						arguments.push_back(context.stack.popAs(descriptor.arguments[--i]));
					}

					return arguments;
				}

				InvokeOperation(const DecompilationContext& context, const MethodDescriptor& descriptor):
						descriptor(descriptor), arguments(popArguments(context)) {}

				InvokeOperation(const DecompilationContext& context, uint16_t index):
						InvokeOperation(context, context.constPool.get<MethodrefConstant>(index)) {}

			public:
				virtual const Type* getReturnType() const override {
					return descriptor.returnType;
				}
		};


		struct InvokeNonStaticOperation: InvokeOperation {
			public:
				const Operation* const object;

			protected:
				InvokeNonStaticOperation(const DecompilationContext& context, const MethodDescriptor& descriptor):
						InvokeOperation(context, descriptor), object(context.stack.popAs(&descriptor.clazz)) {}

				InvokeNonStaticOperation(const DecompilationContext& context, const Operation* object, const MethodDescriptor& descriptor):
						InvokeOperation(context, descriptor), object(object) {
					object->castReturnTypeTo(&descriptor.clazz);
				}

				inline string nonStaticMethodToString(const StringifyContext& context) const {
					return (!(context.modifiers & ACC_STATIC) && instanceof<const ALoadOperation*>(object) ?
							EMPTY_STRING : toStringPriority(object, context, Associativity::LEFT) + '.') + descriptor.name + '(' +
							rjoin<const Operation*>(arguments, [&context] (const Operation* operation) { return operation->toString(context); }) + ')';
				}

			public:
				virtual string toString(const StringifyContext& context) const override {
					return nonStaticMethodToString(context);
				}
		};


		struct InvokevirtualOperation: InvokeNonStaticOperation {
			InvokevirtualOperation(const DecompilationContext& context, const MethodDescriptor& descriptor):
					InvokeNonStaticOperation(context, descriptor) {}
		};


		struct InvokespecialOperation: InvokeNonStaticOperation {
			public:
				const bool isConstructor, isSuperConstructor, isEnumSuperConstructor;

				const Type* const returnType;

			private:
				inline bool getIsConstructor() {
					return descriptor.type == MethodDescriptor::MethodType::CONSTRUCTOR;
				}

				inline bool getIsSuperConstructor(const DecompilationContext& context) const {
					return (!(context.modifiers & ACC_STATIC) && // check that we invoking this (or super) constructor
							instanceof<const ALoadOperation*>(object) && static_cast<const ALoadOperation*>(object)->index == 0 &&
							descriptor.clazz == context.classinfo.superType);
				}

				inline bool getIsEnumSuperConstructor(const DecompilationContext& context) const {
					return isSuperConstructor && context.classinfo.modifiers & ACC_ENUM;
				}

				inline const Type* getReturnType(const DecompilationContext& context) const {
					return isConstructor && checkDup<Dup1Operation>(context, object) ?
							object->getReturnType() : InvokeNonStaticOperation::getReturnType();
				}


				inline void checkEnumSuperConstructor(const DecompilationContext& context) const {
					if((isEnumSuperConstructor && arguments.size() != 2) || *descriptor.arguments[0] != *STRING || *descriptor.arguments[1] != *INT) {
						context.warning("invocation of non-standart enum super constructor");
					}
				}

			public:
				InvokespecialOperation(const DecompilationContext& context, const MethodDescriptor& descriptor):
						InvokeNonStaticOperation(context, descriptor),
						isConstructor(getIsConstructor()), isSuperConstructor(getIsSuperConstructor(context)),
						isEnumSuperConstructor(getIsEnumSuperConstructor(context)), returnType(getReturnType(context)) {}

				InvokespecialOperation(const DecompilationContext& context, const Operation* object, const MethodDescriptor& descriptor):
						InvokeNonStaticOperation(context, object, descriptor),
						isConstructor(getIsConstructor()), isSuperConstructor(getIsSuperConstructor(context)),
						isEnumSuperConstructor(getIsEnumSuperConstructor(context)), returnType(getReturnType(context)) {}

				virtual string toString(const StringifyContext& context) const override {
					if(isConstructor) {
						if(const NewOperation* newOperation = dynamic_cast<const NewOperation*>(object->getOriginalOperation())) {
							const ClassType& classType = newOperation->clazz;
							if(classType.isAnonymous) {
								const Class* clazz = JDecompiler::getInstance().getClass(classType.getEncodedName());
								if(clazz != nullptr) {
									clazz->classinfo.copyFormattingFrom(context.classinfo);
									const string result = "new " + clazz->toString();
									clazz->classinfo.resetFormatting();

									return result;
								} else {
									context.warning("cannot load inner class " + classType.getName());
								}
							}
						}

						return (isSuperConstructor ? "super" : toStringPriority(object, context, Associativity::LEFT)) +
							'(' + rjoin<const Operation*>(arguments,
								[&context] (const Operation* operation) { return operation->toString(context); }) + ')';
					}

					return InvokeNonStaticOperation::toString(context);
				}

				virtual const Type* getReturnType() const override {
					return returnType;
				}

				virtual bool canAddToCode() const override {
					return !(isSuperConstructor && arguments.empty()) && !(isEnumSuperConstructor && arguments.size() == 2);
				}
		};


		struct InvokestaticOperation: InvokeOperation {
			public:
				InvokestaticOperation(const DecompilationContext& context, const MethodDescriptor& descriptor):
						InvokeOperation(context, descriptor) {}

				virtual string toString(const StringifyContext& context) const override {
					return staticMethodToString(context) + '(' +
							rjoin<const Operation*>(arguments, [&context] (const Operation* operation) { return operation->toString(context); }) + ')';
				}

			private:
				inline string staticMethodToString(const StringifyContext& context) const {
					return descriptor.clazz == context.classinfo.thisType ?
							descriptor.name : descriptor.clazz.toString(context.classinfo) + '.' + descriptor.name;
				}
		};


		struct InvokeinterfaceOperation: InvokeNonStaticOperation {
			InvokeinterfaceOperation(const DecompilationContext& context, const MethodDescriptor& descriptor):
					InvokeNonStaticOperation(context, descriptor) {}
		};


		struct ConcatStringsOperation: InvokeOperation {
			const StringConstOperation* const pattern;

			struct StringOperand {
				enum Type { OPERATION, STRING };

				uint_fast8_t type;
				union {
					const Operation* operation;
					const string* stringConstant;
				} value;

				inline StringOperand(const Operation* operation): type(OPERATION), value{ .operation = operation } {}

				inline StringOperand(const string* stringConstant): type(STRING), value{ .stringConstant = stringConstant } {}
			};

			vector<StringOperand> operands;

			ConcatStringsOperation(const DecompilationContext& context, const MethodDescriptor& concater):
					InvokeOperation(context, concater), pattern(safe_cast<const StringConstOperation*>(context.stack.popAs(STRING))) {
				auto arg = arguments.end();
				string str;

				for(const char* cp = pattern->value->value->c_str(); *cp != '\0'; cp++) {
					if(*cp == '\1') {
						if(!str.empty()) {
							operands.push_back(StringOperand(new string(str)));
							str.clear();
						}
						operands.push_back(StringOperand(*(--arg)));
					} else {
						str += *cp;
					}
				}

				if(!str.empty())
					operands.push_back(StringOperand(new string(str)));

				if((operands.size() == 1 && operands[0].type == StringOperand::OPERATION) ||
						(operands.size() > 1 &&
						operands[0].type == StringOperand::OPERATION && *operands[0].value.operation->getReturnType() != *STRING &&
						operands[1].type == StringOperand::OPERATION && *operands[1].value.operation->getReturnType() != *STRING)) {
					operands.insert(operands.begin(), StringOperand(&EMPTY_STRING));
				}
			}

			virtual string toString(const StringifyContext& context) const override {
				return join<StringOperand>(operands, [&context, this] (const StringOperand operand) {
					return operand.type == StringOperand::OPERATION ? toStringPriority(operand.value.operation, context, Associativity::RIGHT) :
							stringToLiteral(*operand.value.stringConstant);
				}, " + ");
			}

			virtual const Type* getReturnType() const override {
				return STRING;
			}

			virtual Priority getPriority() const override {
				return Priority::PLUS;
			}
		};


		struct NewArrayOperation: Operation {
			protected:
				const ArrayType* const arrayType;
				vector<const Operation*> lengths;

				mutable vector<const Operation*> initializer;
				friend struct ArrayStoreOperation;

			public:
				NewArrayOperation(const DecompilationContext& context, const ArrayType* arrayType):
					NewArrayOperation(context, arrayType, 1) {}

			protected:
				NewArrayOperation(const DecompilationContext& context, const ArrayType* arrayType, uint16_t dimensions):
						arrayType(arrayType), lengths(arrayType->nestingLevel) {

					if(dimensions > arrayType->nestingLevel)
						throw DecompilationException("instruction newarray (or another derivative of it)"
								"has too many dimensions (" + to_string(dimensions) + ") for its array type " + arrayType->toString());

					for(uint16_t i = dimensions; i > 0; )
						lengths[--i] = context.stack.popAs(INT);
				}


				virtual const Type* getReturnType() const override {
					return arrayType;
				}

			protected:
				bool canInitAsList() const {
					return !initializer.empty() || ((lengths.size() == 1 || (lengths.size() > 1 && lengths[1] == nullptr)) &&
							instanceof<const IConstOperation*>(lengths[0]) && static_cast<const IConstOperation*>(lengths[0])->value == 0);
				}

			public:
				virtual string toString(const StringifyContext& context) const override {
					return canInitAsList() ? "new " + arrayType->toString(context.classinfo) + ' ' + toArrayInitString(context) : toArrayInitString(context) ;
				}

				virtual string toArrayInitString(const StringifyContext& context) const override {

					if(canInitAsList()) {
						const bool useSpaces = arrayType->nestingLevel == 1 && !initializer.empty();
						return (useSpaces ? "{ " : "{") + join<const Operation*>(initializer,
								[&context] (const Operation* element) { return element->toArrayInitString(context); }) + (useSpaces ? " }" : "}");
					}

					return "new " + arrayType->memberType->toString(context.classinfo) + join<const Operation*>(lengths,
								[&context] (auto length) { return length == nullptr ? "[]" : '[' + length->toString(context) + ']'; }, EMPTY_STRING);
				}
		};

		struct ANewArrayOperation: NewArrayOperation {
			ANewArrayOperation(const DecompilationContext& context, uint16_t index):
					NewArrayOperation(context, new ArrayType(parseReferenceType(*context.constPool.get<ClassConstant>(index)->name))) {}
		};


		struct MultiANewArrayOperation: NewArrayOperation {
			MultiANewArrayOperation(const DecompilationContext& context, uint16_t index, uint16_t dimensions):
					NewArrayOperation(context, safe_cast<const ArrayType*>(parseReferenceType(*context.constPool.get<ClassConstant>(index)->name)),
					dimensions) {
				if(dimensions > arrayType->nestingLevel) {
					throw DecompilationException("The nesting level of the multianewarray instruction (" + to_string(dimensions) + ")"
							" greater than nesting level of the array type " + arrayType->toString());
				}
			}
		};




		struct ArrayStoreOperation: VoidOperation {
			protected:
				const Operation *const value, *const index, *const array;
				bool isInitializer = false;

			public:
				ArrayStoreOperation(const Type* elementType, const DecompilationContext& context): value(context.stack.popAs(elementType)),
						index(context.stack.popAs(INT)), array(context.stack.popAs(new ArrayType(elementType))) {

					//checkDup<Dup1Operation>(context, array);

					if(const Dup1Operation* dupArray = dynamic_cast<const Dup1Operation*>(array)) {
						if(const NewArrayOperation* newArray = dynamic_cast<const NewArrayOperation*>(dupArray->operation)) {
							newArray->initializer.push_back(value);
							isInitializer = true;
						}
					}
				};

				virtual string toString(const StringifyContext& context) const override {
					return isInitializer ? value->toString(context) :
							array->toString(context) + '[' + index->toString(context) + "] = " + value->toString(context);
				}

				virtual bool canAddToCode() const override {
					return !isInitializer;
				}

				virtual Priority getPriority() const override {
					return Priority::ASSIGNMENT;
				}
		};


		struct IAStoreOperation: ArrayStoreOperation {
			IAStoreOperation(const DecompilationContext& context): ArrayStoreOperation(INT, context) {}
		};

		struct LAStoreOperation: ArrayStoreOperation {
			LAStoreOperation(const DecompilationContext& context): ArrayStoreOperation(LONG, context) {}
		};

		struct FAStoreOperation: ArrayStoreOperation {
			FAStoreOperation(const DecompilationContext& context): ArrayStoreOperation(FLOAT, context) {}
		};

		struct DAStoreOperation: ArrayStoreOperation {
			DAStoreOperation(const DecompilationContext& context): ArrayStoreOperation(DOUBLE, context) {}
		};

		struct AAStoreOperation: ArrayStoreOperation {
			AAStoreOperation(const DecompilationContext& context): ArrayStoreOperation(
					context.stack.lookup(2)->getReturnTypeAs(AnyType::getArrayTypeInstance())->elementType, context) {}
		};

		struct BAStoreOperation: ArrayStoreOperation {
			BAStoreOperation(const DecompilationContext& context): ArrayStoreOperation(BYTE, context) {}
		};

		struct CAStoreOperation: ArrayStoreOperation {
			CAStoreOperation(const DecompilationContext& context): ArrayStoreOperation(CHAR, context) {}
		};

		struct SAStoreOperation: ArrayStoreOperation {
			SAStoreOperation(const DecompilationContext& context): ArrayStoreOperation(SHORT, context) {}
		};



		struct ArrayLengthOperation: IntOperation {
			protected:
				const Operation* const array;

			public:
				ArrayLengthOperation(const DecompilationContext& context): array(context.stack.popAs(AnyType::getArrayTypeInstance())) {}

				virtual string toString(const StringifyContext& context) const override {
					return toStringPriority(array, context, Associativity::LEFT) + ".length";
				}
		};



		struct AThrowOperation: VoidOperation {
			protected: const Operation* const exceptionOperation;

			public:
				AThrowOperation(const DecompilationContext& context): exceptionOperation(context.stack.pop()) {}

				virtual string toString(const StringifyContext& context) const override {
					return "throw " + exceptionOperation->toString(context);
				}
		};



		struct ReturnOperation: VoidOperation {
			public:
				const Operation* const value;

				ReturnOperation(const DecompilationContext& context, const Type* type):
						value(context.stack.popAs(type->castTo(context.descriptor.returnType))) {}

				virtual string toString(const StringifyContext& context) const override {
					return "return " + value->toString(context);
				}
		};


		struct IReturnOperation: ReturnOperation {
			IReturnOperation(const DecompilationContext& context): ReturnOperation(context, ANY_INT_OR_BOOLEAN) {}
		};

		struct LReturnOperation: ReturnOperation {
			LReturnOperation(const DecompilationContext& context): ReturnOperation(context, LONG) {}
		};

		struct FReturnOperation: ReturnOperation {
			FReturnOperation(const DecompilationContext& context): ReturnOperation(context, FLOAT) {}
		};

		struct DReturnOperation: ReturnOperation {
			DReturnOperation(const DecompilationContext& context): ReturnOperation(context, DOUBLE) {}
		};

		struct AReturnOperation: ReturnOperation {
			AReturnOperation(const DecompilationContext& context): ReturnOperation(context, AnyObjectType::getInstance()) {}
		};



		template<typename T>
		const Operation* ConstOperation<T>::valueOf(T value, const FieldInfo* fieldinfo) {

			if constexpr(is_floating_point<T>()) {

				static const FieldDescriptor
						NaNField("NaN", TYPE),
						PositiveInfinityField("POSITIVE_INFINITY", TYPE),
						NegativeInfinityField("NEGATIVE_INFINITY", TYPE);

				if(isnan(value)) {
					return JDecompiler::getInstance().canUseNaNAndInfinity() && canUseConstant(fieldinfo, WRAPPER_CLASS, NaNField) ?
							(const Operation*)new GetStaticFieldOperation(WRAPPER_CLASS, NaNField) :
							new DivOperatorOperation(TYPE, new ConstOperation<T>(0), new ConstOperation<T>(0));
				}

				if(value == numeric_limits<T>::infinity()) {
					return JDecompiler::getInstance().canUseNaNAndInfinity() && canUseConstant(fieldinfo, WRAPPER_CLASS, PositiveInfinityField) ?
						(const Operation*)new GetStaticFieldOperation(WRAPPER_CLASS, PositiveInfinityField) :
						new DivOperatorOperation(TYPE, new ConstOperation<T>(1), new ConstOperation<T>(0));
				}

				if(value == -numeric_limits<T>::infinity()) {
					return JDecompiler::getInstance().canUseNaNAndInfinity() && canUseConstant(fieldinfo, WRAPPER_CLASS, NegativeInfinityField) ?
						(const Operation*)new GetStaticFieldOperation(WRAPPER_CLASS, PositiveInfinityField) :
						new DivOperatorOperation(TYPE, new ConstOperation<T>(-1), new ConstOperation<T>(0));
				}

				if(JDecompiler::getInstance().canUseConstants()) {
					static const FieldDescriptor DenormMinValueField("MIN_VALUE", TYPE);

					if(value == numeric_limits<T>::denorm_min() && canUseConstant(fieldinfo, WRAPPER_CLASS, DenormMinValueField)) {
						return new GetStaticFieldOperation(WRAPPER_CLASS, DenormMinValueField);
					}

					if(value == -numeric_limits<T>::denorm_min() && canUseConstant(fieldinfo, WRAPPER_CLASS, DenormMinValueField)) {
						return new NegOperatorOperation(TYPE, new GetStaticFieldOperation(WRAPPER_CLASS, DenormMinValueField));
					}
				}
			}

			if(JDecompiler::getInstance().canUseConstants()) {
				static const FieldDescriptor
						MaxValueField("MAX_VALUE", TYPE),
						MinValueField(is_floating_point<T>() ? "MIN_NORMAL" : "MIN_VALUE", TYPE);

				if(value == numeric_limits<T>::max() && canUseConstant(fieldinfo, WRAPPER_CLASS, MaxValueField)) {
					return new GetStaticFieldOperation(WRAPPER_CLASS, MaxValueField);
				}

				if(value == -numeric_limits<T>::max() && canUseConstant(fieldinfo, WRAPPER_CLASS, MaxValueField)) {
					return new NegOperatorOperation(TYPE, new GetStaticFieldOperation(WRAPPER_CLASS, MaxValueField));
				}

				if(value == numeric_limits<T>::min() && canUseConstant(fieldinfo, WRAPPER_CLASS, MinValueField)) {
					return new GetStaticFieldOperation(WRAPPER_CLASS, MinValueField);
				}

				if constexpr(is_floating_point<T>()) {
					if(value == -numeric_limits<T>::min() && canUseConstant(fieldinfo, WRAPPER_CLASS, MinValueField)) { // For int and long MIN_VALUE == -MIN_VALUE
						return new NegOperatorOperation(TYPE, new GetStaticFieldOperation(WRAPPER_CLASS, MinValueField));
					}
				}
			}
			return nullptr;
		}

		template<typename T>
		const Operation* FPConstOperation<T>::valueOf(T value, const FieldInfo* fieldinfo) {
			if(JDecompiler::getInstance().canUseConstants()) {
				static const ClassType MathClass("java/lang/Math");

				static const FieldDescriptor PIField("PI", DOUBLE);

				if(value == (T)M_PI && ConstOperation<T>::canUseConstant(fieldinfo, MathClass, PIField)) {
					if constexpr(is_float<T>()) {
						return new CastOperation<true>(new GetStaticFieldOperation(MathClass, PIField), FLOAT); // cast Math.PI to float
					} else {
						return new GetStaticFieldOperation(MathClass, PIField);
					}
				}

				static const FieldDescriptor EField("E", DOUBLE);

				if(value == (T)M_E && ConstOperation<T>::canUseConstant(fieldinfo, MathClass, EField)) {
					if constexpr(is_float<T>()) {
						return new CastOperation<true>(new GetStaticFieldOperation(MathClass, EField), FLOAT); // cast Math.E to float
					} else {
						return new GetStaticFieldOperation(MathClass, EField);
					}
				}
			}

			const Operation* result = ConstOperation<T>::valueOf(value, fieldinfo);
			return result == nullptr ? new FPConstOperation<T>(value) : result;
		}

	}

	const Operation* StringConstant::toOperation(const FieldInfo* fieldinfo)  const { return new operations::StringConstOperation(this); }
	const Operation* ClassConstant::toOperation(const FieldInfo* fieldinfo)   const { return new operations::ClassConstOperation(this); }
	const Operation* IntegerConstant::toOperation(const FieldInfo* fieldinfo) const { return operations::IConstOperation::valueOf(value, fieldinfo); }
	const Operation* FloatConstant::toOperation(const FieldInfo* fieldinfo)   const { return operations::FConstOperation::valueOf(value, fieldinfo); }
	const Operation* LongConstant::toOperation(const FieldInfo* fieldinfo)    const { return operations::LConstOperation::valueOf(value, fieldinfo); }
	const Operation* DoubleConstant::toOperation(const FieldInfo* fieldinfo)  const { return operations::DConstOperation::valueOf(value, fieldinfo); }
	const Operation* MethodTypeConstant::toOperation(const FieldInfo* fieldinfo) const { return new operations::MethodTypeConstOperation(this); }
	const Operation* MethodHandleConstant::toOperation(const FieldInfo* fieldinfo) const { return new operations::MethodHandleConstOperation(this); }


	void StaticInitializerScope::addOperation(const Operation* operation, const StringifyContext& context) const {
		using namespace operations;

		if(!fieldsInitialized) {
			const PutStaticFieldOperation* putOperation = dynamic_cast<const PutStaticFieldOperation*>(operation);
			if(putOperation != nullptr && ClassType(putOperation->clazz) == context.classinfo.thisType) {
				if(const Field* field = context.classinfo.clazz.getField(putOperation->descriptor.name)) {
					if(field->initializer == nullptr) {
						field->initializer = putOperation->value;
						field->context = &context;
					} else {
						code.push_back(operation);
					}
				}
			} else {
				fieldsInitialized = true;
				code.push_back(operation);
			}
		} else {
			code.push_back(operation);
		}
	}
}

#include "condition-operations.cpp"

#endif
