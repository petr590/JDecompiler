#ifndef JDECOMPILER_INSTRUCTIONS_CPP
#define JDECOMPILER_INSTRUCTIONS_CPP

#include "operations.cpp"

namespace jdecompiler {
	namespace instructions {

		using namespace operations;


		struct InstructionWithIndex: Instruction {
			public:
				const uint16_t index;

			protected:
				InstructionWithIndex(uint16_t index): index(index) {}
		};


		struct InstructionAndOperation: Instruction, Operation {
			public: virtual const Operation* toOperation(const DecompilationContext& context) const override { return this; }
		};

		struct VoidInstructionAndOperation: InstructionAndOperation {
			public: virtual const Type* getReturnType() const override { return VOID; }
		};


		struct AConstNull final: InstructionAndOperation {
			private: AConstNull() {}

			public:
				virtual string toString(const StringifyContext& context) const override {
					return "null";
				}

				virtual const Type* getReturnType() const override {
					return AnyType::getInstance();
				}

				static inline AConstNull* getInstance() {
					static AConstNull instance;
					return &instance;
				}
		};

		template<typename T>
		struct NumberConstInstruction: Instruction {
			static_assert(is_fundamental<T>(), "Type T of struct NumberConstInstruction must be primitive");

			const T value;

			NumberConstInstruction(const T value): value(value) {}
		};


		struct IConstInstruction: NumberConstInstruction<int32_t> {
			IConstInstruction(int32_t value): NumberConstInstruction(value) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return IConstOperation::valueOf(value, context.classinfo);
			}
		};

		struct LConstInstruction: NumberConstInstruction<int64_t> {
			LConstInstruction(int64_t value): NumberConstInstruction(value) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return LConstOperation::valueOf(value, context.classinfo);
			}
		};

		struct FConstInstruction: NumberConstInstruction<float> {
			FConstInstruction(float value): NumberConstInstruction(value) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return FConstOperation::valueOf(value, context.classinfo);
			}
		};

		struct DConstInstruction: NumberConstInstruction<double> {
			DConstInstruction(double value): NumberConstInstruction(value) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return DConstOperation::valueOf(value, context.classinfo);
			}
		};


		template<typename T>
		struct IPushInstruction: Instruction {
			public:
				const T value;

				IPushInstruction(T value): value(value) {}

				virtual const Operation* toOperation(const DecompilationContext& context) const override {
					return IConstOperation::valueOf(value, context.classinfo);
				}
		};

		using BIPushInstruction = IPushInstruction<int8_t>;

		using SIPushInstruction = IPushInstruction<int16_t>;



		template<TypeSize size>
		struct LdcInstruction: InstructionWithIndex {
			public:
				LdcInstruction(const DisassemblerContext& context, uint16_t index): InstructionWithIndex(index) {}

				virtual const Operation* toOperation(const DecompilationContext& context) const override {
					return context.constPool.get<ConstValueConstant>(index)->toOperation(context.classinfo);
				}
		};


		struct LoadInstruction: InstructionWithIndex {
			LoadInstruction(uint16_t index): InstructionWithIndex(index) {}
		};

		struct ILoadInstruction: LoadInstruction {
			ILoadInstruction(uint16_t index): LoadInstruction(index) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return isNullOperation ? nullptr : new ILoadOperation(context, index);
			}

			private:
				friend operations::IIncOperation::IIncOperation(const DecompilationContext&, uint16_t, int16_t);

				mutable bool isNullOperation = false;

				inline void setNullOperation() const noexcept {
					isNullOperation = true;
				}
		};

		struct LLoadInstruction: LoadInstruction {
			LLoadInstruction(uint16_t index): LoadInstruction(index) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new LLoadOperation(context, index); }
		};

		struct FLoadInstruction: LoadInstruction {
			FLoadInstruction(uint16_t index): LoadInstruction(index) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new FLoadOperation(context, index); }
		};

		struct DLoadInstruction: LoadInstruction {
			DLoadInstruction(uint16_t index): LoadInstruction(index) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new DLoadOperation(context, index); }
		};

		struct ALoadInstruction: LoadInstruction {
			ALoadInstruction(uint16_t index): LoadInstruction(index) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new ALoadOperation(context, index); }
		};


		struct ArrayLoadInstruction: Instruction {};

		struct IALoadInstruction: ArrayLoadInstruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new IALoadOperation(context); }
		};

		struct LALoadInstruction: ArrayLoadInstruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new LALoadOperation(context); }
		};

		struct FALoadInstruction: ArrayLoadInstruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new FALoadOperation(context); }
		};

		struct DALoadInstruction: ArrayLoadInstruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new DALoadOperation(context); }
		};

		struct AALoadInstruction: ArrayLoadInstruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new AALoadOperation(context); }
		};

		struct BALoadInstruction: ArrayLoadInstruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new BALoadOperation(context); }
		};

		struct CALoadInstruction: ArrayLoadInstruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new CALoadOperation(context); }
		};

		struct SALoadInstruction: ArrayLoadInstruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new SALoadOperation(context); }
		};


		struct StoreInstruction: InstructionWithIndex {
			public: StoreInstruction(uint16_t index): InstructionWithIndex(index) {}
		};

		struct IStoreInstruction: StoreInstruction {
			IStoreInstruction(uint16_t index): StoreInstruction(index) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new IStoreOperation(context, index); }
		};

		struct LStoreInstruction: StoreInstruction {
			LStoreInstruction(uint16_t index): StoreInstruction(index) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new LStoreOperation(context, index); }
		};

		struct FStoreInstruction: StoreInstruction {
			FStoreInstruction(uint16_t index): StoreInstruction(index) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new FStoreOperation(context, index); }
		};

		struct DStoreInstruction: StoreInstruction {
			DStoreInstruction(uint16_t index): StoreInstruction(index) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new DStoreOperation(context, index); }
		};

		struct AStoreInstruction: StoreInstruction {
			AStoreInstruction(uint16_t index): StoreInstruction(index) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new AStoreOperation(context, index); }
		};


		struct ArrayStoreInstruction: Instruction {};

		struct IAStoreInstruction: ArrayStoreInstruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new IAStoreOperation(context); }
		};

		struct LAStoreInstruction: ArrayStoreInstruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new LAStoreOperation(context); }
		};

		struct FAStoreInstruction: ArrayStoreInstruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new FAStoreOperation(context); }
		};

		struct DAStoreInstruction: ArrayStoreInstruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new DAStoreOperation(context); }
		};

		struct AAStoreInstruction: ArrayStoreInstruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new AAStoreOperation(context); }
		};

		struct BAStoreInstruction: ArrayStoreInstruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new BAStoreOperation(context); }
		};

		struct CAStoreInstruction: ArrayStoreInstruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new CAStoreOperation(context); }
		};

		struct SAStoreInstruction: ArrayStoreInstruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new SAStoreOperation(context); }
		};


		template<TypeSize size>
		struct PopInstruction: Instruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new PopOperation<size>(context); }
		};


		template<TypeSize size>
		struct DupInstruction: Instruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new DupOperation<size>(context); }
		};

		using Dup1Instruction = DupInstruction<TypeSize::FOUR_BYTES>;
		using Dup2Instruction = DupInstruction<TypeSize::EIGHT_BYTES>;

		struct DupX1Instruction: Instruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new DupX1Operation(context); }
		};

		struct DupX2Instruction: Instruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new DupX2Operation(context); }
		};

		struct Dup2X1Instruction: Instruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new Dup2X1Operation(context); }
		};

		struct Dup2X2Instruction: Instruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new Dup2X2Operation(context); }
		};


		struct SwapInstruction: Instruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				context.stack.push(context.stack.pop(), context.stack.pop());
				return nullptr;
			}
		};


		template<char32_t operation, Priority priority, bool canUseBoolean = false>
		struct OperatorInstruction: Instruction {
			protected:
				const Type *const type;

				static inline const Type* getTypeByCode(uint16_t code) {
					switch(code) {
						case 0: return canUseBoolean ? ANY_INT_OR_BOOLEAN : ANY_INT;
						case 1: return LONG;
						case 2: return FLOAT;
						case 3: return DOUBLE;
						default: throw Exception("Illegal type code: " + hexWithPrefix(code));
					}
				}

			public:
				OperatorInstruction(uint16_t typeCode): type(getTypeByCode(typeCode)) {}
		};


		template<char32_t operation, Priority priority, bool canUseBoolean = false>
		struct BinaryOperatorInstruction: OperatorInstruction<operation, priority, canUseBoolean> {
			BinaryOperatorInstruction(uint16_t typeCode): OperatorInstruction<operation, priority, canUseBoolean>(typeCode) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return new BinaryOperatorOperationImpl<operation, priority>(this->type, context);
			}
		};


		template<char32_t operation, Priority priority>
		struct ShiftOperatorInstruction: BinaryOperatorInstruction<operation, priority> {
			ShiftOperatorInstruction(uint16_t typeCode): BinaryOperatorInstruction<operation, priority>(typeCode) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return new BinaryOperatorOperationImpl<operation, priority>(this->type, INT, context);
			}
		};

		template<char32_t operation, Priority priority>
		using BooleanBinaryOperatorInstruction = BinaryOperatorInstruction<operation, priority, true>;



		template<char32_t operation, Priority priority>
		struct UnaryOperatorInstruction: OperatorInstruction<operation, priority> {
			UnaryOperatorInstruction(uint16_t typeCode): OperatorInstruction<operation, priority>(typeCode) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return new UnaryOperatorOperation<operation, priority>(this->type, context);
			}
		};

		using AddOperatorInstruction = BinaryOperatorInstruction<'+', Priority::PLUS>;
		using SubOperatorInstruction = BinaryOperatorInstruction<'-', Priority::MINUS>;
		using MulOperatorInstruction = BinaryOperatorInstruction<'*', Priority::MULTIPLE>;
		using DivOperatorInstruction = BinaryOperatorInstruction<'/', Priority::DIVISION>;
		using RemOperatorInstruction = BinaryOperatorInstruction<'%', Priority::REMAINDER>;

		using NegOperatorInstruction = UnaryOperatorInstruction<'-', Priority::UNARY_MINUS>;

		using ShiftLeftOperatorInstruction = ShiftOperatorInstruction<"<<"_c32, Priority::SHIFT>;
		using ShiftRightOperatorInstruction = ShiftOperatorInstruction<">>"_c32, Priority::SHIFT>;
		using UShiftRightOperatorInstruction = ShiftOperatorInstruction<">>>"_c32, Priority::SHIFT>;

		using AndOperatorInstruction = BooleanBinaryOperatorInstruction<'&', Priority::BIT_AND>;
		using OrOperatorInstruction  = BooleanBinaryOperatorInstruction<'|', Priority::BIT_OR>;
		using XorOperatorInstruction = BooleanBinaryOperatorInstruction<'^', Priority::BIT_XOR>;


		/* maybe TODO
		template<PrimitiveType type> using AddOperatorInstruction = BinaryOperatorInstruction<type, '+', 11>;

		template<PrimitiveType type> using SubOperatorInstruction = BinaryOperatorInstruction<type, '-', 11>;

		template<PrimitiveType type> using MulOperatorInstruction = BinaryOperatorInstruction<type, '*', 12>;

		template<PrimitiveType type> using DivOperatorInstruction = BinaryOperatorInstruction<type, '/', 12>;

		template<PrimitiveType type> using RemOperatorInstruction = BinaryOperatorInstruction<type, '%', 12>;

		template<PrimitiveType type> using NegOperatorInstruction = UnaryOperatorInstruction<type, '-', 13>;

		template<PrimitiveType type> using ShiftLeftOperatorInstruction = BinaryOperatorInstruction<type, '<<', 10>;

		template<PrimitiveType type> using ShiftRightOperatorInstruction = BinaryOperatorInstruction<type, '>>', 10>;

		template<PrimitiveType type> using UShiftRightOperatorInstruction = BinaryOperatorInstruction<type, '>>>', 10>;

		template<PrimitiveType type> using AndOperatorInstruction = BinaryOperatorInstruction<type, '&', 7>;

		template<PrimitiveType type> using OrOperatorInstruction = BinaryOperatorInstruction<type, '|', 5>;

		template<PrimitiveType type> using XorOperatorInstruction = BinaryOperatorInstruction<type, '^', 6>;
		*/



		struct IIncInstruction: InstructionWithIndex {
			const int16_t value;

			IIncInstruction(uint16_t index, int16_t value): InstructionWithIndex(index), value(value) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new IIncOperation(context, index, value); }
		};

	}

	namespace operations {
		IIncOperation::IIncOperation(const DecompilationContext& context, uint16_t index, int16_t value):
				variable(context.getCurrentScope()->getVariable(index, true)), value(value),
				isShortInc(value == 1 || value == -1) /* isShortInc true when we can write ++ or -- */ {

			using namespace instructions;

			const Type* variableType = variable.castTypeTo(ANY_INT);

			const ILoadOperation* iloadOperation = context.stack.empty() ? nullptr : dynamic_cast<const ILoadOperation*>(context.stack.top());

			if(isShortInc && iloadOperation != nullptr && iloadOperation->variable == variable) {
				context.stack.pop();
				returnType = variableType;
				isPostInc = true;
			} else {
				const ILoadInstruction* iloadInstruction =
						dynamic_cast<const ILoadInstruction*>(context.getInstructionNoexcept(context.index + 1));
				if(iloadInstruction != nullptr && iloadInstruction->index == index) {
					iloadInstruction->setNullOperation();
					returnType = variableType;
				} else {
					returnType = VOID;
				}
			}
		}
	}

	namespace instructions {

		struct CastInstruction: Instruction {
			protected:
				const Type *const requiredType, *const type;
				const bool required;

			public:
				CastInstruction(const Type* requiredType, const Type* type, bool required):
						requiredType(requiredType), type(type), required(required) {}

				virtual const Operation* toOperation(const DecompilationContext& context) const override {
					return new CastOperation(context, requiredType, type, required);
				}
		};


		struct LCmpInstruction: Instruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new LCmpOperation(context); }
		};

		struct FCmpInstruction: Instruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new FCmpOperation(context); }
		};

		struct DCmpInstruction: Instruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new DCmpOperation(context); }
		};
	}
}

#include "blocks.cpp"
#include "block-instructions.cpp"

namespace jdecompiler {
	namespace instructions {

		struct SwitchInstruction: Instruction {
			protected:
				const offset_t defaultOffset;
				map<int32_t, offset_t> offsetTable;

			public:
				SwitchInstruction(offset_t defaultOffset, map<int32_t, offset_t> offsetTable): defaultOffset(defaultOffset), offsetTable(offsetTable) {}

				virtual const Operation* toOperation(const DecompilationContext& context) const override {
					return new SwitchScope(context, defaultOffset, offsetTable);
				}
		};


		struct FieldInstruction: Instruction {
			const ClassType clazz;
			const FieldDescriptor descriptor;

			FieldInstruction(const FieldrefConstant* fieldref):
				clazz(fieldref->clazz), descriptor(fieldref->nameAndType) {}

			FieldInstruction(const DisassemblerContext& context, uint16_t index):
				FieldInstruction(context.constPool.get<FieldrefConstant>(index)) {}
		};


		struct GetStaticFieldInstruction: FieldInstruction {
			GetStaticFieldInstruction(const DisassemblerContext& context, uint16_t index): FieldInstruction(context, index) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return new GetStaticFieldOperation(clazz, descriptor);
			}
		};


		struct PutStaticFieldInstruction: FieldInstruction {
			PutStaticFieldInstruction(const DisassemblerContext& context, uint16_t index): FieldInstruction(context, index) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return new PutStaticFieldOperation(context, clazz, descriptor);
			}
		};


		struct GetInstanceFieldInstruction: FieldInstruction {
			GetInstanceFieldInstruction(const DisassemblerContext& context, uint16_t index): FieldInstruction(context, index) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return new GetInstanceFieldOperation(context, clazz, descriptor);
			}
		};


		struct PutInstanceFieldInstruction: FieldInstruction {
			PutInstanceFieldInstruction(const DisassemblerContext& context, uint16_t index): FieldInstruction(context, index) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return new PutInstanceFieldOperation(context, clazz, descriptor);
			}
		};



		struct InvokeInstruction: InstructionWithIndex {
			const MethodDescriptor& descriptor;

			InvokeInstruction(uint16_t index, const ConstantPool& constPool):
					InstructionWithIndex(index), descriptor(*new MethodDescriptor(constPool.get<MethodrefConstant>(index))) {}
		};


		struct InvokevirtualInstruction: InvokeInstruction {
			InvokevirtualInstruction(uint16_t index, const ConstantPool& constPool): InvokeInstruction(index, constPool) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				if(JDecompiler::getInstance().castWrappers()) {
					static const MethodDescriptor
							byteValue   (javaLang::Byte,      "byteValue",   BYTE),
							charValue   (javaLang::Character, "charValue",   CHAR),
							shortValue  (javaLang::Short,     "shortValue",  SHORT),
							intValue    (javaLang::Integer,   "intValue",    INT),
							longValue   (javaLang::Long,      "longValue",   LONG),
							floatValue  (javaLang::Float,     "floatValue",  FLOAT),
							doubleValue (javaLang::Double,    "doubleValue", DOUBLE);

					if(descriptor == byteValue)   return new CastOperation(context, &javaLang::Byte,      BYTE,   false);
					if(descriptor == charValue)   return new CastOperation(context, &javaLang::Character, CHAR,   false);
					if(descriptor == shortValue)  return new CastOperation(context, &javaLang::Short,     SHORT,  false);
					if(descriptor == intValue)    return new CastOperation(context, &javaLang::Integer,   INT,    false);
					if(descriptor == longValue)   return new CastOperation(context, &javaLang::Long,      LONG,   false);
					if(descriptor == floatValue)  return new CastOperation(context, &javaLang::Float,     FLOAT,  false);
					if(descriptor == doubleValue) return new CastOperation(context, &javaLang::Double,    DOUBLE, false);
				}
				return new InvokevirtualOperation(context, descriptor);
			}
		};


		struct InvokespecialInstruction: InvokeInstruction {
			InvokespecialInstruction(uint16_t index, const ConstantPool& constPool): InvokeInstruction(index, constPool) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return new InvokespecialOperation(context, descriptor);
			}
		};


		struct InvokestaticInstruction: InvokeInstruction {
			InvokestaticInstruction(uint16_t index, const ConstantPool& constPool): InvokeInstruction(index, constPool) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				if(JDecompiler::getInstance().castWrappers()) {
					static const MethodDescriptor
							byteValueOf      (javaLang::Byte,      "valueOf", javaLang::Byte,      {BYTE}),
							characterValueOf (javaLang::Character, "valueOf", javaLang::Character, {CHAR}),
							shortValueOf     (javaLang::Short,     "valueOf", javaLang::Short,     {SHORT}),
							integerValueOf   (javaLang::Integer,   "valueOf", javaLang::Integer,   {INT}),
							longValueOf      (javaLang::Long,      "valueOf", javaLang::Long,      {LONG}),
							floatValueOf     (javaLang::Float,     "valueOf", javaLang::Float,     {FLOAT}),
							doubleValueOf    (javaLang::Double,    "valueOf", javaLang::Double,    {DOUBLE});

					if(descriptor == byteValueOf)      return new CastOperation(context, BYTE,   &javaLang::Byte,      false);
					if(descriptor == characterValueOf) return new CastOperation(context, CHAR,   &javaLang::Character, false);
					if(descriptor == shortValueOf)     return new CastOperation(context, SHORT,  &javaLang::Short,     false);
					if(descriptor == integerValueOf)   return new CastOperation(context, INT,    &javaLang::Integer,   false);
					if(descriptor == longValueOf)      return new CastOperation(context, LONG,   &javaLang::Long,      false);
					if(descriptor == floatValueOf)     return new CastOperation(context, FLOAT,  &javaLang::Float,     false);
					if(descriptor == doubleValueOf)    return new CastOperation(context, DOUBLE, &javaLang::Double,    false);
				}
				return new InvokestaticOperation(context, descriptor);
			}
		};


		struct InvokeinterfaceInstruction: InvokeInstruction {
			InvokeinterfaceInstruction(uint16_t index, uint16_t count, uint8_t zeroByte, const DisassemblerContext& context):
					InvokeInstruction(index, context.constPool) {

				if(count == 0)
					context.warning("illegal format of instruction invokeinterface at pos " + hexWithPrefix(context.getPos()) +
							": by specification, count must not be zero");

				if(zeroByte != 0)
					context.warning("illegal format of instruction invokeinterface at pos " + hexWithPrefix(context.getPos()) +
							": by specification, fourth byte must be zero");
			}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return new InvokeinterfaceOperation(context, descriptor);
			}
		};


		struct InvokedynamicInstruction: InstructionWithIndex {
			InvokedynamicInstruction(uint16_t index, uint16_t zeroShort, const DisassemblerContext& context):
					InstructionWithIndex(index) {

				if(zeroShort != 0)
					context.warning("illegal format of instruction invokedynamic at pos " + hexWithPrefix(context.getPos()) +
							": by specification, third and fourth bytes must be zero");
			}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				const InvokeDynamicConstant* invokeDynamicConstant = context.constPool.get<InvokeDynamicConstant>(index);
				const BootstrapMethod* bootstrapMethod =
						(*context.classinfo.attributes.getExact<BootstrapMethodsAttribute>())[invokeDynamicConstant->bootstrapMethodAttrIndex];

				const ReferenceConstant* const referenceConstant = bootstrapMethod->methodHandle->referenceConstant;

				typedef MethodHandleConstant::ReferenceKind RefKind;
				typedef MethodHandleConstant::KindType KindType;

				switch(bootstrapMethod->methodHandle->kindType) {
					case KindType::FIELD:
						switch(bootstrapMethod->methodHandle->referenceKind) {
							case RefKind::GETFIELD: return new GetInstanceFieldOperation(context, safe_cast<const FieldrefConstant*>(referenceConstant));
							case RefKind::GETSTATIC: return new GetStaticFieldOperation(          safe_cast<const FieldrefConstant*>(referenceConstant));
							case RefKind::PUTFIELD: return new PutInstanceFieldOperation(context, safe_cast<const FieldrefConstant*>(referenceConstant));
							case RefKind::PUTSTATIC: return new PutStaticFieldOperation( context, safe_cast<const FieldrefConstant*>(referenceConstant));
							default: throw IllegalStateException((string)"Illegal referenceConstant kind " +
									to_string((unsigned int)bootstrapMethod->methodHandle->referenceKind));
						}
					case KindType::METHOD: {
						const MethodDescriptor& descriptor =
								*new const MethodDescriptor(bootstrapMethod->methodHandle->referenceConstant->clazz->name, invokeDynamicConstant->nameAndType);

						vector<const Operation*> arguments(descriptor.arguments.size());

						static const ArrayType OBJECT_ARRAY(OBJECT);
						static const ClassType CALL_SITE("java/lang/invoke/CallSite");
						static const ClassType LOOKUP("java/lang/invoke/MethodHandles$Lookup");
						static const ClassType STRING_CONCAT_FACTORY("java/lang/invoke/StringConcatFactory");

						// pop arguments that already on stack
						for(uint32_t i = descriptor.arguments.size(); i > 0; )
							arguments[--i] = context.stack.pop();


						if(bootstrapMethod->methodHandle->referenceKind == RefKind::INVOKESTATIC && descriptor.name == "makeConcatWithConstants" &&
							MethodDescriptor(bootstrapMethod->methodHandle->referenceConstant) ==
							MethodDescriptor(STRING_CONCAT_FACTORY, "makeConcatWithConstants", &CALL_SITE, {&LOOKUP, STRING, METHOD_TYPE, STRING, &OBJECT_ARRAY}))
						{ // String concat
							if(bootstrapMethod->arguments.size() < 1)
								throw DecompilationException("Method java.lang.invoke.StringConcatFactory::makeConcatWithConstants"
										" must have one or more static arguments");


							const auto getWrongTypeMessage = [&bootstrapMethod] (uint32_t i) {
								return "Method java.lang.invoke.StringConcatFactory::makeConcatWithConstants"
										": wrong type of static argument #" + to_string(i) +
										": expected String, got " + bootstrapMethod->arguments[i]->getConstantName();
							};


							if(!instanceof<const StringConstant*>(bootstrapMethod->arguments[0]))
								throw DecompilationException(getWrongTypeMessage(0));

							const StringConstOperation* pattern = new StringConstOperation(static_cast<const StringConstant*>(bootstrapMethod->arguments[0]));


							const uint32_t argumentsCount = bootstrapMethod->arguments.size();
							vector<const Operation*> staticArguments;
							staticArguments.reserve(argumentsCount - 1);

							for(uint32_t i = 1; i < argumentsCount; i++) {
								if(!instanceof<const StringConstant*>(bootstrapMethod->arguments[i]))
									throw DecompilationException(getWrongTypeMessage(i));

								staticArguments.push_back(StringConstOperation::valueOf(
										static_cast<const StringConstant*>(bootstrapMethod->arguments[i]), context.classinfo));
							}


							// push non-static arguments on stack
							for(const Operation* operation : arguments)
								context.stack.push(operation);

							return new ConcatStringsOperation(context, *new MethodDescriptor(descriptor), pattern, staticArguments);
						}


						// push lookup argument
						context.stack.push(new InvokestaticOperation(context,
								*new MethodDescriptor(STRING_CONCAT_FACTORY, "publicLookup", CALL_SITE)));

						context.stack.push(StringConstOperation::valueOf(invokeDynamicConstant->nameAndType->name, context.classinfo)); // name argument

						context.stack.push(new MethodTypeConstOperation(
								new MethodTypeConstant(invokeDynamicConstant->nameAndType->descriptor))); // type argument

						// push static arguments on stack
						for(uint32_t i = 0, argumentsCount = bootstrapMethod->arguments.size(); i < argumentsCount; i++)
							context.stack.push(bootstrapMethod->arguments[i]->toOperation(context.classinfo));

						// push non-static arguments on stack
						for(const Operation* operation : arguments)
							context.stack.push(operation);

						switch(bootstrapMethod->methodHandle->referenceKind) {
							case RefKind::INVOKEVIRTUAL: return new InvokevirtualOperation(context, descriptor);
							case RefKind::INVOKESTATIC:  return new InvokestaticOperation(context, descriptor);
							case RefKind::INVOKESPECIAL: return new InvokespecialOperation(context, descriptor);
							case RefKind::NEWINVOKESPECIAL: return new InvokespecialOperation(context, descriptor,
										new NewOperation(context, bootstrapMethod->methodHandle->referenceConstant->clazz));
							case RefKind::INVOKEINTERFACE: return new InvokeinterfaceOperation(context, descriptor);
							default: throw IllegalStateException((string)"Illegal referenceConstant kind " +
									to_string((unsigned int)bootstrapMethod->methodHandle->referenceKind));
						}
					}

					default: throw IllegalStateException((string)"Illegal kind type " + to_string((unsigned int)bootstrapMethod->methodHandle->kindType));
				}
			}
		};



		struct NewInstruction: InstructionWithIndex {
			NewInstruction(uint16_t classIndex): InstructionWithIndex(classIndex) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new NewOperation(context, index); }
		};


		static const Type* getArrayTypeByCode(uint8_t code) {
			switch(code) {
				case 0x4: return BOOLEAN;
				case 0x5: return CHAR;
				case 0x6: return FLOAT;
				case 0x7: return DOUBLE;
				case 0x8: return BYTE;
				case 0x9: return SHORT;
				case 0xA: return INT;
				case 0xB: return LONG;
				default: throw DecompilationException("Illegal array type code: " + hexWithPrefix(code));
			}
		}


		struct NewArrayInstruction: Instruction {
			protected:
				const ArrayType *const arrayType;

			public:
				NewArrayInstruction(uint8_t code): arrayType(new ArrayType(getArrayTypeByCode(code))) {}

				virtual const Operation* toOperation(const DecompilationContext& context) const override {
					return new NewArrayOperation(context, arrayType);
				}
		};

		struct ANewArrayInstruction: InstructionWithIndex {
			ANewArrayInstruction(uint16_t index): InstructionWithIndex(index) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new ANewArrayOperation(context, index); }
		};


		struct ArrayLengthInstruction: Instruction {
			public: virtual const Operation* toOperation(const DecompilationContext& context) const override { return new ArrayLengthOperation(context); }
		};


		struct AThrowInstruction: Instruction {
			public: virtual const Operation* toOperation(const DecompilationContext& context) const override { return new AThrowOperation(context); }
		};


		struct CheckCastInstruction: InstructionWithIndex {
			CheckCastInstruction(uint16_t index): InstructionWithIndex(index) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new CheckCastOperation(context, index); }
		};

		struct InstanceofInstruction: InstructionWithIndex {
			InstanceofInstruction(uint16_t index): InstructionWithIndex(index) {}
			virtual const Operation* toOperation(const DecompilationContext& context) const override { return new InstanceofOperation(context, index); }
		};


		struct MultiANewArrayInstruction: InstructionWithIndex {
			protected: const uint16_t dimensions;

			public: MultiANewArrayInstruction(uint16_t index, uint16_t dimensions): InstructionWithIndex(index), dimensions(dimensions) {}

			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return new MultiANewArrayOperation(context, index, dimensions);
			}
		};


		template<class ReturnOperation>
		struct ReturnInstruction: Instruction {
			virtual const Operation* toOperation(const DecompilationContext& context) const override {
				return new ReturnOperation(context);
			}
		};

		using IReturnInstruction = ReturnInstruction<IReturnOperation>;
		using LReturnInstruction = ReturnInstruction<LReturnOperation>;
		using FReturnInstruction = ReturnInstruction<FReturnOperation>;
		using DReturnInstruction = ReturnInstruction<DReturnOperation>;
		using AReturnInstruction = ReturnInstruction<AReturnOperation>;


		struct VReturn: VoidInstructionAndOperation {
			private: VReturn() {}

			public:
				virtual string toString(const StringifyContext& context) const override { return "return"; }

				static VReturn* getInstance() {
					static VReturn instance;
					return &instance;
				}
		};



		static IConstInstruction
				*const ICONST_M1 = new IConstInstruction(-1),
				*const ICONST_0 = new IConstInstruction(0),
				*const ICONST_1 = new IConstInstruction(1),
				*const ICONST_2 = new IConstInstruction(2),
				*const ICONST_3 = new IConstInstruction(3),
				*const ICONST_4 = new IConstInstruction(4),
				*const ICONST_5 = new IConstInstruction(5);
		static LConstInstruction
				*const LCONST_0 = new LConstInstruction(0),
				*const LCONST_1 = new LConstInstruction(1);
		static FConstInstruction
				*const FCONST_0 = new FConstInstruction(0),
				*const FCONST_1 = new FConstInstruction(1),
				*const FCONST_2 = new FConstInstruction(2);
		static DConstInstruction
				*const DCONST_0 = new DConstInstruction(0),
				*const DCONST_1 = new DConstInstruction(1);
	}
}

#endif
