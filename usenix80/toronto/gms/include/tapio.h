/*
 * This is the header file for the raw magtape i/o library.
 */

/*
 * Parameters needed by the user of the library (should NOT be changed)
 */

#define	OK	0		/* non-error return */
#define	ERR	(-1)		/* error return */
#define	NULL	0		/* error return for pointer-valued functions */
#define	TAPE	struct	_tapio	/* analogous to FILE in stdio.h */

/*
 * Parameters used by routines (should NOT be changed)
 */

/* check to see if a _tapio pointer is in bounds */
#define	_TPCHK	(tp < &_tapio[0]) || (tp >= &_tapio[_NTAPES])

/*
 * Parameters for tuning (may be changed)
 */

#define	_NTAPES	2		/* maximum number of tapes open at once */

/*
 * Structures
 */

struct	_tapio {		/* tape i/o structure */
	int	_tapfd;		/* file descriptor */
	unsigned _taprec;	/* size of a record (in bytes) */
	long	_tappos;	/* current position on the tape (in bytes) */
	unsigned _tapcnt;	/* number of characters in the buffer */
	char	*_tapptr;	/* pointer to the next character */
	char	*_tapbuf;	/* pointer to the buffer */
} _tapio[_NTAPES];

/*
 * Externals
 */

/* functions in the library that do not return integers */

unsigned _tapread();
unsigned tread();
long	ttell();
TAPE	*topen();
