
int _Byte[256]  = {            /* byte table */

/*	0  1  2  3    4  5  6  7    8  9  A  B    C  D  E  F  */

	0, 0, 0, 1,   1, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,
	0, 0, 2, 1,   1, 0, 2, 0,   0, 0, 0, 0,   0, 0, 0, 0,
	0, 0, 0, 1,   1, 0, 2, 0,   0, 0, 0, 0,   0, 0, 0, 0,
	0, 0, 2, 0,   1, 0, 2, 0,   0, 0, 0, 0,   0, 0, 0, 0,

	0, 0, 0, 1,   1, 0, 2, 0,   0, 0, 0, 0,   0, 0, 0, 0,
	0, 0, 2, 1,   1, 0, 2, 0,   0, 0, 0, 0,   0, 0, 0, 0,
	0, 0, 0, 0,   1, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,
	0, 0, 2, 0,   1, 0, 2, 0,   0, 0, 0, 0,   0, 0, 0, 0,

	0, 0, 0, 0,   1, 0, 2, 0,   1, 1, 1, 0,   0, 0, 0, 0,
	0, 0, 2, 0,   1, 0, 2, 0,   1, 1, 1, 0,   0, 0, 0, 0,
	0, 0, 0, 0,   1, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,
	1, 1, 2, 0,   1, 0, 2, 0,   1, 1, 1, 1,   1, 1, 1, 1,

	0, 0, 0, 0,   1, 0, 2, 0,   0, 0, 0, 0,   0, 0, 0, 0,
	0, 0, 2, 1,   1, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,
	0, 0, 0, 0,   1, 0, 2, 0,   2, 2, 2, 2,   2, 2, 2, 2,
	0, 0, 2, 0,   1, 0, 2, 0,   0, 0, 0, 0,   0, 0, 0, 0   

};

static int trans1[8] = { -1, -1, 0, 1, -1, 2, 3, 4 };  
					/* translation table for *Row */

static char *Row[5][16] = {
{ "???",      "JB0 ",     "???",      "JB1 ",     "MOV A,T",  "JB2 ",     "MOV T,A",  "JB3 ", 
  "???",      "JB4 ",     "???",      "JB5 ",     "???",      "JB6 ",     "???",      "JB7 " },
{ "ADD A,#",  "ADDC A,#", "MOV A,#",  "???",      "ORL A,#",  "ANL A,#",  "???",      "???", 
  "RET",      "RETR",     "MOVP A,@A","** JMPP @A **",  "???","XRL A,#",  "MOVP3 A,@A", "???" },
{ "EN I",     "DIS I",    "EN T",     "DIS T",    "START E",  "START T",  "STOP T",   "ENT0 CLK", 
  "CLR F0",   "CPL F0",   "CLR F1",   "CPL F1",   "SEL RB0",  "SEL RB1",  "** SEL MB0 **", "** SEL MB1 **" },
{ "???",      "JTF ",     "JNT0 ",    "JT0 ",     "JNT1 ",    "JT1 ",     "???",  "JF1 ",     
  "JNI ",     "JNZ ",     "???",      "JF0 ",     "JZ ",      "???",      "JNC ",  "JC " },
{ "DEC A",    "INC A",    "CLR A",    "CPL A",    "SWAP A",   "DA A",     "RRC A", "RR A",
  "???",      "CLR C",    "CPL C",    "???",      "MOV A,PSW","MOV PSW,A","RL A",  "RLC A" }

};

static char *Col[16] = {
  "MOVD A,Pp","INC @Rr",  "XCH A,@Rr", "MOVD Pp,A","ORL A,@Rr", "ANL A, @Rr", "ADD A,@Rr", "ADDC A,@Rr", 
  "ORLD Pp,A","ANLD Pp,A","MOV @Rr, A", "MOV @Rr,#", "DEC Rr", "XRL A,@Rr", "DJNZ Rr,", "MOV A,@Rr" };

static int trans2[10] = { 0, -1, -1, 1, -1, -1, -1, -1, 2, 3 };
static int trans3[12] = { 0,  1, -1, -1, -1, -1, -1, -1, 2, 3, 4, 5 };

static char *ir[4][6] = {
{ "NOP",        "OUTL BUS,A", "INS A,BUS",  "IN A,P1", "IN A,P2", "???" },
{ "XCHD A,@R0", "XCHD A,@R1", "???",      "OUTL P1,A", "OUTL P2,A", "???" },
{ "MOVX A,@R0", "MOVX A,@R1", "ORL BUS,#", "ORL P1,#", "ORL P2,#", "???" },
{ "MOVX @R0,A", "MOVX @R1,A", "ANL BUS,#", "ANL P1,#", "ANL P2,#", "???" }  };
