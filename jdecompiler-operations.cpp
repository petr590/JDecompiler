#ifndef JDECOMPILER_OPERATIONS_CPP
#define JDECOMPILER_OPERATIONS_CPP

namespace Operations {

	template<class T = Type>
	struct ReturnableOperation: Operation { // ReturnableOperation is an operation which returns specified type
		static_assert(is_base_of<Type, T>::value, "T is not subclass of class Type");

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
	struct ConstOperation: ReturnableOperation<> {
		protected:
			const T value;

		public:
			ConstOperation(const Type* returnType, T value): ReturnableOperation(returnType), value(value) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return primitiveToString(value);
			}
	};


	struct IConstOperation: ConstOperation<int32_t> {
		IConstOperation(int32_t value): ConstOperation(INT, value) {}
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
		static_assert(is_same<T, int8_t>::value || is_same<T, int16_t>::value, "T is not uint8_t or uint16_t type");

		protected:
			const T value;

		public:
			IPushOperation(T value): value(value) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return primitiveToString(value);
			}
	};


	struct AbstractLdcOperation: Operation {
		#define THROW_ILLEGAL_CONSTANT_TYPE_EXCEPTION throw IllegalStateException("Illegal constant type " + (int)type)

		protected:
			uint16_t index;
			const Constant* const value;

		public:
			AbstractLdcOperation(const CodeEnvironment& environment, uint16_t index): index(index), value(environment.constPool.get<Constant>(index)) {}
	};

	struct LdcOperation: AbstractLdcOperation {
		private:
			enum class ConstantType {
				STRING, CLASS, INTEGER, FLOAT, METHOD_TYPE, METHOD_HANDLE
			};

			const ConstantType type;

			ConstantType getConstantType() {
				if(safe_cast<const StringConstant*>(value)) return ConstantType::STRING;
				if(safe_cast<const ClassConstant*>(value)) return ConstantType::CLASS;
				if(safe_cast<const IntegerConstant*>(value)) return ConstantType::INTEGER;
				if(safe_cast<const FloatConstant*>(value)) return ConstantType::FLOAT;
				if(safe_cast<const MethodTypeConstant*>(value)) return ConstantType::METHOD_TYPE;
				if(safe_cast<const MethodHandleConstant*>(value)) return ConstantType::METHOD_HANDLE;
				throw IllegalStateException("Illegal constant pointer 0x" + hex(index) +
						": expected String, Class, Integer, Float, MethodType or MethodHandle constant");
			}

		public:
			LdcOperation(const CodeEnvironment& environment, uint16_t index): AbstractLdcOperation(environment, index), type(getConstantType()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				switch(type) {
					case ConstantType::STRING: return safe_cast<const StringConstant*>(value)->toLiteral();
					case ConstantType::CLASS: return parseReferenceType(*safe_cast<const ClassConstant*>(value)->name)->toString(environment.classinfo) + ".class";
					case ConstantType::INTEGER: return primitiveToString(safe_cast<const IntegerConstant*>(value)->value);
					case ConstantType::FLOAT: return primitiveToString(safe_cast<const FloatConstant*>(value)->value);
					case ConstantType::METHOD_TYPE: throw Exception("MethodType: Unsupported toString operation");
					case ConstantType::METHOD_HANDLE: throw Exception("MethodHadle: Unsupported toString operation");
					default: THROW_ILLEGAL_CONSTANT_TYPE_EXCEPTION;
				}
			}

			virtual const Type* getReturnType() const override {
				switch(type) {
					case ConstantType::STRING: return STRING;
					case ConstantType::CLASS: return CLASS;
					case ConstantType::INTEGER: return INT;
					case ConstantType::FLOAT: return FLOAT;
					case ConstantType::METHOD_TYPE: throw Exception("MethodHadle: Unsupported getReturnType operation");
					case ConstantType::METHOD_HANDLE: throw Exception("MethodHadle: Unsupported getReturnType operation");
					default: THROW_ILLEGAL_CONSTANT_TYPE_EXCEPTION;
				}
			}
	};

	struct Ldc2Operation: AbstractLdcOperation {
		private:
			enum class ConstantType {
				LONG, DOUBLE
			};

			ConstantType getConstantType() {
				if(safe_cast<const LongConstant*>(value)) return ConstantType::LONG;
				if(safe_cast<const DoubleConstant*>(value)) return ConstantType::DOUBLE;
				throw IllegalStateException("Illegal constant pointer 0x" + hex(index) + ": expected Long or Double constant");
			}

			const ConstantType type;

		public:
			Ldc2Operation(const CodeEnvironment& environment, uint16_t index): AbstractLdcOperation(environment, index), type(getConstantType()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				switch(type) {
					case ConstantType::LONG: return primitiveToString(safe_cast<const LongConstant*>(value)->value);
					case ConstantType::DOUBLE: return primitiveToString(safe_cast<const DoubleConstant*>(value)->value);
					default: THROW_ILLEGAL_CONSTANT_TYPE_EXCEPTION;
				}
			}

			virtual const Type* getReturnType() const override {
				switch(type) {
					case ConstantType::LONG: return LONG;
					case ConstantType::DOUBLE: return DOUBLE;
					default: THROW_ILLEGAL_CONSTANT_TYPE_EXCEPTION;
				}
			}
	};


	struct LoadOperation: ReturnableOperation<> {
		protected:
			const Variable* const localVariable;

		public:
			LoadOperation(const Type* returnType, const CodeEnvironment& environment, uint16_t index): ReturnableOperation(returnType), localVariable(environment.getCurrentScope()->getVariable(index)) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return localVariable->name;
			}
	};

	struct ILoadOperation: LoadOperation {
		ILoadOperation(const CodeEnvironment& environment, uint16_t index): LoadOperation(INT, environment, index) {}
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
		ALoadOperation(const CodeEnvironment& environment, uint16_t index): LoadOperation(environment.getCurrentScope()->getVariable(index)->type, environment, index) {}
	};


	struct ArrayLoadOperation: ReturnableOperation<> {
		protected:
			const Operation * const index, * const array;

		public:
			ArrayLoadOperation(const Type* returnType, const CodeEnvironment& environment): ReturnableOperation(returnType), index(environment.stack->pop()), array(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return array->toString(environment) + "[" + index->toString(environment) + "]";
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
		AALoadOperation(const CodeEnvironment& environment): ArrayLoadOperation(safe_cast<const ArrayType*>(environment.stack->lookup(1)->getReturnType())->memberType, environment) {} // TOCHECK
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
		protected:
			const Operation* const value;
			const Variable* const localVariable;

		public:
			StoreOperation(const CodeEnvironment& environment, uint16_t index): value(environment.stack->pop()), localVariable(environment.getCurrentScope()->getVariable(index)) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return localVariable->name + " = " + value->toString(environment);
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


	struct ArrayStoreOperation: VoidOperation {
		protected:
			const Operation * const array, * const index, * const value;

		public:
			ArrayStoreOperation(const Type* returnType, const CodeEnvironment& environment): array(environment.stack->pop()), index(environment.stack->pop()), value(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return array->toString(environment) + "[" + index->toString(environment) + "] = " + value->toString(environment);
			}
	};

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
		AAStoreOperation(const CodeEnvironment& environment): ArrayStoreOperation(safe_cast<const ArrayType*>(environment.stack->top()->getReturnType())->memberType, environment) {} // TOCHECK
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


	struct PopOperation: VoidOperation {
		protected: const Operation* const operation;

		public:
			PopOperation(const CodeEnvironment& environment): operation(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return operation->toString(environment);
			}
	};


	struct DupOperation: Operation {
		protected: const Operation* const operation;

		public:
			DupOperation(const CodeEnvironment& environment): operation(environment.stack->top()) {}

			virtual string toString(const CodeEnvironment& environment) const override { return operation->toString(environment); }

			virtual const Type* getReturnType() const override { return operation->getReturnType(); }
	};


	struct SwapOperation: VoidOperation {
		SwapOperation(const CodeEnvironment& environment) {
			environment.stack->push({environment.stack->pop(), environment.stack->pop()});
		}

		virtual string toString(const CodeEnvironment& environment) const override { return EMPTY_STRING; }
	};


	struct OperatorOperation: ReturnableOperation<> {
		protected:
			const string operation;

		public: OperatorOperation(const Type* type, char32_t operation, uint16_t priority): ReturnableOperation(type, priority), operation(char32ToString(operation)) {}
	};


	struct BinaryOperatorOperation: OperatorOperation {
		protected: const Operation *const operand2, *const operand1;

		public:
			BinaryOperatorOperation(const Type* type, const CodeEnvironment& environment, char32_t operation, uint16_t priority): OperatorOperation(type, operation, priority), operand2(environment.stack->pop()), operand1(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return operand1->toString(environment, priority, LEFT) + ' ' + operation + ' ' + operand2->toString(environment, priority, RIGHT);
			}
	};

	struct UnaryOperatorOperation: OperatorOperation {
		protected: const Operation* const operand;

		public:
			UnaryOperatorOperation(const Type* type, const CodeEnvironment& environment, char32_t operation, uint16_t priority): OperatorOperation(type, operation, priority), operand(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return operation + operand->toString(environment, priority, RIGHT);
			}
	};



	struct IIncOperation: Operation {
		protected:
			const Variable* const variable;
			const int16_t value;
			const Type* returnType;
			bool isShortInc, isPostInc = false;

		public:
			IIncOperation(const CodeEnvironment& environment, uint16_t index, int16_t value): variable(environment.getCurrentScope()->getVariable(index)), value(value), isShortInc(value == 1 || value == -1) /* isShortInc true when we can write ++ or -- */ {
				if(isShortInc && !environment.stack->empty() && dynamic_cast<const ILoadOperation*>(environment.stack->top())) {
					environment.stack->pop();
					returnType = INT;
					isPostInc = true;
				}else
					returnType = VOID;
			}

			virtual string toString(const CodeEnvironment& environment) const override {
				if(isShortInc) {
					const char* inc = value == 1 ? "++" : "--";
					return isPostInc || returnType == VOID ? variable->name + inc : inc + variable->name;
				}
				return variable->name + (string)(value < 0 ? " -" : " +") + "= " + to_string(abs(value));
			}

			virtual const Type* getReturnType() const override {
				return returnType;
			}
	};


	template<bool required>
	struct CastOperation: ReturnableOperation<> {
		protected:
			const Operation* const operation;
			const Type* const type;

		public:
			CastOperation(const CodeEnvironment& environment, const Type* type): ReturnableOperation(type), operation(environment.stack->pop()), type(type) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return required ? "(" + type->name + ")" + operation->toString(environment, priority, LEFT) : operation->toString(environment);
			}
	};



	struct CmpOperation: BooleanOperation {
		const Operation* const operand2, * const operand1;

		CmpOperation(const CodeEnvironment& environment): operand2(environment.stack->pop()), operand1(environment.stack->pop()) {}
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


	class CompareType;

	class EqualsCompareType;

	class CompareType {
		public:
			static const EqualsCompareType EQUALS, NOT_EQUALS;
			static const CompareType GREATER, GREATER_OR_EQUALS, LESS, LESS_OR_EQUALS;

			const char* const stringOperator;

			CompareType(const char* const stringOperator): stringOperator(stringOperator) {}
	};

	class EqualsCompareType: public CompareType {
		public: EqualsCompareType(const char* const stringOperator): CompareType(stringOperator) {}
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

		CompareBinaryOperation(const CmpOperation* cmpOperation, const CompareType& compareType): CompareOperation(compareType), operand2(cmpOperation->operand2), operand1(cmpOperation->operand1) {}

		virtual string toString(const CodeEnvironment& environment) const override {
			return operand1->toString(environment) + " " + compareType.stringOperator + " " + operand2->toString(environment);
		}
	};


	struct CompareWithZeroOperation: CompareOperation {
		const Operation* const operand;

		CompareWithZeroOperation(const Operation* operand, const CompareType& compareType): CompareOperation(compareType), operand(operand) {}

		virtual string toString(const CodeEnvironment& environment) const override {
			return operand->getReturnType() == BOOLEAN ? operand->toString(environment) : operand->toString(environment) + " " + compareType.stringOperator + " 0";
		}
	};


	struct IfScope;


	struct ElseScope: Scope {
		protected: const IfScope* const ifScope;

		public:
			ElseScope(const CodeEnvironment& environment, const uint32_t to, const IfScope* ifScope);

			virtual string getHeader(const CodeEnvironment& environment) const override {
				return " else ";
			}

			virtual inline string getFrontSeparator(const ClassInfo& classinfo) const override {
				return EMPTY_STRING;
			}
	};


	struct IfScope: Scope {
		protected: const CompareOperation* const condition;

		private: mutable const ElseScope* elseScope = nullptr;

		friend ElseScope::ElseScope(const CodeEnvironment& environment, const uint32_t to, const IfScope* ifScope);

		public:
			IfScope(const CodeEnvironment& environment, const int16_t offset, const CompareOperation* condition): Scope(environment.index, environment.bytecode.posToIndex(offset + environment.pos) - 1, environment.getCurrentScope()), condition(condition) {}

			virtual string getHeader(const CodeEnvironment& environment) const override {
				return "if(" + condition->toString(environment) + ") ";
			}

			virtual inline string getBackSeparator(const ClassInfo& classinfo) const override {
				return elseScope == nullptr ? this->Scope::getBackSeparator(classinfo) : EMPTY_STRING;
			}
	};

	struct IfCmpScope: IfScope {
		protected: const CompareType& compareType;

		public: IfCmpScope(const CodeEnvironment& environment, const int16_t offset, const CompareType& compareType): IfScope(environment, offset, getCondition(environment, compareType)), compareType(compareType) {}

		private: static const CompareOperation* getCondition(const CodeEnvironment& environment, const CompareType& compareType) {
			const Operation* operation = environment.stack->pop();
			if(const CmpOperation* cmpOperation = dynamic_cast<const CmpOperation*>(operation))
				return new CompareBinaryOperation(cmpOperation, compareType);
			return new CompareWithZeroOperation(operation, compareType);
		}
	};


	ElseScope::ElseScope(const CodeEnvironment& environment, const uint32_t to, const IfScope* ifScope): Scope(environment.index, to, ifScope->parentScope), ifScope(ifScope) {
		ifScope->elseScope = this;
	}



	struct EmptyInfiniteLoopScope: Scope {
		EmptyInfiniteLoopScope(const CodeEnvironment& environment): Scope(environment.index, environment.index, environment.getCurrentScope()) {}

		virtual string toString(const CodeEnvironment& environment) const override { return "while(true);"; }
	};



	struct SwitchScope: Scope {
		protected:
			const Operation* const value;
			uint32_t defaultIndex;
			map<int32_t, uint32_t> indexTable;

		public:
			SwitchScope(const CodeEnvironment& environment, int32_t defaultOffset, map<int32_t, int32_t> offsetTable): Scope(environment.index, environment.bytecode.posToIndex(defaultOffset + environment.pos), environment.getCurrentScope()), value(environment.stack->pop()), defaultIndex(this->to) {
				for(auto& entry : offsetTable)
					indexTable[entry.first] = environment.bytecode.posToIndex(environment.pos + entry.second);
			}

			virtual string toString(const CodeEnvironment& environment) const override {
				environment.classinfo.increaseIndent(2);

				string str = "switch(" + value->toString(environment) + ") {\n";
				const size_t baseSize = str.size();

				uint32_t i = this->from;
				for(const Operation* operation : code) {
					i++;
					if(i == defaultIndex) {
						environment.classinfo.reduceIndent();
						str += environment.classinfo.getIndent() + (string)"default:\n";
						environment.classinfo.increaseIndent();
					}else {
						//cout << "----------" << endl; // DEBUG
						for(auto& entry : indexTable) {
							//cout << i << ' ' << entry.second << endl; // DEBUG
							if(i == entry.second) {
								environment.classinfo.reduceIndent();
								str += environment.classinfo.getIndent() + (string)"case " + to_string(entry.first) + ":\n";
								environment.classinfo.increaseIndent();
								break;
							}
						}
					}
					str += environment.classinfo.getIndent() + operation->toString(environment) + (dynamic_cast<const Scope*>(operation) ? "\n" : ";\n");
				}

				environment.classinfo.reduceIndent(2);

				if(str.size() == baseSize) {
					str[baseSize - 1] = '}';
					return str;
				}

				return str + environment.classinfo.getIndent() + "}";
			}
	};



	struct ReturnOperation: VoidOperation {
		protected: const Operation* const value;

		public:
			ReturnOperation(const CodeEnvironment& environment): value(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return "return " + value->toString(environment);
			}
	};



	struct GetstaticOperation: Operation {
		protected:
			const FieldrefConstant* const fieldref;

		public:
			GetstaticOperation(const CodeEnvironment& environment, uint16_t index): fieldref(environment.constPool.get<FieldrefConstant>(index)) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return ClassType(*fieldref->clazz->name).toString(environment.classinfo) + "." + (string)*fieldref->nameAndType->name;
			}

			virtual const Type* getReturnType() const override {
				return parseType(*fieldref->nameAndType->descriptor);
			}
	};


	struct PutstaticOperation: VoidOperation {
		protected:
			const FieldrefConstant* const fieldref;
			const Operation* const value;

		public:
			PutstaticOperation(const CodeEnvironment& environment, uint16_t index): fieldref(environment.constPool.get<FieldrefConstant>(index)), value(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return ClassType(*fieldref->clazz->name).toString(environment.classinfo) + "." + (string)*fieldref->nameAndType->name + " = " + value->toString(environment);
			}
	};


	struct GetfieldOperation: Operation {
		protected:
			const FieldrefConstant* const fieldref;
			const Operation* const object;

		public:
			GetfieldOperation(const CodeEnvironment& environment, uint16_t index): fieldref(environment.constPool.get<FieldrefConstant>(index)), object(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return object->toString(environment) + "." + (string)*fieldref->nameAndType->name;
			}

			virtual const Type* getReturnType() const override {
				return parseType(*fieldref->nameAndType->descriptor);
			}
	};


	struct PutfieldOperation: VoidOperation {
		protected:
			const FieldrefConstant* const fieldref;
			const Operation* const object, * const value;

		public:
			PutfieldOperation(const CodeEnvironment& environment, uint16_t index): fieldref(environment.constPool.get<FieldrefConstant>(index)), object(environment.stack->pop()), value(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return object->toString(environment) + "." + (string)*fieldref->nameAndType->name + " = " + value->toString(environment);
			}
	};


	struct InvokeOperation: Operation {
		protected:
			const MethodDescriptor* const methodDescriptor;
			vector<const Operation*> arguments;

			InvokeOperation(const CodeEnvironment& environment, const MethodDescriptor* methodDescriptor): methodDescriptor(methodDescriptor) {
				for(int i = methodDescriptor->arguments.size(); i > 0; i--)
					arguments.push_back(environment.stack->pop());
			}

			virtual const Type* getReturnType() const override {
				return methodDescriptor->returnType;
			}
	};


	struct InvokeNonStaticOperation: InvokeOperation {
		protected:
			const Operation* const objectOperation;

			InvokeNonStaticOperation(const CodeEnvironment& environment, const MethodDescriptor* methodDescriptor):
				InvokeOperation(environment, methodDescriptor), objectOperation(environment.stack->pop()) {}

			InvokeNonStaticOperation(const CodeEnvironment& environment, uint16_t index): InvokeNonStaticOperation(environment, new MethodDescriptor(environment.constPool.get<MethodrefConstant>(index)->nameAndType)) {}

		public:
			virtual string toString(const CodeEnvironment& environment) const override {
				return objectOperation->toString(environment, priority, LEFT) + "." + methodDescriptor->name + "(" + rjoin<const Operation*>(arguments, [environment](const Operation* operation) { return operation->toString(environment); }) + ")";
			}
	};


	struct InvokevirtualOperation: InvokeNonStaticOperation {
		InvokevirtualOperation(const CodeEnvironment& environment, uint16_t index): InvokeNonStaticOperation(environment, index) {}
	};


	struct InvokespecialOperation: InvokeNonStaticOperation {
		protected: const bool isConstructor;

		public:
			InvokespecialOperation(const CodeEnvironment& environment, uint16_t index): InvokeNonStaticOperation(environment, index), isConstructor(methodDescriptor->name == "<init>") {
				if(isConstructor && dynamic_cast<const DupOperation*>(objectOperation))
					environment.stack->pop();
			}

			virtual string toString(const CodeEnvironment& environment) const override {
				if(isConstructor)
					return objectOperation->toString(environment, priority, LEFT) + "(" + rjoin<const Operation*>(arguments, [environment](const Operation* operation) { return operation->toString(environment); }) + ")";
				return InvokeNonStaticOperation::toString(environment);
			}

			virtual const Type* getReturnType() const override {
				if(isConstructor)
					return objectOperation->getReturnType();
				return InvokeNonStaticOperation::getReturnType();
			}
	};


	struct InvokestaticOperation: InvokeOperation {
		protected:
			const ClassType* const clazz;

		public:
			InvokestaticOperation(const CodeEnvironment& environment, uint16_t index):
				clazz(new ClassType(*environment.constPool.get<MethodrefConstant>(index)->clazz->name)), InvokeOperation(environment, new MethodDescriptor(environment.constPool.get<MethodrefConstant>(index)->nameAndType)) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return clazz->toString(environment.classinfo) + "." + methodDescriptor->name + "(" +
						rjoin<const Operation*>(arguments, [environment](const Operation* operation) { return operation->toString(environment); }) + ")";
			}
	};


	struct InvokeinterfaceOperation: InvokeNonStaticOperation {
		InvokeinterfaceOperation(const CodeEnvironment& environment, uint16_t index): InvokeNonStaticOperation(environment, new MethodDescriptor(environment.constPool.get<InterfaceMethodrefConstant>(index)->nameAndType)) {}
	};



	struct NewOperation: Operation {
		protected: const ClassType* const classType;

		public:
			NewOperation(const CodeEnvironment& environment, uint16_t classIndex): classType(new ClassType(*environment.constPool.get<ClassConstant>(classIndex)->name)) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return "new " + classType->toString(environment.classinfo);
			}

			virtual const Type* getReturnType() const override {
				return classType;
			}
	};


	struct NewArrayOperation: ReturnableOperation<> {
		protected:
			const Type* const memberType;
			const Operation* const index;

		public:
			NewArrayOperation(const CodeEnvironment& environment, const Type* memberType): ReturnableOperation(new ArrayType(memberType)), memberType(memberType), index(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return "new " + memberType->toString(environment.classinfo) + "[" + index->toString(environment) + "]";
			}
	};

	struct ANewArrayOperation: NewArrayOperation {
			ANewArrayOperation(const CodeEnvironment& environment, uint16_t index): NewArrayOperation(environment, new ClassType(*environment.constPool.get<ClassConstant>(index)->name)) {}
	};


	struct ArrayLengthOperation: IntOperation {
		protected: const Operation* const array;

		public:
			ArrayLengthOperation(const CodeEnvironment& environment): array(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return array->toString(environment, priority, LEFT) + ".length";
			}
	};


	struct AThrowOperation: VoidOperation {
		protected: const Operation* const exceptionOperation;

		public:
			AThrowOperation(const CodeEnvironment& environment): exceptionOperation(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return "throw " + exceptionOperation->toString(environment);
			}
	};


	struct CheckCastOperation: ReturnableOperation<> {
		protected: const Operation* const object;

		public:
			CheckCastOperation(const CodeEnvironment& environment, uint16_t index): ReturnableOperation(parseReferenceType(*environment.constPool.get<ClassConstant>(index)->name), 13), object(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return "(" + returnType->toString(environment.classinfo) + ")" + object->toString(environment, priority, RIGHT);
			}
	};

	struct InstanceofOperation: ReturnableOperation<> {
		protected:
			const Type* const type;
			const Operation* const object;

		public:
			InstanceofOperation(const CodeEnvironment& environment, uint16_t index): ReturnableOperation(BOOLEAN, 9), type(parseType(*environment.constPool.get<ClassConstant>(index)->name)), object(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return object->toString(environment, priority, LEFT) + " instanceof " + type->toString(environment.classinfo);
			}
	};


	struct MultiANewArrayOperation: ReturnableOperation<ArrayType> {
		protected:
			vector<const Operation*> indexes;

		public:
			MultiANewArrayOperation(const CodeEnvironment& environment, uint16_t index, uint16_t dimensions): ReturnableOperation(new ArrayType(*environment.constPool.get<ClassConstant>(index)->name, dimensions)) {
				indexes.reserve(dimensions);
				for(uint16_t i = 0; i < dimensions; i++)
					indexes.push_back(environment.stack->pop());
			}

			virtual string toString(const CodeEnvironment& environment) const override {
				return "new " + returnType->memberType->toString(environment.classinfo) +
						rjoin<const Operation*>(indexes, [environment](const Operation* index) { return "[" + index->toString(environment) + "]"; }, EMPTY_STRING);
			}
	};


	struct CompareWithNullOperation: CompareOperation {
		protected:
			const Operation* const operand;

		public:
			CompareWithNullOperation(const CodeEnvironment& environment, const EqualsCompareType& compareType): CompareOperation(compareType), operand(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const {
				return operand->toString(environment) + " " + compareType.stringOperator + " null";
			}
	};


	struct IfNullScope: IfScope {
		public: IfNullScope(const CodeEnvironment& environment, int16_t offset): IfScope(environment, offset, new CompareWithNullOperation(environment, CompareType::EQUALS)) {}
	};

	struct IfNonNullScope: IfScope {
		public: IfNonNullScope(const CodeEnvironment& environment, int16_t offset): IfScope(environment, offset, new CompareWithNullOperation(environment, CompareType::NOT_EQUALS)) {}
	};



	struct InvokedynamicOperation: InvokeOperation {
		InvokedynamicOperation(const CodeEnvironment& environment, uint16_t index): InvokeOperation(environment, new MethodDescriptor(environment.constPool.get<InvokeDynamicConstant>(index)->nameAndType)) {}

			virtual string toString(const CodeEnvironment& environment) const {
				return "INVOKEDYNAMIC";
			}
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

#endif
