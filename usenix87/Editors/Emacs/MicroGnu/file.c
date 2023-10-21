/*
 * 		File commands.
 */
#include	"def.h"

BUFFER	*findbuffer();

/*
 * insert a file into the current buffer. Real easy - just call the
 * insertfile routine with the file name.
 */
/*ARGSUSED*/
fileinsert(f, n, k) {
	register int	s;
	char		fname[NFILEN];

	if ((s=ereply("Insert file: ", fname, NFILEN)) != TRUE)
		return (s);
	adjustcase(fname);
	return (insertfile(fname, (char *) NULL)); /* don't set buffer name */
}

/*
 * Select a file for editing.
 * Look around to see if you can find the
 * fine in another buffer; if you can find it
 * just switch to the buffer. If you cannot find
 * the file, create a new buffer, read in the
 * text, and switch to the new buffer.
 */
/*ARGSUSED*/
filevisit(f, n, k) {
	register BUFFER	*bp;
	int		s;
	char		fname[NFILEN];

	if ((s=ereply("Find file: ", fname, NFILEN)) != TRUE)
		return (s);
	if ((bp = findbuffer(fname, &s)) == NULL) return s;
	curbp = bp;
	if (showbuffer(bp, curwp, WFHARD) != TRUE) return FALSE;
	if (bp->b_fname[0] == 0)
		return (readin(fname));		/* Read it in.		*/
	return TRUE;
}

/*
 * Pop to a file in the other window. Same as last function, just
 * popbuf instead of showbuffer.
 */
/*ARGSUSED*/
poptofile(f, n, k) {
	register BUFFER	*bp;
	register WINDOW	*wp;
	int		s;
	char		fname[NFILEN];

	if ((s=ereply("Find file in other window: ", fname, NFILEN)) != TRUE)
		return (s);
	if ((bp = findbuffer(fname, &s)) == NULL) return s;
	if ((wp = popbuf(bp)) == NULL) return FALSE;
	curbp = bp;
	curwp = wp;
	if (bp->b_fname[0] == 0)
		return (readin(fname));		/* Read it in.		*/
	return TRUE;
}

/*
 * given a file name, either find the buffer it uses, or create a new
 * empty buffer to put it in.
 */
BUFFER *
findbuffer(fname, s) char *fname; int *s; {
	register BUFFER	*bp;
	char		bname[NBUFN];

	adjustcase(fname);
	for (bp=bheadp; bp!=NULL; bp=bp->b_bufp) {
		if (strcmp(bp->b_fname, fname) == 0) {
			return bp;
		}
	}
	makename(bname, fname);			/* New buffer name.	*/
	while ((bp=bfind(bname, FALSE)) != NULL) {
		*s = ereply("Buffer name: ", bname, NBUFN);
		if (*s == ABORT)		/* ^G to just quit	*/
			return NULL;
		if (*s == FALSE) {		/* CR to clobber it	*/
			bp->b_fname[0] = '\0';
			break;
		}
	}
	if (bp == NULL) bp = bfind(bname, TRUE);
	*s = FALSE;
	return bp;
}

/*
 * Read the file "fname" into the current buffer.
 * Make all of the text in the buffer go away, after checking
 * for unsaved changes. This is called by the "read" command, the
 * "visit" command, and the mainline (for "uemacs file").
 */
readin(fname) char *fname; {
	register int		status;
	register WINDOW		*wp;

	if (bclear(curbp) != TRUE)		/* Might be old.	*/
		return TRUE;
	status = insertfile(fname, fname) ;
	curbp->b_flag &= ~BFCHG;		/* No change.		*/
	for (wp=wheadp; wp!=NULL; wp=wp->w_wndp) {
		if (wp->w_bufp == curbp) {
			wp->w_linep = lforw(curbp->b_linep);
			wp->w_dotp  = lforw(curbp->b_linep);
			wp->w_doto  = 0;
			wp->w_markp = NULL;
			wp->w_marko = 0;
		}
	}
	return status;
}
/*
 * insert a file in the current buffer, after dot. Set mark
 * at the end of the text inserted, point at the beginning.
 * Return a standard status. Print a summary (lines read,
 * error message) out as well. If the
 * BACKUP conditional is set, then this routine also does the read
 * end of backup processing. The BFBAK flag, if set in a buffer,
 * says that a backup should be taken. It is set when a file is
 * read in, but not on a new file (you don't need to make a backup
 * copy of nothing).
 *
 * Warning: Adds a trainling nl to files that don't end in one!
 * Need to fix, but later (I suspect that it will require a change
 * in the fileio files for all systems involved).
 */
insertfile(fname, newname) char fname[], newname[]; {
	register LINE	*lp1;
	register LINE	*lp2;
	LINE		*olp;			/* Line we started at */
	int		opos;			/* and offset into it */
	register WINDOW	*wp;
	register int	i;
	register int	nbytes;
	int		s, nline;
	BUFFER		*bp;
	char		line[NLINE];

	bp = curbp;				/* Cheap.		*/
	if (newname != (char *) NULL)
		(VOID) strcpy(bp->b_fname, newname);
	if ((s=ffropen(fname)) == FIOERR) 	/* Hard file open.	*/
		goto out;
	if (s == FIOFNF) {			/* File not found.	*/
		if (kbdmop == NULL)
			if (newname != NULL)
				ewprintf("(New file)");
			else	ewprintf("(File not found)");
		goto out;
	}
	opos = curwp->w_doto;
	/* Open a new line, at point, and start inserting after it */
	(VOID) lnewline();
	olp = lback(curwp->w_dotp);
	nline = 0;			/* Don't count fake line at end */
	while ((s=ffgetline(line, NLINE)) == FIOSUC) {
		nbytes = strlen(line);
		if ((lp1=lalloc((RSIZE) nbytes)) == NULL) {
			s = FIOERR;		/* Keep message on the	*/
			break;			/* display.		*/
		}
		lp2 = lback(curwp->w_dotp);
		lp2->l_fp = lp1;
		lp1->l_fp = curwp->w_dotp;
		lp1->l_bp = lp2;
		curwp->w_dotp->l_bp = lp1;
		for (i=0; i<nbytes; ++i)
			lputc(lp1, i, line[i]);
		++nline;
	}
	(VOID) ffclose();			/* Ignore errors.	*/
	if (s==FIOEOF && kbdmop==NULL) {	/* Don't zap an error.	*/
		if (nline == 1)
			ewprintf("(Read 1 line)");
		else
			ewprintf("(Read %d lines)", nline);
	}
	/* Set mark at the end of the text */
	curwp->w_markp = curwp->w_dotp;
	curwp->w_marko = curwp->w_doto;
	/* Now, delete the results of the lnewline we started with */
	curwp->w_dotp = olp;
	curwp->w_doto = opos;
	(VOID) ldelnewline();
	curwp->w_doto = opos;			/* and dot is right	*/
#ifdef	BACKUP
	if (newname != NULL)
		bp->b_flag |= BFCHG | BFBAK;	/* Need a backup.	*/
	else	bp->b_flag |= BFCHG;
#else
	bp->b_flag |= BFCHG;
#endif
	/* if the insert was at the end of buffer, set lp1 to the end of
	 * buffer line, and lp2 to the beginning of the newly inserted
	 * text.  (Otherwise lp2 is set to NULL.)  This is 
	 * used below to set pointers in other windows correctly if they
	 * are also at the end of buffer.
	 */
	lp1 = bp->b_linep;
	if (curwp->w_markp == lp1)
		lp2 = curwp->w_dotp;
	else {
out:		lp2 = NULL;
	}
	for (wp=wheadp; wp!=NULL; wp=wp->w_wndp) {
		if (wp->w_bufp == curbp) {
			wp->w_flag |= WFMODE|WFEDIT;
			if (wp != curwp && lp2 != NULL) {
				if (wp->w_dotp == lp1)
					wp->w_dotp = lp2;
				if (wp->w_markp == lp1)
					wp->w_markp = lp2;
				if (wp->w_linep == lp1)
					wp->w_linep = lp2;
			}
		}
	}
	if (s == FIOERR)			/* False if error.	*/
		return (FALSE);
	return (TRUE);
}

/*
 * Take a file name, and from it
 * fabricate a buffer name. This routine knows
 * about the syntax of file names on the target system.
 * BDC1		left scan delimiter.
 * BDC2		optional second left scan delimiter.
 * BDC3		optional right scan delimiter.
 */
makename(bname, fname) char bname[]; char fname[]; {
	register char	*cp1;
	register char	*cp2;

	cp1 = &fname[0];
	while (*cp1 != 0)
		++cp1;
#ifdef	BDC2
	while (cp1!=&fname[0] && cp1[-1]!=BDC1 && cp1[-1]!=BDC2)
		--cp1;
#else
	while (cp1!=&fname[0] && cp1[-1]!=BDC1)
		--cp1;
#endif
	cp2 = &bname[0];
#ifdef	BDC3
	while (cp2!=&bname[NBUFN-1] && *cp1!=0 && *cp1!=BDC3)
		*cp2++ = *cp1++;
#else
	while (cp2!=&bname[NBUFN-1] && *cp1!=0)
		*cp2++ = *cp1++;
#endif
	*cp2 = 0;
}

/*
 * Ask for a file name, and write the
 * contents of the current buffer to that file.
 * Update the remembered file name and clear the
 * buffer changed flag. This handling of file names
 * is different from the earlier versions, and
 * is more compatable with Gosling EMACS than
 * with ITS EMACS.
 */
/*ARGSUSED*/
filewrite(f, n, k) {
	register int	s;
	char		fname[NFILEN];

	if ((s=ereply("Write file: ", fname, NFILEN)) != TRUE)
		return (s);
	adjustcase(fname);
	if ((s=writeout(curbp, fname)) == TRUE) {
		(VOID) strcpy(curbp->b_fname, fname);
#ifdef	BACKUP
		curbp->b_flag &= ~(BFBAK | BFCHG);
#else
		curbp->b_flag &= ~BFCHG;
#endif
		upmodes(curbp);
	}
	return (s);
}

/*
 * Save the contents of the current buffer back into
 * its associated file. Do nothing if there have been no changes
 * (is this a bug, or a feature). Error if there is no remembered
 * file name. If this is the first write since the read or visit,
 * then a backup copy of the file is made.
 */
/*ARGSUSED*/
filesave(f, n, k) {
	register int	s;

	if ((curbp->b_flag&BFCHG) == 0)	{	/* Return, no changes.	*/
		if (kbdmop == NULL) ewprintf("(No changes need to be saved)");
		return (TRUE);
	}
	if (curbp->b_fname[0] == 0) {		/* Must have a name.	*/
		ewprintf("No file name");
		return (FALSE);
	}
#ifdef	BACKUP
	if ((curbp->b_flag&BFBAK) != 0) {
		s = fbackupfile(curbp->b_fname);
		if (s == ABORT)			/* Hard error.		*/
			return FALSE;
		if (s == FALSE			/* Softer error.	*/
		&& (s=eyesno("Backup error, save anyway")) != TRUE)
			return (s);
	}
#endif
	if ((s=writeout(curbp, curbp->b_fname)) == TRUE) {
#ifdef	BACKUP
		curbp->b_flag &= ~(BFCHG | BFBAK);
#else
		curbp->b_flag &= ~BFCHG;
#endif
		upmodes(curbp);
	}
	return (s);
}

/*
 * This function performs the details of file
 * writing; writing the file in buffer bp to
 * file fn. Uses the file management routines
 * in the "fileio.c" package. Most of the grief
 * is checking of some sort.
 */
writeout(bp, fn) register BUFFER *bp; char *fn; {
	register int	s;
	register LINE	*lp;

	if ((s=ffwopen(fn)) != FIOSUC)		/* Open writes message.	*/
		return (FALSE);
	lp = lforw(bp->b_linep);		/* First line.		*/
	while (lp != bp->b_linep) {
		if ((s=ffputline(&(ltext(lp))[0], llength(lp))) != FIOSUC)
			break;
		lp = lforw(lp);
	}
	if (s == FIOSUC) {			/* No write error.	*/
		s = ffclose();
		if (s==FIOSUC && kbdmop==NULL)
			ewprintf("Wrote %s", fn);
	} else					/* Ignore close error	*/
		(VOID) ffclose();		/* if a write error.	*/
	if (s != FIOSUC)			/* Some sort of error.	*/
		return (FALSE);
	return (TRUE);
}

/*
 * Tag all windows for bp (all windows if bp NULL) as needing their
 * mode line updated.
 */
upmodes(bp) register BUFFER *bp; {
	register WINDOW	*wp;

	for (wp = wheadp; wp != NULL; wp = wp->w_wndp)
		if (bp == NULL || curwp->w_bufp == bp) wp->w_flag |= WFMODE;
}

