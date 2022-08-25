#ifndef JDECOMPILER_INVOKE_INSTRUCTIONS_CPP
#define JDECOMPILER_INVOKE_INSTRUCTIONS_CPP

namespace jdecompiler {
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
				context.warning("illegal format of instruction invokeinterface at pos " + hexWithPrefix(context.pos) +
						": by specification, count must not be zero");

			if(zeroByte != 0)
				context.warning("illegal format of instruction invokeinterface at pos " + hexWithPrefix(context.pos) +
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
				context.warning("illegal format of instruction invokedynamic at pos " + hexWithPrefix(context.pos) +
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

							staticArguments.push_back(new StringConstOperation(static_cast<const StringConstant*>(bootstrapMethod->arguments[i])));
						}


						// push non-static arguments on stack
						for(const Operation* operation : arguments)
							context.stack.push(operation);

						return new ConcatStringsOperation(context, *new MethodDescriptor(descriptor), pattern, staticArguments);
					}


					// push lookup argument
					context.stack.push(new InvokestaticOperation(context,
							*new MethodDescriptor(STRING_CONCAT_FACTORY, "publicLookup", CALL_SITE)));

					context.stack.push(new StringConstOperation(invokeDynamicConstant->nameAndType->name)); // name argument

					context.stack.push(new MethodTypeConstOperation(
							new MethodTypeConstant(invokeDynamicConstant->nameAndType->descriptor))); // type argument

					// push static arguments on stack
					for(uint32_t i = 0, argumentsCount = bootstrapMethod->arguments.size(); i < argumentsCount; i++)
						context.stack.push(bootstrapMethod->arguments[i]->toOperation());

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
}

#endif
