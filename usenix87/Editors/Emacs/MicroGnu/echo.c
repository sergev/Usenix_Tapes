/*
 *		Echo line reading and writing.
 *
 * Common routines for reading
 * and writing characters in the echo line area
 * of the display screen. Used by the entire
 * known universe.
 */
#include	"def.h"
#ifdef	VARARGS
#  include	<varargs.h>
	static veread();
#endif
static eformat();

int	epresf	= FALSE;		/* Stuff in echo line flag.	*/
/*
 * Erase the echo line.
 */
eerase() {
	ttcolor(CTEXT);
	ttmove(nrow-1, 0);
	tteeol();
	ttflush();
	epresf = FALSE;
}

/*
 * Ask "yes" or "no" question.
 * Return ABORT if the user answers the question
 * with the abort ("^G") character. Return FALSE
 * for "no" and TRUE for "yes". No formatting
 * services are available. No newline required.
 */
eyorn(sp) char *sp; {
	register KEY	s;

	if (kbdmop == NULL) ewprintf("%s? (y or n) ", sp);
	for (;;) {
		s = getkey(0);
		if (s == 'y' || s == 'Y') return (TRUE);
		if (s == 'n' || s == 'N') return (FALSE);
		if (s == (KCTRL|'G') || s == (KCTLX|KCTRL|'G')
		||  s == (KMETA|KCTRL|'G')) {
			(VOID) ctrlg(FALSE, 1, KRANDOM);
			return ABORT;
		}
		if (kbdmop == NULL)
			ewprintf("Please answer y or n.  %s? (y or n) ", sp);
	}
}

/*
 * Like eyorn, but for more important question. User must type either all of
 * "yes" or "no", and the trainling newline.
 */
eyesno(sp) char *sp; {
	register int	s;
	char		buf[64];

	s = ereply("%s? (yes or no) ", buf, sizeof(buf), sp);
	for (;;) {
		if (s == ABORT) return ABORT;
		if (s != FALSE) {
			if ((buf[0] == 'y' || buf[0] == 'Y')
			&&  (buf[1] == 'e' || buf[1] == 'E')
			&&  (buf[2] == 's' || buf[2] == 'S')) return TRUE;
			if ((buf[0] == 'n' || buf[0] == 'N')
			&&  (buf[1] == 'o' || buf[0] == 'O')) return FALSE;
		}
		s = ereply("Please answer yes or no.  %s? (yes or no) ",
			   buf, sizeof(buf), sp);
	}
}
/*
 * Write out a prompt, and read back a
 * reply. The prompt is now written out with full "ewprintf"
 * formatting, although the arguments are in a rather strange
 * place. This is always a new message, there is no auto
 * completion, and the return is echoed as such.
 */
#ifdef	VARARGS
ereply(va_alist)
va_dcl
{
	register int i;
	va_list pvar;
	register char *fp, *buf;
	register int nbuf;
	
	va_start(pvar);
	fp = va_arg(pvar, char *);
	buf = va_arg(pvar, char *);
	nbuf = va_arg(pvar, int);
	i = veread(fp, buf, nbuf, EFNEW|EFCR, &pvar);
	va_end(pvar);
	return i;
}
#else
/* VARARGS3 */
ereply(fp, buf, nbuf, arg) char *fp, *buf; int nbuf; long arg; {
	return (eread(fp, buf, nbuf, EFNEW|EFCR, (char *)&arg));
}
#endif

/*
 * This is the general "read input from the
 * echo line" routine. The basic idea is that the prompt
 * string "prompt" is written to the echo line, and a one
 * line reply is read back into the supplied "buf" (with
 * maximum length "len"). The "flag" contains EFNEW (a
 * new prompt), an EFFUNC (autocomplete), or EFCR (echo
 * the carriage return as CR).
 */
/* VARARGS4 */
#ifdef	VARARGS
eread(va_alist)
va_dcl
{
	va_list pvar;
	char *fp, *buf;
	int nbuf, flag, i;
	va_start(pvar);
	fp   = va_arg(pvar, char *);
	buf  = va_arg(pvar, char *);
	nbuf = va_arg(pvar, int);
	flag = va_arg(pvar, int);
	i = veread(fp, buf, nbuf, flag, &pvar);
	va_end(pvar);
	return i;
}
#endif

#ifdef	VARARGS
static veread(fp, buf, nbuf, flag, ap) char *fp; char *buf; va_list *ap; {
#else
eread(fp, buf, nbuf, flag, ap) char *fp; char *buf; char *ap; {
#endif
	register int	cpos;
	register int	i;
	register KEY	c;

	cpos = 0;
	if (kbdmop != NULL) {			/* In a macro.		*/
		while ((c = *kbdmop++) != '\0')
			buf[cpos++] = (char) c;
		buf[cpos] = '\0';
		goto done;
	}
	if ((flag&EFNEW)!=0 || ttrow!=nrow-1) {
		ttcolor(CTEXT);
		ttmove(nrow-1, 0);
		epresf = TRUE;
	} else
		eputc(' ');
	eformat(fp, ap);
	tteeol();
	ttflush();
	for (;;) {
		c = getkey(KQUOTE|KNOMAC);
		if ((flag&EFAUTO) != 0 && (c == ' ' || c == CCHR('I'))) {
			cpos += complete(flag, c, buf, cpos);
			continue;
		}
		switch (c) {
		case 0x0D:			/* Return, done.	*/
			if ((flag&EFFUNC) != 0) {
				if ((i = complete(flag, c, buf, cpos)) == 0)
					continue;
				if (i > 0) cpos += i;
			}
			buf[cpos] = '\0';
			if ((flag&EFCR) != 0) {
				ttputc(0x0D);
				ttflush();
			}
			if (kbdmip != NULL) {
				if (kbdmip+cpos+1 > &kbdm[NKBDM-3]) {
					ewprintf("Keyboard macro overflow");
					ttflush();
					return FALSE;
				}
				for (i = 0; i <= cpos; ++i)
					*kbdmip++ = (KEY) buf[i];
			}
			goto done;

		case CCHR('G'):			/* Bell, abort.		*/
			eputc(CCHR('G'));
			(VOID) ctrlg(FALSE, 0, KRANDOM);
			ttflush();
			return (ABORT);

		case 0x7F:			/* Rubout, erase.	*/
			if (cpos != 0) {
				ttputc('\b');
				ttputc(' ');
				ttputc('\b');
				--ttcol;
				if (ISCTRL(buf[--cpos]) != FALSE) {
					ttputc('\b');
					ttputc(' ');
					ttputc('\b');
					--ttcol;
				}
				ttflush();
			}
			break;

		case CCHR('X'):			/* C-X			*/
		case CCHR('U'):			/* C-U, kill line.	*/
			while (cpos != 0) {
				ttputc('\b');
				ttputc(' ');
				ttputc('\b');
				--ttcol;
				if (ISCTRL(buf[--cpos]) != FALSE) {
					ttputc('\b');
					ttputc(' ');
					ttputc('\b');
					--ttcol;
				}
			}
			ttflush();
			break;

		case CCHR('Q'):			/* C-Q, quote next	*/
			c = getkey(KQUOTE|KNOMAC) ;
		default:			/* All the rest.	*/
			if (cpos < nbuf-1) {
				buf[cpos++] = (char) c;
				eputc((char) c);
				ttflush();
			}
		}
	}
done:
	if (buf[0] == '\0')
		return (FALSE);
	return (TRUE);
}

/*
 * do completion on a list of objects.
 */
complete(flags, c, buf, cpos) register char *buf; register int cpos; {
	register LIST	*lh, *lh2;
#ifndef	MANX
	register	/* too many registers mess Manx up */
#endif
	int		i, nxtra;
	int		nhits, bxtra;
	int		wflag = FALSE;
	int		msglen, nshown;
	char		*msg;

	if ((flags&EFFUNC) != 0) lh = &(symbol[0]->s_list);
	else if ((flags&EFBUF) != 0) lh = &(bheadp->b_list);
	else panic("broken complete call: flags");

	if (c == ' ') wflag = TRUE;
	else if (c != '\t' && c != 0x0D) panic("broken complete call: c");

	nhits = 0;
	nxtra = HUGE;

	while (lh != NULL) {
		for (i=0; i<cpos; ++i) {
			if (buf[i] != lh->l_name[i])
				break;
		}
		if (i == cpos) {
			if (nhits == 0)
				lh2 = lh;
			++nhits;
			if (lh->l_name[i] == '\0') nxtra = -1;
			else {
				bxtra = getxtra(lh, lh2, cpos, wflag);
				if (bxtra < nxtra) nxtra = bxtra;
				lh2 = lh;
			}
		}
		lh = lh->l_next;
	}
	if (nhits == 0)
		msg = " [No match]";
	else if (nhits > 1 && nxtra == 0)
		msg = " [Ambiguous]";
	else {		/* Got a match, do it to it */
		/*
		 * Being lazy - ought to check length, but all things
		 * autocompleted have known types/lengths.
		 */ 
		if (nxtra < 0 && nhits > 1 && c == ' ') nxtra = 1;
		for (i = 0; i < nxtra; ++i) {
			buf[cpos] = lh2->l_name[cpos];
			eputc(buf[cpos++]);
		}
		ttflush();
		if (nxtra < 0 && c != 0x0D) return 0;
		return nxtra;
	}
        /* Set up backspaces, etc., being mindful of echo line limit */
	msglen = strlen(msg);
	nshown = (ttcol + msglen + 2 > ncol) ? 
			ncol - ttcol - 2 : msglen;
	eputs(msg);
	ttcol -= (i = nshown);		/* update ttcol!		*/
	while (i--)			/* move back before msg		*/
		ttputc('\b');
	ttflush();			/* display to user		*/
	i = nshown;
	while (i--)			/* blank out	on next flush	*/
		eputc(' ');
	ttcol -= (i = nshown);		/* update ttcol on BS's		*/
	while (i--)
		ttputc('\b');		/* update ttcol again!		*/
	return 0;

}

/*
 * The "lp1" and "lp2" point to list structures. The
 * "cpos" is a horizontal position in the name.
 * Return the longest block of characters that can be
 * autocompleted at this point. Sometimes the two
 * symbols are the same, but this is normal.
  */
getxtra(lp1, lp2, cpos, wflag) register LIST *lp1, *lp2; register int wflag; {
	register int	i;

	i = cpos;
	for (;;) {
		if (lp1->l_name[i] != lp2->l_name[i]) break;
		if (lp1->l_name[i] == '\0') break;
		++i;
		if (wflag && !ISWORD(lp1->l_name[i-1])) break;
	}
	return (i - cpos);
}

/*
 * Special "printf" for the echo line.
 * Each call to "ewprintf" starts a new line in the
 * echo area, and ends with an erase to end of the
 * echo line. The formatting is done by a call
 * to the standard formatting routine.
 */
#ifdef	VARARGS
ewprintf(va_alist)
va_dcl
{
	va_list pvar;
	register char *fp;

	va_start(pvar);
	fp = va_arg(pvar, char *);
#else
/* VARARGS1 */
ewprintf(fp, arg) char *fp; {
#endif
	ttcolor(CTEXT);
	ttmove(nrow-1, 0);
#ifdef	VARARGS
	eformat(fp, &pvar);
	va_end(pvar);
#else
	eformat(fp, (char *)&arg);
#endif
	tteeol();
	ttflush();
	epresf = TRUE;
}

/*
 * Printf style formatting. This is
 * called by both "ewprintf" and "ereply" to provide
 * formatting services to their clients. The move to the
 * start of the echo line, and the erase to the end of
 * the echo line, is done by the caller.
 * Note: %c works, and prints the "name" of the key. However
 * the key must be cast to an int to avoid tripping over
 * various oddities in C argument passing.
 */
static eformat(fp, ap) register char *fp;
#ifdef VARARGS
register va_list *ap;
#else
register char *ap;
#endif
{
	register int	c;
	char		kname[NKNAME];

	while ((c = *fp++) != '\0') {
		if (c != '%')
			eputc(c);
		else {
			c = *fp++;
			switch (c) {
			case 'c':
#ifdef	VARARGS
				keyname(kname, va_arg(*ap, int));
#else
				/*NOSTRICT*/
				keyname(kname, *(int *)ap);
				ap += sizeof(int);
#endif
				eputs(kname);
				break;

			case 'd':
#ifdef	VARARGS
				eputi(va_arg(*ap, int), 10);
#else
				/*NOSTRICT*/
				eputi(*(int *)ap, 10);
				ap += sizeof(int);
#endif
				break;

			case 'o':
#ifdef	VARARGS
				eputi(va_arg(*ap, int), 8);
#else
				/*NOSTRICT*/
				eputi(*(int *)ap,  8);
				ap += sizeof(int);
#endif
				break;

			case 's':
#ifdef	VARARGS
				eputs(va_arg(*ap, char *));
#else
				/*NOSTRICT*/
				eputs(*(char **)ap);
				ap += sizeof(char *);
#endif
				break;
			case 'l':/* explicit longword */
				c = *fp++;
				switch(c) {
				case 'd':
#ifdef	VARARGS
					eputl((long)va_arg(*ap, long), 10L);
#else
					/*NOSTRICT*/
					eputl(*(long *)ap, 10L);
					ap += sizeof(long);
#endif
					break;
				default:
					eputc(c);
					break;
				}
				break;

			default:
				eputc(c);
			}
		}
	}
}

/*
 * Put integer, in radix "r".
 */
static eputi(i, r) register int i; register int r; {
	register int	q;

	if ((q=i/r) != 0)
		eputi(q, r);
	eputc(i%r+'0');
}

/*
 * Put long, in radix "r".
 */
static eputl(l, r) register long l; register long r; {
	register long	q;

	if ((q=l/r) != 0)
		eputl(q, r);
	eputc((int)(l%r)+'0');
}

/*
 * Put string.
 */
eputs(s) register char *s; {
	register int	c;

	while ((c = *s++) != '\0')
		eputc(c);
}

/*
 * Put character. Watch for
 * control characters, and for the line
 * getting too long.
 */
static eputc(c) register char c; {
	if (ttcol+2 < ncol) {
		if (ISCTRL(c) != FALSE) {
			eputc('^');
			c ^= 0x40;
		}
		ttputc(c);
		++ttcol;
	}
}

