/*
 * The routines in this file provide support for the IBM-PC EGA and other
 * compatible terminals. It goes directly to the graphics RAM to do
 * screen output. It compiles into nothing if not an IBM-PC EGA driver
 */

#define	termdef	1			/* don't define "term" external */

#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

#if     EGA

#define NROW    43                      /* Screen size.                 */
#define NCOL    80                      /* Edit if you want to.         */
#define	MARGIN	8			/* size of minimim margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define	NPAUSE	200			/* # times thru update to pause */
#define BEL     0x07                    /* BEL character.               */
#define ESC     0x1B                    /* ESC character.               */
#define	SPACE	32			/* space character		*/
#define	SCADD	0xb8000000L		/* address of screen RAM	*/

int *scptr[NROW];			/* pointer to screen lines	*/
int sline[NCOL];			/* screen line image		*/

extern  int     ttopen();               /* Forward references.          */
extern  int     ttgetc();
extern  int     ttputc();
extern  int     ttflush();
extern  int     ttclose();
extern	int	egakopen();
extern	int	egakclose();
extern  int     egamove();
extern  int     egaeeol();
extern  int     egaeeop();
extern  int     egabeep();
extern  int     egaopen();
extern	int	egarev();
extern	int	egacres();
extern	int	egaclose();
extern	int	egaputc();

#if	COLOR
extern	int	egafcol();
extern	int	egabcol();

int	cfcolor = -1;		/* current forground color */
int	cbcolor = -1;		/* current background color */
int	ctrans[] =		/* ansi to ega color translation table */
	{0, 4, 2, 6, 1, 5, 3, 7};
#endif

/*
 * Standard terminal interface dispatch table. Most of the fields point into
 * "termio" code.
 */
TERM    term    = {
	NROW-1,
        NROW-1,
        NCOL,
        NCOL,
	MARGIN,
	SCRSIZ,
	NPAUSE,
        egaopen,
        egaclose,
	egakopen,
	egakclose,
        ttgetc,
	egaputc,
        ttflush,
        egamove,
        egaeeol,
        egaeeop,
        egabeep,
	egarev,
	egacres
#if	COLOR
	, egafcol,
	egabcol
#endif
};

extern union REGS rg;

#if	COLOR
egafcol(color)		/* set the current output color */

int color;	/* color to set */

{
	cfcolor = ctrans[color];
}

egabcol(color)		/* set the current background color */

int color;	/* color to set */

{
        cbcolor = ctrans[color];
}
#endif

egamove(row, col)
{
	rg.h.ah = 2;		/* set cursor position function code */
	rg.h.dl = col;
	rg.h.dh = row;
	rg.h.bh = 0;		/* set screen page number */
	int86(0x10, &rg, &rg);
}

egaeeol()	/* erase to the end of the line */

{
	int attr;	/* attribute byte mask to place in RAM */
	int *lnptr;	/* pointer to the destination line */
	int i;
	int ccol;	/* current column cursor lives */
	int crow;	/*	   row	*/

	/* find the current cursor position */
	rg.h.ah = 3;		/* read cursor position function code */
	rg.h.bh = 0;		/* current video page */
	int86(0x10, &rg, &rg);
	ccol = rg.h.dl;		/* record current column */
	crow = rg.h.dh;		/* and row */

	/* build the attribute byte and setup the screen pointer */
#if	COLOR
	attr = (((cbcolor & 15) << 4) | (cfcolor & 15)) << 8;
#else
	attr = 0x0700;
#endif
	lnptr = &sline[0];
	for (i=0; i < NCOL; i++)
		*lnptr++ = SPACE | attr;

	if (flickcode) {
		/* wait for vertical retrace to be off */
		while ((inp(0x3da) & 8))
			;

		/* and to be back on */
		while ((inp(0x3da) & 8) == 0)
			;
	}

	/* and send the string out */
	movmem(&sline[0], scptr[crow]+ccol, (NCOL-ccol)*2);

}

egaputc(ch)	/* put a character at the current position in the
		   current colors */

int ch;

{
	rg.h.ah = 14;		/* write char to screen with current attrs */
	rg.h.al = ch;
#if	COLOR
	rg.h.bl = cfcolor;
#else
	rg.h.bl = 0x07;
#endif
	int86(0x10, &rg, &rg);
}

egaeeop()
{
	int attr;		/* attribute to fill screen with */

	rg.h.ah = 6;		/* scroll page up function code */
	rg.h.al = 0;		/* # lines to scroll (clear it) */
	rg.x.cx = 0;		/* upper left corner of scroll */
	rg.x.dx = 0x2a4f;	/* lower right corner of scroll */
#if	COLOR
	attr = ((ctrans[gbcolor] & 15) << 4) | (ctrans[gfcolor] & 15);
#else
	attr = 0;
#endif
	rg.h.bh = attr;
	int86(0x10, &rg, &rg);
}

egarev(state)		/* change reverse video state */

int state;	/* TRUE = reverse, FALSE = normal */

{
	/* This never gets used under the ega-PC driver */
}

egacres()	/* change screen resolution */

{
	return(TRUE);
}

egabeep()
{
	bdos(6, BEL, 0);
}

egaopen()
{
	char buf;	/* buffer for peek/poke */

	/* initialize pointers to the screen ram */
	scinit();

	/* and put the beast into EGA 43 row mode */
	rg.x.ax = 3;
	int86(0x10, &rg, &rg);

	rg.x.ax = 0x1112;
	rg.h.bl = 0;
	int86(0x10, &rg, &rg);

	peek(0x40, 0x87, &buf, 1);
	buf |= 1;
	poke(0x40, 0x87, &buf, 1);
	buf = 0xf8;
	poke(0x40, 0x88, &buf, 1);

	rg.x.ax = 0x0100;
	rg.h.bh = 0;
	rg.x.cx = 0x0007;
	int86(0x10, &rg, &rg);

	buf = 0xf9;
	poke(0x40, 0x88, &buf, 1);

	strcpy(sres, "43LINE");
	revexist = TRUE;
        ttopen();
}

egaclose()

{
	char buf;	/* buffer for peek/poke */

#if	COLOR
	egafcol(7);
	egabcol(0);
#endif
	/* and put the beast into 80 column mode */
	rg.x.ax = 0002;
	int86(0x10, &rg, &rg);
	ttclose();

#if	0
	peek(0x40, 0x87, &buf, 1);
	buf--;
	poke(0x40, 0x87, &buf, 1);
#endif

	/* and restore the normal cursor */
	rg.x.ax = 0x0100;
	rg.h.bl = 0;
	rg.x.cx = 0x0b0d;
	int86(0x10, &rg, &rg);
}

egakopen()

{
}

egakclose()

{
}

scinit()	/* initialize the screen head pointers */

{
	union {
		long laddr;	/* long form of address */
		int *paddr;	/* pointer form of address */
	} addr;
	int i;

	/* initialize the screen pointer array */
	for (i = 0; i < NROW; i++) {
		addr.laddr = SCADD + (long)(NCOL * i * 2);
		scptr[i] = addr.paddr;
	}
}

scwrite(row, outstr, forg, bacg)	/* write a line out*/

int row;	/* row of screen to place outstr on */
char *outstr;	/* string to write out (must be NCOL long) */
int forg;	/* forground color of string to write */
int bacg;	/* background color */

{
	int attr;	/* attribute byte mask to place in RAM */
	int *lnptr;	/* pointer to the destination line */
	int i;

	/* build the attribute byte and setup the screen pointer */
#if	COLOR
	attr = (((ctrans[bacg] & 15) << 4) | (ctrans[forg] & 15)) << 8;
#else
	attr = (((bacg & 15) << 4) | (forg & 15)) << 8;
#endif
	lnptr = &sline[0];
	for (i=0; i<term.t_ncol; i++)
		*lnptr++ = (outstr[i] & 255) | attr;

	if (flickcode) {
		/* wait for vertical retrace to be off */
		while ((inp(0x3da) & 8))
			;

		/* and to be back on */
		while ((inp(0x3da) & 8) == 0)
			;
	}

	/* and send the string out */
	movmem(&sline[0], scptr[row],term.t_ncol*2);
}

#if	FLABEL
fnclabel(f, n)		/* label a function key */

int f,n;	/* default flag, numeric argument [unused] */

{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif
#else
egahello()
{
}
#endif
