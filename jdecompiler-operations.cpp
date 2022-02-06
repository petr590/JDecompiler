#ifndef JDECOMPILER_OPERATIONS_CPP
#define JDECOMPILER_OPERATIONS_CPP

#include <algorithm>

#undef LOG_PREFIX
#define LOG_PREFIX "jdecompiler-operations.cpp"

namespace JDecompiler {
	namespace Operations {

		template<class T = Type>
		struct ReturnableOperation: Operation { // ReturnableOperation is an operation which returns specified type
			static_assert(is_base_of<Type, T>::value, "template class T of struct ReturnableOperation is not subclass of class Type");

			protected: const T* const returnType;

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
				virtual void castReturnTypeTo(const Type* newType) const override {
					returnType = newType;
				}
		};


		struct IConstOperation: ConstOperation<int32_t> {
			IConstOperation(int32_t value): ConstOperation(ANY_INT_OR_BOOLEAN, value) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				if(returnType->isInstanceof(BOOLEAN) && (bool)value == value)
					return primitiveToString((bool)value);
				if(returnType->isInstanceof(BYTE) && (int8_t)value == value)
					return primitiveToString((int8_t)value);
				if(returnType->isInstanceof(CHAR) && (char16_t)value == value)
					return primitiveToString((char16_t)value);
				if(returnType->isInstanceof(SHORT) && (int16_t)value == value)
					return primitiveToString((int16_t)value);
				if(returnType->isInstanceof(INT))
					return primitiveToString(value);
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
		struct IPushOperation: IntOperation {
			static_assert(is_same<T, int8_t>::value || is_same<T, int16_t>::value, "template type T of struct IPushOperation is not uint8_t or uint16_t type");

			protected:
				const T value;

			public:
				IPushOperation(T value): value(value) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return primitiveToString(value);
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
					throw IllegalStateException("Illegal constant pointer 0x" + hex(index) +
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
				const Variable* const variable;

				LoadOperation(const Type* returnType, const CodeEnvironment& environment, uint16_t index):
						ReturnableOperation(returnType), variable(environment.getCurrentScope()->getVariable(index)) {
					variable->type->castTo(returnType);
				}

				virtual string toString(const CodeEnvironment& environment) const override {
					return variable->getName();
				}

				virtual void castReturnTypeTo(const Type* newType) const override {
					variable->type = newType;
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
					LoadOperation(environment.getCurrentScope()->getVariable(index)->type, environment, index) {}
		};


		struct ArrayLoadOperation: ReturnableOperation<> {
			public:
				const Operation *const index, *const array;

				ArrayLoadOperation(const Type* returnType, const CodeEnvironment& environment):
						ReturnableOperation(returnType), index(environment.stack.pop()), array(environment.stack.pop()) {}

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
			BALoadOperation(const CodeEnvironment& environment): ArrayLoadOperation(BYTE, environment) {}
		};

		struct CALoadOperation: ArrayLoadOperation {
			CALoadOperation(const CodeEnvironment& environment): ArrayLoadOperation(CHAR, environment) {}
		};

		struct SALoadOperation: ArrayLoadOperation {
			SALoadOperation(const CodeEnvironment& environment): ArrayLoadOperation(SHORT, environment) {}
		};


		struct StoreOperation: VoidOperation {
			public:
				const Operation* const value;
				const Variable* const variable;

				StoreOperation(const CodeEnvironment& environment, uint16_t index):
						value(environment.stack.pop()), variable(environment.getCurrentScope()->getVariable(index)) {
					variable->type = variable->type->castTo(value->getReturnType());
				}

				virtual string toString(const CodeEnvironment& environment) const override {
					return variable->getName() + " = " + value->toString(environment);
				}
		};

		struct IStoreOperation: StoreOperation {
			IStoreOperation(const CodeEnvironment& environment, uint16_t index): StoreOperation(environment, index) {}
		};

		struct LStoreOperation: StoreOperation {
			LStoreOperation(const CodeEnvironment& environment, uint16_t index): StoreOperation(environment, index) {}
		};

		struct FStoreOperation: StoreOperation {
			FStoreOperation(const CodeEnvironment& environment, uint16_t index): StoreOperation(environment, index) {}
		};

		struct DStoreOperation: StoreOperation {
			DStoreOperation(const CodeEnvironment& environment, uint16_t index): StoreOperation(environment, index) {}
		};

		struct AStoreOperation: StoreOperation {
			AStoreOperation(const CodeEnvironment& environment, uint16_t index): StoreOperation(environment, index) {}
		};


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
		struct PopOperation: VoidOperation, TypeSizeTemplatedOperation<size> {
			const Operation* const operation;

			PopOperation(const CodeEnvironment& environment): operation(environment.stack.pop()) {
				TypeSizeTemplatedOperation<size>::checkTypeSize(operation->getReturnType());
			}

			virtual string toString(const CodeEnvironment& environment) const override {
				return operation->toString(environment);
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
			protected: const Operation *const operand2, *const operand1;

			public:
				BinaryOperatorOperation(const Type* type, const CodeEnvironment& environment, char32_t operation, uint16_t priority):
						OperatorOperation(type, operation, priority), operand2(environment.stack.pop()), operand1(environment.stack.pop()) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return operand1->toString(environment, priority, Associativity::LEFT) + ' ' + operation + ' ' +
							operand2->toString(environment, priority, Associativity::RIGHT);
				}
		};

		struct UnaryOperatorOperation: OperatorOperation {
			protected: const Operation* const operand;

			public:
				UnaryOperatorOperation(const Type* type, const CodeEnvironment& environment, char32_t operation, uint16_t priority):
						OperatorOperation(type, operation, priority), operand(environment.stack.pop()) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return operation + operand->toString(environment, priority, Associativity::RIGHT);
				}
		};



		struct IIncOperation: Operation {
			public:
				const Variable* const variable;
				const int16_t value;

			protected:
				const Type* returnType;
				bool isShortInc, isPostInc = false;

			public:
				IIncOperation(const CodeEnvironment& environment, uint16_t index, int16_t value);

				virtual string toString(const CodeEnvironment& environment) const override {
					if(isShortInc) {
						const char* inc = value == 1 ? "++" : "--";
						return isPostInc || returnType == VOID ? variable->getName() + inc : inc + variable->getName();
					}
					return variable->getName() + (string)(value < 0 ? " -" : " +") + "= " + to_string(abs(value));
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
					return required ? '(' + type->name + ')' + operation->toString(environment, priority, Associativity::LEFT) :
							operation->toString(environment);
				}
		};



		struct CmpOperation: BooleanOperation {
			const Operation* const operand2, * const operand1;

			CmpOperation(const CodeEnvironment& environment): operand2(environment.stack.pop()), operand1(environment.stack.pop()) {}
		};


		struct ICmpOperation: CmpOperation {
			ICmpOperation(const CodeEnvironment& environment): CmpOperation(environment) {}

			virtual string toString(const CodeEnvironment& environment) const override { throw Exception("illegal using of icmp: toString"); }
		};

		struct LCmpOperation: CmpOperation {
			LCmpOperation(const CodeEnvironment& environment): CmpOperation(environment) {}

			virtual string toString(const CodeEnvironment& environment) const override { throw Exception("illegal using of lcmp: toString"); }
		};

		struct FCmpOperation: CmpOperation {
			FCmpOperation(const CodeEnvironment& environment): CmpOperation(environment) {}

			virtual string toString(const CodeEnvironment& environment) const override { throw Exception("illegal using of fcmp: toString"); }
		};

		struct DCmpOperation: CmpOperation {
			DCmpOperation(const CodeEnvironment& environment): CmpOperation(environment) {}

			virtual string toString(const CodeEnvironment& environment) const override { throw Exception("illegal using of dcmp: toString"); }
		};

		struct ACmpOperation: CmpOperation {
			ACmpOperation(const CodeEnvironment& environment): CmpOperation(environment) {}

			virtual string toString(const CodeEnvironment& environment) const override { throw Exception("illegal using of acmp: toString"); }
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
			const Operation* const operand2, * const operand1;

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
				IfScope(const CodeEnvironment& environment, const int16_t offset, const CompareOperation* condition):
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
					return !continueOperation || continueOperation->ifScope != this;
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

				IfCmpScope(const CodeEnvironment& environment, const int16_t offset, const CompareType& compareType):
						IfScope(environment, offset, getCondition(environment, compareType)), compareType(compareType) {}

			private: static const CompareOperation* getCondition(const CodeEnvironment& environment, const CompareType& compareType) {
				const Operation* operation = environment.stack.pop();
				if(const CmpOperation* cmpOperation = dynamic_cast<const CmpOperation*>(operation))
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

			protected:
				map<int32_t, uint32_t> indexTable;

			public:
				SwitchScope(const CodeEnvironment& environment, int32_t defaultOffset, map<int32_t, int32_t> offsetTable):
						Scope(environment.index,
							environment.bytecode.posToIndex(max(defaultOffset, max_element(offsetTable.begin(), offsetTable.end(),
								[] (auto& e1, auto& e2) { return e1.second < e2.second; })->second) + environment.pos),
							environment.getCurrentScope()),
						value(environment.stack.pop()), defaultIndex(environment.bytecode.posToIndex(defaultOffset + environment.pos))
				{
					for(auto& entry : offsetTable)
						indexTable[entry.first] = environment.bytecode.posToIndex(environment.pos + entry.second);
				}

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



		struct ReturnOperation: VoidOperation {
			public:
				const Operation* const value;
				const Type* const type;

				ReturnOperation(const CodeEnvironment& environment, const Type* type): value(environment.stack.pop()), type(type) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return "return " + value->toString(environment);
				}
		};


		struct IReturnOperation: ReturnOperation {
			IReturnOperation(const CodeEnvironment& environment): ReturnOperation(environment, INT) {}
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
			AReturnOperation(const CodeEnvironment& environment): ReturnOperation(environment, &AnyType::getInstance()) {}
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
					return clazz == *environment.classinfo.type && !environment.currentScope->hasVariable(descriptor.name) ?
							(string)descriptor.name : clazz.toString(environment.classinfo) + '.' + descriptor.name;
				}

				string instanceFieldToString(const CodeEnvironment& environment, const Operation* object) const {
					const ALoadOperation* aloadOperation = dynamic_cast<const ALoadOperation*>(object);
					return aloadOperation != nullptr && aloadOperation->variable->getName() == "this" &&
							!environment.currentScope->hasVariable(descriptor.name) ?
							(string)descriptor.name : object->toString(environment) + '.' + descriptor.name;
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

		struct PutFieldOperation: FieldOperation {
			public:
				const Operation* const value;

			protected:
				PutFieldOperation(const CodeEnvironment& environment, uint16_t index): FieldOperation(environment, index), value(environment.stack.pop()) {
					value->getReturnTypeAs(&descriptor.type);
				}

			public:
				virtual const Type* getReturnType() const override { return VOID; }
		};


		struct GetStaticFieldOperation: GetFieldOperation {
			public:
				GetStaticFieldOperation(const CodeEnvironment& environment, uint16_t index): GetFieldOperation(environment, index) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return staticFieldToString(environment);
				}
		};

		struct PutStaticFieldOperation: PutFieldOperation {
			public:
				PutStaticFieldOperation(const CodeEnvironment& environment, uint16_t index): PutFieldOperation(environment, index) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return staticFieldToString(environment) + " = " + value->toString(environment);
				}
		};


		struct GetInstanceFieldOperation: GetFieldOperation {
			public:
				const Operation* const object;

				GetInstanceFieldOperation(const CodeEnvironment& environment, uint16_t index):
						GetFieldOperation(environment, index), object(environment.stack.pop()) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return instanceFieldToString(environment, object);
				}
		};

		struct PutInstanceFieldOperation: PutFieldOperation {
			public:
				const Operation* const object;

				PutInstanceFieldOperation(const CodeEnvironment& environment, uint16_t index):
						PutFieldOperation(environment, index), object(environment.stack.pop()) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return instanceFieldToString(environment, object) + " = " + value->toString(environment);
				}
		};



		struct InvokeOperation: Operation {
			public:
				const MethodDescriptor& descriptor;
				const vector<const Operation*> arguments;

			protected:
				const vector<const Operation*> popArguments(Stack& stack) const {
					vector<const Operation*> arguments;

					for(uint32_t i = descriptor.arguments.size(); i > 0; i--)
						arguments.push_back(stack.pop());

					return arguments;
				}

				InvokeOperation(const CodeEnvironment& environment, const MethodDescriptor& descriptor):
						descriptor(descriptor), arguments(popArguments(environment.stack)) {}

				InvokeOperation(const CodeEnvironment& environment, uint16_t index):
						InvokeOperation(environment, *new MethodDescriptor(environment.constPool.get<MethodrefConstant>(index)->nameAndType)) {}

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
						InvokeOperation(environment, descriptor), objectOperation(environment.stack.pop()) {}

				InvokeNonStaticOperation(const CodeEnvironment& environment, uint16_t index):
						InvokeOperation(environment, index), objectOperation(environment.stack.pop()) {}

				InvokeNonStaticOperation(const CodeEnvironment& environment, const Operation* objectOperation, uint16_t index):
						InvokeOperation(environment, index), objectOperation(objectOperation) {}

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
				const bool isConstructor;

				InvokespecialOperation(const CodeEnvironment& environment, uint16_t index):
						InvokeNonStaticOperation(environment, index), isConstructor(descriptor.type == MethodDescriptor::MethodType::CONSTRUCTOR) {
					if(isConstructor && dynamic_cast<const DupOperation<TypeSize::FOUR_BYTES>*>(objectOperation))
						environment.stack.pop();
				}

				InvokespecialOperation(const CodeEnvironment& environment, const Operation* objectOperation, uint16_t index):
						InvokeNonStaticOperation(environment, objectOperation, index),
						isConstructor(descriptor.type == MethodDescriptor::MethodType::CONSTRUCTOR) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					if(isConstructor)
						return objectOperation->toString(environment, priority, Associativity::LEFT) + '(' + rjoin<const Operation*>(arguments,
								[&environment] (const Operation* operation) { return operation->toString(environment); }) + ')';
					return InvokeNonStaticOperation::toString(environment);
				}

				virtual const Type* getReturnType() const override {
					if(isConstructor)
						return objectOperation->getReturnType();
					return InvokeNonStaticOperation::getReturnType();
				}
		};


		struct InvokestaticOperation: InvokeOperation {
			public:
				const ClassType* const clazz;

				InvokestaticOperation(const CodeEnvironment& environment, uint16_t index): InvokeOperation(environment, index),
						clazz(new ClassType(environment.constPool.get<MethodrefConstant>(index)->clazz)) {}

				InvokestaticOperation(const CodeEnvironment& environment, const MethodDescriptor& descriptor, const ClassType* clazz):
						InvokeOperation(environment, descriptor), clazz(clazz) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return staticMethodToString(environment) + '(' +
							rjoin<const Operation*>(arguments, [&environment] (const Operation* operation) { return operation->toString(environment); }) + ')';
				}

			private:
				string staticMethodToString(const CodeEnvironment& environment) const {
					return *clazz == *environment.classinfo.type ?
							descriptor.name : clazz->toString(environment.classinfo) + '.' + descriptor.name;
				}
		};


		struct InvokeinterfaceOperation: InvokeNonStaticOperation {
			InvokeinterfaceOperation(const CodeEnvironment& environment, uint16_t index):
					InvokeNonStaticOperation(environment, *new MethodDescriptor(environment.constPool.get<InterfaceMethodrefConstant>(index)->nameAndType)) {}
		};



		struct NewOperation: Operation {
			public:
				const ClassType* const clazz;

				NewOperation(const CodeEnvironment& environment, uint16_t classIndex):
						clazz(new ClassType(environment.constPool.get<ClassConstant>(classIndex))) {}

				NewOperation(const CodeEnvironment& environment, const ClassConstant* classConstant): clazz(new ClassType(*classConstant->name)) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return "new " + clazz->toString(environment.classinfo);
				}

				virtual const Type* getReturnType() const override {
					return clazz;
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



		ArrayStoreOperation::ArrayStoreOperation(const Type* returnType, const CodeEnvironment& environment):
				value(environment.stack.pop()), index(environment.stack.pop()), array(environment.stack.pop()) {
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
						type(parseType(*environment.constPool.get<ClassConstant>(index)->name)), object(environment.stack.pop()) {}

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
			public: IfNullScope(const CodeEnvironment& environment, int16_t offset):
					IfScope(environment, offset, new CompareWithNullOperation(environment, CompareType::EQUALS)) {}
		};

		struct IfNonNullScope: IfScope {
			public: IfNonNullScope(const CodeEnvironment& environment, int16_t offset):
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
			if(putOperation != nullptr && ClassType(putOperation->clazz) == *environment.classinfo.type) {
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

#endif
