/*
 * Name:	MicroEMACS
 *		Commodore Amiga system header file.
 * Version:	Gnu v30
 */

#ifdef MANX
#define	PCC	0			/* "[]" works.			*/
#else
#define	PCC	1			/* "[]" does not work.		*/
#endif

#define	VARARGS
#define	KBLOCK	4096			/* Kill grow.			*/
#define	GOOD	0			/* Good exit status.		*/

/*
 * Macros used by the buffer name making code.
 * Start at the end of the file name, scan to the left
 * until BDC1 (or BDC2, if defined) is reached. The buffer
 * name starts just to the right of that location, and
 * stops at end of string (or at the next BDC3 character,
 * if defined). BDC2 and BDC3 are mainly for VMS.
 */
#define	BDC1	':'			/* Buffer names.		*/
#define	BDC2	'/'

/*
 * Typedefs for internal key type and how big a region can be.
 */

typedef	short	KEY;	/* 16-bit ints		*/
typedef	long	RSIZE;	/* size of a region	*/

#define	bcopy(src,dest,len) movmem(src,dest,len)
