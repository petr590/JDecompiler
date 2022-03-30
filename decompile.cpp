#ifndef JDECOMPILER_DECOMPILE_CPP
#define JDECOMPILER_DECOMPILE_CPP

#include "instructions.cpp"

namespace jdecompiler {

	Instruction* DisassemblerContext::nextInstruction() {
		using namespace instructions;

		switch(current()) {
			case 0x00: return nullptr;
			case 0x01: return AConstNull::getInstance();
			case 0x02: return ICONST_M1;
			case 0x03: return ICONST_0;
			case 0x04: return ICONST_1;
			case 0x05: return ICONST_2;
			case 0x06: return ICONST_3;
			case 0x07: return ICONST_4;
			case 0x08: return ICONST_5;
			case 0x09: return LCONST_0;
			case 0x0A: return LCONST_1;
			case 0x0B: return FCONST_0;
			case 0x0C: return FCONST_1;
			case 0x0D: return FCONST_2;
			case 0x0E: return DCONST_0;
			case 0x0F: return DCONST_1;
			case 0x10: return new BIPushInstruction(nextByte());
			case 0x11: return new SIPushInstruction(nextShort());
			case 0x12: return new LdcInstruction<TypeSize::FOUR_BYTES>(*this, nextUByte());
			case 0x13: return new LdcInstruction<TypeSize::FOUR_BYTES>(*this, nextUShort());
			case 0x14: return new LdcInstruction<TypeSize::EIGHT_BYTES>(*this, nextUShort());
			case 0x15: return new ILoadInstruction(nextUByte());
			case 0x16: return new LLoadInstruction(nextUByte());
			case 0x17: return new FLoadInstruction(nextUByte());
			case 0x18: return new DLoadInstruction(nextUByte());
			case 0x19: return new ALoadInstruction(nextUByte());
			case 0x1A: return new ILoadInstruction(0);
			case 0x1B: return new ILoadInstruction(1);
			case 0x1C: return new ILoadInstruction(2);
			case 0x1D: return new ILoadInstruction(3);
			case 0x1E: return new LLoadInstruction(0);
			case 0x1F: return new LLoadInstruction(1);
			case 0x20: return new LLoadInstruction(2);
			case 0x21: return new LLoadInstruction(3);
			case 0x22: return new FLoadInstruction(0);
			case 0x23: return new FLoadInstruction(1);
			case 0x24: return new FLoadInstruction(2);
			case 0x25: return new FLoadInstruction(3);
			case 0x26: return new DLoadInstruction(0);
			case 0x27: return new DLoadInstruction(1);
			case 0x28: return new DLoadInstruction(2);
			case 0x29: return new DLoadInstruction(3);
			case 0x2A: return new ALoadInstruction(0);
			case 0x2B: return new ALoadInstruction(1);
			case 0x2C: return new ALoadInstruction(2);
			case 0x2D: return new ALoadInstruction(3);
			case 0x2E: return new IALoadInstruction();
			case 0x2F: return new LALoadInstruction();
			case 0x30: return new FALoadInstruction();
			case 0x31: return new DALoadInstruction();
			case 0x32: return new AALoadInstruction();
			case 0x33: return new BALoadInstruction();
			case 0x34: return new CALoadInstruction();
			case 0x35: return new SALoadInstruction();
			case 0x36: return new IStoreInstruction(nextUByte());
			case 0x37: return new LStoreInstruction(nextUByte());
			case 0x38: return new FStoreInstruction(nextUByte());
			case 0x39: return new DStoreInstruction(nextUByte());
			case 0x3A: return new AStoreInstruction(nextUByte());
			case 0x3B: return new IStoreInstruction(0);
			case 0x3C: return new IStoreInstruction(1);
			case 0x3D: return new IStoreInstruction(2);
			case 0x3E: return new IStoreInstruction(3);
			case 0x3F: return new LStoreInstruction(0);
			case 0x40: return new LStoreInstruction(1);
			case 0x41: return new LStoreInstruction(2);
			case 0x42: return new LStoreInstruction(3);
			case 0x43: return new FStoreInstruction(0);
			case 0x44: return new FStoreInstruction(1);
			case 0x45: return new FStoreInstruction(2);
			case 0x46: return new FStoreInstruction(3);
			case 0x47: return new DStoreInstruction(0);
			case 0x48: return new DStoreInstruction(1);
			case 0x49: return new DStoreInstruction(2);
			case 0x4A: return new DStoreInstruction(3);
			case 0x4B: return new AStoreInstruction(0);
			case 0x4C: return new AStoreInstruction(1);
			case 0x4D: return new AStoreInstruction(2);
			case 0x4E: return new AStoreInstruction(3);
			case 0x4F: return new IAStoreInstruction();
			case 0x50: return new LAStoreInstruction();
			case 0x51: return new FAStoreInstruction();
			case 0x52: return new DAStoreInstruction();
			case 0x53: return new AAStoreInstruction();
			case 0x54: return new BAStoreInstruction();
			case 0x55: return new CAStoreInstruction();
			case 0x56: return new SAStoreInstruction();
			case 0x57: return new PopInstruction<TypeSize::FOUR_BYTES>();
			case 0x58: return new PopInstruction<TypeSize::EIGHT_BYTES>();
			case 0x59: return new Dup1Instruction();
			case 0x5A: return new DupX1Instruction();
			case 0x5B: return new DupX2Instruction();
			case 0x5C: return new Dup2Instruction();
			case 0x5D: return new Dup2X1Instruction();
			case 0x5E: return new Dup2X2Instruction();
			case 0x5F: return new SwapInstruction();
			case 0x60: case 0x61: case 0x62: case 0x63: return new AddOperatorInstruction(current() & 3);
			case 0x64: case 0x65: case 0x66: case 0x67: return new SubOperatorInstruction(current() & 3);
			case 0x68: case 0x69: case 0x6A: case 0x6B: return new MulOperatorInstruction(current() & 3);
			case 0x6C: case 0x6D: case 0x6E: case 0x6F: return new DivOperatorInstruction(current() & 3);
			case 0x70: case 0x71: case 0x72: case 0x73: return new RemOperatorInstruction(current() & 3);
			case 0x74: case 0x75: case 0x76: case 0x77: return new NegOperatorInstruction(current() & 3);
			case 0x78: case 0x79: return new ShiftLeftOperatorInstruction(current() & 1);
			case 0x7A: case 0x7B: return new ShiftRightOperatorInstruction(current() & 1);
			case 0x7C: case 0x7D: return new UShiftRightOperatorInstruction(current() & 1);
			case 0x7E: case 0x7F: return new AndOperatorInstruction(current() & 1);
			case 0x80: case 0x81: return new OrOperatorInstruction(current() & 1);
			case 0x82: case 0x83: return new XorOperatorInstruction(current() & 1);
			case 0x84: return new IIncInstruction(nextUByte(), nextByte());
			case 0x85: return new CastInstruction<false>(INT, LONG); // int -> long
			case 0x86: return new CastInstruction<false>(INT, FLOAT); // int -> float
			case 0x87: return new CastInstruction<false>(INT, DOUBLE); // int -> double
			case 0x88: return new CastInstruction<true>(LONG, INT); // long -> int
			case 0x89: return new CastInstruction<false>(LONG, FLOAT); // long -> float
			case 0x8A: return new CastInstruction<false>(LONG, DOUBLE); // long -> double
			case 0x8B: return new CastInstruction<true>(FLOAT, INT); // float -> int
			case 0x8C: return new CastInstruction<true>(FLOAT, LONG); // float -> long
			case 0x8D: return new CastInstruction<false>(FLOAT, DOUBLE); // float -> double
			case 0x8E: return new CastInstruction<true>(DOUBLE, INT); // double -> int
			case 0x8F: return new CastInstruction<true>(DOUBLE, LONG); // double -> long
			case 0x90: return new CastInstruction<true>(DOUBLE, FLOAT); // double -> float
			case 0x91: return new CastInstruction<true>(INT, BYTE); // int -> byte
			case 0x92: return new CastInstruction<true>(INT, CHAR); // int -> char
			case 0x93: return new CastInstruction<true>(INT, SHORT); // int -> short
			case 0x94: return new LCmpInstruction();
			case 0x95: return new FCmpInstruction();
			case 0x96: return new FCmpInstruction();
			case 0x97: return new DCmpInstruction();
			case 0x98: return new DCmpInstruction();
			case 0x99: return new IfEqInstruction(nextShort());
			case 0x9A: return new IfNotEqInstruction(nextShort());
			case 0x9B: return new IfLtInstruction(nextShort());
			case 0x9C: return new IfGeInstruction(nextShort());
			case 0x9D: return new IfGtInstruction(nextShort());
			case 0x9E: return new IfLeInstruction(nextShort());
			case 0x9F: return new IfIEqInstruction(nextShort());
			case 0xA0: return new IfINotEqInstruction(nextShort());
			case 0xA1: return new IfILtInstruction(nextShort());
			case 0xA2: return new IfIGeInstruction(nextShort());
			case 0xA3: return new IfIGtInstruction(nextShort());
			case 0xA4: return new IfILeInstruction(nextShort());
			case 0xA5: return new IfAEqInstruction(nextShort());
			case 0xA6: return new IfANotEqInstruction(nextShort());
			case 0xA7: return new GotoInstruction(nextShort());
			/*case 0xA8: i+=2; return "JSR";
			case 0xA9: i++ ; return "RET";*/
			case 0xAA: {
				skip(3 - (pos & 0x3)); // alignment by 4 bytes
				offset_t defaultOffset = nextInt();
				int32_t low = nextInt(), high = nextInt();
				if(high < low)
					throw InstructionFormatError("tableswitch: high < low (low = " + to_string(low) +
							", high = " + to_string(high) + ")");

				map<int32_t, offset_t> offsetTable;
				for(int32_t i = 0, size = high - low + 1; i < size; i++)
					offsetTable[i + low] = nextInt();
				return new SwitchInstruction(defaultOffset, offsetTable);
			}
			case 0xAB: {
				skip(3 - (pos & 0x3)); // alignment by 4 bytes
				offset_t defaultOffset = nextInt();
				map<int32_t, offset_t> offsetTable;
				for(uint32_t i = nextUInt(); i > 0; i--) {
					int32_t value = nextInt();
					offsetTable[value] = nextInt();
				}
				return new SwitchInstruction(defaultOffset, offsetTable);
			}
			case 0xAC: return new IReturnInstruction();
			case 0xAD: return new LReturnInstruction();
			case 0xAE: return new FReturnInstruction();
			case 0xAF: return new DReturnInstruction();
			case 0xB0: return new AReturnInstruction();
			case 0xB1: return VReturn::getInstance();
			case 0xB2: return new GetStaticFieldInstruction(nextUShort());
			case 0xB3: return new PutStaticFieldInstruction(nextUShort());
			case 0xB4: return new GetInstanceFieldInstruction(nextUShort());
			case 0xB5: return new PutInstanceFieldInstruction(nextUShort());
			case 0xB6: return new InvokevirtualInstruction(nextUShort());
			case 0xB7: return new InvokespecialInstruction(nextUShort());
			case 0xB8: return new InvokestaticInstruction(nextUShort());
			case 0xB9: return new InvokeinterfaceInstruction(nextUShort(), nextUByte(), nextUByte(), *this);
			case 0xBA: return new InvokedynamicInstruction(nextUShort(), nextUShort(), *this);
			case 0xBB: return new NewInstruction(nextUShort());
			case 0xBC: return new NewArrayInstruction(nextUByte());
			case 0xBD: return new ANewArrayInstruction(nextUShort());
			case 0xBE: return new ArrayLengthInstruction();
			case 0xBF: return new AThrowInstruction();
			case 0xC0: return new CheckCastInstruction(nextUShort());
			case 0xC1: return new InstanceofInstruction(nextUShort());
			case 0xC2: return nullptr; // TODO: MonitorEnter
			case 0xC3: return nullptr; // TODO: MonitorExit
			case 0xC4: switch(nextUByte()) {
				case 0x15: return new ILoadInstruction(nextUShort());
				case 0x16: return new LLoadInstruction(nextUShort());
				case 0x17: return new FLoadInstruction(nextUShort());
				case 0x18: return new DLoadInstruction(nextUShort());
				case 0x19: return new ALoadInstruction(nextUShort());
				case 0x36: return new IStoreInstruction(nextUShort());
				case 0x37: return new LStoreInstruction(nextUShort());
				case 0x38: return new FStoreInstruction(nextUShort());
				case 0x39: return new DStoreInstruction(nextUShort());
				case 0x3A: return new AStoreInstruction(nextUShort());
				case 0x84: return new IIncInstruction(nextUShort(), nextShort());
				//case 0xA9: i+=2 ; return "RET";
				default: throw IllegalOpcodeException("Illegal wide opcode: " + hexWithPrefix(current()));
			}
			case 0xC5: return new MultiANewArrayInstruction(nextUShort(), nextUByte());
			case 0xC6: return new IfNullInstruction(nextShort());
			case 0xC7: return new IfNonNullInstruction(nextShort());
			case 0xC8: return new GotoInstruction(nextInt());
			/*case 0xC9: i+=4; return "JSR_W";
			case 0xCA: return "BREAKPOINT";
			case 0xFE: return "IMPDEP1";
			case 0xFF: return "IMPDEP2";*/
			default:
				throw IllegalOpcodeException(hexWithPrefix<2>(current()));
		}
	}

	const StringifyContext& Method::decompileCode(const ClassInfo& classinfo) {
		using namespace operations;
		using namespace instructions;

		LOG("decompiling of " << descriptor.toString());

		const bool hasCodeAttribute = codeAttribute != nullptr;

		const uint32_t methodScopeEndPos = hasCodeAttribute ? codeAttribute->codeLength : 0;
		const uint16_t localsCount = hasCodeAttribute ? codeAttribute->maxLocals : descriptor.arguments.size();

		MethodScope* methodScope = descriptor.type == MethodDescriptor::MethodType::STATIC_INITIALIZER ?
				new StaticInitializerScope(0, methodScopeEndPos, localsCount) : new MethodScope(0, methodScopeEndPos, localsCount);

		if(!(modifiers & ACC_STATIC))
			methodScope->addVariable(new NamedVariable(&classinfo.thisType, true, "this"));

		// -------------------------------------------------- Add arguments --------------------------------------------------
		{
			const uint32_t argumentsCount = descriptor.arguments.size();

			static const ArrayType STRING_ARRAY(STRING);

			if(descriptor.name == "main" && descriptor.returnType == VOID && modifiers == (ACC_PUBLIC | ACC_STATIC) &&
					argumentsCount == 1 && *descriptor.arguments[0] == STRING_ARRAY) { // public static void main(String[] args)
				methodScope->addVariable(new NamedVariable(&STRING_ARRAY, true, "args"));
			} else {
				for(uint32_t i = 0; i < argumentsCount; i++)
					methodScope->addVariable(new UnnamedVariable(descriptor.arguments[i], true));
			}
		}

		if(!hasCodeAttribute)
			return *new StringifyContext(*new DisassemblerContext(classinfo.constPool, 0, (const uint8_t*)""), classinfo, methodScope, modifiers, descriptor, attributes, 0);

		DisassemblerContext& disassemblerContext = *new DisassemblerContext(classinfo.constPool, codeAttribute->codeLength, codeAttribute->code);

		StringifyContext& stringifyContext =
				*new StringifyContext(disassemblerContext, classinfo, methodScope, modifiers, descriptor, attributes, codeAttribute->maxLocals);

		DecompilationContext& decompilationContext =
				*new DecompilationContext(stringifyContext, disassemblerContext, classinfo, methodScope, modifiers, descriptor, attributes, codeAttribute->maxLocals);

		// -------------------------------------------------- Add try-catch blocks --------------------------------------------------
		// TODO
		/*vector<TryBlock*> tryBlocks;

		for(const CodeAttribute::ExceptionHandler* exceptionAttribute : codeAttribute->exceptionTable) {

			const index_t
					tryStartIndex = decompilationContext.disassemblerContext.posToIndex(exceptionAttribute->startPos),
					tryEndIndex = decompilationContext.disassemblerContext.posToIndex(exceptionAttribute->endPos);

			const auto tryBlocksFindResult = find_if(tryBlocks.begin(), tryBlocks.end(),
					[tryStartIndex, tryEndIndex] (TryBlock* tryBlock) { return tryBlock->startIndex == tryStartIndex && tryBlock->endIndex == tryEndIndex; });


			TryBlock* tryBlock;

			if(tryBlocksFindResult != tryBlocks.end()) {
				tryBlock = *tryBlocksFindResult;
			} else {
				tryBlock = new TryBlock(tryStartIndex, tryEndIndex);
				tryBlocks.push_back(tryBlock);
				disassemblerContext.addBlock(tryBlock);
			}

			const index_t catchStartIndex = disassemblerContext.posToIndex(exceptionAttribute->handlerPos) - 1;

			const auto handlersFindResult = find_if(tryBlock->handlers.begin(), tryBlock->handlers.end(),
					[catchStartIndex] (CatchBlockDataHolder& handlerData) { return handlerData.startIndex == catchStartIndex; });

			if(handlersFindResult != tryBlock->handlers.end()) {
				handlersFindResult->catchTypes.push_back();
			} else {
				tryBlock->handlers.push_back(new CatchBlock(catchStartIndex, exceptionAttribute->catchType, exceptionAttribute->catchType));
			}
		}*/


		const vector<Instruction*>& instructions = disassemblerContext.getInstructions();
		vector<const Block*> blocks = disassemblerContext.getBlocks();

		for(uint32_t i = 0, exprIndex = 0, instructionsSize = instructions.size(); i < instructionsSize; i++) {

			decompilationContext.index = i;
			decompilationContext.pos = disassemblerContext.indexToPos(i);

			decompilationContext.exprIndexTable[i] = exprIndex;

			if(decompilationContext.stack.empty())
				decompilationContext.exprStartIndex = i;

			/*if(instructions[i] != nullptr && decompilationContext.addOperation(instructions[i]->toOperation(decompilationContext))) {
				exprIndex++;
			}*/

			if(instructions[i] != nullptr) {
				const Operation* operation = instructions[i]->toOperation(decompilationContext);
				if(operation != nullptr) {

					if(operation->getReturnType() != VOID) {
						decompilationContext.stack.push(operation);
					} else if(operation->canAddToCode() && !(i == instructionsSize - 1 && operation == VReturn::getInstance())) {
						decompilationContext.getCurrentScope()->addOperation(operation, stringifyContext);
						exprIndex++;
					}

					if(instanceof<const Scope*>(operation))
						decompilationContext.addScope(static_cast<const Scope*>(operation));
				}
			}

			for(auto iter = blocks.begin(); iter != blocks.end(); ) {
				const Block* block = *iter;

				assert(!(block->start() < i));

				if(block->start() == i) { // Do not increment iterator when erase element

					const Operation* operation = block->toOperation(decompilationContext);
					if(operation != nullptr) {
						if(operation->getReturnType() != VOID) {
							decompilationContext.stack.push(operation);
						} else if(operation->canAddToCode() && !(i == instructionsSize - 1 && operation == VReturn::getInstance())) {
							decompilationContext.getCurrentScope()->addOperation(operation, stringifyContext);
							exprIndex++;
						}

						if(instanceof<const Scope*>(operation))
							decompilationContext.addScope(static_cast<const Scope*>(operation));
					}
					blocks.erase(iter);
				} else {
					++iter;
				}
			}

			decompilationContext.updateScopes();
		}

		return stringifyContext;
	}
}

#endif
