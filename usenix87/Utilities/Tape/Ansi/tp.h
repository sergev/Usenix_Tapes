#ifndef TPFILE
#ifndef FILE
#include	<stdio.h>
#endif

#define	TP_CDEV	(-1)
#define	TP_IMAG	(-2)
#define	_TP_PREF	8

struct tpdes	{
	int _tp_unit;		/* unit number or TP_IMAG  or TP_CDEV  */
	char *_tp_nmdns;	/* density     or filename or filename */
	char _tp_rw;		/* 'r' for reading, 'w' for writing */
	char _tp_x;		/* 'x' for persistent, ' ' for normal */
	char _tp_fildes;	/* file descriptor after `open' */

	long _tp_blkc;		/* block counter */
	int _tp_mkc;		/* accumulative TM counter */
	int _tp_mc;		/* consecutive TM counter */
	char _tp_eof;		/* EOF-flag */
};
#define	TPFILE	struct tpdes

#define	char2int(c)	((c)&0377)
#define	ebc2asc(c)	(_etoa[char2int(c)])
#define	english(i)	(i), (i) == 1 ? "" : "s"	/* plural */
#define	n_items(a)	(sizeof (a)/sizeof (a)[0])
extern TPFILE *tpopen();
extern char *_tpname();
extern char _etoa[];
extern FILE *tperr;		/* for error messages */

#ifdef	lint
int __void__;	/* to tell `lint' not to care */
#define	VOID(x)	(__void__ = (int)(x))
#else	lint
#define	VOID(x)	(x)
#endif	lint

#include	"tploc.h"

#endif
