#ifndef JDECOMPILER_DECOMPILE_CPP
#define JDECOMPILER_DECOMPILE_CPP

#include "instructions.cpp"

namespace jdecompiler {

	tuple<int32_t, int32_t, Instruction*> DisassemblerContext::nextInstruction() {
		using namespace instructions;

		switch(current()) {
			case 0x00: return {0, 0, nullptr};
			case 0x01: return {0, 1, AConstNull::getInstance()};
			case 0x02: return {0, 1, ICONST_M1};
			case 0x03: return {0, 1, ICONST_0};
			case 0x04: return {0, 1, ICONST_1};
			case 0x05: return {0, 1, ICONST_2};
			case 0x06: return {0, 1, ICONST_3};
			case 0x07: return {0, 1, ICONST_4};
			case 0x08: return {0, 1, ICONST_5};
			case 0x09: return {0, 1, LCONST_0};
			case 0x0A: return {0, 1, LCONST_1};
			case 0x0B: return {0, 1, FCONST_0};
			case 0x0C: return {0, 1, FCONST_1};
			case 0x0D: return {0, 1, FCONST_2};
			case 0x0E: return {0, 1, DCONST_0};
			case 0x0F: return {0, 1, DCONST_1};
			case 0x10: return {0, 1, new BIPushInstruction(nextByte())};
			case 0x11: return {0, 1, new SIPushInstruction(nextShort())};
			case 0x12: return {0, 1, new LdcInstruction<TypeSize::FOUR_BYTES>(*this, nextUByte())};
			case 0x13: return {0, 1, new LdcInstruction<TypeSize::FOUR_BYTES>(*this, nextUShort())};
			case 0x14: return {0, 1, new LdcInstruction<TypeSize::EIGHT_BYTES>(*this, nextUShort())};
			case 0x15: return {0, 1, new ILoadInstruction(nextUByte())};
			case 0x16: return {0, 1, new LLoadInstruction(nextUByte())};
			case 0x17: return {0, 1, new FLoadInstruction(nextUByte())};
			case 0x18: return {0, 1, new DLoadInstruction(nextUByte())};
			case 0x19: return {0, 1, new ALoadInstruction(nextUByte())};
			case 0x1A: return {0, 1, new ILoadInstruction(0)};
			case 0x1B: return {0, 1, new ILoadInstruction(1)};
			case 0x1C: return {0, 1, new ILoadInstruction(2)};
			case 0x1D: return {0, 1, new ILoadInstruction(3)};
			case 0x1E: return {0, 1, new LLoadInstruction(0)};
			case 0x1F: return {0, 1, new LLoadInstruction(1)};
			case 0x20: return {0, 1, new LLoadInstruction(2)};
			case 0x21: return {0, 1, new LLoadInstruction(3)};
			case 0x22: return {0, 1, new FLoadInstruction(0)};
			case 0x23: return {0, 1, new FLoadInstruction(1)};
			case 0x24: return {0, 1, new FLoadInstruction(2)};
			case 0x25: return {0, 1, new FLoadInstruction(3)};
			case 0x26: return {0, 1, new DLoadInstruction(0)};
			case 0x27: return {0, 1, new DLoadInstruction(1)};
			case 0x28: return {0, 1, new DLoadInstruction(2)};
			case 0x29: return {0, 1, new DLoadInstruction(3)};
			case 0x2A: return {0, 1, new ALoadInstruction(0)};
			case 0x2B: return {0, 1, new ALoadInstruction(1)};
			case 0x2C: return {0, 1, new ALoadInstruction(2)};
			case 0x2D: return {0, 1, new ALoadInstruction(3)};
			case 0x2E: return {2, 1, new IALoadInstruction()};
			case 0x2F: return {2, 1, new LALoadInstruction()};
			case 0x30: return {2, 1, new FALoadInstruction()};
			case 0x31: return {2, 1, new DALoadInstruction()};
			case 0x32: return {2, 1, new AALoadInstruction()};
			case 0x33: return {2, 1, new BALoadInstruction()};
			case 0x34: return {2, 1, new CALoadInstruction()};
			case 0x35: return {2, 1, new SALoadInstruction()};
			case 0x36: return {0, -1, new IStoreInstruction(nextUByte())};
			case 0x37: return {0, -1, new LStoreInstruction(nextUByte())};
			case 0x38: return {0, -1, new FStoreInstruction(nextUByte())};
			case 0x39: return {0, -1, new DStoreInstruction(nextUByte())};
			case 0x3A: return {0, -1, new AStoreInstruction(nextUByte())};
			case 0x3B: return {0, -1, new IStoreInstruction(0)};
			case 0x3C: return {0, -1, new IStoreInstruction(1)};
			case 0x3D: return {0, -1, new IStoreInstruction(2)};
			case 0x3E: return {0, -1, new IStoreInstruction(3)};
			case 0x3F: return {0, -1, new LStoreInstruction(0)};
			case 0x40: return {0, -1, new LStoreInstruction(1)};
			case 0x41: return {0, -1, new LStoreInstruction(2)};
			case 0x42: return {0, -1, new LStoreInstruction(3)};
			case 0x43: return {0, -1, new FStoreInstruction(0)};
			case 0x44: return {0, -1, new FStoreInstruction(1)};
			case 0x45: return {0, -1, new FStoreInstruction(2)};
			case 0x46: return {0, -1, new FStoreInstruction(3)};
			case 0x47: return {0, -1, new DStoreInstruction(0)};
			case 0x48: return {0, -1, new DStoreInstruction(1)};
			case 0x49: return {0, -1, new DStoreInstruction(2)};
			case 0x4A: return {0, -1, new DStoreInstruction(3)};
			case 0x4B: return {0, -1, new AStoreInstruction(0)};
			case 0x4C: return {0, -1, new AStoreInstruction(1)};
			case 0x4D: return {0, -1, new AStoreInstruction(2)};
			case 0x4E: return {0, -1, new AStoreInstruction(3)};
			case 0x4F: return {3, 0, new IAStoreInstruction()};
			case 0x50: return {3, 0, new LAStoreInstruction()};
			case 0x51: return {3, 0, new FAStoreInstruction()};
			case 0x52: return {3, 0, new DAStoreInstruction()};
			case 0x53: return {3, 0, new AAStoreInstruction()};
			case 0x54: return {3, 0, new BAStoreInstruction()};
			case 0x55: return {3, 0, new CAStoreInstruction()};
			case 0x56: return {3, 0, new SAStoreInstruction()};
			case 0x57: return {1, 0, new PopInstruction<TypeSize::FOUR_BYTES>()};
			case 0x58: return {1, 0, new PopInstruction<TypeSize::EIGHT_BYTES>()};
			case 0x59: return {1, 2, new Dup1Instruction()};
			case 0x5A: return {2, 3, new DupX1Instruction()};
			case 0x5B: return {3, 4, new DupX2Instruction()};
			case 0x5C: return {1, 2, new Dup2Instruction()};
			case 0x5D: return {2, 3, new Dup2X1Instruction()};
			case 0x5E: return {2, 3, new Dup2X2Instruction()};
			case 0x5F: return {2, 2, new SwapInstruction()};
			case 0x60: case 0x61: case 0x62: case 0x63: return {2, 1, new AddOperatorInstruction(current() & 3)};
			case 0x64: case 0x65: case 0x66: case 0x67: return {2, 1, new SubOperatorInstruction(current() & 3)};
			case 0x68: case 0x69: case 0x6A: case 0x6B: return {2, 1, new MulOperatorInstruction(current() & 3)};
			case 0x6C: case 0x6D: case 0x6E: case 0x6F: return {2, 1, new DivOperatorInstruction(current() & 3)};
			case 0x70: case 0x71: case 0x72: case 0x73: return {2, 1, new RemOperatorInstruction(current() & 3)};
			case 0x74: case 0x75: case 0x76: case 0x77: return {1, 1, new NegOperatorInstruction(current() & 3)};
			case 0x78: case 0x79: return {2, 1, new ShiftLeftOperatorInstruction(current() & 1)};
			case 0x7A: case 0x7B: return {2, 1, new ShiftRightOperatorInstruction(current() & 1)};
			case 0x7C: case 0x7D: return {2, 1, new UShiftRightOperatorInstruction(current() & 1)};
			case 0x7E: case 0x7F: return {2, 1, new AndOperatorInstruction(current() & 1)};
			case 0x80: case 0x81: return {2, 1, new OrOperatorInstruction(current() & 1)};
			case 0x82: case 0x83: return {2, 1, new XorOperatorInstruction(current() & 1)};
			case 0x84: return {0, 0, new IIncInstruction(nextUByte(), nextByte())};
			case 0x85: return {1, 1, new CastInstruction<false>(INT, LONG)}; // int -> long
			case 0x86: return {1, 1, new CastInstruction<false>(INT, FLOAT)}; // int -> float
			case 0x87: return {1, 1, new CastInstruction<false>(INT, DOUBLE)}; // int -> double
			case 0x88: return {1, 1, new CastInstruction<true>(LONG, INT)}; // long -> int
			case 0x89: return {1, 1, new CastInstruction<false>(LONG, FLOAT)}; // long -> float
			case 0x8A: return {1, 1, new CastInstruction<false>(LONG, DOUBLE)}; // long -> double
			case 0x8B: return {1, 1, new CastInstruction<true>(FLOAT, INT)}; // float -> int
			case 0x8C: return {1, 1, new CastInstruction<true>(FLOAT, LONG)}; // float -> long
			case 0x8D: return {1, 1, new CastInstruction<false>(FLOAT, DOUBLE)}; // float -> double
			case 0x8E: return {1, 1, new CastInstruction<true>(DOUBLE, INT)}; // double -> int
			case 0x8F: return {1, 1, new CastInstruction<true>(DOUBLE, LONG)}; // double -> long
			case 0x90: return {1, 1, new CastInstruction<true>(DOUBLE, FLOAT)}; // double -> float
			case 0x91: return {1, 1, new CastInstruction<true>(INT, BYTE)}; // int -> byte
			case 0x92: return {1, 1, new CastInstruction<true>(INT, CHAR)}; // int -> char
			case 0x93: return {1, 1, new CastInstruction<true>(INT, SHORT)}; // int -> short
			case 0x94:            return {2, 1, new LCmpInstruction()};
			case 0x95: case 0x96: return {2, 1, new FCmpInstruction()};
			case 0x97: case 0x98: return {2, 1, new DCmpInstruction()};
			case 0x99: return {1, 0, new IfEqInstruction(nextShort())};
			case 0x9A: return {1, 0, new IfNotEqInstruction(nextShort())};
			case 0x9B: return {1, 0, new IfLtInstruction(nextShort())};
			case 0x9C: return {1, 0, new IfGeInstruction(nextShort())};
			case 0x9D: return {1, 0, new IfGtInstruction(nextShort())};
			case 0x9E: return {1, 0, new IfLeInstruction(nextShort())};
			case 0x9F: return {1, 0, new IfIEqInstruction(nextShort())};
			case 0xA0: return {1, 0, new IfINotEqInstruction(nextShort())};
			case 0xA1: return {1, 0, new IfILtInstruction(nextShort())};
			case 0xA2: return {1, 0, new IfIGeInstruction(nextShort())};
			case 0xA3: return {1, 0, new IfIGtInstruction(nextShort())};
			case 0xA4: return {1, 0, new IfILeInstruction(nextShort())};
			case 0xA5: return {1, 0, new IfAEqInstruction(nextShort())};
			case 0xA6: return {1, 0, new IfANotEqInstruction(nextShort())};
			case 0xA7: return {0, 0, new GotoInstruction(nextShort())};
			/*case 0xA8: i+=2; return {0, 1, jsr};
			case 0xA9: i++ ; return {0, 0, ret};*/
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
				return {1, 0, new SwitchInstruction(defaultOffset, offsetTable)};
			}
			case 0xAB: {
				skip(3 - (pos & 0x3)); // alignment by 4 bytes
				offset_t defaultOffset = nextInt();
				map<int32_t, offset_t> offsetTable;
				for(uint32_t i = nextUInt(); i > 0; i--) {
					int32_t value = nextInt();
					offsetTable[value] = nextInt();
				}
				return {1, 0, new SwitchInstruction(defaultOffset, offsetTable)};
			}
			case 0xAC: return {1, 0, new IReturnInstruction()};
			case 0xAD: return {1, 0, new LReturnInstruction()};
			case 0xAE: return {1, 0, new FReturnInstruction()};
			case 0xAF: return {1, 0, new DReturnInstruction()};
			case 0xB0: return {1, 0, new AReturnInstruction()};
			case 0xB1: return {0, 0, VReturn::getInstance()};
			case 0xB2: return {0, 1, new GetStaticFieldInstruction(nextUShort())};
			case 0xB3: return {1, 0, new PutStaticFieldInstruction(nextUShort())};
			case 0xB4: return {1, 1, new GetInstanceFieldInstruction(nextUShort())};
			case 0xB5: return {2, 0, new PutInstanceFieldInstruction(nextUShort())};
			case 0xB6: return /*TODO*/{0, 0, new InvokevirtualInstruction(nextUShort())};
			case 0xB7: return {0, 0, new InvokespecialInstruction(nextUShort())};
			case 0xB8: return {0, 0, new InvokestaticInstruction(nextUShort())};
			case 0xB9: return {0, 0, new InvokeinterfaceInstruction(nextUShort(), nextUByte(), nextUByte(), *this)};
			case 0xBA: return /*TODO*/{0, 0, new InvokedynamicInstruction(nextUShort(), nextUShort(), *this)};
			case 0xBB: return {0, 1, new NewInstruction(nextUShort())};
			case 0xBC: return {1, 1, new NewArrayInstruction(nextUByte())};
			case 0xBD: return {1, 1, new ANewArrayInstruction(nextUShort())};
			case 0xBE: return {1, 1, new ArrayLengthInstruction()};
			case 0xBF: return {1, 0, new AThrowInstruction()};
			case 0xC0: return {1, 1, new CheckCastInstruction(nextUShort())};
			case 0xC1: return {1, 1, new InstanceofInstruction(nextUShort())};
			case 0xC2: return {1, 0, nullptr}; // TODO: MonitorEnter
			case 0xC3: return {1, 0, nullptr}; // TODO: MonitorExit
			case 0xC4: switch(nextUByte()) {
				case 0x15: return {0, 1, new ILoadInstruction(nextUShort())};
				case 0x16: return {0, 1, new LLoadInstruction(nextUShort())};
				case 0x17: return {0, 1, new FLoadInstruction(nextUShort())};
				case 0x18: return {0, 1, new DLoadInstruction(nextUShort())};
				case 0x19: return {0, 1, new ALoadInstruction(nextUShort())};
				case 0x36: return {1, 0, new IStoreInstruction(nextUShort())};
				case 0x37: return {1, 0, new LStoreInstruction(nextUShort())};
				case 0x38: return {1, 0, new FStoreInstruction(nextUShort())};
				case 0x39: return {1, 0, new DStoreInstruction(nextUShort())};
				case 0x3A: return {1, 0, new AStoreInstruction(nextUShort())};
				case 0x84: return {0, 0, new IIncInstruction(nextUShort(), nextShort())};
				//case 0xA9: i+=2 ; return {0, 0, ret};
				default: throw IllegalOpcodeError("Illegal wide opcode " + hexWithPrefix(current()));
			}
			case 0xC5: { uint8_t dimensions = nextUByte();
			  return {dimensions, 1, new MultiANewArrayInstruction(nextUShort(), dimensions)}; }
			case 0xC6: return {1, 0, new IfNullInstruction(nextShort())};
			case 0xC7: return {1, 0, new IfNonNullInstruction(nextShort())};
			case 0xC8: return {0, 0, new GotoInstruction(nextInt())};
			/*case 0xC9: i+=4; return {0, 1, jsr_w};
			case 0xCA: return {0, 0, breakpoint};
			case 0xFE: return {0, 0, impdep1};
			case 0xFF: return {0, 0, impdep2};*/
			default:
				throw IllegalOpcodeError(hexWithPrefix<2>(current()));
		}
	}

	const StringifyContext& Method::decompileCode(const ClassInfo& classinfo) {
		using namespace operations;
		using namespace instructions;

		log("decompiling of", descriptor.toString());

		const bool hasCodeAttribute = codeAttribute != nullptr;
		const bool isNonStatic = !(modifiers & ACC_STATIC);

		const uint32_t methodScopeEndPos = hasCodeAttribute ? codeAttribute->codeLength : 0;
		uint16_t localsCount;

		if(hasCodeAttribute) {
			localsCount = codeAttribute->maxLocals;
		} else {
			localsCount = isNonStatic ? 1 : 0;

			for(const Type* arg : descriptor.arguments) {
				localsCount += arg->getSize() != TypeSize::EIGHT_BYTES ? 1 : 2;
			}
		}

		MethodScope* methodScope = descriptor.type == MethodDescriptor::MethodType::STATIC_INITIALIZER ?
				new StaticInitializerScope(0, methodScopeEndPos, localsCount) : new MethodScope(0, methodScopeEndPos, localsCount);

		if(isNonStatic)
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
			return *new StringifyContext(*new DisassemblerContext(classinfo.constPool, 0, (const uint8_t*)""),
					classinfo, methodScope, modifiers, descriptor, attributes);

		DisassemblerContext& disassemblerContext = *new DisassemblerContext(classinfo.constPool, codeAttribute->codeLength, codeAttribute->code);

		DecompilationContext& decompilationContext =
				*new DecompilationContext(disassemblerContext, classinfo, methodScope, modifiers, descriptor, attributes, codeAttribute->maxLocals);

		StringifyContext& stringifyContext = *new StringifyContext(decompilationContext);
		decompilationContext.stringifyContext = &stringifyContext;

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
