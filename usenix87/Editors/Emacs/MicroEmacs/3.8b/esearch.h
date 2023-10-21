/*
 * ESEARCH.H
 *
 * Defines, typdefs, and global variables that are of use for the
 * routines in search.c and isearch.c.
 *
 */

/*
 * PTBEG, PTEND, FORWARD, and REVERSE are all toggle-able values for
 * the scan routines.
 */
#define	PTBEG	0	/* Leave the point at the beginning on search.*/
#define	PTEND	1	/* Leave the point at the end on search.*/

#if	MAGIC

/*
 * Defines for the metacharacters in the regular expressions.
 */

#define	MCNIL		0	/* Like the '\0' for strings.*/
#define	LITCHAR		1
#define	ANY		2
#define	CCL		3
#define	NCCL		4
#define	BOL		5
#define	EOL		6
#define	CLOSURE		256	/* An or-able value.*/
#define	MASKCL		CLOSURE - 1

#define	MC_ANY		'.'	/* 'Any' character (except newline).*/
#define	MC_CCL		'['	/* Character class.*/
#define	MC_NCCL		'^'	/* Negate character class.*/
#define	MC_RCCL		'-'	/* Range in character class.*/
#define	MC_ECCL		']'	/* End of character class.*/
#define	MC_BOL		'^'	/* Beginning of line.*/
#define	MC_EOL		'$'	/* End of line.*/
#define	MC_CLOSURE	'*'	/* Closure - does not extend past newline.*/

#define	MC_ESC		'\\'	/* Escape - suppress meta-meaning.*/

#define	BIT(n)		(1 << (n))	/* An integer with one bit set.*/
#define	CHCASE(c)	((c) ^ DIFCASE)	/* Toggle the case of a letter.*/

/* HICHAR - 1 is the largest character we will deal with.
 * HIBYTE represents the number of bytes in the bitmap.
 */

#define	HICHAR		256
#define	HIBYTE		HICHAR >> 3

typedef char	*BITMAP;

typedef	struct {
	short int	mc_type;
	union {
		int	lchar;
		BITMAP	cclmap;
	} u;
} MC;
#endif

/* Incremental search defines.
 */
#if	ISRCH
#define	CMDBUFLEN	256	/* Length of our command buffer */

#define	IS_ABORT	0x07	/* Abort the isearch */
#define IS_BACKSP	0x08	/* Delete previous char */
#define	IS_TAB		0x09	/* Tab character (allowed search char) */
#define IS_NEWLINE	0x0D	/* New line from keyboard (Carriage return) */
#define	IS_QUOTE	0x11	/* Quote next character */
#define IS_REVERSE	0x12	/* Search backward */
#define	IS_FORWARD	0x13	/* Search forward */
#define	IS_VMSQUOTE	0x16	/* VMS quote character */
#define	IS_VMSFORW	0x18	/* Search forward for VMS */
#define	IS_QUIT		0x1B	/* Exit the search */
#define	IS_RUBOUT	0x7F	/* Delete previous character */

/* IS_QUIT is no longer used, the variable metac is used instead */

#endif
