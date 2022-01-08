#ifndef JDECOMPILER_OPERATIONS_CPP
#define JDECOMPILER_OPERATIONS_CPP

namespace Operations {

	struct ReturnableOperation: Operation {
		protected: const Type* returnType;

		public:
			ReturnableOperation(const Type* returnType, uint16_t priority = 15): Operation(priority), returnType(returnType) {}

			virtual const Type* getReturnType() const override { return returnType; }
	};

	struct IntOperation: Operation {
		IntOperation(): Operation() {}
		IntOperation(uint16_t priority): Operation(priority) {}

		virtual const Type* getReturnType() const override { return INT; }
	};

	struct VoidOperation: Operation {
		VoidOperation(): Operation() {}
		VoidOperation(uint16_t priority): Operation(priority) {}

		virtual const Type* getReturnType() const override { return VOID; }
	};


	struct ConstOperation: ReturnableOperation {
		ConstOperation(const Type* returnType): ReturnableOperation(returnType) {}
	};

	template<typename T>
	struct NumberConstOperation: ConstOperation {
		protected:
			const T value;

		public:
			NumberConstOperation(const Type* returnType, T value): ConstOperation(returnType), value(value) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return primitiveToString(value);
			}
	};


	struct IConstOperation: NumberConstOperation<int32_t> {
		IConstOperation(int32_t value): NumberConstOperation(INT, value) {}
	};

	struct LConstOperation: NumberConstOperation<int64_t> {
		LConstOperation(int64_t value): NumberConstOperation(LONG, value) {}
	};

	struct FConstOperation: NumberConstOperation<float> {
		FConstOperation(float value): NumberConstOperation(FLOAT, value) {}
	};

	struct DConstOperation: NumberConstOperation<double> {
		DConstOperation(double value): NumberConstOperation(DOUBLE, value) {}
	};


	template<typename T>
	struct IPushOperation: IntOperation {
		protected:
			const T value;

		public:
			IPushOperation(T value): value(value) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return primitiveToString(value);
			}
	};

	struct BIPushOperation: IPushOperation<int8_t> {
		BIPushOperation(int8_t value): IPushOperation(value) {}
	};

	struct SIPushOperation: IPushOperation<int16_t> {
		SIPushOperation(int16_t value): IPushOperation(value) {}
	};


	struct AbstractLdcOperation: Operation {
		protected:
			uint16_t index;
			const Constant* const value;
			const ClassInfo& classinfo;

		public:
			AbstractLdcOperation(const ConstantPool* constPool, const ClassInfo& classinfo, uint16_t index): index(index), value(constPool->get<Constant>(index)), classinfo(classinfo) {}
	};

	struct LdcOperation: AbstractLdcOperation {
		public:
			LdcOperation(const ConstantPool* constPool, const ClassInfo& classinfo, uint16_t index): AbstractLdcOperation(constPool, classinfo, index) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				if(const StringConstant* s = dynamic_cast<const StringConstant*>(value)) return (string)s->toLiteral();
				if(const ClassConstant* c = dynamic_cast<const ClassConstant*>(value)) return parseReferenceType(*c->name)->toString(classinfo) + ".class";
				if(const IntegerConstant* i = dynamic_cast<const IntegerConstant*>(value)) return primitiveToString(i->value);
				if(const FloatConstant* f = dynamic_cast<const FloatConstant*>(value)) return primitiveToString(f->value);
				if(dynamic_cast<const MethodTypeConstant*>(value)) throw Exception("Unsupported constant type MethodType");
				if(dynamic_cast<const MethodHandleConstant*>(value)) throw Exception("Unsupported constant type MethodHadle");
				throw IllegalConstantPointerException("0x" + hex(index));
			}

			virtual const Type* getReturnType() const override {
				if(dynamic_cast<const StringConstant*>(value)) return STRING;
				if(dynamic_cast<const ClassConstant*>(value)) return CLASS;
				if(dynamic_cast<const IntegerConstant*>(value)) return INT;
				if(dynamic_cast<const FloatConstant*>(value)) return FLOAT;
				if(dynamic_cast<const MethodTypeConstant*>(value)) throw Exception("Unsupported constant type MethodType");
				if(dynamic_cast<const MethodHandleConstant*>(value)) throw Exception("Unsupported constant type MethodHadle");
				throw IllegalConstantPointerException("0x" + hex(index));
			}
	};

	struct Ldc2Operation: AbstractLdcOperation {
		public:
			Ldc2Operation(const ConstantPool* constPool, const ClassInfo& classinfo, uint16_t index): AbstractLdcOperation(constPool, classinfo, index) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				if(const LongConstant* l = dynamic_cast<const LongConstant*>(value)) return primitiveToString(l->value);
				if(const DoubleConstant* d = dynamic_cast<const DoubleConstant*>(value)) return primitiveToString(d->value);
				throw IllegalConstantPointerException("0x" + hex(index));
			}

			virtual const Type* getReturnType() const override {
				if(dynamic_cast<const LongConstant*>(value)) return LONG;
				if(dynamic_cast<const DoubleConstant*>(value)) return DOUBLE;
				throw IllegalConstantPointerException("0x" + hex(index));
			}
	};


	struct LoadOperation: ReturnableOperation {
		protected:
			const Variable* localVariable;

		public:
			LoadOperation(const Type* returnType, const CodeEnvironment& environment, uint16_t index): ReturnableOperation(returnType), localVariable(environment.getCurrentScope()->getVariable(index)) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				//cout << "A          " << localVariable << endl; // DEBUG
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


	struct ArrayLoadOperation: ReturnableOperation {
		protected:
			const Operation *index, *array;

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
		AALoadOperation(const CodeEnvironment& environment): ArrayLoadOperation(dynamic_cast<const ArrayType*>(environment.stack->lookup(1)->getReturnType())->memberType, environment) {} // TOCHECK
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
			const Operation* value;
			Variable* localVariable;

		public:
			StoreOperation(const CodeEnvironment& environment, uint16_t index): value(environment.stack->pop()), localVariable(environment.getCurrentScope()->getVariable(index)) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return (string)localVariable->name + " = " + value->toString(environment);
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
			const Operation *array, *index, *value;

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
		AAStoreOperation(const CodeEnvironment& environment): ArrayStoreOperation(dynamic_cast<const ArrayType*>(environment.stack->top()->getReturnType())->memberType, environment) {} // TOCHECK
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


	struct OperatorOperation: ReturnableOperation {
		protected:
			const string stringOperation;

		public: OperatorOperation(const Type* type, const string stringOperation, uint16_t priority): ReturnableOperation(type, priority), stringOperation(stringOperation) {}
	};


	struct BinaryOperatorOperation: OperatorOperation {
		protected: const Operation *const operand2, *const operand1;

		public:
			BinaryOperatorOperation(const Type* type, const CodeEnvironment& environment, const string stringOperation, uint16_t priority): OperatorOperation(type, stringOperation, priority), operand2(environment.stack->pop()), operand1(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return operand1->toString(environment, priority, LEFT) + ' ' + stringOperation + ' ' + operand2->toString(environment, priority, RIGHT);
			}
	};

	struct UnaryOperatorOperation: OperatorOperation {
		protected: const Operation* const operand;

		public:
			UnaryOperatorOperation(const Type* type, const CodeEnvironment& environment, const string stringOperation, uint16_t priority): OperatorOperation(type, stringOperation, priority), operand(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return stringOperation + operand->toString(environment, priority, RIGHT);
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


	struct CastOperation: ReturnableOperation {
		protected:
			const Operation* const operation;
			const Type* const type;
			const bool reqiured;

		public:
			CastOperation(const CodeEnvironment& environment, const Type* type, bool reqiured): ReturnableOperation(type), operation(environment.stack->pop()), type(type), reqiured(reqiured) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return reqiured ? "(" + type->name + ")" + operation->toString(environment, priority, LEFT) : operation->toString(environment);
			}
	};



	struct CmpOperation: ReturnableOperation {
		const Operation* const operand2, * const operand1;

		CmpOperation(const CodeEnvironment& environment): ReturnableOperation(BOOLEAN), operand2(environment.stack->pop()), operand1(environment.stack->pop()) {}
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


	struct CompareOperation: ReturnableOperation {
		const CompareType& compareType;

		CompareOperation(const CompareType& compareType): ReturnableOperation(BOOLEAN), compareType(compareType) {}
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

			virtual string getFrontSeparator(const ClassInfo& classinfo) const override {
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

			virtual string getBackSeparator(const ClassInfo& classinfo) const override {
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



	struct LookupswitchScope: Scope {
		protected:
			const Operation* const value;
			uint32_t defaultIndex;
			map<int32_t, uint32_t> indexTable;

		public:
			LookupswitchScope(const CodeEnvironment& environment, int32_t defaultOffset, map<int32_t, int32_t> offsetTable): Scope(environment.index, environment.bytecode.posToIndex(defaultOffset + environment.pos), environment.getCurrentScope()), value(environment.stack->pop()), defaultIndex(this->to) {
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
			GetstaticOperation(const CodeEnvironment& environment, uint16_t index): fieldref(environment.constPool->get<FieldrefConstant>(index)) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return ClassType(*fieldref->clazz->name).toString(environment.classinfo) + "." + fieldref->nameAndType->name->bytes;
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
			PutstaticOperation(const CodeEnvironment& environment, uint16_t index): fieldref(environment.constPool->get<FieldrefConstant>(index)), value(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return ClassType(fieldref->clazz->name->bytes).toString(environment.classinfo) + "." + fieldref->nameAndType->name->bytes + " = " + value->toString(environment);
			}
	};


	struct GetfieldOperation: Operation {
		protected:
			const FieldrefConstant* const fieldref;
			const Operation* const object;

		public:
			GetfieldOperation(const CodeEnvironment& environment, uint16_t index): fieldref(environment.constPool->get<FieldrefConstant>(index)), object(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				//cout << "G          " << fieldref->clazz->name->bytes << ' ' << typeid(*environment.classinfo).name() << endl; // DEBUG
				return object->toString(environment) + "." + fieldref->nameAndType->name->bytes;
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
			PutfieldOperation(const CodeEnvironment& environment, uint16_t index): fieldref(environment.constPool->get<FieldrefConstant>(index)), object(environment.stack->pop()), value(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return object->toString(environment) + "." + fieldref->nameAndType->name->bytes + " = " + value->toString(environment);
			}
	};


	struct InvokeNonStaticOperation: Operation {
		protected:
			const Operation* objectOperation;
			const MethodDescriptor* const methodDescriptor;
			vector<const Operation*> arguments;


			InvokeNonStaticOperation(const CodeEnvironment& environment, const MethodDescriptor* methodDescriptor): objectOperation(nullptr), methodDescriptor(methodDescriptor) {
				for(int i = methodDescriptor->arguments.size(); i > 0; i--)
					arguments.push_back(environment.stack->pop());

				objectOperation = environment.stack->pop();
			}

			InvokeNonStaticOperation(const CodeEnvironment& environment, uint16_t index): InvokeNonStaticOperation(environment, new MethodDescriptor(environment.constPool->get<MethodrefConstant>(index)->nameAndType)) {}

		public:
			virtual string toString(const CodeEnvironment& environment) const override {
				return objectOperation->toString(environment, priority, LEFT) + "." + methodDescriptor->name + "(" + rjoin<const Operation*>(arguments, [environment](const Operation* operation) { return operation->toString(environment); }) + ")";
			}

			virtual const Type* getReturnType() const override {
				return methodDescriptor->returnType;
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


	struct InvokestaticOperation: Operation {
		protected:
			const ClassType* const clazz;
			const MethodDescriptor* const methodDescriptor;
			vector<const Operation*> arguments;

		public:
			InvokestaticOperation(const CodeEnvironment& environment, uint16_t index): clazz(new ClassType(*environment.constPool->get<MethodrefConstant>(index)->clazz->name)), methodDescriptor(new MethodDescriptor(environment.constPool->get<MethodrefConstant>(index)->nameAndType)) {
				for(int i = methodDescriptor->arguments.size(); i > 0; i--)
					arguments.push_back(environment.stack->pop());
			}

			virtual string toString(const CodeEnvironment& environment) const override {
				//cout << arguments.size() << endl; // DEBUG
				return clazz->toString(environment.classinfo) + "." + methodDescriptor->name + "(" + rjoin<const Operation*>(arguments, [environment](const Operation* operation) { return operation->toString(environment); }) + ")";
			}

			virtual const Type* getReturnType() const override {
				return methodDescriptor->returnType;
			}
	};


	struct InvokeinterfaceOperation: InvokeNonStaticOperation {
		InvokeinterfaceOperation(const CodeEnvironment& environment, uint16_t index): InvokeNonStaticOperation(environment, new MethodDescriptor(environment.constPool->get<InterfaceMethodrefConstant>(index)->nameAndType)) {}
	};



	struct NewOperation: Operation {
		protected: const ClassType* const classType;

		public:
			NewOperation(const CodeEnvironment& environment, uint16_t classIndex): classType(new ClassType(*environment.constPool->get<ClassConstant>(classIndex)->name)) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return "new " + classType->toString(environment.classinfo);
			}

			virtual const Type* getReturnType() const override {
				return classType;
			}
	};


	struct NewArrayOperation: ReturnableOperation {
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
			ANewArrayOperation(const CodeEnvironment& environment, uint16_t index): NewArrayOperation(environment, new ClassType(*environment.constPool->get<ClassConstant>(index)->name)) {}
	};


	struct ArrayLengthOperation: ReturnableOperation {
		protected: const Operation* array;

		public:
			ArrayLengthOperation(const CodeEnvironment& environment): ReturnableOperation(INT), array(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return array->toString(environment) + ".length";
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


	struct CheckCastOperation: ReturnableOperation {
		protected: const Operation* const object;

		public:
			CheckCastOperation(const CodeEnvironment& environment, uint16_t index): ReturnableOperation(parseReferenceType(*environment.constPool->get<ClassConstant>(index)->name), 13), object(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return "(" + returnType->toString(environment.classinfo) + ")" + object->toString(environment, priority, RIGHT);
			}
	};

	struct InstanceofOperation: ReturnableOperation {
		protected:
			const Type* const type;
			const Operation* const object;

		public:
			InstanceofOperation(const CodeEnvironment& environment, uint16_t index): ReturnableOperation(BOOLEAN, 9), type(parseType(*environment.constPool->get<ClassConstant>(index)->name)), object(environment.stack->pop()) {}

			virtual string toString(const CodeEnvironment& environment) const override {
				return object->toString(environment, priority, LEFT) + " instanceof " + type->toString(environment.classinfo);
			}
	};


	struct MultiANewArrayOperation: ReturnableOperation {
		protected:
			vector<const Operation*> indexes;

		public:
			MultiANewArrayOperation(const CodeEnvironment& environment, uint16_t index, uint16_t dimensions): ReturnableOperation(new ArrayType(*environment.constPool->get<ClassConstant>(index)->name, dimensions)) {
				indexes.reserve(dimensions);
				for(uint16_t i = 0; i < dimensions; i++)
					indexes.push_back(environment.stack->pop());
			}

			virtual string toString(const CodeEnvironment& environment) const override {
				return "new " + ((ArrayType*)returnType)->memberType->toString(environment.classinfo) + rjoin<const Operation*>(indexes, [environment](const Operation* index) { return "[" + index->toString(environment) + "]"; }, EMPTY_STRING);
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
		protected: const Operation* const operand;

		public: IfNullScope(const CodeEnvironment& environment, const int16_t offset): IfScope(environment, offset, new CompareWithNullOperation(environment, CompareType::EQUALS)), operand(environment.stack->pop()) {}
	};

	struct IfNonNullScope: IfScope {
		protected: const Operation* const operand;

		public: IfNonNullScope(const CodeEnvironment& environment, const int16_t offset): IfScope(environment, offset, new CompareWithNullOperation(environment, CompareType::NOT_EQUALS)), operand(environment.stack->pop()) {}
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
