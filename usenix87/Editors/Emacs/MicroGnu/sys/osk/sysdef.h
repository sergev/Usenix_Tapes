/*
 * Os9/68K specific definitions for micrognuemacs
 */
#define	PCC			/* "[]" gets an error.			*/
#define	KBLOCK	1024		/* Kill grow.				*/
#define	GOOD	0		/* Good exit status.			*/
#define	VARARGS			/* use my varargs.h rather than		*/
				/* the non-portable code supplied	*/
#define NO_VOID_TYPE		/* void not supported yet		*/
#define	SEOL	'\l'		/* used in search (because \r==\n)	*/
#define PC PC_			/* compiler can't handle variable called PC */
#define SR SR_			/* or SR				*/
#define NO_RESIZE		/* terminal doesn't change size		*/

/* typedefs for gnu version */
typedef int	RSIZE;		/* Type for file/region sizes	*/
typedef short	KEY;		/* Type for internal keystrokes	*/

/*
 * Macros used by the buffer name making code.
 * Start at the end of the file name, scan to the left
 * until BDC1 (or BDC2, if defined) is reached. The buffer
 * name starts just to the right of that location, and
 * stops at end of string (or at the next BDC3 character,
 * if defined). BDC2 and BDC3 are mainly for VMS.
 */
#define	BDC1	'/'			/* Buffer names.		*/

/*
 * Needed for lots of small mallocs on os9/68k.  _memmins should be 
 * (maximum malloced memory)/16.  (defalt _memmins is 4096) 
 * Note that malloc may now fail if there isn't _memmins bytes 
 * contiguous free memory.  _memins could be reduced and the malloc 
 * tried again.  (Not currently implemented.)
 */
#ifdef MAXMEM
#  define SYSINIT	{extern int _memmins;	_memmins=MAXMEM*64; }
#else
#  define SYSINIT	{extern int _memmins;	_memmins=32768; }
#endif

/* see "caveates" in the osk C manual on _strass */
#define bcopy(from,to,len)	_strass(to,from,len)

/* see comments on these in display.c.  OSK can't stand the wasted memory
 * without making the score array "remote", which generates lousy code.  
 * Besides, I don't have an extra 100kb of memory for the score array.
 */
#define	XCHAR	char
#define XSHORT	short
