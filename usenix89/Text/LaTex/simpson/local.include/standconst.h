/* @(#)standconst.h	1.1 (TRW) 6/13/84 */
/* Standard constants. */

/*
 * These are the only values boolean variables may be set to,
 * or that boolean functions may return.
 */
#define	TRUE 1
#define	FALSE 0

/*
 * Program exit status.
 * These two codes are intended to be used as arguments to the
 * exit(2) system call.  Obviously, more failure codes may be
 * defined but for simple programs that need indicate only
 * success or failure these will suffice.
 */
#define	SUCCEED 0	/* successful program execution	*/
#define	FAIL 1		/* some error in running program */

/* All bits on or off. */
#define	ON ~(long)0	/* all bits set	*/
#define	OFF (long)0	/* all bits off	*/

/* UNIX file descriptor numbers for standard input, output, and error. */
#define	STANDARD_IN 0
#define	STANDARD_OUT 1
#define	STANDARD_ERROR 2


/*
 * Extreme values.
 * These constants are the largest and smallest values
 * that variables of the indicated type may hold.
 */
#if defined(vax) || defined(pyr) || defined(pyramid)
#   define MAX_TINY TINY(0x7f)
#   define MIN_TINY TINY(0x80)

#   define MAX_UNSIGNED_TINY UNSIGNED_TINY(0xff)
#   define MIN_UNSIGNED_TINY 0

#   define MAX_SHORT ((short)0x7fff)
#   define MIN_SHORT ((short)0x8000)

#   define MAX_UNSIGNED_SHORT 0xffff
#   define MIN_UNSIGNED_SHORT 0

#   define MAX_INTEGER 0x7fffffff
#   define MIN_INTEGER 0x80000000

#   define MAX_UNSIGNED_INTEGER 0xffffffff
#   define MIN_UNSIGNED_INTEGER 0

#   define MAX_LONG ((long)0x7fffffff)
#   define MIN_LONG ((long)0x80000000)
#   define MAX_UNSIGNED_LONG 0xffffffff
#   define MIN_UNSIGNED_LONG 0
#   define BITS_PER_BYTE 8
#endif

/* for pointers */
#define NIL ((long)0)
