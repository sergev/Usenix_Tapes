/*
 *	Object Code Optimizer for the M6800 C Compiler
 *
 *	Original version written by Bell Labs for the PDP-11.
 *
 *	Modified for the M6800 microprocessor by:
 *		Gary Luckenbaugh
 *		University of Delaware
 *		Electrical Engineering Dept.
 *
 *	Added functions in opt2b.c (#10 on) for the M6800
 *		Richard Hammond
 *		University of Delaware
 *		Electrical Engineering Dept.
 *
 *	Description:
 *
 *	     This program is used to optimize M6800 assembly language
 *	programs that are output from the M6800 C compiler.  Here is
 *	a list of the things that are optimized:
 *
 *	1. Jumps to jumps are changed to direct jumps.
 *	2. Impossible to reach code is removed.
 *	3. Jumps to the following instruction are removed.
 *	4. Redundant labels are removed.
 *	5. Cross jumps are shortened.
 *	6. Common sequences of code before jumps are collapsed into one
 *	   copy of the code.
 *	7. Conditional branches over jumps are changed into one conditional
 *	   branch if possible.
 *	8. Jumps are changed to branches if possible.
 *	9. Loads that load the constant zero are changed to clears.
 *     10. Extra LDX INDEX codes are removed.
 *     11. LDA's and CLR's of registers which are not referenced before
 *	   they are LDA'ed or CLR'ed again are removed.
 *     12. Constants are propagated to check for conditional branches
 *	   which may be changed to BRA's (or JMP's) and the TST removed.
 *     13. Redundant LDA's (LDA's of the location just stored to) are
 *	   removed.
 *
 */

/*
 *	The following table defines the internal coding for each
 *	instruction as it appears in the linked list.  All of the
 *	M6800 machine instructions and ca pseudo-ops are included in
 *	this table.
 *	They are arranged in order of maximum number of bytes needed
 *	for the opcode, from 0 to 3.  Don't change the order!
 */
#define	LABEL	 1		/* Compiler generated labels Ln */
#define	DLABEL	 2		/* Any other label */
#define	EROU	 3		/* End of routine */
#define TEXT	 4
#define	EQU	 5
#define	RMB	 6
#define	FCB	 7
#define	FDB	 8
#define	ORG	 9
#define	END	10
#define	COMMEN	11
#define	DATA	12
#define	BSS	13
#define	ABA	14
#define	CBA	15
#define	CLC	16
#define	CLI	17
#define	CLR	18
#define	CLV	19
#define	DAA	20
#define	DES	21
#define	DEX	22
#define	INS	23
#define	INX	24
#define	NOP	25
#define	PSH	26
#define	PUL	27
#define	RTI	28
#define	RTS	29
#define	SBA	30
#define	SEC	31
#define	SEI	32
#define	SEV	33
#define	SWI	34
#define	TAB	35
#define	TAP	36
#define	TBA	37
#define	TPA	38
#define	TSX	39
#define	TXS	40
#define	WAI	41
#define BSR	42
#define	JSW	43		/* Switch statement */
#define	BRA	44
#define	CBR	45		/* Conditional branch */
#define	ADC	46
#define	ADD	47
#define	AND	48
#define	ASL	49
#define	ASR	50
#define BIT	51
#define	CMP	52
#define	COM	53
#define	CPX	54
#define	DEC	55
#define	EOR	56
#define	INC	57
#define	JMP	58
#define	JSR	59
#define	LDA	60
#define	LDS	61
#define	LDX	62
#define	LSR	63
#define	NEG	64
#define	ORA	65
#define	ROL	66
#define	ROR	67
#define	SBC	68
#define	STA	69
#define	STS	70
#define	STX	71
#define	SUB	72
#define	TST	73

 /* AREG and BREG indicate the registers used in a given instruction */

#define	AREG	100
#define	BREG	101

 /* The following are the types of conditional branches in the M6800 */

#define	BCC	0
#define	BCS	1
#define	BEQ	2
#define	BGE	3
#define	BGT	4
#define	BHI	5
#define	BLE	6
#define	BLS	7
#define	BLT	8
#define	BMI	9
#define	BNE	10
#define	BPL	11
#define	BVC	12
#define	BVS	13


/*
 *	Following are the global variables used throughtout the program.
 *
 */


 /* The following structure defines the format for a node in the linked
  * list.  The code being optimized is stored one line per node in this
  * linked list.  The members of the structure are used as follows:
  *
  *	op	Machine op-code as defined in above table.
  *
  *	subop	Additional op-code information such as type of conditional
  *		branch or registers used.  Also a flag to mark visited labels.
  *
  *	forw	Forward pointer in the linked list.
  *
  *	back	Backward pointer in the list.
  *
  *	ref	For a jump or branch instruction this points to the 
  *		node of the referenced line.
  *
  *	labno	For LABELS or JUMPS this variable contains the label
  *		number referenced.
  *
  *	code	This points to a string containing the operands.
  *
  *	refc	For labels this variable indicates how many jumps or
  *		branches are refrencing it.
  */

struct node {
	char	op;
	char	subop;
	struct	node	*forw;
	struct	node	*back;
	struct	node	*ref;
	int	labno;
	char	*code;
	int	refc;
};

struct labnode {
	char	op;
	char	subop;
	struct	node	*forw;
	struct	node	*back;
	int	count;		/* in labels used by ckldx(), rmldx()	*/
	int	labno;
	char	*code;
	int	refc;
};

struct {
	int	combop;
};

struct optab {
	char	*opstring;
	int	opcode;
};

char	line[512];
struct	node	first;
char	*curlp;
int	nbrbr;
int	nsaddr;
int	redunm;
int	iaftbr;
int	njp1;
int	nrlab;
int	nxjump;
int	ncmot;
int	nrevbr;
int	nredunj;
int	nskip;
int	ncomj;
int	jmpbr;
int	redop;	/* number of codes removed by redcode()	*/

int	nchange;
int	isn;
int	debug;
char	*lasta;
char	*lastr;
char	*firstr;

#define	LABHS	127
#define	OPHS	97

struct { char lbyte; };
