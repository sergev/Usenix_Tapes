/*
 *	Termcap terminal file, nothing special, just make it big
 *	enough for windowing systems.
 */

#define	GOSLING			/* Compile in fancy display.	*/
/* #define	MEMMAP		*/	/* Not memory mapped video.	*/

#define	NROW	66    			/* Rows.			*/
#define	NCOL	132			/* Columns.			*/
/* #define	MOVE_STANDOUT		/* don't move in standout mode	*/
#define	STANDOUT_GLITCH			/* possible standout glitch	*/
#define	TERMCAP				/* for possible use in ttyio.c	*/

/*
 * Termcap function keys.  The last 10 keys correspond to the
 * non-standard termcap entries K0-K9 (instead of k0-k9).
 */
#ifdef	XKEYS
#define	KFIRST	K01
#define KLAST	K1A

#define KHOME	K01
#define KDOWN	K02
#define	KUP	K03
#define	KLEFT	K04
#define	KRIGHT	K05
#define	KF0	K06
#define	KF1	K07
#define KF2	K08
#define KF3	K09
#define KF4	K0A
#define KF5	K0B
#define KF6	K0C
#define KF7	K0D
#define KF8	K0E
#define	KF9	K0F
#define KF10	K10
#define	KF11	K11
#define	KF12	K12
#define	KF13	K13
#define	KF14	K14
#define	KF15	K15
#define	KF16	K16
#define	KF17	K17
#define	KF18	K18
#define	KF19	K19
#define	KF20	K1A

#define	NFKEYS	20		/* # of function keys (k0-k9, K0-K9) */
#endif
