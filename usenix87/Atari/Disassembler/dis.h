#include <stdio.h>

#define NPREDEF 10

extern char *predef[];
extern int  npredef;
extern char *file;
extern char *progname;
extern int  bopt;
extern unsigned char f[];
extern unsigned char d[];

#define getword(x) (d[x] + (d[x+1] << 8))
#define getbyte(x) (d[x])

/* f bits */

#define LOADED 1			/* Location loaded */
#define JREF   2			/* Referenced as jump/branch dest */
#define DREF   4			/* Referenced as data */
#define SREF   8			/* Referenced as subroutine dest */
#define NAMED  0x10			/* Has a name */
#define TDONE  0x20			/* Has been traced */
#define ISOP   0x40			/* Is a valid instruction opcode */

struct info {
	char *opn;
	int  nb;
	int  flag;
};

extern struct info optbl[];

/* Flags */

/* Where control goes */

#define NORM 1
#define JUMP 2
#define FORK 4
#define STOP 8

#define CTLMASK (NORM|JUMP|FORK|STOP)

/* Instruction format */

#define IMM  0x20
#define ABS  0x40
#define ACC  0x80
#define IMP  0x100
#define INX  0x200
#define INY  0x400
#define ZPX  0x800
#define ABX  0x1000
#define ABY  0x2000
#define REL  0x4000
#define IND  0x8000
#define ZPY  0x10000
#define ZPG  0x20000
#define ILL  0x40000

#define ADRMASK (IMM|ABS|ACC|IMP|INX|INY|ZPX|ABX|ABY|REL|IND|ZPY|ZPG|ILL)

struct ref_chain {
	struct ref_chain *next;
	int who;
};

struct ref_chain *get_ref();
char *get_name();

/* lex junk */

#define EQ 256
#define NUMBER 257
#define NAME 258
#define COMMENT 259
#define LI 260
#define TSTART 261
#define TSTOP 262

extern FILE *yyin, *yyout;
int lineno;

int yywrap(), yyerror();
char *emalloc();

typedef union  {
	int ival;
	char *sval;
} VALUE;

extern VALUE token;
