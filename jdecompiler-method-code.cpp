#ifndef JDECOMPILER_METHOD_CODE_CPP
#define JDECOMPILER_METHOD_CODE_CPP

#include <string>
#include <map>
#include "jdecompiler.h"
#include "jdecompiler-util.cpp"
#include "jdecompiler-const-pool.cpp"
#include "jdecompiler-main.cpp"

using namespace std;

namespace JDecompiler {


	#include "jdecompiler-operations.cpp"
	#include "jdecompiler-instructions.cpp"


	Instruction* Bytecode::nextInstruction0() {
		using namespace Instructions;
		//cout << hex << current() << dec << endl; // DEBUG
		switch(current()) {
			case 0x00: return nullptr;
			case 0x01: return &AConstNull::getInstance();
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
			case 0x12: return new LdcInstruction(nextByte());
			case 0x13: return new LdcInstruction(nextShort());
			case 0x14: return new Ldc2Instruction(nextShort());
			case 0x15: return new ILoadInstruction(nextByte());
			case 0x16: return new LLoadInstruction(nextByte());
			case 0x17: return new FLoadInstruction(nextByte());
			case 0x18: return new DLoadInstruction(nextByte());
			case 0x19: return new ALoadInstruction(nextByte());
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
			case 0x36: return new IStoreInstruction(nextByte());
			case 0x37: return new LStoreInstruction(nextByte());
			case 0x38: return new FStoreInstruction(nextByte());
			case 0x39: return new DStoreInstruction(nextByte());
			case 0x3A: return new AStoreInstruction(nextByte());
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
			case 0x57: return new PopInstruction();
			case 0x58: return new PopInstruction(); // pop2
			case 0x59: return new DupInstruction();
			/*case 0x5A: return "DUP_X1";
			case 0x5B: return "DUP_X2";
			case 0x5C: return "DUP2";
			case 0x5D: return "DUP2_X1";
			case 0x5E: return "DUP2_X2";*/
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
			case 0x84: return new IIncInstruction(nextByte(), nextByte());
			case 0x85: return new CastInstruction(LONG, false); // int -> long
			case 0x86: return new CastInstruction(FLOAT, false); // int -> float
			case 0x87: return new CastInstruction(DOUBLE, false); // int -> double
			case 0x88: return new CastInstruction(INT, true); // long -> int
			case 0x89: return new CastInstruction(FLOAT, false); // long -> float
			case 0x8A: return new CastInstruction(DOUBLE, false); // long -> double
			case 0x8B: return new CastInstruction(INT, true); // float -> int
			case 0x8C: return new CastInstruction(LONG, true); // float -> long
			case 0x8D: return new CastInstruction(DOUBLE, false); // float -> double
			case 0x8E: return new CastInstruction(INT, true); // double -> int
			case 0x8F: return new CastInstruction(LONG, true); // double -> long
			case 0x90: return new CastInstruction(FLOAT, true); // double -> float
			case 0x91: return new CastInstruction(BYTE, true); // int -> byte
			case 0x92: return new CastInstruction(CHAR, true); // int -> char
			case 0x93: return new CastInstruction(SHORT, true); // int -> short
			case 0x94: return new LCmpInstruction();
			case 0x95: return new FCmpInstruction();
			case 0x96: return new FCmpInstruction();
			case 0x97: return new DCmpInstruction();
			case 0x98: return new DCmpInstruction();
			case 0x99: return new IfNotEqInstruction(nextShort());
			case 0x9A: return new IfEqInstruction(nextShort());
			case 0x9B: return new IfGeInstruction(nextShort());
			case 0x9C: return new IfLtInstruction(nextShort());
			case 0x9D: return new IfLeInstruction(nextShort());
			case 0x9E: return new IfGtInstruction(nextShort());
			case 0x9F: return new IfINotEqInstruction(nextShort());
			case 0xA0: return new IfIEqInstruction(nextShort());
			case 0xA1: return new IfIGeInstruction(nextShort());
			case 0xA2: return new IfILtInstruction(nextShort());
			case 0xA3: return new IfILeInstruction(nextShort());
			case 0xA4: return new IfIGtInstruction(nextShort());
			case 0xA5: return new IfANotEqInstruction(nextShort());
			case 0xA6: return new IfAEqInstruction(nextShort());
			case 0xA7: return new GotoInstruction(nextShort());
			/*case 0xA8: i+=2; return "JSR";
			case 0xA9: i++ ; return "RET";*/
			//case 0xAA: return "TABLESWITCH";
			case 0xAB: {
				skip(3 - pos % 4); // alignment by 4 bytes
				int32_t defaultOffset = nextInt();
				map<int32_t, int32_t> offsetTable;
				for(uint32_t i = nextInt(); i > 0; i--) {
					int32_t value = nextInt(), offset = nextInt();
					offsetTable[value] = offset;
				}
				return new LookupswitchInstruction(defaultOffset, offsetTable);
			}
			case 0xAC: case 0xAD: case 0xAE: case 0xAF: case 0xB0: return new ReturnInstruction();
			case 0xB1: return &VReturn::getInstance();
			case 0xB2: return new GetstaticInstruction(nextShort());
			case 0xB3: return new PutstaticInstruction(nextShort());
			case 0xB4: return new GetfieldInstruction(nextShort());
			case 0xB5: return new PutfieldInstruction(nextShort());
			case 0xB6: return new InvokevirtualInstruction(nextShort());
			case 0xB7: return new InvokespecialInstruction(nextShort());
			case 0xB8: return new InvokestaticInstruction(nextShort());
			case 0xB9: {
				Instruction* const instruction = new InvokeinterfaceInstruction(nextShort());
				nextShort();
				return instruction;
			}
			//case 0xBA: i+=4; return "INVOKEDYNAMIC";
			case 0xBB: return new NewInstruction(nextShort());
			case 0xBC: return new NewArrayInstruction(nextByte());
			case 0xBD: return new ANewArrayInstruction(nextShort());
			case 0xBE: return new ArrayLengthInstruction();
			case 0xBF: return new AThrowInstruction();
			case 0xC0: return new CheckCastInstruction(nextShort());
			case 0xC1: return new InstanceofInstruction(nextShort());
			/*case 0xC2: return "MONITORENTER";
			case 0xC3: return "MONITOREXIT";*/
			case 0xC4: switch(nextByte()) {
				case 0x15: return new ILoadInstruction(nextShort());
				case 0x16: return new LLoadInstruction(nextShort());
				case 0x17: return new FLoadInstruction(nextShort());
				case 0x18: return new DLoadInstruction(nextShort());
				case 0x19: return new ALoadInstruction(nextShort());
				case 0x36: return new IStoreInstruction(nextShort());
				case 0x37: return new LStoreInstruction(nextShort());
				case 0x38: return new FStoreInstruction(nextShort());
				case 0x39: return new DStoreInstruction(nextShort());
				case 0x3A: return new AStoreInstruction(nextShort());
				case 0x84: return new IIncInstruction(nextShort(), nextShort());
				//case 0xA9: i+=2 ; return "RET";
				default: throw IllegalOpcodeException("Illegal wide opcode: 0x" + hex(current()));
			}
			case 0xC5: return new MultiANewArrayInstruction(nextShort(), nextByte());
			case 0xC6: return new IfNonNullInstruction(nextShort());
			case 0xC7: return new IfNullInstruction(nextShort());
			case 0xC8: return new GotoInstruction(nextInt());
			/*case 0xC9: i+=4; return "JSR_W";
			case 0xCA: return "BREAKPOINT";
			case 0xFE: return "IMPDEP1";
			case 0xFF: return "IMPDEP2";*/
			default:
				throw IllegalOpcodeException("0x" + hex<2>(current()));
		}
	}

	static string decompileCode(const ConstantPool& constPool, const CodeAttribute* attribute, Scope* scope, const ClassInfo& classinfo) {
		using namespace Operations;
		using namespace Instructions;

		//cout << "decompileCode" << endl; // DEBUG
		Bytecode bytecode = Bytecode(attribute->codeLength, attribute->code);

		CodeEnvironment environment = CodeEnvironment(bytecode, constPool, scope, attribute->codeLength, attribute->maxLocals, classinfo);

		while(bytecode.available()) {
			bytecode.nextInstruction();
			bytecode.nextByte();
		}

		const vector<Instruction*>& instructions = bytecode.getInstructions();
		const uint32_t instructionsSize = instructions.size();

		for(uint32_t i = 0; i < instructionsSize; i++) {
			environment.index = i;
			environment.pos = bytecode.getPosMap()[i];
			const Operation* operation = instructions[i]->toOperation(environment);

			if(operation->getReturnType() != VOID)
				environment.stack->push(operation);
			else if(!dynamic_cast<const Scope*>(operation) && (i != instructionsSize - 1 || operation != &VReturn::getInstance()))
				environment.currentScope->add(operation);

			if(Scope* scope = const_cast<Scope*>(dynamic_cast<const Scope*>(operation)))
				environment.addScope(scope);

			environment.checkCurrentScope();
		}

		//cout << "decompileCode end" << endl; // DEBUG

		return environment.scope->toString(environment);
	}
}

#endif
