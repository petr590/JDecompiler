#ifndef JDECOMPILER_OPERATIONS_CPP
#define JDECOMPILER_OPERATIONS_CPP

#include "class.cpp"

namespace jdecompiler {

	using namespace std;


	namespace operations {

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
			inline IntOperation(): Operation() {}
			inline IntOperation(uint16_t priority): Operation(priority) {}

			virtual const Type* getReturnType() const override { return INT; }
		};

		struct AnyIntOperation: Operation {
			inline AnyIntOperation(): Operation() {}
			inline AnyIntOperation(uint16_t priority): Operation(priority) {}

			virtual const Type* getReturnType() const override { return ANY_INT; }
		};

		struct BooleanOperation: Operation {
			inline BooleanOperation(): Operation() {}
			inline BooleanOperation(uint16_t priority): Operation(priority) {}

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
				TransientReturnableOperation(): returnType(nullptr) {}

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
				template<TypeSize S = size>
				void checkTypeSize(const Type* type) const {
					if(type->getSize() != S)
						throw TypeSizeMismatchException(TypeSize_nameOf(S), TypeSize_nameOf(type->getSize()), type->toString());
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
	using namespace operations;

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

namespace operations {


		template<typename T>
		struct ConstOperation: Operation {
			protected:
				mutable const Type* returnType;

			public:
				const T value;

				ConstOperation(const Type* returnType, const T value): returnType(returnType), value(value) {}

				ConstOperation(const T value): returnType(TypeByBuiltinType<T>::value), value(value) {}

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

		using LConstOperation = ConstOperation<int64_t>;

		using FConstOperation = ConstOperation<float>;

		using DConstOperation = ConstOperation<double>;


		template<TypeSize size, class CT, typename RT>
		struct LdcOperation: ReturnableOperation<> {
			static_assert(is_base_of<ConstValueConstant, CT>::value,
					"template type CT of struct LdcOperation is not subclass of class ConstValueConstant");

			public:
				const uint16_t index;
				const CT* const value;

				LdcOperation(uint16_t index, const CT* value): ReturnableOperation(TypeByBuiltinType<RT>::value), index(index), value(value) {
					if(returnType->getSize() != size)
						throw TypeSizeMismatchException(TypeSize_nameOf(size), TypeSize_nameOf(returnType->getSize()), returnType->toString());
				}

				LdcOperation(const CodeEnvironment& environment, uint16_t index): LdcOperation(index, environment.constPool.get<CT>(index)) {}

				LdcOperation(const CT* value): LdcOperation(0, value) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return value->toString(environment.classinfo);
				}
		};


		using StringConstOperation = LdcOperation<TypeSize::FOUR_BYTES, StringConstant, BuiltinTypes::String>;
		using ClassConstOperation = LdcOperation<TypeSize::FOUR_BYTES, ClassConstant, BuiltinTypes::Class>;
		using MethodTypeConstOperation = LdcOperation<TypeSize::FOUR_BYTES, MethodTypeConstant, BuiltinTypes::MethodType>;
		using MethodHandleConstOperation = LdcOperation<TypeSize::FOUR_BYTES, MethodHandleConstant, BuiltinTypes::MethodHandle>;


		static const Operation* LdcOperation_valueOf(uint16_t index, const ConstValueConstant* value) {
			if(instanceof<const StringConstant*>(value))       return new StringConstOperation(static_cast<const StringConstant*>(value));
			if(instanceof<const ClassConstant*>(value))        return new ClassConstOperation(static_cast<const ClassConstant*>(value));
			if(instanceof<const IntegerConstant*>(value))      return new IConstOperation(static_cast<const IntegerConstant*>(value)->value);
			if(instanceof<const FloatConstant*>(value))        return new FConstOperation(static_cast<const FloatConstant*>(value)->value);
			if(instanceof<const LongConstant*>(value))         return new LConstOperation(static_cast<const LongConstant*>(value)->value);
			if(instanceof<const DoubleConstant*>(value))       return new DConstOperation(static_cast<const DoubleConstant*>(value)->value);
			if(instanceof<const MethodTypeConstant*>(value))   return new MethodTypeConstOperation(static_cast<const MethodTypeConstant*>(value));
			if(instanceof<const MethodHandleConstant*>(value)) return new MethodHandleConstOperation(static_cast<const MethodHandleConstant*>(value));
			throw IllegalStateException("Illegal constant pointer " + to_string(index) +
					": expected String, Class, Integer, Float, Long, Double, MethodType or MethodHandle constant");
		}

		static inline const Operation* LdcOperation_valueOf(const CodeEnvironment& environment, uint16_t index) {
			return LdcOperation_valueOf(index, environment.constPool.get<ConstValueConstant>(index));
		}



		struct LoadOperation: ReturnableOperation<> {
			public:
				const uint16_t index;
				const Variable& variable;
				//const Type* t = PrimitiveByType<int32_t>::value;

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
					environment.stack.lookup(1)->getReturnTypeAs(AnyType::getArrayTypeInstance())->elementType, environment) {}
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
					variable.type = variable.type->castTo(value->getReturnType()->castTo(requiredType));
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
			AStoreOperation(const CodeEnvironment& environment, uint16_t index): StoreOperation(AnyObjectType::getInstance(), environment, index) {}
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
				const char* const operation;

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
						return isPostInc || returnType == VOID ? environment.getCurrentScope()->getNameFor(variable) + inc :
								inc + environment.getCurrentScope()->getNameFor(variable);
					}
					return environment.getCurrentScope()->getNameFor(variable) + (value < 0 ? " -" : " +") + "= " + to_string(abs(value));
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
							environment.bytecode.posToIndex(environment.pos + max(defaultOffset, max_element(offsetTable.begin(), offsetTable.end(),
								[] (auto& e1, auto& e2) { return e1.second < e2.second; })->second)),
							environment.getCurrentScope()),
						value(environment.stack.pop()), defaultIndex(environment.bytecode.posToIndex(environment.pos + defaultOffset)),
						indexTable(offsetTableToIndexTable(environment, offsetTable)) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					environment.classinfo.increaseIndent(2);

					string str = "switch(" + value->toString(environment) + ") {\n";
					const size_t baseSize = str.size();

					const map<uint32_t, uint32_t>& exprIndexTable = environment.exprIndexTable;

					const uint32_t defaultExprIndex = exprIndexTable.at(defaultIndex);

					uint32_t i = exprIndexTable.at(this->startPos);
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
						str += environment.classinfo.getIndent() + operation->toString(environment) + (instanceof<const Scope*>(operation) ? "\n" : ";\n");
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
			uint32_t startPos;
			vector<const ClassType*> catchTypes;

			CatchScopeDataHolder(uint32_t startPos, const ClassType* catchType): startPos(startPos), catchTypes{catchType} {}
		};


		struct TryScope: Scope {
			protected:
				vector<CatchScopeDataHolder> handlersData;
				friend const CodeEnvironment& Method::decompileCode(const ClassInfo&);

			public:
				TryScope(uint32_t startPos, uint32_t endPos, Scope* parentScope):
						Scope(startPos, endPos, parentScope) {}

				virtual string getHeader(const CodeEnvironment& environment) const override {
					return "try ";
				}

				virtual string getBackSeparator(const ClassInfo& classinfo) const override {
					return EMPTY_STRING;
				}

				virtual void finalize(const CodeEnvironment& environment) override;
		};


		struct CatchScope: Scope {
			public:
				const vector<const ClassType*> catchTypes;
				const ClassType* const catchType;

			protected:
				vector<const Operation*> tmpStack;
				Variable* exceptionVariable = nullptr;
				uint16_t exceptionVariableIndex;
				CatchScope* const nextHandler;

			public:
				CatchScope(const CodeEnvironment& environment, uint32_t startPos, uint32_t endPos,
						const vector<const ClassType*>& catchTypes, CatchScope* nextHandler):
						Scope(startPos, endPos, environment.getCurrentScope()), catchTypes(catchTypes), catchType(catchTypes[0]), nextHandler(nextHandler) {
				}

				CatchScope(const CodeEnvironment& environment, const CatchScopeDataHolder& dataHolder, uint32_t endPos, CatchScope* nextHandler):
						CatchScope(environment, dataHolder.startPos, endPos, dataHolder.catchTypes, nextHandler) {}


				virtual void add(const Operation* operation, const CodeEnvironment& environment) override {
					if(exceptionVariable == nullptr) {
						if(instanceof<const StoreOperation*>(operation)) {
							exceptionVariableIndex = static_cast<const StoreOperation*>(operation)->index;
							exceptionVariable = new NamedVariable(catchType, "ex");
							return;
						} else {
							environment.warning("first instruction in the catch or finally block should be `astore`");
						}
					}
					Scope::add(operation, environment);
				}

				virtual const Variable& getVariable(uint32_t index) const {
					if(exceptionVariable != nullptr && index == exceptionVariableIndex)
						return *exceptionVariable;
					return Scope::getVariable(index);
				}


				virtual string getFrontSeparator(const ClassInfo& classinfo) const override {
					return " ";
				}

				virtual string getHeader(const CodeEnvironment& environment) const override {
					return catchType == nullptr ? "finally" :
							"catch(" + join<const ClassType*>(catchTypes,
									[&environment] (const ClassType* catchType) { return catchType->toString(environment.classinfo); }, " | ") +
										' ' + environment.getCurrentScope()->getNameFor(*exceptionVariable) + ") ";
				}

				virtual string getBackSeparator(const ClassInfo& classinfo) const override {
					return nextHandler == nullptr ? "\n" : EMPTY_STRING;
				}

				void initiate(const CodeEnvironment& environment) {
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
					reverse(tmpStack.begin(), tmpStack.end());
					for(const Operation* operation : tmpStack)
						environment.stack.push(operation);

					if(nextHandler != nullptr) {
						environment.addScope(nextHandler);
						nextHandler->initiate(environment);
					}
				}
		};


		void TryScope::finalize(const CodeEnvironment& environment) {
			assert(handlersData.size() > 0);
			sort(handlersData.begin(), handlersData.end(), [] (auto& handler1, auto& handler2) { return handler1.startPos > handler2.startPos; });

			CatchScope* lastHandler = nullptr;

			for(const CatchScopeDataHolder& handlerData : handlersData)
				lastHandler = new CatchScope(environment, handlerData,
						lastHandler == nullptr ? environment.getCurrentScope()->end() : lastHandler->start(), lastHandler);

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
			AReturnOperation(const CodeEnvironment& environment): ReturnOperation(environment, AnyObjectType::getInstance()) {}
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

				inline string staticFieldToString(const CodeEnvironment& environment) const {
					return clazz == environment.classinfo.thisType && !environment.getCurrentScope()->hasVariable(descriptor.name) ?
							descriptor.name : clazz.toString(environment.classinfo) + '.' + descriptor.name;
				}

				inline string instanceFieldToString(const CodeEnvironment& environment, const Operation* object) const {
					return !(environment.modifiers & ACC_STATIC) && instanceof<const ALoadOperation*>(object) &&
							static_cast<const ALoadOperation*>(object)->index == 0 && !environment.getCurrentScope()->hasVariable(descriptor.name) ?
								descriptor.name : object->toString(environment) + '.' + descriptor.name;
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
				const Operation* const object;

			protected:
				InvokeNonStaticOperation(const CodeEnvironment& environment, const MethodDescriptor& descriptor):
						InvokeOperation(environment, descriptor), object(environment.stack.popAs(&descriptor.clazz)) {}

				InvokeNonStaticOperation(const CodeEnvironment& environment, uint16_t index):
						InvokeOperation(environment, index), object(environment.stack.popAs(&descriptor.clazz)) {}

				InvokeNonStaticOperation(const CodeEnvironment& environment, const Operation* object, uint16_t index):
						InvokeOperation(environment, index), object(object) {
					object->castReturnTypeTo(&descriptor.clazz);
				}

				inline string nonStaticMethodToString(const CodeEnvironment& environment) const {
					return (!(environment.modifiers & ACC_STATIC) && instanceof<const ALoadOperation*>(object) ?
							EMPTY_STRING : object->toString(environment, priority, Associativity::LEFT) + '.') + descriptor.name + '(' +
							rjoin<const Operation*>(arguments, [&environment] (const Operation* operation) { return operation->toString(environment); }) + ')';
				}

			public:
				virtual string toString(const CodeEnvironment& environment) const override {
					return nonStaticMethodToString(environment);
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
					return isConstructor && checkDup<DupOperation<TypeSize::FOUR_BYTES>>(environment, object) ?
							object->getReturnType() : InvokeNonStaticOperation::getReturnType();
				}

				inline bool getIsConstructor() {
					return descriptor.type == MethodDescriptor::MethodType::CONSTRUCTOR;
				}

				inline bool getIsSuperConstructor(const CodeEnvironment& environment) {
					return (!(environment.modifiers & ACC_STATIC) && // check that we invoking this (or super) constructor
							instanceof<const ALoadOperation*>(object) && static_cast<const ALoadOperation*>(object)->index == 0 &&
							descriptor.clazz == environment.classinfo.superType);
				}

			public:
				InvokespecialOperation(const CodeEnvironment& environment, uint16_t index):
						InvokeNonStaticOperation(environment, index),
						isConstructor(getIsConstructor()), isSuperConstructor(getIsSuperConstructor(environment)), returnType(getReturnType(environment)) {}

				InvokespecialOperation(const CodeEnvironment& environment, const Operation* object, uint16_t index):
						InvokeNonStaticOperation(environment, object, index),
						isConstructor(getIsConstructor()), isSuperConstructor(getIsSuperConstructor(environment)), returnType(getReturnType(environment)) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					if(isConstructor) {
						if(const NewOperation* newOperation = castOperationTo<const NewOperation*>(object)) {
							const ClassType& classType = newOperation->clazz;
							if(classType.isAnonymous) {
								const Class* clazz = JDecompiler::instance.getClass(classType.getEncodedName());
								if(clazz != nullptr) {
									clazz->classinfo.copyFormattingFrom(environment.classinfo);
									const string result = "new " + clazz->toString();
									clazz->classinfo.resetFormatting();

									return result;
								}
							}
						}

						return (isSuperConstructor ? "super" : object->toString(environment, priority, Associativity::LEFT)) +
							'(' + rjoin<const Operation*>(arguments,
								[&environment] (const Operation* operation) { return operation->toString(environment); }) + ')';
					}

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
				inline string staticMethodToString(const CodeEnvironment& environment) const {
					return descriptor.clazz == environment.classinfo.thisType ?
							descriptor.name : descriptor.clazz.toString(environment.classinfo) + '.' + descriptor.name;
				}
		};


		struct InvokeinterfaceOperation: InvokeNonStaticOperation {
			InvokeinterfaceOperation(const CodeEnvironment& environment, uint16_t index):
					InvokeNonStaticOperation(environment, index) {}
		};


		struct ConcatStringsOperation: InvokeOperation {
			const StringConstOperation* const pattern;

			struct StringOperand {
				bool isOperation;
				union {
					const Operation* operation;
					const string* stringConstant;
				} value;
			};

			vector<StringOperand> operands;

			ConcatStringsOperation(const CodeEnvironment& environment, const MethodDescriptor& concater):
					InvokeOperation(environment, concater), pattern(safe_cast<const StringConstOperation*>(environment.stack.popAs(STRING))) {
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
						lengths.push_back(environment.stack.popAs(INT));
				}

				NewArrayOperation(const CodeEnvironment& environment, const Type* memberType, uint16_t dimensions = 1):
						NewArrayOperation(environment, new ArrayType(memberType, dimensions)) {}


				virtual const Type* getReturnType() const override {
					return arrayType;
				}

				virtual string toString(const CodeEnvironment& environment) const override {
					if(initializer.empty()) {
						return "new " + arrayType->memberType->toString(environment.classinfo) + rjoin<const Operation*>(lengths,
								[&environment] (auto length) { return '[' + length->toString(environment) + ']'; }, EMPTY_STRING);
					}

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
					environment.stack.lookup(2)->getReturnTypeAs(AnyType::getArrayTypeInstance())->elementType, environment) {}
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

		struct InstanceofOperation: BooleanOperation {
			protected:
				const Type* const type;
				const Operation* const object;

			public:
				InstanceofOperation(const CodeEnvironment& environment, uint16_t index): BooleanOperation(9),
						type(parseReferenceType(*environment.constPool.get<ClassConstant>(index)->name)), object(environment.stack.pop()) {}

				virtual string toString(const CodeEnvironment& environment) const override {
					return object->toString(environment, priority, Associativity::LEFT) + " instanceof " + type->toString(environment.classinfo);
				}
		};
	}


	void StaticInitializerScope::add(const Operation* operation, const CodeEnvironment& environment) {
		using namespace operations;

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

#include "condition-operations.cpp"

#endif
