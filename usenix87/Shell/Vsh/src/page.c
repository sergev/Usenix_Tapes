#include "hd.h"
#include "strings.h"
#include "mydir.h"
#include <signal.h>

#define	PROMPTLINE	1		/* Line count used by prompt */
#define	OVERLAP		2		/* Lines of page overlap */
#define	MMINSIZE	100		/* Tiny more buffer */
#define	topwin()	((ewindow && CS) ? window-PROMPTLINE : 1)

int nointer;
char *mbegin, *mend, *mbufp;
unsigned mbsize;
long maxline, topline, botline;
long omaxline;
int curwin;
int noprint;
int isfile;
int putch();
char *oldline();

page (stream, name)
FILE *stream; 
char *name;
{
#ifdef	MBUFSTACK
	char mbuf[MBUFSIZE];
#else
	char mbuf[MMINSIZE];
#endif
	register int linelim;
	register ch;
	register int ttych;
	int replot, fwin;
	char *ll;
	char number[20];
	char **v;
	int serror;
	int (*oldsig)(); extern catch();

	fwin = replot = 1;
	oldsig = signal (SIGINT, catch); nointer = 1;
	if (name && *name == 0) {
		isfile = 0;
		name = 0;
	}
	else
		isfile = 1;

#ifdef	MBUFSTACK
	mbsize = MBUFSIZE;
	mbufp = mbuf;
#else
	mbsize = atoi(MORESIZE);
	/* If not a pipe, stat the file */
	if (name && stat(name, &scr_stb) == 0
		&& (scr_stb.st_mode&S_IFMT) == S_IFREG
#ifdef	V6
		&& scr_stb.st_size0 == 0 && scr_stb.st_size1 < mbsize)
		mbsize = scr_stb.st_size1+1;
#else
		&& scr_stb.st_size < mbsize)
		mbsize = scr_stb.st_size+1;
#endif
	mbsize += mbsize/CO;			/* add extra for long lines */
	if (mbsize < MMINSIZE) {
		mbufp = mbuf;
		mbsize = sizeof mbuf;
	}
	else {
		mbufp = (char *)malloc(mbsize);
		if (mbufp == NULL) {
			fprintf(stderr, "No memory for pager\n\r");
			mbufp = mbuf;
			mbsize = sizeof mbuf;
		}
	}
#endif

	bufout ();
	if (ewindow && CS) {
		curwin = LI-(window-VSHBOT)-PROMPTLINE;
		fwin = replot = !ewin();
	}
	else {
		curwin = LI-PROMPTLINE;
		noprint = ewindow;
		ewindow = 0;
		erase ();
		ewindow = noprint;
	}
	if (name)
		hilite("%s\r\n", name);

	noprint = 0;
	initmore();
	linelim = curwin-PROMPTLINE;
	do
	{
		ch = more(stream, linelim, &ll);
		if (topline < 0)		/* Only if file is short */
			topline = 0;
		if (ch == EOF && linelim >= 0)
			hilite("Done:");
		else {
			ch = '\n';
			hilite("More?");
		}
		fflush (stdout);

		ttych = getch ();
		printf("\r       \r");
		fflush(stdout);
		/* dyt flush input */
		tty_push(COOKEDMODE);
		tty_pop();
		switch(ttych) {
		case EOT:
		case EOF:
			linelim = curwin / 2;
			break;
		case 020:	/* ^P, EMACS style */
		case 'j':
		case '\r':
		case '\n':
			linelim = 1;
			break;
		case 025:	/* ^U, VI style */
			linelim = - curwin / 2;
			break;
		case 016:	/* ^N, EMACS style */
		case 'k':
		case '-':
			linelim = -1;
			break;
		case '^':
		case 02:	/* ^B, VI style */
		case 033:	/* ESC, EMACS ESC-v */
			linelim = - (curwin-OVERLAP);
			break;
		case 'e':
		case 'v':
			linelim = 0;
			if (name == 0) {
				putch(07);
				break;
			}
			printf("\r%s %s", EDITOR, name);
			fflush(stdout);
			sprintf(number, "+%ld", (botline+topline)/2);
			replot = nedit(name, number);
			if (replot == NOREPLOT) {
				printf(" : Bad file");
				fflush(stdout);
				sleep(1);
				break;
			}
		case 'q':
		case 'n':
			ttych = 'n';
			break;	/* No more */
		case 022:		/* ^R, EMACS style */
		case 023:		/* ^S, EMACS style */
		case '?':
		case '/':
			ch = '\n';
			linelim = search(stream, ttych, &serror);
			break;
		case 'G':
		case '>':
			linelim = prttail(stream);
			break;
		case '1':
		case '<':
			ch = '\n';
			linelim = prthead(stream);
			break;
		case '!':
			tty_push(COOKEDMODE);
			v = &ll;
			ll = CNULL;
			callshell(v);
			replot = REPLOT;
			tty_pop();
			ewin();
		case 014:		/* ^L */
			topline += curwin;
			botline += curwin;
			prtback(-curwin, &ll);
			linelim = 0;
			break;
		default:
			linelim = curwin-OVERLAP;
			break;
		}

	} while ((ch != EOF || linelim <= 0) && ttych != 'n' && nointer);

	unbufout ();
	signal (SIGINT, oldsig);
	if (!fwin)
		vwin();
#ifndef	MBUFSTACK
	if (mbufp != mbuf)
		free(mbufp);
#endif
	omaxline = 0;		/* Reset for oldline() */
	return replot;
}

more(stream, linelim, lastline)
FILE *stream;
register int linelim;
char **lastline;
{
	register char *s;
	register int newlines;
	register int col, maxcol;
	int ch;

	newlines = 0;
	*lastline = 0;
	ch = '\n';
	if (linelim == 0)
		return ch;
	/* Looking forwards from current botline */
	if (linelim > 0) {
		newlines = botline + linelim - maxline;
		if (newlines < 0)
			newlines = 0;
		/* Number of old lines to fetch forwards */
		linelim -= newlines;
	}
	/* Looking backwards from current topline */
	else {
		if (curwin >= maxline) {
			printf("\r        \r\07");
			return EOF;
		}
		if (SR || noprint) {
			if (!noprint)
				atxy(1, topwin());
			while (linelim < 0) {
				if (topline <= 1) {
					putch(07);
					break;
				}
				if ((s = oldline((int)(maxline - topline + 2)))
					== 0)
					break;
				linelim++;
				*lastline = s;
				if (!noprint) {
					tputs(SR, 0, putch);
					putch ('\r');
					prtold(s);
				}
				--topline;
				--botline;
			}
		}
		/* No reverse scroll (sigh...) */
		else
			linelim = prtback(linelim, lastline);
		if (linelim < 0)
			ch = EOF;
		if (!noprint) {
			atxy(1, LI);
			clearline();
		}
	}

	while (linelim-- > 0) {
		if ((s = oldline((int)(maxline - botline))) == 0) {
			if (newlines <= 0)
				ch = EOF;
			break;
		}
		*lastline = s;
		if (!noprint) {
			prtold(s);
			printf("\r\n");
		}
		topline++;
		botline++;
	}

	col = 0;
	maxcol = CO - (AM && !XN ? 1 : 0);
	while (newlines-- > 0) {
		s = mend;
		while ((ch = getc (stream)) != EOF) {
			/* Put new character in circular buffer (yuck) */
			switch (ch) {
			case '\t':
				col += 8 - (col&7);
				break;
			case '\b':
				if (col > 0)
					col--;
				break;
			case '\r':
				col = 0;
				break;
			case 0177:
				break;
			default:
				if (ch >= ' ')
					col++;
				break;
			}
			/*
			 * Fake long lines
			 *	a kludge, but I don't have time for better
			 */
			if (col > maxcol) {
				ungetc(ch, stream);
				ch = '\n';
			}
			*mend++ = ch;
			if (mend >= mbufp+mbsize)
				mend = mbufp;
			if (mend == mbegin) {
				mbegin++;
				if (mbegin >= mbufp+mbsize)
					mbegin = mbufp;
			}
			if (ch == '\n')
				break;
			if (!nointer) {
				ch = EOF;
				break;
			}
			if (!noprint)
				putch (ch);
		}
		col = 0;
		if (ch == EOF)
			break;
		else {
			maxline++;
			botline++;
			topline++;
			*lastline = s;
		}
		if (!noprint) {
			putch ('\n');
			putch ('\r');
			fflush (stdout);
		}
	}
	return ch;
}

/*
 * Print tail of file
 */
prttail(stream)
FILE *stream;
{
	char *ll;

	/* Skip to end of file */
	noprint = 1;
	while (more(stream, curwin, &ll) != EOF);
	more(stream, -curwin, &ll);
	noprint = 0;
	return curwin;
}

/*
 * Print head of file (as much as saved)
 */
prthead(stream)
FILE *stream;
{
	char *ll;

	/* If not a pipe, rewind by seeking */
	if (isfile && fseek(stream, 0L, 0) == 0) {
		initmore();
		return curwin;
	}
	else {
		noprint = 1;
		while (more(stream, -curwin, &ll) != EOF);
		more(stream, curwin, &ll);
		noprint = 0;
		return -curwin;
	}
}

/*
 * Initmore
 */
initmore()
{
	mbegin = mend = mbufp;
	maxline = botline = 0;
	topline = 1 - curwin;
}

/*
 * Print -n lines before topline to botline+n
 */
prtback(n, lastline)
register int n;
char **lastline;
{
	register int i, j;
	register int k;
	register char *s;
	int found;

	if (!noprint) {
		erasebelow(topwin());
		atxy(1, topwin());
	}
	/*
	 * i is the number of lines to print
	 * j is the offset (lines into the old buffer)
	 * n is the line currently being tried (it may not exist)
	 * k becomes the lines left undone (didn't exist)
	 */
	i = curwin-PROMPTLINE;
	if (i > maxline-PROMPTLINE)
		i = maxline-PROMPTLINE;
	j = maxline - (n+topline) + 2;
	*lastline = 0;
	found = 0;
	k = 0;
	while (i > 0) {
		if (found)
			i--;
		n++;
		if ((s = oldline(--j)) == 0) {
			k--;
			continue;
		}
		/* First line actually existant */
		if (!found) {
			found++;
			topline += n-1;
			botline += n-1;
			*lastline = s;
		}
		if (n < 0)
			n++;
		if (!noprint) {
			prtold(s);
			putch('\n');
			putch('\r');
		}
	}
	return k;
}

/*
 * Print an old line
 */
prtold(t)
register char *t;
{
	while (*t != '\n') {
		putchar (*t);
		if (t == mend)
			break;
		if (++t >= mbufp+mbsize)
			t = mbufp;
	}
}

/*
 * Find the n'th old line
 */
char *oldline(ln)
int ln;
{
	register int n;
	register char *s, *t;
	int i, first;
	static int on;
	static char *os;

	n = ln;
	i = 0;
	/* Try to start from previous position (an optimization) */
	if (maxline == omaxline) {
		i = n - on;
		if (i == 0)
			return os;
		if (i == -1 && on > 1) {
			s = t = os;
			for (;;) {
				t++;
				if (t >= mbufp+mbsize)
					t = mbufp;
				if (t == mend)
					break;
				if (*s == '\n')
					goto ret;
				s = t;
			}
		}
	}
	if (i > 0 && os != mbegin) {
		n = i;
		s = os;
	}
	else
		s = mend;
	n++;				/* Newline count is one more */
	first = 1;
	while (n-- > 0) {
		for (;;) {
			t = s;
			if (s <= mbufp)
				s = mbufp+mbsize;
			s--;
			if (s == mbegin) {
				/* Just an approximation */
				if (mbegin != mbufp)
					putch(07);
				if (n > 0)
					return 0;
				t = s;
				break;
			}
			if (*s == '\n') {
				first = 0;
				break;
			}
			/*
			 * If first char is not a newline,
			 *	last line in file is without newline
			 *	adjust n
			 */
			if (first) {
				n--;
				first = 0;
			}
		}
	}
	/* Save for future reference */
ret:
	omaxline = maxline;
	on = ln;
	os = t;
	return t;
}

/*
 * search string
 */
#define	SFOR	-1
#define	SREV	-2

search(stream, way, failp)
FILE *stream;
int way;
int *failp;
{
	register int i;
	register int savec;
	register char *s;
	char *ll;
	int serror;
	static char sbuf[STRMAX];

	*failp = 0;
	/* Get string to search for */
	if (way == 022)
		way = '?';			/* ^R */
	else if (way == 023)
		way = '/';			/* ^S */
	if (way > 0) {
		tty_push(COOKEDMODE);
		putch(way);
		fflush(stdout);
		savec = sbuf[0];
		i = getline(sbuf);
		/* Adjust for linefeed user just hit */
		tty_pop();
		if (UP)
			tputs(UP, 0, putch);
		clearline();
		fflush(stdout);
		if (i == 0) {
			if (savec == 0) {
				if (botline >= curwin) {
					topline++;
					botline++;
				}
				putch(07);
				*failp = 1;
				return -1;
			}
			sbuf[0] = savec;
		}
	}
	i = 0;
	noprint = 1;
	/* Reverse search, does not wraparound */
	if (way == '?' || way == SREV) {
		while (more(stream, -1, &ll) != EOF) {
			if (ll == 0)
				break;
			i++;
			if (match(sbuf, ll)) {
				noprint = 0;
				prtback(-(curwin/2), &ll);
				return 0;
			}
		}
		/* Not found, try to restore screen */
		if (way == '?')
			while (i-- >= 0)
				more(stream, 1, &ll);
	}
	/* Forward search, which wraps around */
	else {
		while (more(stream, 1, &ll) != EOF) {
			if (ll == 0)
				break;
			i++;
			if (match(sbuf, ll)) {
				more(stream, curwin/2, &ll);
				noprint = 0;
				prtback(0, &ll);
				return 0;
			}
		}
		/* Not found below, try wraparound (slow) */
		if (way > 0) {
			/* Rewind to beginning (yuck) */
			if (isfile && fseek(stream, 0L, 0) == 0) {
				initmore();
				more(stream, curwin, &ll);
			}
			while (more(stream, -curwin, &ll) != EOF);
			topline -= curwin;
			botline -= curwin;
			savec = search(stream, SFOR, &serror);
			if (!serror)
				return savec;
			/* Not found, try to restore screen, seek to end */
			noprint = 1;
			while (more(stream, curwin, &ll) != EOF);
			if (i >= maxline-(curwin+curwin/2)) {
				noprint = 0;
				*failp = 1;
				return prthead(stream);
			}
			else {
				while (--i > 0)
					if (more(stream, -1, &ll) == EOF)
						break;
				if (i && SR) {
					noprint = 0;
					topline += curwin;
					botline += curwin;
					prtback(-curwin, &ll);
				}
			}
		}
	}
	noprint = 0;
	if (way > 0) {
		putch(07);
		/* Adjust for pattern entry */
		if (botline == maxline) {
			topline++;
			botline++;
		}
	}
	*failp = 1;
	return -1;
}

/*
 * Find a match in the oldlines
 *	Limited RE's: ^ . $
 */
match(a, b)
char *a;
register char *b;
{
	register char *s, *t;
	int carat;

	/* Carat matches beginning of line */
	if (*a == '^') {
		a++;
		if (*a == 0)
			return 1;
		carat = 1;
	}
	else
		carat = 0;
	for (;;) {
		s = a;
		t = b;
		for (;;) {
			if (*t == '\n') {
				/* Dollar matches endline of line */
				if (*s == '$')
					return 1;
				break;
			}
			/* Dot matches any character */
			if (*s == '.') {
				s++;
				t++;
			}
			else if (*s++ != *t++)
				break;
			if (*s == 0)
				return 1;
			if (t >= mbufp+mbsize)
				t = mbufp;
		}
		if (carat)
			break;
		if (*b == '\n')
			break;
		b++;
		if (b >= mbufp+mbsize)
			b = mbufp;
	}
	return 0;
}

catch () 
{		/* Catch an interrupt */
	nointer = 0;
	signal (SIGINT, catch);
}
