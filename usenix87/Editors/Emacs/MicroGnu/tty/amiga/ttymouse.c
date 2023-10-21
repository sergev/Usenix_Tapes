/*
 * Name:	MicroEmacs
 *		Commodore Amiga mouse handling (no longer simple!)
 * Version:	Gnu v30
 * Last edit:	02-Aug-86  ...!ihnp4!seismo!ut-sally!ut-ngp!mic
 */

#include <exec/types.h>
#include <intuition/intuition.h>
#undef	TRUE
#undef	FALSE
#include "def.h"

extern	int	ttmouse();		/* Defined by "ttyio.c"		*/
extern	int	forwline();		/* Defined by "line.c"		*/
extern	int	forwchar();		/* Defined by "basic.c"		*/
extern	int	setmark();		/* Defined by "random.c"	*/

/* stuff for go-to-window-and-do-it functions */
extern	int	reposition();
extern	int	delfword();
extern	int	killline();
extern	int	forwdel();
extern	int	delwhite();
extern	int	killregion();
extern	int	yank();
extern	int	forwpage();
extern	int	backpage();
extern	int	splitwind();
extern	int	delwind();
extern	int	gotobob();
extern	int	gotoeob();
extern	int	enlargewind();
extern	int	shrinkwind();

/*
 * Handle the mouse click that's been passed
 * by ttgetc() and position dot where the
 * user pointed at.  If this is the same position
 * where the user pointed the last time, set the mark,
 * whether or not this is a true double-click.
 * This isn't a true double-click, but it does
 * most of what we want.
 */

static USHORT	oldrow = HUGE, oldcol = HUGE;	/* last mouse click	*/
static USHORT	newrow, newcol;			/* next mouse click	*/

amigamouse(f, n, k)
{
	if (!dottomouse())			/* sets newrow, newcol	*/
		return (FALSE);
	if ((newrow == oldrow) && (newcol == oldcol))
		setmark(FALSE, 0, KRANDOM);	/* double-click		*/
	oldrow = newrow;		    	/* save state		*/
	oldcol = newcol;
	return (TRUE);
}

/*
 * Recenter on selected line
 */
mreposition(f, n, k)
{
	if (!dottomouse())
		return (FALSE);
	return (reposition(f, n, k));
}

/*
 * Delete word after selected char
 */
mdelfword(f, n, k)
{
	if (!dottomouse())
		return (FALSE);
	return (delfword(f, n, k));
}

/*
 * Move to selection, kill line
 */
mkillline(f, n, k)
{
	if (!dottomouse())
		return (FALSE);
	return (killline(f, n, k));
}

/*
 * Move to selection, kill line
 */
mforwdel(f, n, k)
{
	if (!dottomouse())
		return (FALSE);
	return (forwdel(f, n, k));
}

/*
 * Move to selection, kill line
 */
mdelwhite(f, n, k)
{
	if (!dottomouse())
		return (FALSE);
	return (delwhite(f, n, k));
}

/*
 * Move to selection, kill region.
 * This makes it easy to mark a
 * region with a double-click, then
 * hold down some shift keys to
 * delete the region.
 */
mkillregion(f, n, k)
{
	if (!dottomouse())
		return (FALSE);
	return (killregion(f, n, k));
}

/*
 * Move to selection, yank kill buffer
 */
myank(f, n, k)
{
	if (!dottomouse())
		return (FALSE);
	return (yank(f, n, k));
}

/*
 * Select window pointed to by mouse, then scroll down
 */

mforwpage(f, n, k)
{
	if (!dottomouse())
		return (FALSE);
	return (forwpage(f, n, k));
}

/*
 * Select buffer, scroll page down
 */
mbackpage(f, n, k)
{
	if (!dottomouse())
		return (FALSE);
	return (backpage(f, n, k));
}

/*
 * Select the window, split it.
 */
msplitwind(f, n, k)
{
	if (!dottomouse())
		return (FALSE);
	return (splitwind(f, n, k));
}

/*
 * Select the buffer, delete it.
 */
mdelwind(f, n, k)
{
	if (!dottomouse())
		return (FALSE);
	return (delwind(f, n, k));
}

/*
 * Select window, goto beginning of buffer
 */
mgotobob(f, n, k)
{
	if (!dottomouse())
		return (FALSE);
	return (gotobob(f, n, k));
}

/*
 * Select window, go to end of buffer
 */
mgotoeob(f, n, k)
{
	if (!dottomouse())
		return (FALSE);
	return (gotoeob(f, n, k));
}

/*
 * Select window, enlarge it.
 */
menlargewind(f, n, k)
{
	if (!dottomouse())
		return (FALSE);
	return (enlargewind(f, n, k));
}
	
/*
 * Select window, shrink it.
 */
mshrinkwind(f, n, k)
{
	if (!dottomouse())
		return (FALSE);
	return (shrinkwind(f, n, k));
}

/*
 * Utility routine to move dot to where
 * the user clicked.  If in mode line,
 * chooses that buffer as the one
 * to work on.
 */

static dottomouse()
{
	register WINDOW *wp;
	register int	dot;
	register int	c;
	register int	col;
	int		qualifier;

	/* figure out new screen position of dot, throw away mouse evt */
	if (!ttmouse(TRUE, &newrow, &newcol, &qualifier))
		return (FALSE);			/* out of synch!	*/

	/* find out which window was clicked in				*/
	for (wp = wheadp; wp != NULL; wp = wp->w_wndp)
		if ((newrow >= wp->w_toprow) && 
			(newrow <= (wp->w_toprow + wp->w_ntrows)))
			break;

	if (wp == NULL)				/* echo line		*/
		return (ABORT);
	else if (newrow == wp->w_toprow + wp->w_ntrows) {/* mode line */
		curwp = wp;			/* just change buffer	 */
		curbp = wp->w_bufp;
	} else {
		/* move to selected window, move dot to top left	*/
		curwp = wp;
		curbp = wp->w_bufp;
		curwp->w_dotp = wp->w_linep;
		curwp->w_doto = 0;

		/* go forward the correct # of lines 		*/
		forwline(FALSE, newrow - curwp->w_toprow, KRANDOM);
	
		/* go forward the correct # of characters	*/
		/* need to count them out because of tabs	*/
		col = dot = 0;
		while ((col < newcol) && (dot < llength(curwp->w_dotp))) {
			c = lgetc(curwp->w_dotp, dot++);
			if (c == '\t')
				col |= 0x07;
			else if (ISCTRL(c) != FALSE)
				++col;
			++col;
		}
#ifdef		MEYN
		if (col > newcol) dot--;	/* back up to tab/ctrl char */
#endif
		forwchar(FALSE, dot, KRANDOM);
	}
	return (TRUE);
}
