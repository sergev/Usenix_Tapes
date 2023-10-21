/*
 *		Text line handling.
 * The functions in this file
 * are a general set of line management
 * utilities. They are the only routines that
 * touch the text. They also touch the buffer
 * and window structures, to make sure that the
 * necessary updating gets done. There are routines
 * in this file that handle the kill buffer too.
 * It isn't here for any good reason.
 *
 * Note that this code only updates the dot and
 * mark values in the window list. Since all the code
 * acts on the current window, the buffer that we
 * are editing must be being displayed, which means
 * that "b_nwnd" is non zero, which means that the
 * dot and mark values in the buffer headers are
 * nonsense.
 */
#include	"def.h"

#define	NBLOCK	16			/* Line block chunk size	*/

#ifndef	KBLOCK
#define	KBLOCK	256			/* Kill buffer block size.	*/
#endif

static char	*kbufp	= NULL;		/* Kill buffer data.		*/
static RSIZE	kused	= 0;		/* # of bytes used in KB.	*/
static RSIZE	ksize	= 0;		/* # of bytes allocated in KB.	*/
static RSIZE	kstart  = 0;		/* # of first used byte in KB.	*/
char	*malloc();
/*
 * This routine allocates a block
 * of memory large enough to hold a LINE
 * containing "used" characters. The block is
 * always rounded up a bit. Return a pointer
 * to the new block, or NULL if there isn't
 * any memory left. Print a message in the
 * message line if no space.
 */
LINE	*
lalloc(used) register RSIZE used; {
	register LINE	*lp;
	register int	size;

	/*NOSTRICT*/
	size = (NBLOCK-1+used) & ~(NBLOCK-1);
	if (size == 0)				/* Assume that an empty	*/
		size = NBLOCK;			/* line is for type-in.	*/
	/*NOSTRICT*/
	if ((lp=(LINE *)malloc(sizeof(LINE)+size)) == NULL) {
		ewprintf("Can't get %d bytes", sizeof(LINE) + size);
		return (NULL);
	}
	lp->l_size = size;
	/*NOSTRICT*/
	lp->l_used = used;
	return (lp);
}

/*
 * Delete line "lp". Fix all of the
 * links that might point at it (they are
 * moved to offset 0 of the next line.
 * Unlink the line from whatever buffer it
 * might be in. Release the memory. The
 * buffers are updated too; the magic conditions
 * described in the above comments don't hold
 * here.
 */
lfree(lp) register LINE *lp; {
	register BUFFER	*bp;
	register WINDOW	*wp;

	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_linep == lp)
			wp->w_linep = lp->l_fp;
		if (wp->w_dotp  == lp) {
			wp->w_dotp  = lp->l_fp;
			wp->w_doto  = 0;
		}
		if (wp->w_markp == lp) {
			wp->w_markp = lp->l_fp;
			wp->w_marko = 0;
		}
		wp = wp->w_wndp;
	}
	bp = bheadp;
	while (bp != NULL) {
		if (bp->b_nwnd == 0) {
			if (bp->b_dotp  == lp) {
				bp->b_dotp = lp->l_fp;
				bp->b_doto = 0;
			}
			if (bp->b_markp == lp) {
				bp->b_markp = lp->l_fp;
				bp->b_marko = 0;
			}
		}
		bp = bp->b_bufp;
	}
	lp->l_bp->l_fp = lp->l_fp;
	lp->l_fp->l_bp = lp->l_bp;
	free((char *) lp);
}

/*
 * This routine gets called when
 * a character is changed in place in the
 * current buffer. It updates all of the required
 * flags in the buffer and window system. The flag
 * used is passed as an argument; if the buffer is being
 * displayed in more than 1 window we change EDIT to
 * HARD. Set MODE if the mode line needs to be
 * updated (the "*" has to be set).
 */
lchange(flag) register int flag; {
	register WINDOW	*wp;

	if (curbp->b_nwnd != 1)			/* Ensure hard.		*/
		flag = WFHARD;
	if ((curbp->b_flag&BFCHG) == 0) {	/* First change, so 	*/
		flag |= WFMODE;			/* update mode lines.	*/
		curbp->b_flag |= BFCHG;
	}
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_bufp == curbp)
			wp->w_flag |= flag;
		wp = wp->w_wndp;
	}
}

/*
 * Insert "n" copies of the character "c"
 * at the current location of dot. In the easy case
 * all that happens is the text is stored in the line.
 * In the hard case, the line has to be reallocated.
 * When the window list is updated, take special
 * care; I screwed it up once. You always update dot
 * in the current window. You update mark, and a
 * dot in another window, if it is greater than
 * the place where you did the insert. Return TRUE
 * if all is well, and FALSE on errors.
 */
linsert(n, c) RSIZE n; {
	register char	*cp1;
	register char	*cp2;
	register LINE	*lp1;
	register LINE	*lp2;
	register LINE	*lp3;
	register int	doto;
	register RSIZE	i;
	register WINDOW	*wp;

	lchange(WFEDIT);
	lp1 = curwp->w_dotp;			/* Current line		*/
	if (lp1 == curbp->b_linep) {		/* At the end: special	*/
		if (curwp->w_doto != 0) {
			ewprintf("bug: linsert");
			return (FALSE);
		}
		if ((lp2=lalloc(n)) == NULL)	/* Allocate new line	*/
			return (FALSE);
		lp3 = lp1->l_bp;		/* Previous line	*/
		lp3->l_fp = lp2;		/* Link in		*/
		lp2->l_fp = lp1;
		lp1->l_bp = lp2;
		lp2->l_bp = lp3;
		for (i=0; i<n; ++i)
			lp2->l_text[i] = c;
		wp = wheadp;			/* Update windows	*/
		while (wp != NULL) {
			if (wp->w_linep == lp1)
				wp->w_linep = lp2;
			if (wp->w_dotp == lp1)
				wp->w_dotp = lp2;
			if (wp->w_markp == lp1)
				wp->w_markp = lp2;
			wp = wp->w_wndp;
		}
		/*NOSTRICT*/
		curwp->w_doto = n;
		return (TRUE);
	}
	doto = curwp->w_doto;			/* Save for later.	*/
	/*NOSTRICT (2) */
	if (lp1->l_used+n > lp1->l_size) {	/* Hard: reallocate	*/
		if ((lp2=lalloc((RSIZE) (lp1->l_used+n))) == NULL)
			return (FALSE);
		cp1 = &lp1->l_text[0];
		cp2 = &lp2->l_text[0];
		while (cp1 != &lp1->l_text[doto])
			*cp2++ = *cp1++;
		/*NOSTRICT*/
		cp2 += n;
		while (cp1 != &lp1->l_text[lp1->l_used])
			*cp2++ = *cp1++;
		lp1->l_bp->l_fp = lp2;
		lp2->l_fp = lp1->l_fp;
		lp1->l_fp->l_bp = lp2;
		lp2->l_bp = lp1->l_bp;
		free((char *) lp1);
	} else {				/* Easy: in place	*/
		lp2 = lp1;			/* Pretend new line	*/
		/*NOSTRICT*/
		lp2->l_used += n;
		cp2 = &lp1->l_text[lp1->l_used];

		cp1 = cp2-n;
		while (cp1 != &lp1->l_text[doto])
			*--cp2 = *--cp1;
	}
	for (i=0; i<n; ++i)			/* Add the characters	*/
		lp2->l_text[doto+i] = c;
	wp = wheadp;				/* Update windows	*/
	while (wp != NULL) {
		if (wp->w_linep == lp1)
			wp->w_linep = lp2;
		if (wp->w_dotp == lp1) {
			wp->w_dotp = lp2;
			if (wp==curwp || wp->w_doto>doto)
				/*NOSTRICT*/
				wp->w_doto += n;
		}
		if (wp->w_markp == lp1) {
			wp->w_markp = lp2;
			if (wp->w_marko > doto)
				/*NOSTRICT*/
				wp->w_marko += n;
		}
		wp = wp->w_wndp;
	}
	return (TRUE);
}

/*
 * Insert a newline into the buffer
 * at the current location of dot in the current
 * window. The funny ass-backwards way it does things
 * is not a botch; it just makes the last line in
 * the file not a special case. Return TRUE if everything
 * works out and FALSE on error (memory allocation
 * failure). The update of dot and mark is a bit
 * easier then in the above case, because the split
 * forces more updating.
 */
lnewline() {
	register char	*cp1;
	register char	*cp2;
	register LINE	*lp1;
	register LINE	*lp2;
	register int	doto;
	register WINDOW	*wp;

	lchange(WFHARD);
	lp1  = curwp->w_dotp;			/* Get the address and	*/
	doto = curwp->w_doto;			/* offset of "."	*/
	if ((lp2=lalloc((RSIZE) doto)) == NULL)	/* New first half line	*/
		return (FALSE);
	cp1 = &lp1->l_text[0];			/* Shuffle text around	*/
	cp2 = &lp2->l_text[0];
	while (cp1 != &lp1->l_text[doto])
		*cp2++ = *cp1++;
	cp2 = &lp1->l_text[0];
	while (cp1 != &lp1->l_text[lp1->l_used])
		*cp2++ = *cp1++;
	lp1->l_used -= doto;
	lp2->l_bp = lp1->l_bp;
	lp1->l_bp = lp2;
	lp2->l_bp->l_fp = lp2;
	lp2->l_fp = lp1;
	wp = wheadp;				/* Windows		*/
	while (wp != NULL) {
		if (wp->w_linep == lp1)
			wp->w_linep = lp2;
		if (wp->w_dotp == lp1) {
			if (wp->w_doto < doto)
				wp->w_dotp = lp2;
			else
				wp->w_doto -= doto;
		}
		if (wp->w_markp == lp1) {
			if (wp->w_marko < doto)
				wp->w_markp = lp2;
			else
				wp->w_marko -= doto;
		}
		wp = wp->w_wndp;
	}	
	return (TRUE);
}

/*
 * This function deletes "n" bytes,
 * starting at dot. It understands how do deal
 * with end of lines, etc. It returns TRUE if all
 * of the characters were deleted, and FALSE if
 * they were not (because dot ran into the end of
 * the buffer. The "kflag" indicates either no insertion,
 * or direction of insertion into the kill buffer.
 */
ldelete(n, kflag) RSIZE n; {
	register char	*cp1;
	register char	*cp2;
	register LINE	*dotp;
	register int	doto;
	register RSIZE	chunk;
	register WINDOW	*wp;

	/*
	 * HACK - doesn't matter, and fixes back-over-nl bug for empty
	 *	kill buffers.
	 */
	if (kused == kstart) kflag = KFORW;

	while (n != 0) {
		dotp = curwp->w_dotp;
		doto = curwp->w_doto;
		if (dotp == curbp->b_linep)	/* Hit end of buffer.	*/
			return (FALSE);
		chunk = dotp->l_used-doto;	/* Size of chunk.	*/
		if (chunk > n)
			chunk = n;
		if (chunk == 0) {		/* End of line, merge.	*/
			lchange(WFHARD);
			if (ldelnewline() == FALSE
			|| (kflag!=KNONE && kinsert('\n', kflag)==FALSE))
				return (FALSE);
			--n;
			continue;
		}
		lchange(WFEDIT);
		cp1 = &dotp->l_text[doto];	/* Scrunch text.	*/
		cp2 = cp1 + chunk;
		if (kflag == KFORW) {
			while (ksize - kused < chunk)
				if (kgrow(FALSE) == FALSE) return FALSE;
			bcopy(cp1, &(kbufp[kused]), (int) chunk);
			kused += chunk;
		} else if (kflag == KBACK) {
			while (kstart < chunk)
				if (kgrow(TRUE) == FALSE) return FALSE;
			bcopy(cp1, &(kbufp[kstart-chunk]), (int) chunk);
			kstart -= chunk;
		} else if (kflag != KNONE) panic("broken ldelete call");
		while (cp2 != &dotp->l_text[dotp->l_used])
			*cp1++ = *cp2++;
		dotp->l_used -= (int) chunk;
		wp = wheadp;			/* Fix windows		*/
		while (wp != NULL) {
			if (wp->w_dotp==dotp && wp->w_doto>=doto) {
				/*NOSTRICT*/
				wp->w_doto -= chunk;
				if (wp->w_doto < doto)
					wp->w_doto = doto;
			}	
			if (wp->w_markp==dotp && wp->w_marko>=doto) {
				/*NOSTRICT*/
				wp->w_marko -= chunk;
				if (wp->w_marko < doto)
					wp->w_marko = doto;
			}
			wp = wp->w_wndp;
		}
		n -= chunk;
	}
	return (TRUE);
}

/*
 * Delete a newline. Join the current line
 * with the next line. If the next line is the magic
 * header line always return TRUE; merging the last line
 * with the header line can be thought of as always being a
 * successful operation, even if nothing is done, and this makes
 * the kill buffer work "right". Easy cases can be done by
 * shuffling data around. Hard cases require that lines be moved
 * about in memory. Return FALSE on error and TRUE if all
 * looks ok.
 */
ldelnewline() {
	register char	*cp1;
	register char	*cp2;
	register LINE	*lp1;
	register LINE	*lp2;
	register LINE	*lp3;
	register WINDOW	*wp;

	lp1 = curwp->w_dotp;
	lp2 = lp1->l_fp;
	if (lp2 == curbp->b_linep) {		/* At the buffer end.	*/
		if (lp1->l_used == 0)		/* Blank line.		*/
			lfree(lp1);
		return (TRUE);
	}
	if (lp2->l_used <= lp1->l_size-lp1->l_used) {
		cp1 = &lp1->l_text[lp1->l_used];
		cp2 = &lp2->l_text[0];
		while (cp2 != &lp2->l_text[lp2->l_used])
			*cp1++ = *cp2++;
		wp = wheadp;
		while (wp != NULL) {
			if (wp->w_linep == lp2)
				wp->w_linep = lp1;
			if (wp->w_dotp == lp2) {
				wp->w_dotp  = lp1;
				wp->w_doto += lp1->l_used;
			}
			if (wp->w_markp == lp2) {
				wp->w_markp  = lp1;
				wp->w_marko += lp1->l_used;
			}
			wp = wp->w_wndp;
		}		
		lp1->l_used += lp2->l_used;
		lp1->l_fp = lp2->l_fp;
		lp2->l_fp->l_bp = lp1;
		free((char *) lp2);
		return (TRUE);
	}
	if ((lp3=lalloc((RSIZE) (lp1->l_used+lp2->l_used))) == NULL)
		return (FALSE);
	cp1 = &lp1->l_text[0];
	cp2 = &lp3->l_text[0];
	while (cp1 != &lp1->l_text[lp1->l_used])
		*cp2++ = *cp1++;
	cp1 = &lp2->l_text[0];
	while (cp1 != &lp2->l_text[lp2->l_used])
		*cp2++ = *cp1++;
	lp1->l_bp->l_fp = lp3;
	lp3->l_fp = lp2->l_fp;
	lp2->l_fp->l_bp = lp3;
	lp3->l_bp = lp1->l_bp;
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_linep==lp1 || wp->w_linep==lp2)
			wp->w_linep = lp3;
		if (wp->w_dotp == lp1)
			wp->w_dotp  = lp3;
		else if (wp->w_dotp == lp2) {
			wp->w_dotp  = lp3;
			wp->w_doto += lp1->l_used;
		}
		if (wp->w_markp == lp1)
			wp->w_markp  = lp3;
		else if (wp->w_markp == lp2) {
			wp->w_markp  = lp3;
			wp->w_marko += lp1->l_used;
		}
		wp = wp->w_wndp;
	}
	free((char *) lp1);
	free((char *) lp2);
	return (TRUE);
}

/*
 * Replace plen characters before dot with argument string.
 * Control-J characters in st are interpreted as newlines.
 * There is a casehack disable flag (normally it likes to match
 * case of replacement to what was there).
 */
lreplace(plen, st, f)
register RSIZE	plen;			/* length to remove		*/
char		*st;			/* replacement string		*/
int		f;			/* case hack disable		*/
{
	register RSIZE	rlen;		/* replacement length		*/
	register int	rtype;		/* capitalization 		*/
	register int	c;		/* used for random characters	*/
	register int	doto;		/* offset into line		*/

	/*
	 * Find the capitalization of the word that was found.
	 * f says use exact case of replacement string (same thing that
	 * happens with lowercase found), so bypass check.
	 */
	/*NOSTRICT*/
	(VOID) backchar(TRUE, (int) plen, KRANDOM);
	rtype = _L;
	c = lgetc(curwp->w_dotp, curwp->w_doto);
	if (ISUPPER(c)!=FALSE  &&  f==FALSE) {
		rtype = _U|_L;
		if (curwp->w_doto+1 < llength(curwp->w_dotp)) {
			c = lgetc(curwp->w_dotp, curwp->w_doto+1);
			if (ISUPPER(c) != FALSE) {
				rtype = _U;
			}
		}
	}

	/*
	 * make the string lengths match (either pad the line
	 * so that it will fit, or scrunch out the excess).
	 * be careful with dot's offset.
	 */
	rlen = strlen(st);
	doto = curwp->w_doto;
	if (plen > rlen)
		(VOID) ldelete((RSIZE) (plen-rlen), KNONE);
	else if (plen < rlen) {
		if (linsert((RSIZE) (rlen-plen), ' ') == FALSE)
			return (FALSE);
	}
	curwp->w_doto = doto;

	/*
	 * do the replacement:  If was capital, then place first 
	 * char as if upper, and subsequent chars as if lower.  
	 * If inserting upper, check replacement for case.
	 */
	while ((c = *st++&0xff) != '\0') {
		if ((rtype&_U)!=0  &&  ISLOWER(c)!=0)
			c = TOUPPER(c);
		if (rtype == (_U|_L))
			rtype = _L;
		if (c == SEOL) {
			if (curwp->w_doto == llength(curwp->w_dotp))
				(VOID) forwchar(FALSE, 1, KRANDOM);
			else {
				if (ldelete((RSIZE) 1, KNONE) != FALSE)
					(VOID) lnewline();
			}
		} else if (curwp->w_dotp == curbp->b_linep) {
			(VOID) linsert((RSIZE) 1, c);
		} else if (curwp->w_doto == llength(curwp->w_dotp)) {
			if (ldelete((RSIZE) 1, KNONE) != FALSE)
				(VOID) linsert((RSIZE) 1, c);
		} else
			lputc(curwp->w_dotp, curwp->w_doto++, c);
	}
	lchange(WFHARD);
	return (TRUE);
}

/*
 * Delete all of the text
 * saved in the kill buffer. Called by commands
 * when a new kill context is being created. The kill
 * buffer array is released, just in case the buffer has
 * grown to immense size. No errors.
 */
kdelete() {
	if (kbufp != NULL) {
		free((char *) kbufp);
		kbufp = NULL;
		kstart = kused = ksize = 0;
	}
}

/*
 * Insert a character to the kill buffer,
 * enlarging the buffer if there isn't any room. Always
 * grow the buffer in chunks, on the assumption that if you
 * put something in the kill buffer you are going to put
 * more stuff there too later. Return TRUE if all is
 * well, and FALSE on errors. Print a message on
 * errors. Dir says whether to put it at back or front.
 */
kinsert(c, dir) {

	if (kused == ksize && dir == KFORW && kgrow(FALSE) == FALSE)
		return FALSE;
	if (kstart == 0 && dir == KBACK && kgrow(TRUE) == FALSE)
		return FALSE;
	if (dir == KFORW) kbufp[kused++] = c;
	else if (dir == KBACK) kbufp[--kstart] = c;
	else panic("broken kinsert call");		/* Oh shit! */
	return (TRUE);
}

/*
 * kgrow - just get more kill buffer for the callee. back is true if
 * we are trying to get space at the beginning of the kill buffer.
 */
kgrow(back) {
	register int	nstart;
	register char	*nbufp;

	if ((nbufp=malloc((int)(ksize+KBLOCK))) == NULL) {
		ewprintf("Can't get %ld bytes", (long)(ksize+KBLOCK));
		return (FALSE);
	}
	nstart = (back == TRUE) ? (kstart + KBLOCK) : (KBLOCK / 4) ;
	bcopy(&(kbufp[kstart]), &(nbufp[nstart]), (int) (kused-kstart));
	if (kbufp != NULL)
		free((char *) kbufp);
	kbufp  = nbufp;
	ksize += KBLOCK;
	kused = kused - kstart + nstart;
	kstart = nstart;
	return TRUE;
}

/*
 * This function gets characters from
 * the kill buffer. If the character index "n" is
 * off the end, it returns "-1". This lets the caller
 * just scan along until it gets a "-1" back.
 */
kremove(n) {
	if (n < 0 || n + kstart >= kused)
		return (-1);
	return (kbufp[n + kstart] & 0xFF);
}
