#include "cpu/cpu.h"
#include "memory/memory.h"

/* Global CPU */
CPU cpu;

/* --- Stack operations --- */
static void pushb(BYTE val) {
	Memory_WriteByte(MAP_CPU, (STACK_ADDR | cpu.SP--), val);
}

static void pushw(WORD val) {
	pushb((val >> 8) & 0xFF);
	pushb(val & 0xFF);
}

static BYTE pullb() {
	return Memory_ReadByte(MAP_CPU, (STACK_ADDR | ++cpu.SP));
}

static WORD pullw() {
	return (WORD)pullb() | ((WORD)pullb() << 8);
}

static void (*mode)();

/* --- Addressing mode functions --- */
#include "addressing.partial.c"

/* --- CPU instruction functions --- */
#include "instructions.partial.c"

/* --- Disassembler --- */
#ifdef DEBUG_MODE
#include "disassembler.partial.c"
#endif

/* Addressing mode function pointer table for fetching operands; indexed by opcode */
static void (*addr[256])() = {
/*    | 0  | 1  | 2  | 3  | 4  | 5  | 6  | 7  | 8  | 9  | A  | B  | C  | D  | E  | F  |*/
/* 0 */ IMP, IDX, IMP, IMM, ZPG, ZPG, ZPG, ZPG, IMP, IMM, ACC, IMP, ABS, ABS, ABS, REL,
/* 1 */ REL, IDY, IND, IMM, ZPG, ZPX, ZPX, ZPG, IMP, ABY, IMP, IMP, ABS, ABX, ABX, REL,
/* 2 */ ABS, IDX, IMP, IMM, ZPG, ZPG, ZPG, ZPG, IMP, IMM, ACC, IMP, ABS, ABS, ABS, REL,
/* 3 */ REL, IDY, IND, IMP, ZPX, ZPX, ZPX, ZPG, IMP, ABY, IMP, IMP, ABX, ABX, ABX, REL,
/* 4 */ IMP, IDX, IMP, IMM, REL, ZPG, ZPG, ZPG, IMP, IMM, ACC, IMP, ABS, ABS, ABS, REL,
/* 5 */ REL, IDY, IND, IMM, IMP, ZPX, ZPX, ZPG, IMP, ABY, IMP, IMP, IMP, ABX, ABX, REL,
/* 6 */ IMP, IDX, IMP, IMP, ZPG, ZPG, ZPG, ZPG, IMP, IMM, ACC, IMP, IND, ABS, ABS, REL,
/* 7 */ REL, IDY, IND, BLK, ZPX, ZPX, ZPX, ZPG, IMP, ABY, IMP, IMP, IDX, ABX, ABX, REL,
/* 8 */ REL, IDX, IMM, ZPG, ZPG, ZPG, ZPG, ZPG, IMP, IMM, IMP, IMP, ABS, ABS, ABS, REL,
/* 9 */ REL, IDY, IND, ABS, ZPX, ZPX, ZPY, ZPG, IMP, ABY, IMP, IMP, ABS, ABX, ABX, REL,
/* A */ IMM, IDX, IMM, ZPX, ZPG, ZPG, ZPG, ZPG, IMP, IMM, IMP, IMP, ABS, ABS, ABS, REL,
/* B */ REL, IDY, IND, ABX, ZPX, ZPX, ZPY, ZPG, IMP, ABY, IMP, IMP, ABX, ABX, ABY, REL,
/* C */ IMM, IDX, IMP, IDX, ZPG, ZPG, ZPG, ZPG, IMP, IMM, IMP, IMP, ABS, ABS, ABS, REL,
/* D */ REL, IDY, IND, IDY, IMP, ZPX, ZPX, ZPG, IMP, ABY, IMP, IMP, IMP, ABX, ABX, REL,
/* E */ IMM, IDX, IMP, IDX, ZPG, ZPG, ZPG, ZPG, IMP, IMM, IMP, IMP, ABS, ABS, ABS, REL,
/* F */ REL, IDY, IND, IDY, IMP, ZPX, ZPX, ZPG, IMP, ABY, IMP, IMP, IMP, ABX, ABX, REL
};

/* CPU instruction function pointer table; indexed by opcode */
static void (*instr[256])() = {
/*    | 0  | 1  | 2  | 3  | 4  | 5  | 6  | 7   | 8  | 9  | A   | B  | C  | D  | E  | F  |*/
/* 0 */ BRK, ORA, SXY, ST0, TSB, ORA, ASL, RMB0, PHP, ORA, ASLA, NOP, TSB, ORA, ASL, BBR0,
/* 1 */ BPL, ORA, ORA, ST1, TRB, ORA, ASL, RMB1, CLC, ORA, INC,  NOP, TRB, ORA, ASL, BBR1,
/* 2 */ JSR, AND, SAX, ST2, BIT, AND, ROL, RMB2, PLP, AND, ROLA, NOP, BIT, AND, ROL, BBR2,
/* 3 */ BMI, AND, AND, NOP, BIT, AND, ROL, RMB3, SEC, AND, DEC,  NOP, BIT, AND, ROL, BBR3,
/* 4 */ RTI, EOR, SAY, TMA, BSR, EOR, LSR, RMB4, PHA, EOR, LSRA, NOP, JMP, EOR, LSR, BBR4,
/* 5 */ BVC, EOR, EOR, TAM, CSL, EOR, LSR, RMB5, CLI, EOR, PHY,  NOP, NOP, EOR, LSR, BBR5,
/* 6 */ RTS, ADC, CLA, NOP, STZ, ADC, ROR, RMB6, PLA, ADC, RORA, NOP, JMP, ADC, ROR, BBR6,
/* 7 */ BVS, ADC, ADC, TII, STZ, ADC, ROR, RMB7, SEI, ADC, PLY,  NOP, JMP, ADC, ROR, BBR7,
/* 8 */ BRA, STA, CLX, TST, STY, STA, STX, SMB0, DEY, BIT, TXA,  NOP, STY, STA, STX, BBS0,
/* 9 */ BCC, STA, STA, TST, STY, STA, STX, SMB1, TYA, STA, TXS,  NOP, STZ, STA, STZ, BBS1,
/* A */ LDY, LDA, LDX, TST, LDY, LDA, LDX, SMB2, TAY, LDA, TAX,  NOP, LDY, LDA, LDX, BBS2,
/* B */ BCS, LDA, LDA, TST, LDY, LDA, LDX, SMB3, CLV, LDA, TSX,  NOP, LDY, LDA, LDX, BBS3,
/* C */ CPY, CMP, CLY, TDD, CPY, CMP, DEC, SMB4, INY, CMP, DEX,  NOP, CPY, CMP, DEC, BBS4,
/* D */ BNE, CMP, CMP, TIN, CSH, CMP, DEC, SMB5, CLD, CMP, PHX,  NOP, NOP, CMP, DEC, BBS5,
/* E */ CPX, SBC, NOP, TIA, CPX, SBC, INC, SMB6, INX, SBC, NOP,  NOP, CPX, SBC, INC, BBS6,
/* F */ BEQ, SBC, SBC, TAI, SET, SBC, INC, SMB7, SED, SBC, PLX,  NOP, NOP, SBC, INC, BBS7
};

/* Clock cycle table; indexed by opcode */
static const DWORD cycles[256] = {
/*    | 0  | 1  | 2  | 3  | 4  | 5  | 6  | 7  | 8  | 9  | A  | B  | C  | D  | E  | F  |*/
/* 0 */ 7,   6,   2,   8,   3,   3,   5,   5,   3,   2,   2,   2,   4,   4,   6,   6,
/* 1 */ 2,   5,   2,   8,   4,   4,   6,   6,   2,   4,   2,   7,   4,   4,   7,   7,
/* 2 */ 6,   6,   2,   8,   3,   3,   5,   5,   4,   2,   2,   2,   4,   4,   6,   6,
/* 3 */ 2,   5,   2,   8,   4,   4,   6,   6,   2,   4,   2,   7,   4,   4,   7,   7,
/* 4 */ 6,   6,   2,   8,   3,   3,   5,   5,   3,   2,   2,   2,   3,   4,   6,   6,
/* 5 */ 2,   5,   2,   8,   4,   4,   6,   6,   2,   4,   2,   7,   4,   4,   7,   7,
/* 6 */ 6,   6,   2,   8,   3,   3,   5,   5,   4,   2,   2,   2,   5,   4,   6,   6,
/* 7 */ 2,   5,   2,   8,   4,   4,   6,   6,   2,   4,   2,   7,   4,   4,   7,   7,
/* 8 */ 2,   6,   2,   6,   3,   3,   3,   3,   2,   2,   2,   2,   4,   4,   4,   4,
/* 9 */ 2,   6,   2,   6,   4,   4,   4,   4,   2,   5,   2,   5,   5,   5,   5,   5,
/* A */ 2,   6,   2,   6,   3,   3,   3,   3,   2,   2,   2,   2,   4,   4,   4,   4,
/* B */ 2,   5,   2,   5,   4,   4,   4,   4,   2,   4,   2,   4,   4,   4,   4,   4,
/* C */ 2,   6,   2,   8,   3,   3,   5,   5,   2,   2,   2,   2,   4,   4,   6,   6,
/* D */ 2,   5,   2,   8,   4,   4,   6,   6,   2,   4,   2,   7,   4,   4,   7,   7,
/* E */ 2,   6,   2,   8,   3,   3,   5,   5,   2,   2,   2,   2,   4,   4,   6,   6,
/* F */ 2,   5,   2,   8,   4,   4,   6,   6,   2,   4,   2,   7,   4,   4,   7,   7
};

/* Interrupt functions */
/* TODO: Interrupt hijacking */
void CPU_Interrupt_IRQ() {
	CLEAR_FLAG(FLAG_B);
	pushw(cpu.PC);
	pushb(cpu.S);
	SET_FLAG(FLAG_I);
	cpu.PC = Memory_ReadWord(MAP_CPU, 0xFFFE);
}

void CPU_Interrupt_NMI() {
	CLEAR_FLAG(FLAG_B);
	pushw(cpu.PC);
	pushb(cpu.S);
	SET_FLAG(FLAG_I);
	cpu.PC = Memory_ReadWord(MAP_CPU, 0xFFFA);
}

void CPU_Interrupt_RESET() {
	cpu.PC = Memory_ReadWord(MAP_CPU, 0xFFFC);
	cpu.SP = 0xFD;
	cpu.S |= 0x24;
}

/* Re-initialize all CPU registers and variables */
void CPU_Reset() {
	cpu.PC = Memory_ReadWord(MAP_CPU, 0xFFFC);
	cpu.A = 0;
	cpu.X = 0;
	cpu.Y = 0;
	cpu.SP = 0xFD;
	cpu.S = 0x24;
	cpu.opcode = 0;
	cpu.operaddr = 0;
	cpu.indoperaddr = 0;
	cpu.indoperand = 0;
	cpu.operand = 0;
	cpu.cycles = 0;
	cpu.suspended = 0;
}

void CPU_Suspend(DWORD duration) {
	cpu.suspended = duration;
}

/* Execute one CPU instruction */
DWORD CPU_Step() {
	if (cpu.suspended) {
		cpu.suspended--;
		return 1;
	}

	DWORD lastcycles = cpu.cycles;

	/* Check for interrupts */
	switch(cpu.interrupt) {
		case IRQ:
			if(!(GET_FLAG(FLAG_I))){
				CPU_Interrupt_IRQ();
				cpu.interrupt = NONE;
			}
			break;
		case NMI:
			CPU_Interrupt_NMI();
			cpu.interrupt = NONE;
			break;
		case RESET:
			CPU_Interrupt_RESET();
			cpu.interrupt = NONE;
			break;
	}

	/* Fetch opcode */
	cpu.opcode = Memory_ReadByte(MAP_CPU, cpu.PC++);

	/* Clear T flag unless it is set and the next instruction uses it */
	if (GET_FLAG(FLAG_T)) {
		switch (cpu.opcode) {
			case 0x69:
			case 0x09:
			case 0x49:
			case 0x29:
				cpu.taddr = ((WORD)Memory_ReadByte(MAP_CPU, cpu.PC++) + (WORD)cpu.X) & 0xFF;
				cpu.tval = Memory_ReadByte(MAP_CPU, cpu.taddr);
				break;
			default:
				CLEAR_FLAG(FLAG_T);
				cpu.tval = cpu.A;
				break;
		}
	} else {
		cpu.tval = cpu.A;
	}

#ifdef DEBUG_MODE
	disassemble1();
#endif

	/* Fetch operand address */
	mode = addr[cpu.opcode];
	(*mode)();

	/* Cache operand value */
	if (mode != ACC && mode != IMP)
		cpu.operand = Memory_ReadByte(MAP_CPU, cpu.operaddr);
	else
		cpu.operand = 0;

#ifdef DEBUG_MODE
	disassemble2();
#endif

	/* Execute CPU instruction */
	(*instr[cpu.opcode])();

	/* Add cycles to the total cycle count */
	cpu.cycles += cycles[cpu.opcode];

	return cpu.cycles - lastcycles;
}
