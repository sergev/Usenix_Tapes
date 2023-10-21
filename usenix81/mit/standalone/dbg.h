/* Header file for 68000 debugger		Steve Ward 1/80
 */

struct State {
	long	Regs[16];	/* the registers: d0-7, a0-7	*/
	short	Status;		/* program status word		*/
	long	PC;		/* program counter		*/
	};


#define	INTCHR	('Z' - 0100)	/* Interrupt character		*/
