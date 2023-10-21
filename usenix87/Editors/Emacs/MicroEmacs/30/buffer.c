/*
 * Name:	MicroEMACS
 *		Buffer handling.
 * Version:	30
 * Last edit:	17-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
#include	"def.h"

/*
 * Attach a buffer to a window. The
 * values of dot and mark come from the buffer
 * if the use count is 0. Otherwise, they come
 * from some other window.
 */
usebuffer(f, n, k)
{
	register BUFFER	*bp;
	register WINDOW	*wp;
	register int	s;
	char		bufn[NBUFN];

	if ((s=ereply("Use buffer: ", bufn, NBUFN)) != TRUE)
		return (s);
	if ((bp=bfind(bufn, TRUE)) == NULL)
		return (FALSE);
	if (--curbp->b_nwnd == 0) {		/* Last use.		*/
		curbp->b_dotp  = curwp->w_dotp;
		curbp->b_doto  = curwp->w_doto;
		curbp->b_markp = curwp->w_markp;
		curbp->b_marko = curwp->w_marko;
	}
	curbp = bp;				/* Switch.		*/
	curwp->w_bufp  = bp;
	curwp->w_linep = bp->b_linep;		/* For macros, ignored.	*/
	curwp->w_flag |= WFMODE|WFFORCE|WFHARD;	/* Quite nasty.		*/
	if (bp->b_nwnd++ == 0) {		/* First use.		*/
		curwp->w_dotp  = bp->b_dotp;
		curwp->w_doto  = bp->b_doto;
		curwp->w_markp = bp->b_markp;
		curwp->w_marko = bp->b_marko;
		return (TRUE);
	}
	wp = wheadp;				/* Look for old.	*/
	while (wp != NULL) {
		if (wp!=curwp && wp->w_bufp==bp) {
			curwp->w_dotp  = wp->w_dotp;
			curwp->w_doto  = wp->w_doto;
			curwp->w_markp = wp->w_markp;
			curwp->w_marko = wp->w_marko;
			break;
		}
		wp = wp->w_wndp;
	}
	return (TRUE);
}

/*
 * Dispose of a buffer, by name.
 * Ask for the name. Look it up (don't get too
 * upset if it isn't there at all!). Get quite upset
 * if the buffer is being displayed. Clear the buffer (ask
 * if the buffer has been changed). Then free the header
 * line and the buffer header. Bound to "C-X K".
 */
killbuffer(f, n, k)
{
	register BUFFER	*bp;
	register BUFFER	*bp1;
	register BUFFER	*bp2;
	register int	s;
	char		bufn[NBUFN];

	if ((s=ereply("Kill buffer: ", bufn, NBUFN)) != TRUE)
		return (s);
	if ((bp=bfind(bufn, FALSE)) == NULL)	/* Easy if unknown.	*/
		return (TRUE);
	if (bp->b_nwnd != 0) {			/* Error if on screen.	*/
		eprintf("Buffer is being displayed");
		return (FALSE);
	}
	if ((s=bclear(bp)) != TRUE)		/* Blow text away.	*/
		return (s);
	free((char *) bp->b_linep);		/* Release header line.	*/
	bp1 = NULL;				/* Find the header.	*/
	bp2 = bheadp;
	while (bp2 != bp) {
		bp1 = bp2;
		bp2 = bp2->b_bufp;
	}
	bp2 = bp2->b_bufp;			/* Next one in chain.	*/
	if (bp1 == NULL)			/* Unlink it.		*/
		bheadp = bp2;
	else
		bp1->b_bufp = bp2;
	free((char *) bp);			/* Release buffer block	*/
	return (TRUE);
}

/*
 * Display the buffer list. This is done
 * in two parts. The "makelist" routine figures out
 * the text, and puts it in the buffer whoses header is
 * pointed to by the external "blistp". The "popblist"
 * then pops the data onto the screen. Bound to
 * "C-X C-B".
 */
listbuffers(f, n, k)
{
	register int	s;

	if ((s=makelist()) != TRUE)
		return (s);
	return (popblist());
}

/*
 * Pop the special buffer whose
 * buffer header is pointed to by the external
 * variable "blistp" onto the screen. This is used
 * by the "listbuffers" routine (above) and by
 * some other packages. Returns a status.
 */
popblist()
{
	register WINDOW	*wp;
	register BUFFER	*bp;

	if (blistp->b_nwnd == 0) {		/* Not on screen yet.	*/
		if ((wp=wpopup()) == NULL)
			return (FALSE);
		bp = wp->w_bufp;
		if (--bp->b_nwnd == 0) {
			bp->b_dotp  = wp->w_dotp;
			bp->b_doto  = wp->w_doto;
			bp->b_markp = wp->w_markp;
			bp->b_marko = wp->w_marko;
		}
		wp->w_bufp  = blistp;
		++blistp->b_nwnd;
	}
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_bufp == blistp) {
			wp->w_linep = lforw(blistp->b_linep);
			wp->w_dotp  = lforw(blistp->b_linep);
			wp->w_doto  = 0;
			wp->w_markp = NULL;
			wp->w_marko = 0;
			wp->w_flag |= WFMODE|WFHARD;
		}
		wp = wp->w_wndp;
	}
	return (TRUE);
}

/*
 * This routine rebuilds the
 * text in the special secret buffer
 * that holds the buffer list. It is called
 * by the list buffers command. Return TRUE
 * if everything works. Return FALSE if there
 * is an error (if there is no memory).
 */
makelist()
{
	register char	*cp1;
	register char	*cp2;
	register int	c;
	register BUFFER	*bp;
	register LINE	*lp;
	register int	nbytes;
	register int	s;
	char		b[6+1];
	char		line[128];

	blistp->b_flag &= ~BFCHG;		/* Blow away old.	*/
	if ((s=bclear(blistp)) != TRUE)
		return (s);
	strcpy(blistp->b_fname, "");
	if (addline("C   Size Buffer           File") == FALSE
	||  addline("-   ---- ------           ----") == FALSE)
		return (FALSE);
	bp = bheadp;				/* For all buffers	*/
	while (bp != NULL) {
		cp1 = &line[0];			/* Start at left edge	*/
		if ((bp->b_flag&BFCHG) != 0)	/* "*" if changed	*/
			*cp1++ = '*';
		else
			*cp1++ = ' ';
		*cp1++ = ' ';			/* Gap.			*/
		nbytes = 0;			/* Count bytes in buf.	*/
		lp = lforw(bp->b_linep);
		while (lp != bp->b_linep) {
			nbytes += llength(lp)+1;
			lp = lforw(lp);
		}
		itoa(b, 6, nbytes);		/* 6 digit buffer size.	*/
		cp2 = &b[0];
		while ((c = *cp2++) != 0)
			*cp1++ = c;
		*cp1++ = ' ';			/* Gap.			*/
		cp2 = &bp->b_bname[0];		/* Buffer name		*/
		while ((c = *cp2++) != 0)
			*cp1++ = c;
		cp2 = &bp->b_fname[0];		/* File name		*/
		if (*cp2 != 0) {
			while (cp1 < &line[1+1+6+1+NBUFN+1])
				*cp1++ = ' ';		
			while ((c = *cp2++) != 0) {
				if (cp1 < &line[128-1])
					*cp1++ = c;
			}
		}
		*cp1 = 0;			/* Add to the buffer.	*/
		if (addline(line) == FALSE)
			return (FALSE);
		bp = bp->b_bufp;
	}
	return (TRUE);				/* All done		*/
}

/*
 * Used above.
 */
itoa(buf, width, num)
register char	buf[];
register int	width;
register int	num;
{
	buf[width] = 0;				/* End of string.	*/
	while (num >= 10) {			/* Conditional digits.	*/
		buf[--width] = (num%10) + '0';
		num /= 10;
	}
	buf[--width] = num + '0';		/* Always 1 digit.	*/
	while (width != 0)			/* Pad with blanks.	*/
		buf[--width] = ' ';
}

/*
 * The argument "text" points to
 * a string. Append this line to the
 * buffer list buffer. Handcraft the EOL
 * on the end. Return TRUE if it worked and
 * FALSE if you ran out of room.
 */
addline(text)
char	*text;
{
	register LINE	*lp;
	register int	i;
	register int	ntext;

	ntext = strlen(text);
	if ((lp=lalloc(ntext)) == NULL)
		return (FALSE);
	for (i=0; i<ntext; ++i)
		lputc(lp, i, text[i]);
	blistp->b_linep->l_bp->l_fp = lp;	/* Hook onto the end	*/
	lp->l_bp = blistp->b_linep->l_bp;
	blistp->b_linep->l_bp = lp;
	lp->l_fp = blistp->b_linep;
	if (blistp->b_dotp == blistp->b_linep)	/* If "." is at the end	*/
		blistp->b_dotp = lp;		/* move it to new line	*/
	return (TRUE);
}

/*
 * Look through the list of
 * buffers. Return TRUE if there
 * are any changed buffers. Special buffers
 * like the buffer list buffer don't count, as
 * they are not in the list. Return FALSE if
 * there are no changed buffers.
 */
anycb()
{
	register BUFFER	*bp;

	bp = bheadp;
	while (bp != NULL) {
		if ((bp->b_flag&BFCHG) != 0)
			return (TRUE);
		bp = bp->b_bufp;
	}
	return (FALSE);
}

/*
 * Search for a buffer, by name.
 * If not found, and the "cflag" is TRUE,
 * create a buffer and put it in the list of
 * all buffers. Return pointer to the BUFFER
 * block for the buffer.
 */
BUFFER	*
bfind(bname, cflag)
register char	*bname;
{
	register BUFFER	*bp;

	bp = bheadp;
	while (bp != NULL) {
		if (strcmp(bname, bp->b_bname) == 0)
			return (bp);
		bp = bp->b_bufp;
	}
	if (cflag!=FALSE && (bp=bcreate(bname))!=NULL) {
		bp->b_bufp = bheadp;
		bheadp = bp;
	}
	return (bp);
}

/*
 * Create a buffer, by name.
 * Return a pointer to the BUFFER header
 * block, or NULL if the buffer cannot
 * be created. The BUFFER is not put in the
 * list of all buffers; this is called by
 * "edinit" to create the buffer list
 * buffer.
 */
BUFFER	*
bcreate(bname)
register char	*bname;
{
	register BUFFER	*bp;
	register LINE	*lp;

	if ((bp=(BUFFER *)malloc(sizeof(BUFFER))) == NULL)
		return (NULL);
	if ((lp=lalloc(0)) == NULL) {
		free((char *) bp);
		return (NULL);
	}
	bp->b_bufp  = NULL;
	bp->b_dotp  = lp;
	bp->b_doto  = 0;
	bp->b_markp = NULL;
	bp->b_marko = 0;
	bp->b_flag  = 0;
	bp->b_nwnd  = 0;
	bp->b_linep = lp;
	strcpy(bp->b_fname, "");
	strcpy(bp->b_bname, bname);
	lp->l_fp = lp;
	lp->l_bp = lp;
	return (bp);
}

/*
 * This routine blows away all of the text
 * in a buffer. If the buffer is marked as changed
 * then we ask if it is ok to blow it away; this is
 * to save the user the grief of losing text. The
 * window chain is nearly always wrong if this gets
 * called; the caller must arrange for the updates
 * that are required. Return TRUE if everything
 * looks good.
 */
bclear(bp)
register BUFFER	*bp;
{
	register LINE	*lp;
	register int	s;
	
	if ((bp->b_flag&BFCHG) != 0		/* Changed.		*/
	&& (s=eyesno("Discard changes")) != TRUE)
		return (s);
	bp->b_flag  &= ~BFCHG;			/* Not changed		*/
	while ((lp=lforw(bp->b_linep)) != bp->b_linep)
		lfree(lp);
	bp->b_dotp  = bp->b_linep;		/* Fix "."		*/
	bp->b_doto  = 0;
	bp->b_markp = NULL;			/* Invalidate "mark"	*/
	bp->b_marko = 0;
	return (TRUE);
}
