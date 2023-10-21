#include "dis.h"

struct info optbl[256] = {
	/* 00 */	{ "BRK", 1, IMP|STOP, },
	/* 01 */	{ "ORA", 2, INX|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 05 */	{ "ORA", 2, ZPG|NORM, },
	/* 06 */	{ "ASL", 2, ZPG|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 08 */	{ "PHP", 1, IMP|NORM, },
	/* 09 */	{ "ORA", 2, IMM|NORM, },
	/* 0a */	{ "ASL", 1, ACC|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 0d */	{ "ORA", 3, ABS|NORM, },
	/* 0e */	{ "ASL", 3, ABS|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 10 */	{ "BPL", 2, REL|FORK, },
	/* 11 */	{ "ORA", 2, INY|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 15 */	{ "ORA", 2, ZPX|NORM, },
	/* 16 */	{ "ASL", 2, ZPX|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 18 */	{ "CLC", 1, IMP|NORM, },
	/* 19 */	{ "ORA", 3, ABY|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 1d */	{ "ORA", 3, ABX|NORM, },
	/* 1e */	{ "ASL", 3, ABX|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 20 */	{ "JSR", 3, ABS|FORK, },
	/* 21 */	{ "AND", 2, INX|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 24 */	{ "BIT", 2, ZPG|NORM, },
	/* 25 */	{ "AND", 2, ZPG|NORM, },
	/* 26 */	{ "ROL", 2, ZPG|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 28 */	{ "PLP", 1, IMP|NORM, },
	/* 29 */	{ "AND", 2, IMM|NORM, },
	/* 2a */	{ "ROL", 1, ACC|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 2c */	{ "BIT", 3, ABS|NORM, },
	/* 2d */	{ "AND", 3, ABS|NORM, },
	/* 2e */	{ "ROL", 3, ABS|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 30 */	{ "BMI", 2, REL|FORK, },
	/* 31 */	{ "AND", 2, INY|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 35 */	{ "AND", 2, ZPX|NORM, },
	/* 36 */	{ "ROL", 2, ZPX|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 38 */	{ "SEC", 1, IMP|NORM, },
	/* 39 */	{ "AND", 3, ABY|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 3d */	{ "AND", 3, ABX|NORM, },
	/* 3e */	{ "ROL", 3, ABX|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 40 */	{ "RTI", 1, IMP|STOP, },
	/* 41 */	{ "EOR", 2, INX|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 45 */	{ "EOR", 2, ZPG|NORM, },
	/* 46 */	{ "LSR", 2, ZPG|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 48 */	{ "PHA", 1, IMP|NORM, },
	/* 49 */	{ "EOR", 2, IMM|NORM, },
	/* 4a */	{ "LSR", 1, ACC|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 4c */	{ "JMP", 3, ABS|JUMP, },
	/* 4d */	{ "EOR", 3, ABS|NORM, },
	/* 4e */	{ "LSR", 3, ABS|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 50 */	{ "BVC", 2, REL|FORK, },
	/* 51 */	{ "EOR", 2, INY|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 55 */	{ "EOR", 2, ZPX|NORM, },
	/* 56 */	{ "LSR", 2, ZPX|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 58 */	{ "CLI", 1, IMP|NORM, },
	/* 59 */	{ "EOR", 3, ABY|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 5d */	{ "EOR", 3, ABX|NORM, },
	/* 5e */	{ "LSR", 3, ABX|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 60 */	{ "RTS", 1, IMP|STOP, },
	/* 61 */	{ "ADC", 2, INX|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 65 */	{ "ADC", 2, ZPG|NORM, },
	/* 66 */	{ "ROR", 2, ZPG|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 68 */	{ "PLA", 1, IMP|NORM, },
	/* 69 */	{ "ADC", 2, IMM|NORM, },
	/* 6a */	{ "ROR", 1, ACC|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 6c */	{ "JMP", 3, IND|STOP, },
	/* 6d */	{ "ADC", 3, ABS|NORM, },
	/* 6e */	{ "ROR", 3, ABS|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 70 */	{ "BVS", 2, REL|FORK, },
	/* 71 */	{ "ADC", 2, INY|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 75 */	{ "ADC", 2, ZPX|NORM, },
	/* 76 */	{ "ROR", 2, ZPX|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 78 */	{ "SEI", 1, IMP|NORM, },
	/* 79 */	{ "ADC", 3, ABY|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 7d */	{ "ADC", 3, ABX|NORM, },
	/* 7e */	{ "ROR", 3, ABX|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 81 */	{ "STA", 2, INX|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 84 */	{ "STY", 2, ZPG|NORM, },
	/* 85 */	{ "STA", 2, ZPG|NORM, },
	/* 86 */	{ "STX", 2, ZPG|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 88 */	{ "DEY", 1, IMP|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 8a */	{ "TXA", 1, IMP|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 8c */	{ "STY", 3, ABS|NORM, },
	/* 8d */	{ "STA", 3, ABS|NORM, },
	/* 8e */	{ "STX", 3, ABS|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 90 */	{ "BCC", 2, REL|FORK, },
	/* 91 */	{ "STA", 2, INY|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 94 */	{ "STY", 2, ZPX|NORM, },
	/* 95 */	{ "STA", 2, ZPX|NORM, },
	/* 96 */	{ "STX", 2, ZPY|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 98 */	{ "TYA", 1, IMP|NORM, },
	/* 99 */	{ "STA", 3, ABY|NORM, },
	/* 9a */	{ "TXS", 1, IMP|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 9d */	{ "STA", 3, ABX|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* a0 */	{ "LDY", 2, IMM|NORM, },
	/* a1 */	{ "LDA", 2, INX|NORM, },
	/* a2 */	{ "LDX", 2, IMM|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* a4 */	{ "LDY", 2, ZPG|NORM, },
	/* a5 */	{ "LDA", 2, ZPG|NORM, },
	/* a6 */	{ "LDX", 2, ZPG|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* a8 */	{ "TAY", 1, IMP|NORM, },
	/* a9 */	{ "LDA", 2, IMM|NORM, },
	/* aa */	{ "TAX", 1, IMP|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* ac */	{ "LDY", 3, ABS|NORM, },
	/* ad */	{ "LDA", 3, ABS|NORM, },
	/* ae */	{ "LDX", 3, ABS|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* b0 */	{ "BCS", 2, REL|FORK, },
	/* b1 */	{ "LDA", 2, INY|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* b4 */	{ "LDY", 2, ZPX|NORM, },
	/* b5 */	{ "LDA", 2, ZPX|NORM, },
	/* b6 */	{ "LDX", 2, ZPY|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* b8 */	{ "CLV", 1, IMP|NORM, },
	/* b9 */	{ "LDA", 3, ABY|NORM, },
	/* ba */	{ "TSX", 1, IMP|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* bc */	{ "LDY", 3, ABX|NORM, },
	/* bd */	{ "LDA", 3, ABX|NORM, },
	/* be */	{ "LDX", 3, ABY|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* c0 */	{ "CPY", 2, IMM|NORM, },
	/* c1 */	{ "CMP", 2, INX|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* c4 */	{ "CPY", 2, ZPG|NORM, },
	/* c5 */	{ "CMP", 2, ZPG|NORM, },
	/* c6 */	{ "DEC", 2, ZPG|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* c8 */	{ "INY", 1, IMP|NORM, },
	/* c9 */	{ "CMP", 2, IMM|NORM, },
	/* ca */	{ "DEX", 1, IMP|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* cc */	{ "CPY", 3, ABS|NORM, },
	/* cd */	{ "CMP", 3, ABS|NORM, },
	/* ce */	{ "DEC", 3, ABS|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* d0 */	{ "BNE", 2, REL|FORK, },
	/* d1 */	{ "CMP", 2, INY|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* d5 */	{ "CMP", 2, ZPX|NORM, },
	/* d6 */	{ "DEC", 2, ZPX|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* d8 */	{ "CLD", 1, IMP|NORM, },
	/* d9 */	{ "CMP", 3, ABY|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* dd */	{ "CMP", 3, ABX|NORM, },
	/* de */	{ "DEC", 3, ABX|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* e0 */	{ "CPX", 2, IMM|NORM, },
	/* e1 */	{ "SBC", 2, INX|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* e4 */	{ "CPX", 2, ZPG|NORM, },
	/* e5 */	{ "SBC", 2, ZPG|NORM, },
	/* e6 */	{ "INC", 2, ZPG|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* e8 */	{ "INX", 1, IMP|NORM, },
	/* e9 */	{ "SBC", 2, IMM|NORM, },
	/* ea */	{ "NOP", 1, IMP|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* ec */	{ "CPX", 3, ABS|NORM, },
	/* ed */	{ "SBC", 3, ABS|NORM, },
	/* ee */	{ "INC", 3, ABS|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* f0 */	{ "BEQ", 2, REL|FORK, },
	/* f1 */	{ "SBC", 2, INY|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* f5 */	{ "SBC", 2, ZPX|NORM, },
	/* f6 */	{ "INC", 2, ZPX|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* f8 */	{ "SED", 1, IMP|NORM, },
	/* f9 */	{ "SBC", 3, ABY|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
	/* fd */	{ "SBC", 3, ABX|NORM, },
	/* fe */	{ "INC", 3, ABX|NORM, },
	/* 00 */	{ "???", 1, ILL|NORM, },
};