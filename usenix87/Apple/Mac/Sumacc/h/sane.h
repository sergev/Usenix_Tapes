/*	sane.h	1.0	05/25/84	*/

/*
 * Sane definitions.
 *
 * Copyright Apple Computer, 1982.
 *
 * C language version (C) 1984, Stanford Univ. SUMEX project.
 * May be used but not sold without permission.
 */

/*
 * history
 * 05/25/84	Croft	Created C version.
 */

#define	SIGDIGLEN  20     /* Maximum length of SigDig. */
#define	DECSTRLEN  82     /* Maximum length of DecStr. */

/* Numeric types. */

typedef	long	Single;

typedef struct {
	short	s[4];
} Double;

typedef	struct {
	short	s[4];
} Comp;

typedef	struct {
	short	s[5];
} Extended;


/*
 * Decimal string type and intermediate decimal type,
 * representing the value:
 *      (-1)^sgn * 10^exp * dig
 */
typedef	struct {
	char	sgn;	/* Sign (0 for pos, 1 for neg). */
	char	dummy;
	short	exp;	/* Exponent. */
	char	sig[SIGDIGLEN+2]; /* Pascalstring of significant digits. */
} Decimal;


/*
 * Modes, flags, and selections.
 * NOTE: the values of the style element of the DecForm record
 * have different names from the PCS version to avoid name
 * conflicts.
 */
typedef	short	Environ;

/* RoundDir  = (TONEAREST, UPWARD, DOWNWARD, TOWARDZERO); */
#define	TONEAREST	0
#define	UPWARD		1
#define	DOWNWARD	2
#define	TOWARDZERO	3

/* RelOp     = (GT, LT, GL, EQ, GE, LE, GEL, UNORD);
	         >   <  <>  =   >=  <=  <=> */
#define	GT	0
#define	LT	1
#define	GL	2
#define	EQ	3
#define	GE	4
#define	LE	5
#define	GEL	6
#define	UNORD	7

/* Exception = (INVALID, UNDERFLOW, OVERFLOW, DIVBYZERO, INEXACT); */
#define	INVALID		0
#define	UNDERFLOW	1
#define	OVERFLOW	2
#define	DIVBYZERO	3
#define	INEXACT		4

/* NumClass  = (SNAN, QNAN, INFINITE, ZERO, NORMAL, DENORMAL); */
#define	SNAN		0
#define	QNAN		1
#define	INFINITE	2
#define	ZERO		3
#define	NORMAL		4
#define	DENORMAL	5

/* Decimal to binary conversion is governed by the DecForm struct */
typedef	struct {
	char	style;
	char	dummy;
	short	digits;
} DecForm;
/* values for style */
#define	FloatDecimal	0
#define	FixedDecimal	1
