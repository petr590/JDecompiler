#ifndef JDECOMPILER_OPERATIONS_CPP
#define JDECOMPILER_OPERATIONS_CPP

#include <algorithm>

#define inline INLINE_ATTR

#undef LOG_PREFIX
#define LOG_PREFIX "[ jdecompiler-operations.cpp ]"

namespace JDecompiler {
	namespace Operations {

		template<class T = Type>
		struct ReturnableOperation: Operation { // ReturnableOperation is an operation which returns specified type
			static_assert(is_base_of<Type, T>::value, "template class T of struct ReturnableOperation is not subclass of class Type");

			protected:
				const T* const returnType;

			public:
				ReturnableOperation(const T* returnType): returnType(returnType) {}

				ReturnableOperation(const T* returnType, uint16_t priority): Operation(priority), returnType(returnType) {}

				virtual const Type* getReturnType() const override { return returnType; }
		};

		struct IntOperation: Operation {
			IntOperation(): Operation() {}
			IntOperation(uint16_t priority): Operation(priority) {}

			virtual const Type* getReturnType() const override { return INT; }
		};

		struct AnyIntOperation: Operation {
			AnyIntOperation(): Operation() {}
			AnyIntOperation(uint16_t priority): Operation(priority) {}

			virtual const Type* getReturnType() const override { return ANY_INT; }
		};

		struct BooleanOperation: Operation {
			BooleanOperation(): Operation() {}
			BooleanOperation(uint16_t priority): Operation(priority) {}

			virtual const Type* getReturnType() const override { return BOOLEAN; }
		};

		struct VoidOperation: Operation {
			VoidOperation(): Operation() {}
			VoidOperation(uint16_t priority): Operation(priority) {}

			virtual const Type* getReturnType() const override { return VOID; }
		};


		struct TransientReturnableOperation: Operation {
			protected:
				mutable const Type* returnType;

				TransientReturnableOperation(const Type* returnType): returnType(returnType) {}
				TransientReturnableOperation() {}

				template<class D, class... Ds>
				inline void initReturnType(const CodeEnvironment& environment, const Operation* operation) {
					returnType = getDupReturnType<D, Ds...>(environment, operation);
				}

				virtual const Type* getReturnType() const override {
					return returnType;
				}
		};


		// ----------------------------------------------------------------------------------------------------


		template<TypeSize size>
		struct TypeSizeTemplatedOperation {
			protected:
				template<TypeSize S>
				void checkTypeSize(const Type* type) const {
					if(type->getSize() != S)
						throw TypeSizeMismatchException(TypeSize_nameOf(S), TypeSize_nameOf(type->getSize()), type->toString());
				}

				inline void checkTypeSize(const Type* type) const {
					return checkTypeSize<size>(type);
				}
		};


		template<TypeSize size>
		struct AbstractDupOperation: Operation, TypeSizeTemplatedOperation<size> {
			const Operation* const operation;

			AbstractDupOperation(const CodeEnvironment& environment): operation(environment.stack.top()) {
				TypeSizeTemplatedOperation<size>::checkTypeSize(operation->getReturnType());
			}

			virtual string toString(const CodeEnvironment& environment) const override {
				return operation->toString(environment);
			}

			virtual const Type* getReturnType() const override {
				return operation->getReturnType();
			}
		};


		template<TypeSize size>
		struct DupOperation: AbstractDupOperation<size> {
			DupOperation(const CodeEnvironment& environment): AbstractDupOperation<size>(environment) {}
		};


		struct DupX1Operation: AbstractDupOperation<TypeSize::FOUR_BYTES> {
			DupX1Operation(const CodeEnvironment& environment): AbstractDupOperation<TypeSize::FOUR_BYTES>(environment) {
				if(environment.stack.size() < 2)
					throw IllegalStackStateException("Too less operations on stack for dup_x1: required 2, got " + environment.stack.size());

				const Operation
					*operation1 = environment.stack.pop(),
					*operation2 = environment.stack.pop();
				TypeSizeTemplatedOperation<TypeSize::FOUR_BYTES>::checkTypeSize(operation2->getReturnType());
				environment.stack.push(operation1, operation2);
			}
		};


		struct DupX2Operation: AbstractDupOperation<TypeSize::FOUR_BYTES> {
			DupX2Operation(const CodeEnvironment& environment): AbstractDupOperation<TypeSize::FOUR_BYTES>(environment) {
				if(environment.stack.size() < 3)
					throw IllegalStackStateException("Too less operations on stack for dup_x2: required 3, got " + environment.stack.size());

				const Operation
					*operation1 = environment.stack.pop(),
					*operation2 = environment.stack.pop(),
					*operation3 = environment.stack.pop();

				checkTypeSize(operation2->getReturnType());
				checkTypeSize(operation3->getReturnType());

				environment.stack.push(operation1, operation3, operation2);
			}
		};


		struct Dup2X1Operation: AbstractDupOperation<TypeSize::EIGHT_BYTES> {
			Dup2X1Operation(const CodeEnvironment& environment): AbstractDupOperation<TypeSize::EIGHT_BYTES>(environment) {
				if(environment.stack.size() < 2)
					throw IllegalStackStateException("Too less operations on stack for dup2_x1: required 2, got " + environment.stack.size());

				const Operation
					*operation1 = environment.stack.pop(),
					*operation2 = environment.stack.pop();

				checkTypeSize<TypeSize::FOUR_BYTES>(operation2->getReturnType());

				environment.stack.push(operation1, operation2);
			}
		};


		struct Dup2X2Operation: AbstractDupOperation<TypeSize::EIGHT_BYTES> {
			Dup2X2Operation(const CodeEnvironment& environment): AbstractDupOperation<TypeSize::EIGHT_BYTES>(environment) {
				if(environment.stack.size() < 2)
					throw IllegalStackStateException("Too less operations on stack for dup2_x2: required 2, got " + environment.stack.size());

				const Operation
					*operation1 = environment.stack.pop(),
					*operation2 = environment.stack.pop();

				checkTypeSize(operation2->getReturnType());

				environment.stack.push(operation1, operation2);
			}
		};

}

template<class O>
O Operation::castOperationTo(const Operation* operation) {
	using namespace Operations;

	if(O o = dynamic_cast<O>(operation))
		return o;

	if(const AbstractDupOperation<TypeSize::FOUR_BYTES>* dupOperation = dynamic_cast<const AbstractDupOperation<TypeSize::FOUR_BYTES>*>(operation))
		if(O o = dynamic_cast<O>(dupOperation->operation))
			return o;

	if(const AbstractDupOperation<TypeSize::EIGHT_BYTES>* dupOperation = dynamic_cast<const AbstractDupOperation<TypeSize::EIGHT_BYTES>*>(operation))
		if(O o = dynamic_cast<O>(dupOperation->operation))
			return o;

	return nullptr;
}

namespace Operations {


		template<typename T>
		struct ConstOperation: Operation {
			protected:
				mutable const Type* returnType;

			public:
				const T value;

				ConstOperation(const Type* returnType, T value): returnType(returnType), value(value) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return primitiveToString(value);
				}

				virtual const Type* getReturnType() const override {
					return returnType;
				}

			protected:
				virtual void onCastReturnType(const Type* newType) const override {
					returnType = newType;
				}
		};


		struct IConstOperation: ConstOperation<int32_t> {
			private:
				static inline const Type* getTypeByValue(int32_t value) {
					static const AmbigousType
							CHAR_OR_SHORT_OR_INT({CHAR, SHORT, INT}),
							SHORT_OR_INT({SHORT, INT});

					if((bool)value == value)     return ANY_INT_OR_BOOLEAN;
					if((int8_t)value == value)   return ANY_INT;
					if((char16_t)value == value) return &CHAR_OR_SHORT_OR_INT;
					if((int16_t)value == value)  return &SHORT_OR_INT;
					return INT;
				}

			public:
				IConstOperation(int32_t value): ConstOperation(getTypeByValue(value), value) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					if(returnType->isInstanceof(BOOLEAN)) return primitiveToString((bool)value);
					if(returnType->isInstanceof(BYTE))    return primitiveToString((int8_t)value);
					if(returnType->isInstanceof(CHAR))    return primitiveToString((char16_t)value);
					if(returnType->isInstanceof(SHORT))   return primitiveToString((int16_t)value);
					if(returnType->isInstanceof(INT))     return primitiveToString(value);
					throw IllegalStateException("Illegal type of iconst operation: " + returnType->toString());
				}
		};

		struct LConstOperation: ConstOperation<int64_t> {
			LConstOperation(int64_t value): ConstOperation(LONG, value) {}
		};

		struct FConstOperation: ConstOperation<float> {
			FConstOperation(float value): ConstOperation(FLOAT, value) {}
		};

		struct DConstOperation: ConstOperation<double> {
			DConstOperation(double value): ConstOperation(DOUBLE, value) {}
		};

		template<typename T>
		struct IPushOperation: TransientReturnableOperation {
			static_assert(is_same<T, int8_t>::value || is_same<T, int16_t>::value, "template type T of struct IPushOperation is not uint8_t or uint16_t type");

			public:
				const T value;

				IPushOperation(T value): TransientReturnableOperation(ANY_INT_OR_BOOLEAN), value(value) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					if(returnType->isInstanceof(BOOLEAN)) return primitiveToString((bool)value);
					if(returnType->isInstanceof(BYTE))    return primitiveToString((int8_t)value);
					if(returnType->isInstanceof(CHAR))    return primitiveToString((char16_t)value);
					if(returnType->isInstanceof(SHORT))   return primitiveToString((int16_t)value);
					if(returnType->isInstanceof(INT))     return primitiveToString((int32_t)value);
					throw IllegalStateException("Illegal type of ipush operation: " + returnType->toString());
				}

				virtual void onCastReturnType(const Type* newType) const override {
					returnType = newType;
				}
		};


		template<TypeSize size>
		struct LdcOperation: ReturnableOperation<> {
			public:
				const uint16_t index;
				const ConstValueConstant* const value;

			private:
				static const Type* getReturnTypeFor(uint16_t index, const ConstValueConstant* value) {
					if(dynamic_cast<const StringConstant*>(value)) return STRING;
					if(dynamic_cast<const ClassConstant*>(value)) return CLASS;
					if(dynamic_cast<const IntegerConstant*>(value)) return INT;
					if(dynamic_cast<const FloatConstant*>(value)) return FLOAT;
					if(dynamic_cast<const LongConstant*>(value)) return LONG;
					if(dynamic_cast<const DoubleConstant*>(value)) return DOUBLE;
					if(dynamic_cast<const MethodTypeConstant*>(value)) return METHOD_TYPE;
					if(dynamic_cast<const MethodHandleConstant*>(value)) return METHOD_HANDLE;
					throw IllegalStateException("Illegal constant pointer " + to_string(index) +
							": expected String, Class, Integer, Float, Long, Double, MethodType or MethodHandle constant");
				}

			public:
				LdcOperation(uint16_t index, const ConstValueConstant* value):
						ReturnableOperation(getReturnTypeFor(index, value)), index(index), value(value) {

					if(returnType->getSize() != size)
						throw TypeSizeMismatchException(TypeSize_nameOf(size), TypeSize_nameOf(returnType->getSize()), returnType->toString());
				}

				LdcOperation(const CodeEnvironment& environment, uint16_t index): LdcOperation(index, environment.constPool.get<ConstValueConstant>(index)) {}

				LdcOperation(const StringConstant* value): ReturnableOperation(STRING), index(0), value(value) {}
				LdcOperation(const MethodTypeConstant* value): ReturnableOperation(METHOD_TYPE), index(0), value(value) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return value->toString(environment.classinfo);
				}
		};


		struct LoadOperation: ReturnableOperation<> {
			public:
				const uint16_t index;
				const Variable& variable;

			protected:
				LoadOperation(const Type* returnType, uint16_t index, const Variable& variable):
						ReturnableOperation(variable.type->castTo(returnType)), index(index), variable(variable) {}

			public:
				LoadOperation(const Type* returnType, const CodeEnvironment& environment, uint16_t index):
						LoadOperation(returnType, index, environment.getCurrentScope()->getVariable(index)) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return environment.getCurrentScope()->getNameFor(variable);
				}

				virtual void onCastReturnType(const Type* newType) const override {
					variable.type = newType;
				}
		};

		struct ILoadOperation: LoadOperation {
			ILoadOperation(const CodeEnvironment& environment, uint16_t index): LoadOperation(ANY_INT_OR_BOOLEAN, environment, index) {}
		};

		struct LLoadOperation: LoadOperation {
			LLoadOperation(const CodeEnvironment& environment, uint16_t index): LoadOperation(LONG, environment, index) {}
		};

		struct FLoadOperation: LoadOperation {
			FLoadOperation(const CodeEnvironment& environment, uint16_t index): LoadOperation(FLOAT, environment, index) {}
		};

		struct DLoadOperation: LoadOperation {
			DLoadOperation(const CodeEnvironment& environment, uint16_t index): LoadOperation(DOUBLE, environment, index) {}
		};

		struct ALoadOperation: LoadOperation {
			ALoadOperation(const CodeEnvironment& environment, uint16_t index):
					LoadOperation(environment.getCurrentScope()->getVariable(index).type, environment, index) {}
		};


		struct ArrayLoadOperation: ReturnableOperation<> {
			public:
				const Operation *const index, *const array;

				ArrayLoadOperation(const Type* returnType, const CodeEnvironment& environment):
						ReturnableOperation(returnType), index(environment.stack.popAs(INT)), array(environment.stack.popAs(ArrayType(returnType))) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return array->toString(environment) + '[' + index->toString(environment) + ']';
				}
		};

		struct IALoadOperation: ArrayLoadOperation {
			IALoadOperation(const CodeEnvironment& environment): ArrayLoadOperation(INT, environment) {}
		};

		struct LALoadOperation: ArrayLoadOperation {
			LALoadOperation(const CodeEnvironment& environment): ArrayLoadOperation(LONG, environment) {}
		};

		struct FALoadOperation: ArrayLoadOperation {
			FALoadOperation(const CodeEnvironment& environment): ArrayLoadOperation(FLOAT, environment) {}
		};

		struct DALoadOperation: ArrayLoadOperation {
			DALoadOperation(const CodeEnvironment& environment): ArrayLoadOperation(DOUBLE, environment) {}
		};

		struct AALoadOperation: ArrayLoadOperation {
			AALoadOperation(const CodeEnvironment& environment): ArrayLoadOperation(
					environment.stack.lookup(1)->getReturnTypeAs(&AnyType::getArrayTypeInstance())->elementType, environment) {}
		};

		struct BALoadOperation: ArrayLoadOperation {
			BALoadOperation(const CodeEnvironment& environment): ArrayLoadOperation(ANY_INT, environment) {}
		};

		struct CALoadOperation: ArrayLoadOperation {
			CALoadOperation(const CodeEnvironment& environment): ArrayLoadOperation(ANY_INT, environment) {}
		};

		struct SALoadOperation: ArrayLoadOperation {
			SALoadOperation(const CodeEnvironment& environment): ArrayLoadOperation(ANY_INT, environment) {}
		};


		struct StoreOperation: TransientReturnableOperation {
			public:
				const Operation* const value;
				const uint16_t index;
				const Variable& variable;

				StoreOperation(const Type* requiredType, const CodeEnvironment& environment, uint16_t index):
						value(environment.stack.popAs(requiredType)), index(index), variable(environment.getCurrentScope()->getVariable(index)) {

					initReturnType<DupOperation<TypeSize::FOUR_BYTES>>(environment, value);
					//LOG("constructor StoreOperation 0x" << hex((uint64_t)this * (uint64_t)this) << ' ' << typeid(*environment.getCurrentScope()).name());
					variable.type = variable.type->castTo(requiredType->castTo(value->getReturnType()));
				}

				virtual string toString(const CodeEnvironment& environment) const override {
					return environment.getCurrentScope()->getNameFor(variable) + " = " + value->toString(environment);
				}
		};

		struct IStoreOperation: StoreOperation {
			IStoreOperation(const CodeEnvironment& environment, uint16_t index): StoreOperation(ANY_INT_OR_BOOLEAN, environment, index) {}
		};

		struct LStoreOperation: StoreOperation {
			LStoreOperation(const CodeEnvironment& environment, uint16_t index): StoreOperation(LONG, environment, index) {}
		};

		struct FStoreOperation: StoreOperation {
			FStoreOperation(const CodeEnvironment& environment, uint16_t index): StoreOperation(FLOAT, environment, index) {}
		};

		struct DStoreOperation: StoreOperation {
			DStoreOperation(const CodeEnvironment& environment, uint16_t index): StoreOperation(DOUBLE, environment, index) {}
		};

		struct AStoreOperation: StoreOperation {
			AStoreOperation(const CodeEnvironment& environment, uint16_t index): StoreOperation(&AnyObjectType::getInstance(), environment, index) {}
		};


		template<TypeSize size>
		struct PopOperation: VoidOperation, TypeSizeTemplatedOperation<size> {
			const Operation* const operation;

			PopOperation(const CodeEnvironment& environment): operation(environment.stack.pop()) {
				TypeSizeTemplatedOperation<size>::checkTypeSize(operation->getReturnType());
			}

			virtual string toString(const CodeEnvironment& environment) const override {
				return operation->toString(environment);
			}
		};



		struct SwapOperation: VoidOperation {
			SwapOperation(const CodeEnvironment& environment) {
				environment.stack.push(environment.stack.pop(), environment.stack.pop());
			}

			virtual string toString(const CodeEnvironment& environment) const override { return EMPTY_STRING; }
		};


		struct OperatorOperation: ReturnableOperation<> {
			public:
				const string operation;

				OperatorOperation(const Type* type, char32_t operation, uint16_t priority):
						ReturnableOperation(type, priority), operation(char32ToString(operation)) {}
		};


		struct BinaryOperatorOperation: OperatorOperation {
			public:
				const Operation *const operand2, *const operand1;

				BinaryOperatorOperation(const Type* type1, const Type* type2, const CodeEnvironment& environment, char32_t operation, uint16_t priority):
						OperatorOperation(type1, operation, priority), operand2(environment.stack.popAs(type2)), operand1(environment.stack.popAs(type1)) {}

				BinaryOperatorOperation(const Type* type, const CodeEnvironment& environment, char32_t operation, uint16_t priority):
						BinaryOperatorOperation(type, type, environment, operation, priority) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return operand1->toString(environment, priority, Associativity::LEFT) + ' ' + operation + ' ' +
							operand2->toString(environment, priority, Associativity::RIGHT);
				}
		};

		struct UnaryOperatorOperation: OperatorOperation {
			public:
				const Operation* const operand;

				UnaryOperatorOperation(const Type* type, const CodeEnvironment& environment, char32_t operation, uint16_t priority):
						OperatorOperation(type, operation, priority), operand(environment.stack.popAs(type)) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return operation + operand->toString(environment, priority, Associativity::RIGHT);
				}
		};



		struct IIncOperation: Operation {
			public:
				const Variable& variable;
				const int16_t value;

			protected:
				const Type* returnType;
				bool isShortInc, isPostInc = false;

			public:
				IIncOperation(const CodeEnvironment& environment, uint16_t index, int16_t value);

				virtual string toString(const CodeEnvironment& environment) const override {
					if(isShortInc) {
						const char* inc = value == 1 ? "++" : "--";
						return isPostInc || returnType == VOID ? environment.getCurrentScope()->getNameFor(variable) + inc : inc + environment.getCurrentScope()->getNameFor(variable);
					}
					return environment.getCurrentScope()->getNameFor(variable) + (string)(value < 0 ? " -" : " +") + "= " + to_string(abs(value));
				}

				virtual const Type* getReturnType() const override {
					return returnType;
				}
		};


		template<bool required>
		struct CastOperation: ReturnableOperation<> {
			public:
				const Operation* const operation;
				const Type* const type;

				CastOperation(const CodeEnvironment& environment, const Type* type):
						ReturnableOperation(type), operation(environment.stack.pop()), type(type) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return required ? '(' + type->toString(environment.classinfo) + ')' + operation->toString(environment, priority, Associativity::LEFT) :
							operation->toString(environment);
				}
		};



		//template<BasicType type>
		struct CmpOperation: BooleanOperation {
			const Operation *const operand2, *const operand1;

			CmpOperation(const CodeEnvironment& environment): operand2(environment.stack.pop()), operand1(environment.stack.pop()) {}
		};


		struct ICmpOperation: CmpOperation {
			ICmpOperation(const CodeEnvironment& environment): CmpOperation(environment) {}

			virtual string toString(const CodeEnvironment& environment) const override { throw Exception("Illegal using of icmp: toString()"); }
		};

		struct LCmpOperation: CmpOperation {
			LCmpOperation(const CodeEnvironment& environment): CmpOperation(environment) {}

			virtual string toString(const CodeEnvironment& environment) const override { throw Exception("Illegal using of lcmp: toString()"); }
		};

		struct FCmpOperation: CmpOperation {
			FCmpOperation(const CodeEnvironment& environment): CmpOperation(environment) {}

			virtual string toString(const CodeEnvironment& environment) const override { throw Exception("Illegal using of fcmp: toString()"); }
		};

		struct DCmpOperation: CmpOperation {
			DCmpOperation(const CodeEnvironment& environment): CmpOperation(environment) {}

			virtual string toString(const CodeEnvironment& environment) const override { throw Exception("Illegal using of dcmp: toString()"); }
		};

		struct ACmpOperation: CmpOperation {
			ACmpOperation(const CodeEnvironment& environment): CmpOperation(environment) {}

			virtual string toString(const CodeEnvironment& environment) const override { throw Exception("Illegal using of acmp: toString()"); }
		};


		struct CompareType;

		struct EqualsCompareType;

		struct CompareType {
			static const EqualsCompareType EQUALS, NOT_EQUALS;
			static const CompareType GREATER, GREATER_OR_EQUALS, LESS, LESS_OR_EQUALS;

			const char* const stringOperator;

			CompareType(const char* const stringOperator): stringOperator(stringOperator) {}
		};

		struct EqualsCompareType: CompareType {
			EqualsCompareType(const char* const stringOperator): CompareType(stringOperator) {}
		};


		const EqualsCompareType
				CompareType::EQUALS = EqualsCompareType("=="),
				CompareType::NOT_EQUALS = EqualsCompareType("!=");
		const CompareType
				CompareType::GREATER = CompareType(">"),
				CompareType::GREATER_OR_EQUALS = CompareType(">="),
				CompareType::LESS = CompareType("<"),
				CompareType::LESS_OR_EQUALS = CompareType("<=");


		struct CompareOperation: BooleanOperation {
			const CompareType& compareType;

			CompareOperation(const CompareType& compareType): compareType(compareType) {}
		};


		struct CompareBinaryOperation: CompareOperation {
			const Operation *const operand2, *const operand1;

			CompareBinaryOperation(const CmpOperation* cmpOperation, const CompareType& compareType):
					CompareOperation(compareType), operand2(cmpOperation->operand2), operand1(cmpOperation->operand1) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return operand1->toString(environment) + ' ' + compareType.stringOperator + ' ' + operand2->toString(environment);
			}
		};


		struct CompareWithZeroOperation: CompareOperation {
			const Operation* const operand;

			CompareWithZeroOperation(const Operation* operand, const CompareType& compareType): CompareOperation(compareType), operand(operand) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return operand->getReturnType() == BOOLEAN ? operand->toString(environment) :
						operand->toString(environment) + ' ' + compareType.stringOperator + " 0";
			}
		};


		struct TernaryOperatorOperation: ReturnableOperation<> {
			const Operation *const condition, *const trueCase, *const falseCase;

			const bool isShort;

			TernaryOperatorOperation(const Operation* condition, const Operation* trueCase, const Operation* falseCase):
					ReturnableOperation(trueCase->getReturnTypeAs(falseCase->getReturnType())),
					condition(condition), trueCase(trueCase), falseCase(falseCase),
					isShort(dynamic_cast<const IConstOperation*>(trueCase) && ((const IConstOperation*)trueCase)->value == 1 &&
					dynamic_cast<const IConstOperation*>(falseCase) && ((const IConstOperation*)falseCase)->value == 0) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return isShort ? condition->toString(environment) :
						condition->toString(environment) + " ? " + trueCase->toString(environment) + " : " + falseCase->toString(environment);
			}
		};


		struct IfScope;


		struct ElseScope: Scope {
			public:
				const IfScope* const ifScope;

			protected:
				bool isTernary = false;
				const Operation* ternaryFalseOperation = nullptr;

			public:
				ElseScope(const CodeEnvironment& environment, const uint32_t to, const IfScope* ifScope);

				virtual string getHeader(const CodeEnvironment& environment) const override {
					return " else ";
				}

				virtual inline string getFrontSeparator(const ClassInfo& classinfo) const override {
					return EMPTY_STRING;
				}

				virtual void finalize(const CodeEnvironment& environment) override;

				virtual const Type* getReturnType() const override {
					return isTernary ? ternaryFalseOperation->getReturnType() : this->Scope::getReturnType();
				}
		};



		struct ContinueOperation: VoidOperation {
			const IfScope* const ifScope;

			ContinueOperation(const CodeEnvironment& environment, const IfScope* ifScope);

			virtual string toString(const CodeEnvironment& environment) const override {
				return //ifScope->hasLabel ? "continue " + ifScope->getLabel() : "continue";
					"continue";
			}
		};


		struct IfScope: Scope {
			public:
				const CompareOperation* const condition;

			private:
				mutable const ElseScope* elseScope = nullptr;
				friend ElseScope::ElseScope(const CodeEnvironment&, const uint32_t, const IfScope*);

				mutable bool hasLabel = false;
				friend ContinueOperation::ContinueOperation(const CodeEnvironment&, const IfScope*);

				bool isTernary = false;
				const Operation* ternaryTrueOperation = nullptr;
				friend void ElseScope::finalize(const CodeEnvironment&);

			public: mutable bool isLoop = false;

			public:
				IfScope(const CodeEnvironment& environment, const int32_t offset, const CompareOperation* condition):
						Scope(environment.exprStartIndex, environment.bytecode.posToIndex(offset + environment.pos) - 1, environment.getCurrentScope()),
						condition(condition) {}

			protected:
				virtual string getHeader(const CodeEnvironment& environment) const override {
					return (string)(isLoop ? (hasLabel ? getLabel(environment) + ": while" : "while") : "if") + '(' + condition->toString(environment) + ") ";
				}

				virtual bool printNextOperation(const vector<const Operation*>::const_iterator i) const override {
					if(next(i) != code.end())
						return true;
					const ContinueOperation* continueOperation = dynamic_cast<const ContinueOperation*>(*i);
					return continueOperation == nullptr || continueOperation->ifScope != this;
				}

				virtual inline string getBackSeparator(const ClassInfo& classinfo) const override {
					return isLoop || elseScope == nullptr ? this->Scope::getBackSeparator(classinfo) : EMPTY_STRING;
				}

				string getLabel(const CodeEnvironment& environment) const {
					if(isLoop)
						return "Loop";
					throw DecompilationException("Cannot get label for if scope");
				}

			public:
				virtual void finalize(const CodeEnvironment& environment) override {
					isTernary = elseScope != nullptr && !environment.stack.empty() && code.empty();
					if(isTernary)
						ternaryTrueOperation = environment.stack.pop();
				}

				virtual const Type* getReturnType() const override {
					return isTernary ? ternaryTrueOperation->getReturnType() : this->Scope::getReturnType();
				}
		};

		struct IfCmpScope: IfScope {
			public:
				const CompareType& compareType;

				IfCmpScope(const CodeEnvironment& environment, const int32_t offset, const CompareType& compareType):
						IfScope(environment, offset, getCondition(environment, compareType)), compareType(compareType) {}

			private: static const CompareOperation* getCondition(const CodeEnvironment& environment, const CompareType& compareType) {
				const Operation* operation = environment.stack.pop();
				if(const CmpOperation* cmpOperation = castOperationTo<const CmpOperation*>(operation))
					return new CompareBinaryOperation(cmpOperation, compareType);
				return new CompareWithZeroOperation(operation, compareType);
			}
		};


		ContinueOperation::ContinueOperation(const CodeEnvironment& environment, const IfScope* ifScope): ifScope(ifScope) {
			if(ifScope != environment.getCurrentScope())
				ifScope->hasLabel = true;
		}


		ElseScope::ElseScope(const CodeEnvironment& environment, const uint32_t to, const IfScope* ifScope):
				Scope(environment.index, to, ifScope->parentScope), ifScope(ifScope) {
			ifScope->elseScope = this;
		}

		void ElseScope::finalize(const CodeEnvironment& environment) {
			isTernary = ifScope->isTernary;
			if(isTernary) {
				ternaryFalseOperation = environment.stack.pop();
				environment.stack.push(new TernaryOperatorOperation(ifScope->condition, ifScope->ternaryTrueOperation, ternaryFalseOperation));
			}
		}



		struct EmptyInfiniteLoopScope: Scope {
			EmptyInfiniteLoopScope(const CodeEnvironment& environment): Scope(environment.index, environment.index, environment.getCurrentScope()) {}

			virtual string toString(const CodeEnvironment& environment) const override { return "while(true);"; }
		};



		struct SwitchScope: Scope {
			public:
				const Operation* const value;
				const uint32_t defaultIndex;
				const map<int32_t, uint32_t> indexTable;

			protected:
				static const map<int32_t, uint32_t> offsetTableToIndexTable(const CodeEnvironment& environment, const map<int32_t, int32_t>& offsetTable) {
					map<int32_t, uint32_t> indexTable;

					for(auto& entry : offsetTable)
						indexTable[entry.first] = environment.bytecode.posToIndex(environment.pos + entry.second);

					return indexTable;
				}

			public:
				SwitchScope(const CodeEnvironment& environment, int32_t defaultOffset, map<int32_t, int32_t> offsetTable):
						Scope(environment.index,
							environment.bytecode.posToIndex(max(defaultOffset, max_element(offsetTable.begin(), offsetTable.end(),
								[] (auto& e1, auto& e2) { return e1.second < e2.second; })->second) + environment.pos),
							environment.getCurrentScope()),
						value(environment.stack.pop()), defaultIndex(environment.bytecode.posToIndex(defaultOffset + environment.pos)),
						indexTable(offsetTableToIndexTable(environment, offsetTable)) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					environment.classinfo.increaseIndent(2);

					string str = "switch(" + value->toString(environment) + ") {\n";
					const size_t baseSize = str.size();

					const map<uint32_t, uint32_t>& exprIndexTable = environment.exprIndexTable;

					const uint32_t defaultExprIndex = exprIndexTable.at(defaultIndex);

					uint32_t i = exprIndexTable.at(this->from);
					for(const Operation* operation : code) {
						if(i == defaultExprIndex) {
							environment.classinfo.reduceIndent();
							str += environment.classinfo.getIndent() + (string)"default:\n";
							environment.classinfo.increaseIndent();
						} else {
							for(auto& entry : indexTable) {
								if(i == exprIndexTable.at(entry.second)) {
									environment.classinfo.reduceIndent();
									str += environment.classinfo.getIndent() + (string)"case " + to_string(entry.first) + ":\n";
									environment.classinfo.increaseIndent();
									break;
								}
							}
						}
						str += environment.classinfo.getIndent() + operation->toString(environment) + (dynamic_cast<const Scope*>(operation) ? "\n" : ";\n");
						i++;
					}

					environment.classinfo.reduceIndent(2);

					if(str.size() == baseSize) {
						str[baseSize - 1] = '}';
						return str;
					}

					return str + environment.classinfo.getIndent() + '}';
				}
		};


		struct CatchScopeDataHolder {
			uint32_t from;
			vector<const ClassType*> catchTypes;

			CatchScopeDataHolder(uint32_t from, const ClassType* catchType): from(from), catchTypes({catchType}) {}
		};


		struct TryScope: Scope {
			protected:
				vector<CatchScopeDataHolder> handlersData;
				friend const CodeEnvironment& Method::decompileCode(const ClassInfo&);

			public:
				TryScope(uint32_t from, uint32_t to, Scope* parentScope):
						Scope(from, to, parentScope) {}

				virtual string getHeader(const CodeEnvironment& environment) const override {
					return "try ";
				}

				virtual string getBackSeparator(const ClassInfo& classinfo) const override {
					return "";
				}

				virtual void finalize(const CodeEnvironment& environment) override;
		};


		struct CatchScope: Scope {
			public:
				const vector<const ClassType*> catchTypes;
				const ClassType* const catchType;

			protected:
				vector<const Operation*> tmpStack;
				Variable* exceptionVariable;
				uint16_t exceptionVariableIndex;
				CatchScope* const nextHandler;

			public:
				CatchScope(const CodeEnvironment& environment, uint32_t from, uint32_t to, const vector<const ClassType*>& catchTypes, CatchScope* nextHandler):
						Scope(from, to, environment.getCurrentScope()), catchTypes(catchTypes), catchType(catchTypes[0]), nextHandler(nextHandler) {
					//LOG("constructor CatchScope 0x" << hex((uint64_t)this * (uint64_t)this) << " { from = " << from << ", to = " << to << " }");
				}

				CatchScope(const CodeEnvironment& environment, const CatchScopeDataHolder& dataHolder, uint32_t to, CatchScope* nextHandler):
						CatchScope(environment, dataHolder.from, to, dataHolder.catchTypes, nextHandler) {}


				virtual void add(const Operation* operation, const CodeEnvironment& environment) override {
					const StoreOperation* storeOperation;

					//LOG("add         CatchScope 0x" << hex<4>((uint64_t)this * (uint64_t)this) << ' '\
							<< (exceptionVariable != nullptr ? "not null" : "null") << ' '  << from << ".." << to << ' ' << typeid(*operation).name());

					if(exceptionVariable == nullptr && (storeOperation = dynamic_cast<const StoreOperation*>(operation)) != nullptr) {
						exceptionVariableIndex = storeOperation->index;
						exceptionVariable = new NamedVariable(catchType, "ex");
						return;
					}
					Scope::add(operation, environment);
				}

				virtual const Variable& getVariable(uint32_t index) const {
					//LOG("getVariable CatchScope 0x" << hex<4>((uint64_t)this * (uint64_t)this) << ' '\
							<< (exceptionVariable != nullptr ? "not null" : "null"));

					if(index == exceptionVariableIndex)
						return *exceptionVariable;
					return Scope::getVariable(index);
				}


				virtual string getFrontSeparator(const ClassInfo& classinfo) const override {
					return " ";
				}

				virtual string getHeader(const CodeEnvironment& environment) const override {
					//LOG("getHeader   CatchScope 0x" << hex<4>((uint64_t)this * (uint64_t)this) << ' '\
							<< (exceptionVariable != nullptr ? "not null" : "null"));

					//assert(exceptionVariable != nullptr);
					return catchType == nullptr ? "finally" :
							"catch(" + join<const ClassType*>(catchTypes,
									[&environment] (const ClassType* catchType) { return catchType->toString(environment.classinfo); }, " | ") +
										' ' + environment.getCurrentScope()->getNameFor(*exceptionVariable) + ") ";
				}

				virtual string getBackSeparator(const ClassInfo& classinfo) const override {
					return nextHandler == nullptr ? "\n" : "";
				}

				void initiate(const CodeEnvironment& environment) {
					//LOG("initiate    CatchScope 0x" << hex((uint64_t)this * (uint64_t)this) << " { from = " << from << ", to = " << to << " } at "\
							<< environment.index);

					tmpStack.reserve(environment.stack.size());
					while(!environment.stack.empty())
						tmpStack.push_back(environment.stack.pop());

					environment.stack.push(new LoadCatchedExceptionOperation(catchTypes.empty() ? catchType : THROWABLE));
				}

			protected:
				struct LoadCatchedExceptionOperation: Operation {
					const ClassType* const catchType;

					LoadCatchedExceptionOperation(const ClassType* catchType): catchType(catchType) {}

					virtual const Type* getReturnType() const override {
						return catchType;
					}

					virtual string toString(const CodeEnvironment& environment) const override {
						throw Exception("Illegal using of LoadCatchedExceptionOperation: toString()");
					}
				};

			public:
				virtual void finalize(const CodeEnvironment& environment) override {
					//LOG("finalize    CatchScope 0x" << hex((uint64_t)this * (uint64_t)this) << " { from = " << from << ", to = " << to << " } at "\
							<< environment.index);
					reverse(tmpStack.begin(), tmpStack.end());
					for(const Operation* operation : tmpStack)
						environment.stack.push(operation);

					if(nextHandler != nullptr) {
						//LOG("environment.addScope(nextHandler) CatchScope 0x" << hex((uint64_t)this * (uint64_t)this) << " (0x"\
								<< hex((uint64_t)nextHandler * (uint64_t)nextHandler) << ")");
						environment.addScope(nextHandler);
						nextHandler->initiate(environment);
					}
				}
		};


		void TryScope::finalize(const CodeEnvironment& environment) {
			//LOG("finalize    TryScope 0x" << hex((uint64_t)this * (uint64_t)this) << " { from = " << from << ", to = " << to << " }");

			assert(handlersData.size() > 0);
			sort(handlersData.begin(), handlersData.end(), [](auto& handler1, auto& handler2) { return handler1.from > handler2.from; });

			CatchScope* lastHandler = nullptr;

			for(const CatchScopeDataHolder& handlerData : handlersData)
				lastHandler = new CatchScope(environment, handlerData,
						lastHandler == nullptr ? environment.getCurrentScope()->to : lastHandler->from, lastHandler);

			environment.addScope(lastHandler);
			lastHandler->initiate(environment);
		}



		struct ReturnOperation: VoidOperation {
			public:
				const Operation* const value;

				ReturnOperation(const CodeEnvironment& environment, const Type* type):
						value(environment.stack.popAs(type->castTo(environment.descriptor.returnType))) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return "return " + value->toString(environment);
				}
		};


		struct IReturnOperation: ReturnOperation {
			IReturnOperation(const CodeEnvironment& environment): ReturnOperation(environment, ANY_INT_OR_BOOLEAN) {}
		};

		struct LReturnOperation: ReturnOperation {
			LReturnOperation(const CodeEnvironment& environment): ReturnOperation(environment, LONG) {}
		};

		struct FReturnOperation: ReturnOperation {
			FReturnOperation(const CodeEnvironment& environment): ReturnOperation(environment, FLOAT) {}
		};

		struct DReturnOperation: ReturnOperation {
			DReturnOperation(const CodeEnvironment& environment): ReturnOperation(environment, DOUBLE) {}
		};

		struct AReturnOperation: ReturnOperation {
			AReturnOperation(const CodeEnvironment& environment): ReturnOperation(environment, &AnyObjectType::getInstance()) {}
		};



		struct FieldOperation: Operation {
			public:
				const ClassType clazz;
				const FieldDescriptor descriptor;

			protected:
				FieldOperation(const FieldrefConstant* fieldref):
						clazz(fieldref->clazz), descriptor(fieldref->nameAndType) {}

				FieldOperation(const CodeEnvironment& environment, uint16_t index):
						FieldOperation(environment.constPool.get<FieldrefConstant>(index)) {}

				string staticFieldToString(const CodeEnvironment& environment) const {
					return clazz == environment.classinfo.thisType && !environment.getCurrentScope()->hasVariable(descriptor.name) ?
							(string)descriptor.name : clazz.toString(environment.classinfo) + '.' + descriptor.name;
				}

				string instanceFieldToString(const CodeEnvironment& environment, const Operation* object) const {
					const ALoadOperation* aloadOperation = dynamic_cast<const ALoadOperation*>(object);
					return aloadOperation != nullptr && !(environment.modifiers & ACC_STATIC) && aloadOperation->index == 0 &&
							!environment.getCurrentScope()->hasVariable(descriptor.name) ?
							(string)descriptor.name : object->toString(environment) + '.' + descriptor.name;
				}
		};


		struct PutFieldOperation: FieldOperation {
			public:
				const Operation* const value;

			protected:
				const Type* returnType;

				PutFieldOperation(const CodeEnvironment& environment, uint16_t index):
						FieldOperation(environment, index), value(environment.stack.popAs(&descriptor.type)) {
					if(const LoadOperation* loadOperation = castOperationTo<const LoadOperation*>(value))
						loadOperation->variable.addName(descriptor.name);
				}

			public:
				virtual const Type* getReturnType() const override {
					return returnType;
				}
		};

		struct PutStaticFieldOperation: PutFieldOperation {
			public:
				PutStaticFieldOperation(const CodeEnvironment& environment, uint16_t index): PutFieldOperation(environment, index) {
					returnType = getDupReturnType<DupOperation<TypeSize::FOUR_BYTES>, DupOperation<TypeSize::EIGHT_BYTES>>(environment, value);
				}

				virtual string toString(const CodeEnvironment& environment) const override {
					return staticFieldToString(environment) + " = " + value->toString(environment);
				}
		};

		struct PutInstanceFieldOperation: PutFieldOperation {
			public:
				const Operation* const object;

				PutInstanceFieldOperation(const CodeEnvironment& environment, uint16_t index):
						PutFieldOperation(environment, index), object(environment.stack.popAs(clazz)) {
					returnType = getDupReturnType<DupX1Operation, Dup2X1Operation>(environment, value);
				}

				virtual string toString(const CodeEnvironment& environment) const override {
					return instanceFieldToString(environment, object) + " = " + value->toString(environment);
				}
		};


		struct GetFieldOperation: FieldOperation {
			protected:
				GetFieldOperation(const CodeEnvironment& environment, uint16_t index): FieldOperation(environment, index) {}

			public:
				virtual const Type* getReturnType() const override {
					return &descriptor.type;
				}
		};

		struct GetStaticFieldOperation: GetFieldOperation {
			public:
				GetStaticFieldOperation(const CodeEnvironment& environment, uint16_t index): GetFieldOperation(environment, index) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return staticFieldToString(environment);
				}
		};

		struct GetInstanceFieldOperation: GetFieldOperation {
			public:
				const Operation* const object;

				GetInstanceFieldOperation(const CodeEnvironment& environment, uint16_t index):
						GetFieldOperation(environment, index), object(environment.stack.popAs(clazz)) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return instanceFieldToString(environment, object);
				}
		};



		struct InvokeOperation: Operation {
			public:
				const MethodDescriptor& descriptor;
				const vector<const Operation*> arguments;

			protected:
				const vector<const Operation*> popArguments(CodeStack& stack) const {
					const uint32_t size = descriptor.arguments.size();

					vector<const Operation*> arguments;
					arguments.reserve(size);

					for(uint32_t i = size; i > 0; )
						arguments.push_back(stack.popAs(descriptor.arguments[--i]));

					return arguments;
				}

				InvokeOperation(const CodeEnvironment& environment, const MethodDescriptor& descriptor):
						descriptor(descriptor), arguments(popArguments(environment.stack)) {}

				InvokeOperation(const CodeEnvironment& environment, const MethodrefConstant* methodref):
						InvokeOperation(environment, *new MethodDescriptor(methodref)) {}

				InvokeOperation(const CodeEnvironment& environment, uint16_t index):
						InvokeOperation(environment, environment.constPool.get<MethodrefConstant>(index)) {}

			public:
				virtual const Type* getReturnType() const override {
					return descriptor.returnType;
				}
		};


		struct InvokeNonStaticOperation: InvokeOperation {
			public:
				const Operation* const objectOperation;

			protected:
				InvokeNonStaticOperation(const CodeEnvironment& environment, const MethodDescriptor& descriptor):
						InvokeOperation(environment, descriptor), objectOperation(environment.stack.popAs(&descriptor.clazz)) {}

				InvokeNonStaticOperation(const CodeEnvironment& environment, uint16_t index):
						InvokeOperation(environment, index), objectOperation(environment.stack.popAs(&descriptor.clazz)) {}

				InvokeNonStaticOperation(const CodeEnvironment& environment, const Operation* objectOperation, uint16_t index):
						InvokeOperation(environment, index), objectOperation(objectOperation) {
					objectOperation->getReturnTypeAs(&descriptor.clazz);
				}

			public:
				virtual string toString(const CodeEnvironment& environment) const override {
					return objectOperation->toString(environment, priority, Associativity::LEFT) + '.' + descriptor.name + '(' +
							rjoin<const Operation*>(arguments, [&environment] (const Operation* operation) { return operation->toString(environment); }) + ')';
				}
		};


		struct InvokevirtualOperation: InvokeNonStaticOperation {
			InvokevirtualOperation(const CodeEnvironment& environment, uint16_t index): InvokeNonStaticOperation(environment, index) {}
		};


		struct InvokespecialOperation: InvokeNonStaticOperation {
			public:
				const bool isConstructor, isSuperConstructor;

				const Type* const returnType;

			private:
				inline const Type* getReturnType(const CodeEnvironment& environment) {
					return isConstructor && checkDup<DupOperation<TypeSize::FOUR_BYTES>>(environment, objectOperation) ?
							objectOperation->getReturnType() : InvokeNonStaticOperation::getReturnType();
				}

				inline bool getIsConstructor() {
					return descriptor.type == MethodDescriptor::MethodType::CONSTRUCTOR;
				}

				inline bool getIsSuperConstructor(const CodeEnvironment& environment) {
					return (!(environment.modifiers & ACC_STATIC) && // check that we invoking this (or super) constructor
							dynamic_cast<const ALoadOperation*>(objectOperation) && ((const ALoadOperation*)objectOperation)->index == 0 &&
							descriptor.clazz == environment.classinfo.superType);
				}

			public:
				InvokespecialOperation(const CodeEnvironment& environment, uint16_t index):
						InvokeNonStaticOperation(environment, index),
						isConstructor(getIsConstructor()), isSuperConstructor(getIsSuperConstructor(environment)), returnType(getReturnType(environment)) {}

				InvokespecialOperation(const CodeEnvironment& environment, const Operation* objectOperation, uint16_t index):
						InvokeNonStaticOperation(environment, objectOperation, index),
						isConstructor(getIsConstructor()), isSuperConstructor(getIsSuperConstructor(environment)), returnType(getReturnType(environment)) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					if(isConstructor)
						return (isSuperConstructor ? "super" : objectOperation->toString(environment, priority, Associativity::LEFT)) +
							'(' + rjoin<const Operation*>(arguments,
								[&environment] (const Operation* operation) { return operation->toString(environment); }) + ')';

					return InvokeNonStaticOperation::toString(environment);
				}

				virtual const Type* getReturnType() const override {
					return returnType;
				}

				virtual bool canAddToCode() const override {
					return !(isSuperConstructor && arguments.empty());
				}
		};


		struct InvokestaticOperation: InvokeOperation {
			public:
				InvokestaticOperation(const CodeEnvironment& environment, uint16_t index): InvokeOperation(environment, index) {}

				InvokestaticOperation(const CodeEnvironment& environment, const MethodDescriptor& descriptor): InvokeOperation(environment, descriptor) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return staticMethodToString(environment) + '(' +
							rjoin<const Operation*>(arguments, [&environment] (const Operation* operation) { return operation->toString(environment); }) + ')';
				}

			private:
				string staticMethodToString(const CodeEnvironment& environment) const {
					return descriptor.clazz == environment.classinfo.thisType ?
							descriptor.name : descriptor.clazz.toString(environment.classinfo) + '.' + descriptor.name;
				}
		};


		struct InvokeinterfaceOperation: InvokeNonStaticOperation {
			InvokeinterfaceOperation(const CodeEnvironment& environment, uint16_t index):
					InvokeNonStaticOperation(environment, index) {}
		};


		struct ConcatStringsOperation: InvokeOperation {
			const LdcOperation<TypeSize::FOUR_BYTES>* const pattern;

			struct StringOperand {
				bool isOperation;
				union {
					const Operation* operation;
					const string* stringConstant;
				} value;
			};

			vector<StringOperand> operands;

			ConcatStringsOperation(const CodeEnvironment& environment, const MethodDescriptor& concater):
					InvokeOperation(environment, concater), pattern(safe_cast<const LdcOperation<TypeSize::FOUR_BYTES>*>(environment.stack.popAs(STRING))) {
				auto arg = arguments.begin();
				string str;

				for(const char* cp = safe_cast<const StringConstant*>(pattern->value)->value->c_str(); *cp != '\0'; cp++) {
					if(*cp == '\1') {
						if(!str.empty()) {
							operands.push_back(StringOperand{ false, { .stringConstant = new string(str) } });
							str.clear();
						}
						operands.push_back(StringOperand{ true, { .operation = *arg++ } });
					} else {
						str += *cp;
					}

				}
			}

			virtual string toString(const CodeEnvironment& environment) const override {
				return rjoin<StringOperand>(operands, [&environment] (const StringOperand operand) {
					return operand.isOperation ? operand.value.operation->toString(environment) : stringToLiteral(*operand.value.stringConstant);
				}, " + ");
			}

			virtual const Type* getReturnType() const override {
				return STRING;
			}
		};



		struct NewOperation: Operation {
			public:
				const ClassType clazz;

				NewOperation(const CodeEnvironment& environment, const ClassConstant* classConstant):
						clazz(classConstant) {}

				NewOperation(const CodeEnvironment& environment, uint16_t classIndex):
						NewOperation(environment, environment.constPool.get<ClassConstant>(classIndex)) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return "new " + clazz.toString(environment.classinfo);
				}

				virtual const Type* getReturnType() const override {
					return &clazz;
				}
		};


		struct ArrayStoreOperation: VoidOperation {
			protected:
				const Operation *const value, *const index, *const array;
				bool isInitializer = false;

			public:
				ArrayStoreOperation(const Type* returnType, const CodeEnvironment& environment);

				virtual string toString(const CodeEnvironment& environment) const override {
					return isInitializer ? value->toString(environment) :
							array->toString(environment) + '[' + index->toString(environment) + "] = " + value->toString(environment);
				}

				virtual bool canAddToCode() const override {
					return !isInitializer;
				}
		};


		struct NewArrayOperation: Operation {
			protected:
				const ArrayType* const arrayType;
				vector<const Operation*> lengths;

				mutable vector<const Operation*> initializer;
				friend ArrayStoreOperation::ArrayStoreOperation(const Type*, const CodeEnvironment&);

			public:
				NewArrayOperation(const CodeEnvironment& environment, const ArrayType* arrayType): arrayType(arrayType) {
					const uint16_t dimensions = arrayType->nestingLevel;
					lengths.reserve(dimensions);
					for(uint16_t i = 0; i < dimensions; i++)
						lengths.push_back(environment.stack.pop());
				}

				NewArrayOperation(const CodeEnvironment& environment, const Type* memberType, uint16_t dimensions = 1):
						NewArrayOperation(environment, new ArrayType(memberType, dimensions)) {}


				virtual const Type* getReturnType() const override {
					return arrayType;
				}

				virtual string toString(const CodeEnvironment& environment) const override {
					if(initializer.empty())
						return "new " + arrayType->memberType->toString(environment.classinfo) +
								rjoin<const Operation*>(lengths, [&environment] (auto length) { return '[' + length->toString(environment) + ']'; }, "");
					return "new " + arrayType->toString(environment.classinfo) + ' ' + toArrayInitString(environment);
				}

				virtual string toArrayInitString(const CodeEnvironment& environment) const override {
					return "{ " + join<const Operation*>(initializer,
							[&environment] (auto element) { return element->toArrayInitString(environment); }) + " }";
				}
		};

		struct ANewArrayOperation: NewArrayOperation {
			ANewArrayOperation(const CodeEnvironment& environment, uint16_t index):
					NewArrayOperation(environment, parseReferenceType(*environment.constPool.get<ClassConstant>(index)->name)) {}
		};


		struct MultiANewArrayOperation: NewArrayOperation {
			MultiANewArrayOperation(const CodeEnvironment& environment, uint16_t index, uint16_t dimensions):
					NewArrayOperation(environment, new ArrayType(*environment.constPool.get<ClassConstant>(index)->name)) {}
		};



		ArrayStoreOperation::ArrayStoreOperation(const Type* elementType, const CodeEnvironment& environment):
				value(environment.stack.popAs(elementType)), index(environment.stack.popAs(INT)), array(environment.stack.pop()) {

			//checkDup<DupOperation<TypeSize::FOUR_BYTES>>(environment, array);

			if(const DupOperation<TypeSize::FOUR_BYTES>* dupArray = dynamic_cast<const DupOperation<TypeSize::FOUR_BYTES>*>(array)) {
				if(const NewArrayOperation* newArray = dynamic_cast<const NewArrayOperation*>(dupArray->operation)) {
					newArray->initializer.push_back(value);
					isInitializer = true;
				}
				/*else
					cerr << "type mismatch: " << endl;*/
			}
		}

		struct IAStoreOperation: ArrayStoreOperation {
			IAStoreOperation(const CodeEnvironment& environment): ArrayStoreOperation(INT, environment) {}
		};

		struct LAStoreOperation: ArrayStoreOperation {
			LAStoreOperation(const CodeEnvironment& environment): ArrayStoreOperation(LONG, environment) {}
		};

		struct FAStoreOperation: ArrayStoreOperation {
			FAStoreOperation(const CodeEnvironment& environment): ArrayStoreOperation(FLOAT, environment) {}
		};

		struct DAStoreOperation: ArrayStoreOperation {
			DAStoreOperation(const CodeEnvironment& environment): ArrayStoreOperation(DOUBLE, environment) {}
		};

		struct AAStoreOperation: ArrayStoreOperation {
			AAStoreOperation(const CodeEnvironment& environment): ArrayStoreOperation(
					environment.stack.lookup(2)->getReturnTypeAs(&AnyType::getArrayTypeInstance())->elementType, environment) {}
		};

		struct BAStoreOperation: ArrayStoreOperation {
			BAStoreOperation(const CodeEnvironment& environment): ArrayStoreOperation(BYTE, environment) {}
		};

		struct CAStoreOperation: ArrayStoreOperation {
			CAStoreOperation(const CodeEnvironment& environment): ArrayStoreOperation(CHAR, environment) {}
		};

		struct SAStoreOperation: ArrayStoreOperation {
			SAStoreOperation(const CodeEnvironment& environment): ArrayStoreOperation(SHORT, environment) {}
		};



		struct ArrayLengthOperation: IntOperation {
			protected:
				const Operation* const array;

			public:
				ArrayLengthOperation(const CodeEnvironment& environment): array(environment.stack.pop()) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return array->toString(environment, priority, Associativity::LEFT) + ".length";
				}
		};



		struct AThrowOperation: VoidOperation {
			protected: const Operation* const exceptionOperation;

			public:
				AThrowOperation(const CodeEnvironment& environment): exceptionOperation(environment.stack.pop()) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return "throw " + exceptionOperation->toString(environment);
				}
		};


		struct CheckCastOperation: ReturnableOperation<> {
			protected: const Operation* const object;

			public:
				CheckCastOperation(const CodeEnvironment& environment, uint16_t index):
						ReturnableOperation(parseReferenceType(*environment.constPool.get<ClassConstant>(index)->name), 13),
						object(environment.stack.pop()) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return '(' + returnType->toString(environment.classinfo) + ')' + object->toString(environment, priority, Associativity::RIGHT);
				}
		};

		struct InstanceofOperation: ReturnableOperation<> {
			protected:
				const Type* const type;
				const Operation* const object;

			public:
				InstanceofOperation(const CodeEnvironment& environment, uint16_t index): ReturnableOperation(BOOLEAN, 9),
						type(parseReferenceType(*environment.constPool.get<ClassConstant>(index)->name)), object(environment.stack.pop()) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return object->toString(environment, priority, Associativity::LEFT) + " instanceof " + type->toString(environment.classinfo);
				}
		};


		struct CompareWithNullOperation: CompareOperation {
			protected:
				const Operation* const operand;

			public:
				CompareWithNullOperation(const CodeEnvironment& environment, const EqualsCompareType& compareType):
						CompareOperation(compareType), operand(environment.stack.pop()) {}

				virtual string toString(const CodeEnvironment& environment) const {
					return operand->toString(environment) + ' ' + compareType.stringOperator + " null";
				}
		};


		struct IfNullScope: IfScope {
			IfNullScope(const CodeEnvironment& environment, int32_t offset):
					IfScope(environment, offset, new CompareWithNullOperation(environment, CompareType::EQUALS)) {}
		};

		struct IfNonNullScope: IfScope {
			IfNonNullScope(const CodeEnvironment& environment, int32_t offset):
					IfScope(environment, offset, new CompareWithNullOperation(environment, CompareType::NOT_EQUALS)) {}
		};



		static IConstOperation
				*const ICONST_M1_OPERATION = new IConstOperation(-1),
				*const ICONST_0_OPERATION = new IConstOperation(0),
				*const ICONST_1_OPERATION = new IConstOperation(1),
				*const ICONST_2_OPERATION = new IConstOperation(2),
				*const ICONST_3_OPERATION = new IConstOperation(3),
				*const ICONST_4_OPERATION = new IConstOperation(4),
				*const ICONST_5_OPERATION = new IConstOperation(5);
		static LConstOperation
				*const LCONST_0_OPERATION = new LConstOperation(0),
				*const LCONST_1_OPERATION = new LConstOperation(1);
		static FConstOperation
				*const FCONST_0_OPERATION = new FConstOperation(0),
				*const FCONST_1_OPERATION = new FConstOperation(1),
				*const FCONST_2_OPERATION = new FConstOperation(2);
		static DConstOperation
				*const DCONST_0_OPERATION = new DConstOperation(0),
				*const DCONST_1_OPERATION = new DConstOperation(1);
	}


	void StaticInitializerScope::add(const Operation* operation, const CodeEnvironment& environment) {
		using namespace Operations;

		if(!fieldsInitialized) {
			const PutStaticFieldOperation* putOperation = dynamic_cast<const PutStaticFieldOperation*>(operation);
			if(putOperation != nullptr && ClassType(putOperation->clazz) == environment.classinfo.thisType) {
				if(const Field* field = environment.classinfo.clazz.getField(putOperation->descriptor.name)) {
					field->initializer = putOperation->value;
					field->environment = &environment;
				}
			} else {
				fieldsInitialized = true;
				code.push_back(operation);
			}
		} else
			code.push_back(operation);
	}
}

#undef inline

#endif
