/*
 * 		Search commands.
 * The functions in this file implement the
 * search commands (both plain and incremental searches
 * are supported) and the query-replace command.
 *
 * The plain old search code is part of the original
 * MicroEMACS "distribution". The incremental search code,
 * and the query-replace code, is by Rich Ellison.
 */
#include	"def.h"

#define SRCH_BEGIN	(0)			/* Search sub-codes.	*/
#define	SRCH_FORW	(-1)
#define SRCH_BACK	(-2)
#define SRCH_NOPR	(-3)
#define SRCH_ACCM	(-4)
#define	SRCH_MARK	(-5)

typedef struct  {
	int	s_code;
	LINE	*s_dotp;
	int	s_doto;
}	SRCHCOM;

static	SRCHCOM	cmds[NSRCH];
static	int	cip;

int	srch_lastdir = SRCH_NOPR;		/* Last search flags.	*/

/*
 * Search forward.
 * Get a search string from the user, and search for it,
 * starting at ".". If found, "." gets moved to just after the
 * matched characters, and display does all the hard stuff.
 * If not found, it just prints a message.
 */
/*ARGSUSED*/
forwsearch(f, n, k) {
	register int	s;

	if ((s=readpattern("Search")) != TRUE)
		return (s);
	if (forwsrch() == FALSE) {
		ewprintf("Search failed: \"%s\"", pat);
		return (FALSE);
	}
	srch_lastdir = SRCH_FORW;
	return (TRUE);
}

/*
 * Reverse search.
 * Get a search string from the  user, and search, starting at "."
 * and proceeding toward the front of the buffer. If found "." is left
 * pointing at the first character of the pattern [the last character that
 * was matched].
 */
/*ARGSUSED*/
backsearch(f, n, k) {
	register int	s;

	if ((s=readpattern("Search backward")) != TRUE)
		return (s);
	if (backsrch() == FALSE) {
		ewprintf("Search failed: \"%s\"", pat);
		return (FALSE);
	}
	srch_lastdir = SRCH_BACK;
	return (TRUE);
}

/* 
 * Search again, using the same search string
 * and direction as the last search command. The direction
 * has been saved in "srch_lastdir", so you know which way
 * to go.
 */
/*ARGSUSED*/
searchagain(f, n, k) {
	if (srch_lastdir == SRCH_FORW) {
		if (forwsrch() == FALSE) {
			ewprintf("Search failed: \"%s\"", pat);
			return (FALSE);
		}
		return (TRUE);
	}
	if (srch_lastdir == SRCH_BACK) {
		if (backsrch() == FALSE) {
			ewprintf("Search failed: \"%s\"", pat);
			return (FALSE);
		}
		return (TRUE);
	}
	ewprintf("No last search");
	return (FALSE);
}

/*
 * Use incremental searching, initially in the forward direction.
 * isearch ignores any explicit arguments.
 */
/*ARGSUSED*/
forwisearch(f, n, k) {
	return (isearch(SRCH_FORW));
}

/*
 * Use incremental searching, initially in the reverse direction.
 * isearch ignores any explicit arguments.
 */
/*ARGSUSED*/
backisearch(f, n, k) {
	return (isearch(SRCH_BACK));
}

/*
 * Incremental Search.
 *	dir is used as the initial direction to search.
 *	^S	switch direction to forward
 *	^R	switch direction to reverse
 *	^Q	quote next character (allows searching for ^N etc.)
 *	<ESC>	exit from Isearch
 *	<DEL>	undoes last character typed. (tricky job to do this correctly).
 *	other ^	exit search, don't set mark
 *	else	accumulate into search string
 */
isearch(dir) {
	register int	c;
	register LINE	*clp;
	register int	cbo;
	register int	success;
	int		pptr;
	char		opat[NPAT];

	for (cip=0; cip<NSRCH; cip++)
		cmds[cip].s_code = SRCH_NOPR;
	(VOID) strcpy(opat, pat);
	cip = 0;
	pptr = -1;
	clp = curwp->w_dotp;
	cbo = curwp->w_doto;
	is_lpush();
	is_cpush(SRCH_BEGIN);
	success = TRUE;
	is_prompt(dir, TRUE, success);
	for (;;) {
		update();
		switch (c = (char) getkey(KQUOTE)) {
		case METACH:
			srch_lastdir = dir;
			curwp->w_markp = clp;
			curwp->w_marko = cbo;
			if (kbdmop == NULL) ewprintf("Mark set");
			return (TRUE);

		case CCHR('G'):
			if (success != TRUE) {
				while (is_peek() == SRCH_ACCM)
					if (is_undo(&pptr, &dir) == FALSE)
						break;
				success = TRUE;
				is_prompt(dir, pptr < 0, success);
				break;
			}
			curwp->w_dotp = clp;
			curwp->w_doto = cbo;
			curwp->w_flag |= WFMOVE;
			srch_lastdir = dir;
			(VOID) ctrlg(FALSE, 0, KRANDOM);
			(VOID) strcpy(pat, opat);
			return ABORT;

		case CCHR('S'):
			if (dir == SRCH_BACK) {
				dir = SRCH_FORW;
				is_lpush();
				is_cpush(SRCH_FORW);
				success = TRUE;
			}
			if (success==FALSE && dir==SRCH_FORW)
				break;
			is_lpush();
			pptr = strlen(pat);
			(VOID) forwchar(FALSE, 1, KRANDOM);
			if (is_find(SRCH_FORW) != FALSE) is_cpush(SRCH_MARK);
			else {
				(VOID) backchar(FALSE, 1, KRANDOM);
				ttbeep();
				success = FALSE;
			}
			is_prompt(dir, pptr < 0, success);
			break;

		case CCHR('R'):
			if (dir == SRCH_FORW) {
				dir = SRCH_BACK;
				is_lpush();
				is_cpush(SRCH_BACK);
				success = TRUE;
			}
			if (success==FALSE && dir==SRCH_BACK)
				break;
			is_lpush();
			pptr = strlen(pat);
			(VOID) backchar(FALSE, 1, KRANDOM);
			if (is_find(SRCH_BACK) != FALSE) is_cpush(SRCH_MARK);
			else {
				(VOID) forwchar(FALSE, 1, KRANDOM);
				ttbeep();
				success = FALSE;
			}
			is_prompt(dir, pptr < 0, success);
			break;

		case 0x7F:
			if (is_undo(&pptr, &dir) != TRUE) return FALSE;
			if (is_peek() != SRCH_ACCM) success = TRUE;
			is_prompt(dir, pptr < 0, success);
			break;

		case CCHR('Q'):
			c = (char) getkey(KQUOTE);
			goto  addchar;
		case CCHR('M'):
			c = CCHR('J');
		case CCHR('J'):
			goto  addchar;

		default:
			if (ISCTRL(c) != FALSE) {
				c += '@';
				c |= KCTRL;
				success = execute((KEY) c, FALSE, 1);
				curwp->w_markp = clp;
				curwp->w_marko = cbo;
				if (kbdmop == NULL) ewprintf("Mark set");
				curwp->w_flag |= WFMOVE;
				return (success);
			}				
		addchar:
			if (pptr == -1)
				pptr = 0;
			if (pptr == 0)
				success = TRUE;
			pat[pptr++] = c;
			if (pptr == NPAT) {
				ewprintf("Pattern too long");
				return FALSE;
			}
			pat[pptr] = '\0';
			is_lpush();
			if (success != FALSE) {
				if (is_find(dir) != FALSE)
					is_cpush(c);
				else {
					success = FALSE;
					ttbeep();
					is_cpush(SRCH_ACCM);
				}
			} else
				is_cpush(SRCH_ACCM);
			is_prompt(dir, FALSE, success);
		}
	}
}

is_cpush(cmd) register int cmd; {
	if (++cip >= NSRCH)
		cip = 0;
	cmds[cip].s_code = cmd;
}

is_lpush() {
	register int	ctp;

	ctp = cip+1;
	if (ctp >= NSRCH)
		ctp = 0;
	cmds[ctp].s_code = SRCH_NOPR;
	cmds[ctp].s_doto = curwp->w_doto;
	cmds[ctp].s_dotp = curwp->w_dotp;
}

is_pop() {
	if (cmds[cip].s_code != SRCH_NOPR) {
		curwp->w_doto  = cmds[cip].s_doto; 
		curwp->w_dotp  = cmds[cip].s_dotp;
		curwp->w_flag |= WFMOVE;
		cmds[cip].s_code = SRCH_NOPR;
	}
	if (--cip <= 0)
		cip = NSRCH-1;
}

is_peek() {
	return cmds[cip].s_code;
}

is_undo(pptr, dir) register int *pptr; register int *dir; {
	register int	redo = FALSE ;
	switch (cmds[cip].s_code) {
	case SRCH_BEGIN:
	case SRCH_NOPR:
		*pptr = -1;
	case SRCH_MARK:
		break;

	case SRCH_FORW:
		*dir = SRCH_BACK;
		redo = TRUE;
		break;

	case SRCH_BACK:
		*dir = SRCH_FORW;
		redo = TRUE;
		break;

	case SRCH_ACCM:
	default:
		*pptr -= 1;
		if (*pptr < 0)
			*pptr = 0;
		pat[*pptr] = '\0';
		break;
	}
	is_pop();
	if (redo) return is_undo(pptr, dir);
	return (TRUE);
}

is_find(dir) register int dir; {
	register int	plen, odoto;
	register LINE	*odotp ;

	odoto = curwp->w_doto;
	odotp = curwp->w_dotp;
	plen = strlen(pat);
	if (plen != 0) {
		if (dir==SRCH_FORW) {
			(VOID) backchar(TRUE, plen, KRANDOM);
			if (forwsrch() == FALSE) {
				curwp->w_doto = odoto;
				curwp->w_dotp = odotp;
				return (FALSE);
			}
			return (TRUE);
		}
		if (dir==SRCH_BACK) {
			(VOID) forwchar(TRUE, plen, KRANDOM);
			if (backsrch() == FALSE) {
				curwp->w_doto = odoto;
				curwp->w_dotp = odotp;
				return (FALSE);
			}
			return (TRUE);
		}
		ewprintf("bad call to is_find");
		return FALSE;
	}
	return (FALSE);
}

/*
 * If called with "dir" not one of SRCH_FORW
 * or SRCH_BACK, this routine used to print an error
 * message. It also used to return TRUE or FALSE,
 * depending on if it liked the "dir". However, none
 * of the callers looked at the status, so I just
 * made the checking vanish.
 */
is_prompt(dir, flag, success) {
	VOID is_dspl();

	if (dir == SRCH_FORW) {
		if (success != FALSE)
			is_dspl("I-search", flag);
		else
			is_dspl("Failing I-search", flag);
	} else if (dir == SRCH_BACK) {
		if (success != FALSE)
			is_dspl("I-search backward", flag);
		else
			is_dspl("Failing I-search backward", flag);
	} else ewprintf("Broken call to is_prompt");
}

/*
 * Prompt writing routine for the incremental search. 
 * The "prompt" is just a string. The "flag" determines
 * whether pat should be printed.
 */
VOID
is_dspl(prompt, flag) char *prompt; {

	if (kbdmop != NULL) return;
	if (flag != FALSE)
		ewprintf("%s: ", prompt);
	else
		ewprintf("%s: %s", prompt, pat);
}

/*
 * Query Replace.
 *	Replace strings selectively.  Does a search and replace operation.
 */
/*ARGSUSED*/
queryrepl(f, n, k) {
	register int	s;
	register int	rcnt = 0;	/* Replacements made so far	*/
	register int	plen;		/* length of found string	*/
	char		news[NPAT];	/* replacement string		*/

	if ((s=readpattern("Query replace")) != TRUE)
		return (s);
	if ((s=ereply("Query replace %s with: ",news, NPAT, pat)) == ABORT)
		return (s);
	if (s == FALSE)
		news[0] = '\0';
	if (kbdmop == NULL) ewprintf("Query replacing %s with %s:", pat, news);
	plen = strlen(pat);

	/*
	 * Search forward repeatedly, checking each time whether to insert
	 * or not.  The "!" case makes the check always true, so it gets put
	 * into a tighter loop for efficiency.
	 */

	while (forwsrch() == TRUE) {
	retry:
		update();
		switch (getkey(KQUOTE)) {
		case ' ':
			if (lreplace((RSIZE) plen, news, f) == FALSE)
				return (FALSE);
			rcnt++;
			break;

		case '.':
			if (lreplace((RSIZE) plen, news, f) == FALSE)
				return (FALSE);
			rcnt++;
			goto stopsearch;

		case CCHR('G'):	/* ^G or ESC */
			(VOID) ctrlg(FALSE, 0, KRANDOM);
		case 033:
			goto stopsearch;

		case '!':
			do {
				if (lreplace((RSIZE) plen, news, f) == FALSE)
					return (FALSE);
				rcnt++;
			} while (forwsrch() == TRUE);
			goto stopsearch;

		case 0x7F:		/* To not replace */
			break;

		default:
ewprintf("<SP> replace, [.] rep-end, <DEL> don't, [!] repl rest <ESC> quit");
			goto retry;
		}
	}
stopsearch:
	curwp->w_flag |= WFHARD;
	update();
	if (kbdmop == NULL) {
		if (rcnt == 0)
			ewprintf("(No replacements done)");
		else if (rcnt == 1)
			ewprintf("(1 replacement done)");
		else
			ewprintf("(%d replacements done)", rcnt);
	}
	return TRUE;
}

/*
 * This routine does the real work of a
 * forward search. The pattern is sitting in the external
 * variable "pat". If found, dot is updated, the window system
 * is notified of the change, and TRUE is returned. If the
 * string isn't found, FALSE is returned.
 */
forwsrch() {
	register LINE	*clp;
	register int	cbo;
	register LINE	*tlp;
	register int	tbo;
	register char	*pp;
	register int	c;

	clp = curwp->w_dotp;
	cbo = curwp->w_doto;
	while (clp != curbp->b_linep) {
		if (cbo == llength(clp)) {
			clp = lforw(clp);
			cbo = 0;
			c = SEOL;
		} else
			c = lgetc(clp, cbo++);
		if (eq(c, pat[0]) != FALSE) {
			tlp = clp;
			tbo = cbo;
			pp  = &pat[1];
			while (*pp != 0) {
				if (tlp == curbp->b_linep)
					goto fail;
				if (tbo == llength(tlp)) {
					tlp = lforw(tlp);
					if (tlp == curbp->b_linep)
						goto fail;
					tbo = 0;
					c = SEOL;
				} else
					c = lgetc(tlp, tbo++);
				if (eq(c, *pp++) == FALSE)
					goto fail;
			}
			curwp->w_dotp  = tlp;
			curwp->w_doto  = tbo;
			curwp->w_flag |= WFMOVE;
			return (TRUE);
		}
	fail:	;
	}
	return (FALSE);
}

/*
 * This routine does the real work of a
 * backward search. The pattern is sitting in the external
 * variable "pat". If found, dot is updated, the window system
 * is notified of the change, and TRUE is returned. If the
 * string isn't found, FALSE is returned.
 */
backsrch() {
	register LINE	*clp;
	register int	cbo;
	register LINE	*tlp;
	register int	tbo;
	register int	c;
	register char	*epp;
	register char	*pp;

	for (epp = &pat[0]; epp[1] != 0; ++epp)
		;
	clp = curwp->w_dotp;
	cbo = curwp->w_doto;
	for (;;) {
		if (cbo == 0) {
			clp = lback(clp);
			if (clp == curbp->b_linep)
				return (FALSE);
			cbo = llength(clp)+1;
		}
		if (--cbo == llength(clp))
			c = SEOL;
		else
			c = lgetc(clp,cbo);
		if (eq(c, *epp) != FALSE) {
			tlp = clp;
			tbo = cbo;
			pp  = epp;
			while (pp != &pat[0]) {
				if (tbo == 0) {
					tlp = lback(tlp);
					if (tlp == curbp->b_linep)
						goto fail;
					tbo = llength(tlp)+1;
				}
				if (--tbo == llength(tlp))
					c = SEOL;
				else
					c = lgetc(tlp,tbo);
				if (eq(c, *--pp) == FALSE)
					goto fail;
			}
			curwp->w_dotp  = tlp;
			curwp->w_doto  = tbo;
			curwp->w_flag |= WFMOVE;
			return (TRUE);
		}
	fail:	;
	}
}

/*
 * Compare two characters.
 * The "bc" comes from the buffer.
 * It has its case folded out. The
 * "pc" is from the pattern.
 */
eq(bc, pc) {
	register int	ibc;
	register int	ipc;

	ibc = bc & 0xFF;
	ipc = pc & 0xFF;
	if (ISLOWER(ibc) != FALSE)
		ibc = TOUPPER(ibc);
	if (ISLOWER(ipc) != FALSE)
		ipc = TOUPPER(ipc);
	if (ibc == ipc)
		return (TRUE);
	return (FALSE);
}

/*
 * Read a pattern.
 * Stash it in the external variable "pat". The "pat" is
 * not updated if the user types in an empty line. If the user typed
 * an empty line, and there is no old pattern, it is an error.
 * Display the old pattern, in the style of Jeff Lomicka. There is
 * some do-it-yourself control expansion.
 */
readpattern(prompt) char *prompt; {
	register int	s;
	char		tpat[NPAT];

	if (tpat[0] == '\0') s = ereply("%s: ", tpat, NPAT, prompt);
	else s = ereply("%s: (default %s) ", tpat, NPAT, prompt, pat);

	if (s == TRUE)				/* Specified		*/
		(VOID) strcpy(pat, tpat);
	else if (s==FALSE && pat[0]!=0)		/* CR, but old one	*/
		s = TRUE;
	return (s);
}
