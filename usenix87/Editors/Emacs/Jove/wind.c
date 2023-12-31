/************************************************************************
 * This program is Copyright (C) 1986 by Jonathan Payne.  JOVE is       *
 * provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is *
 * included in all the files.                                           *
 ************************************************************************/

/* This creates/deletes/divides/grows/shrinks windows.  */

#include "jove.h"
#include "termcap.h"

static char	onlyone[] = "You only have one window!",
		toosmall[] = "Resulting window would be too small.";

Window	*curwind,
	*fwind = 0;

/* First line in a Window */

FLine(w)
register Window	*w;
{
	register Window	*wp = fwind;
	register int	lineno = -1;

	do {
		if (wp == w)
			return lineno + 1;
		lineno += wp->w_height;
		wp = wp->w_next;
	} while (wp != fwind);
	complain("window?");
	/* NOTREACHED */
}

/* Delete `wp' from the screen.  If it is the only window left
   on the screen, then complain.  It gives its body
   to the next window if there is one, otherwise the previous
   window gets the body.  */

del_wind(wp)
register Window	*wp;
{
	register Window	*prev = wp->w_prev;

	if (one_windp())
		complain(onlyone);

	wp->w_prev->w_next = wp->w_next;
	wp->w_next->w_prev = wp->w_prev;
	
	if (fwind == wp) {
		fwind = wp->w_next;
		fwind->w_height += wp->w_height;
		/* Here try to do something intelligent for redisplay() */
		SetTop(fwind, prev_line(fwind->w_top, wp->w_height));
		if (curwind == wp)
			SetWind(fwind);
	} else {
		prev->w_height += wp->w_height;
		if (curwind == wp)
			SetWind(prev);
	}
	free((char *) wp);
}

/* Divide the window WP N times, or at least once.  Complains if WP is too
   small to be split into that many pieces.  It returns the new window. */

Window *
div_wind(wp, n)
register Window	*wp;
{
	register Window	*new;
	int	amt;

	if (n < 1)
		n = 1;
	amt = wp->w_height / (n + 1);
	if (amt < 2)
		complain(toosmall);

	while (--n >= 0) {
		new = (Window *) emalloc(sizeof (Window));
		new->w_flags = 0;

		new->w_height = amt;
		wp->w_height -= amt;

		/* set the lines such that w_line is the center in
		   each Window */
		new->w_line = wp->w_line;
		new->w_char = wp->w_char;
		new->w_bufp = wp->w_bufp;
		new->w_top = prev_line(new->w_line, HALF(new));

		/* Link the new window into the list */
		new->w_prev = wp;
		new->w_next = wp->w_next;
		new->w_next->w_prev = new;
		wp->w_next = new;
	}
	return new;
}

/* Initialze the first window setting the bounds to the size of the
   screen.  There is no buffer with this window.  See parse for the
   setting of this window. */

winit()
{
	register Window	*w;

	w = curwind = fwind = (Window *) emalloc(sizeof (Window));
	w->w_line = w->w_top = 0;
	w->w_flags = 0;
	w->w_char = 0;
	w->w_next = w->w_prev = fwind;
	w->w_height = ILI;
}

/* Change to previous window. */

PrevWindow()
{
	register Window	*new = curwind->w_prev;

	if (one_windp())
		complain(onlyone);
	SetWind(new);
}

/* Make NEW the current Window */

SetWind(new)
register Window	*new;
{
	if (!Asking){		/* can you say kludge? */
		curwind->w_line = curline;
		curwind->w_char = curchar;
		curwind->w_bufp = curbuf;
	}
	if (new == curwind)
		return;
	SetBuf(new->w_bufp);
	if (!inlist(new->w_bufp->b_first, new->w_line)) {
		new->w_line = curline;
		new->w_char = curchar;
	}
	DotTo(new->w_line, new->w_char);
	if (curchar > strlen(linebuf))
		new->w_char = curchar = strlen(linebuf);
	curwind = new;
}

/* delete the current window if it isn't the only one left */

DelCurWindow()
{
	SetABuf(curwind->w_bufp);
	del_wind(curwind);
}

/* put the current line of `w' in the middle of the window */

CentWind(w)
register Window	*w;
{
	SetTop(w, prev_line(w->w_line, HALF(w)));
}

int	ScrollStep = 0;		/* full scrolling */

/* Calculate the new topline of the window.  If ScrollStep == 0
   it means we should center the current line in the window. */

CalcWind(w)
register Window	*w;
{
	register int	up;
	Line	*newtop;

	if (ScrollStep == 0)	/* Means just center it */
		CentWind(w);
	else {
		up = inorder(w->w_line, 0, w->w_top, 0);
		if (up == -1) {
			CentWind(w);
			return;
		}
		if (up)		/* Dot is above the screen */
			newtop = prev_line(w->w_line, min(ScrollStep - 1, HALF(w)));
		else
			newtop = prev_line(w->w_line, (SIZE(w) - 1) -
						min(ScrollStep - 1, HALF(w)));
		if (LineDist(newtop, w->w_top) >= SIZE(w) - 1)
			CentWind(w);
		else
			SetTop(w, newtop);
	}
}

/* This is bound to C-X 4 [BTF].  To make the screen stay the
   same we have to remember various things, like the current
   top line in the current window.  It's sorta gross, but it's
   necessary because of the way this is implemented (i.e., in
   terms of do_find(), do_select() which manipulate the windows. */
WindFind()
{
	register Buffer	*obuf = curbuf,
			*nbuf;
	Line	*ltop = curwind->w_top;
	Bufpos	savedot;
	extern int
		FindTag(),
		BufSelect(),
		FindFile();

	DOTsave(&savedot);

	switch (waitchar()) {
	case 't':
	case 'T':
		ExecCmd((data_obj *) FindCmd(FindTag));
		break;

	case 'b':
	case 'B':
		ExecCmd((data_obj *) FindCmd(BufSelect));
		break;

	case 'f':
	case 'F':
		ExecCmd((data_obj *) FindCmd(FindFile));
		break;

	default:
		complain("T: find-tag, F: find-file, B: select-buffer.");
	}

	nbuf = curbuf;
	SetBuf(obuf);
	SetDot(&savedot);
	SetTop(curwind, ltop);	/* there! it's as if we did nothing */

	if (one_windp())
		(void) div_wind(curwind, 1);

	tiewind(curwind->w_next, nbuf);
	SetWind(curwind->w_next);
}

/* Go into one window mode by deleting all the other windows */

OneWindow()
{
	while (curwind->w_next != curwind)
		del_wind(curwind->w_next);
}

Window *
windbp(bp)
register Buffer	*bp;
{

	register Window	*wp = fwind;

	if (bp == 0)
		return 0;
	do {
		if (wp->w_bufp == bp)
			return wp;
		wp = wp->w_next;
	} while (wp != fwind);
	return 0;
}

/* Change window into the next window.  Curwind becomes the new window. */

NextWindow()
{
	register Window	*new = curwind->w_next;

	if (one_windp())
		complain(onlyone);
	SetWind(new);
}

/* Scroll the next Window */

PageNWind()
{
	if (one_windp())
		complain(onlyone);
	NextWindow();
	NextPage();
	PrevWindow();
}

Window *
w_nam_typ(name, type)
register char	*name;
{
	register Window *w;
	register Buffer	*b;

	b = buf_exists(name);
	w = fwind;
	if (b) do {
		if (w->w_bufp == b)
			return w;
	} while ((w = w->w_next) != fwind);

	w = fwind;
	do {
		if (w->w_bufp->b_type == type)
			return w;
	} while ((w = w->w_next) != fwind);

	return 0;
}

/* Put a window with the buffer `name' in it.  Erase the buffer if
   `clobber' is non-zero. */

pop_wind(name, clobber, btype)
register char	*name;
{
	register Window	*wp;
	register Buffer	*newb;

	if (newb = buf_exists(name))
		btype = -1;	/* if the buffer exists, don't change
				   it's type */
	if ((wp = w_nam_typ(name, btype)) == 0) {
		if (one_windp())
			SetWind(div_wind(curwind, 1));
		else
			PrevWindow();
	} else
		SetWind(wp);

	newb = do_select((Window *) 0, name);
	if (clobber) {
		initlist(newb);
		newb->b_modified = NO;
	}
	tiewind(curwind, newb);
	if (btype != -1)
		newb->b_type = btype;
	SetBuf(newb);
}

GrowWindow()
{
	WindSize(curwind, abs(exp));
}

ShrWindow()
{
	WindSize(curwind, -abs(exp));
}

/* Change the size of the window by inc.  First arg is the window,
   second is the increment. */

WindSize(w, inc)
register Window	*w;
register int	inc;
{
	if (one_windp())
		complain(onlyone);

	if (inc == 0)
		return;
	else if (inc < 0) {	/* Shrinking this Window. */
		if (w->w_height + inc < 2)
			complain(toosmall);
		w->w_height += inc;
		w->w_prev->w_height -= inc;
	} else			/* Growing the window. */
		WindSize(w->w_next, -inc);
}

/* Set the topline of the window, calculating its number in the buffer.
   This is for numbering the lines only. */

SetTop(w, line)
Window	*w;
register Line	*line;
{
	register Line	*lp = w->w_bufp->b_first;
	register int	num = 0;

	w->w_top = line;
	if (w->w_flags & W_NUMLINES) {
		while (lp) {
			num++;
			if (line == lp)
				break;
			lp = lp->l_next;
		}
		w->w_topnum = num;
	}
}

WNumLines()
{
	curwind->w_flags ^= W_NUMLINES; 
	SetTop(curwind, curwind->w_top);
}

WVisSpace()
{
	curwind->w_flags ^= W_VISSPACE;
	ClAndRedraw();
}

/* Return the line number that `line' occupies in `windes' */

in_window(windes, line)
register Window	*windes;
register Line	*line;
{
	register int	i;
	register Line	*top = windes->w_top;

	for (i = 0; top && i < windes->w_height - 1; i++, top = top->l_next)
		if (top == line)
			return FLine(windes) + i;
	return -1;
}

SplitWind()
{
	SetWind(div_wind(curwind, exp_p ? (exp - 1) : 1));
}
