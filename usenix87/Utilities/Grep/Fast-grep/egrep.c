
/*
     Hybrid Boyer/Moore/Gosper-assisted 'grep/egrep/fgrep' search, with delta0
     table as in original paper (CACM, October, 1977).  No delta1 or delta2.
     According to experiment (Horspool, Soft. Prac. Exp., 1982), delta2 is of
     minimal practical value.  However, to improve for worst case input,
     integrating the improved Galil strategies (Apostolico/Giancarlo, SIAM. J.
     Comput., Feb. 1986) deserves consideration.

     Method: 	extract longest metacharacter-free string from expression.
		this is done using a side-effect from henry spencer's regcomp().
		use boyer-moore to match such, then pass submatching lines
		to either regexp() or standard 'egrep', depending on certain
		criteria within execstrategy() below.  [this tradeoff is due
		to the general slowness of the regexp() nondeterministic
		machine on complex expressions, as well as the startup time
		of standard 'egrep' on short files.]  alternatively, one may
		change the vendor-supplied 'egrep' automaton to include
		boyer-moore directly.  see accompanying writeup for discussion
		of kanji expression treatment.

		late addition:  apply trickbag for fast match of simple
		alternations (sublinear, in common low-cardinality cases).
		trap fgrep into this lair.

		gnu additions:  -f, newline as |, \< and \> [in regexec()], more
				comments.  inspire better dfa exec() strategy.
				serious testing and help with special cases.

     Algorithm amalgam summary:

		dfa e?grep 		(aho/thompson)
		ndfa regexp() 		(spencer/aho)
		bmg			(boyer/moore/gosper)
		"superimposed" bmg   	(jaw)
		fgrep			(aho/corrasick)

		sorry, but the knuth/morris/pratt machine, horspool's
		"frequentist" code, and the rabin/karp matcher, however cute,
		just don't cut it for this production.

     James A. Woods				Copyright (c) 1986
     NASA Ames Research Center
*/
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <regexp.h>		/* must be henry spencer's version */

#define	MIN(A, B)	((A) > (B) ? (B) : (A))

#ifdef	SLOWSYS
#define read	xread
#endif
/*
 * define existing [ef]?grep program locations below for use by execvp().
 * [execlp() would be used were it not for the possibility of
 * installation-dependent recursion.] 
 */
#ifndef EGREPSTD
#define	EGREPSTD	"/usr/bin/egrep"
#endif
#ifndef GREPSTD
#define	GREPSTD		"/bin/grep"
#endif
#ifndef FGREPSTD
#define	FGREPSTD	"/usr/bin/fgrep"
#endif

#define BUFSIZE	8192		/* make higher for cray */
#define PATSIZE 6000
#define LARGE 	BUFSIZE + PATSIZE

#define ALTSIZE	100		/* generous? */
#define NALT	7		/* tied to scanf() size in alternate() */
#define NMUSH	6		/* loosely relates to expected alt length */

#define	FIRSTFEW	10	/* Always do FIRSTFEW matches with regexec() */
#define	PUNTPERCENT	5	/* After FIRSTFEW, if PUNTPERCENT of the input
				 * was processed by regexp(), exec std egrep. */
#define NL	'\n'
#define	EOS	'\0'
#define	NONASCII	0200	/* Bit mask for Kanji non-ascii chars */
#define META	"\n^$.[]()?+*|\\"	/* egrep meta-characters */
#define SS2	'\216'		/* EUC Katakana (or Chinese2) prefix */
#define SS3	'\217'		/* EUC Kanji2 (or Chinese3) prefix */

extern char *optarg;
extern int optind;
char *progname;

int cflag, iflag, eflag, fflag, lflag, nflag;	/* SVID flags */
int sflag, hflag;		/* v7, v8, bsd */

int firstflag;			/* Stop at first match */
int grepflag;			/* Called as "grep" */
int fgrepflag;			/* Called as "fgrep" */
int altflag;			/* Simple alternation in pattern */
int boyonly;			/* No regexp needed -- all simple */
int flushflag;
int grepold, egrepold, fgrepold;

int nalt;			/* Number of alternatives */
int nsuccess;			/* 1 for match, 2 for error */
int altmin;			/* Minimum length of all the alternate
				 * strings */
int firstfile;			/* argv index of first file argument */
long nmatch;			/* Number of matches in this file */
long incount, counted;		/* Amount of input consumed */
long rxcount;			/* Bytes of input processed by regexec() */
int boyfound;			/* accumulated partial matches (tripped by
				 * FIRSTFEW) */
int prevmatch;			/* next three lines aid fast -n */
long nline, prevnline;
char *prevloc;

regexp *rspencer;
char *pattern;
char *patboy;			/* Pattern for simple Boyer-Moore */
char *patfile;			/* Filename containing pattern(s) */

int delta0[256];		/* Boyer-Moore algorithm core */
char cmap[256];			/* Usually 0-255, but if -i, maps upper to
				 * lower case */
char str[BUFSIZE + 2];
int nleftover;
char linetemp[BUFSIZE];
char altpat[NALT][ALTSIZE];	/* alternation component storage */
int altlen[NALT];
short altset[NMUSH + 1][256];
char preamble[200];		/* match prefix (filename, line no.) */

int fd;
char *
strchr(), *strrchr(), *strcpy(), *strncpy(), *strpbrk(), *sprintf(), *malloc();
char *
grepxlat(), *fold(), *pfile(), *alternate(), *isolate();
char **args;

main(argc, argv)
	int argc;
	char *argv[];
{
	int c;
	int errflag = 0;

	args = argv;

	if ((progname = strrchr(argv[0], '/')) != 0)
		progname++;
	else
		progname = argv[0];
	if (strcmp(progname, "grep") == 0)
		grepflag++;
	if (strcmp(progname, "fgrep") == 0)
		fgrepflag++;

	while ((c = getopt(argc, argv, "bchie:f:lnsvwxy1")) != EOF) {
		switch (c) {

		case 'f':
			fflag++;
			patfile = optarg;
			continue;
		case 'b':
		case 'v':
			egrepold++;	/* boyer-moore of little help here */
			continue;
		case 'c':
			cflag++;
			continue;
		case 'e':
			eflag++;
			pattern = optarg;
			continue;
		case 'h':
			hflag++;
			continue;
		case '1':	/* Stop at very first match */
			firstflag++;	/* spead freaks only */
			continue;
		case 'i':
			iflag++;
			continue;
		case 'l':
			lflag++;
			continue;
		case 'n':
			nflag++;
			continue;
		case 's':
			sflag++;
			continue;
		case 'w':
		case 'y':
			if (!grepflag)
				errflag++;
			grepold++;
			continue;
		case 'x':	/* needs more work, like -b above */
			if (!fgrepflag)
				errflag++;
			fgrepold++;
			continue;
		case '?':
			errflag++;
		}
	}
	if (errflag || ((argc <= optind) && !fflag && !eflag)) {
		if (grepflag)
oops("usage: grep [-bcihlnsvwy] [-e] pattern [file ...]");
		else if (fgrepflag)
oops("usage: fgrep [-bcilnv] {-f patfile | [-e] strings} [file ...]");
		else		/* encourage SVID options, though we provide
				 * others */
oops("usage: egrep [-bcilnv] {-f patfile | [-e] pattern} [file ...]");
	}
	if (fflag)
		pattern = pfile(patfile);
	else if (!eflag)
		pattern = argv[optind++];

	if ((argc - optind) <= 1)	/* Filename invisible given < 2 files */
		hflag++;
	if (pattern[0] == EOS)
		oops("empty expression");
	/*
	 * 'grep/egrep' merger -- "old" grep is called to handle: tagged
	 * exprs \( \), word matches \< and \>, -w and -y options, char
	 * classes with '-' at end (egrep bug?), and patterns beginning with
	 * an asterisk (don't ask why). otherwise, characters meaningful to
	 * 'egrep' but not to 'grep' are escaped; the entire expr is then
	 * passed to 'egrep'. 
	 */
	if (grepflag && !grepold) {
		if (strindex(pattern, "\\(") >= 0 ||
		    strindex(pattern, "\\<") >= 0 ||
		    strindex(pattern, "\\>") >= 0 ||
		    strindex(pattern, "-]") >= 0 ||
		    pattern[0] == '*')	/* grep bug */
			grepold++;
		else
			pattern = grepxlat(pattern);
	}
	if (grepold || egrepold || fgrepold)
		kernighan(argv);

	if (iflag)
		strcpy(pattern, fold(pattern));
	/*
	 * If the pattern is a plain string, just run boyer-moore. If it
	 * consists of meta-free alternatives, run "superimposed" bmg.
	 * Otherwise, find best string, and compile pattern for regexec(). 
	 */
	if (strpbrk(pattern, META) == NULL) {	/* do boyer-moore only */
		boyonly++;
		patboy = pattern;
	} else {
		if ((patboy = alternate(pattern)) != NULL)
			boyonly++;
		else {
			if ((patboy = isolate(pattern)) == NULL)
				kernighan(argv);	/* expr too involved */
#ifndef NOKANJI
			for (c = 0; pattern[c] != EOS; c++)
				if (pattern[c] & NONASCII)	/* kanji + meta */
					kernighan(argv);
#endif
			if ((rspencer = regcomp(pattern)) == NULL)
				oops("regcomp failure");
		}
	}
	gosper(patboy);		/* "pre-conditioning is wonderful"
				 * -- v. strassen */

	if ((firstfile = optind) >= argc) {
		/* Grep standard input */
		if (lflag)	/* We don't know its name! */
			exit(1);
		egsecute((char *) NULL);
	} else {
		while (optind < argc) {
			egsecute(argv[optind]);
			optind++;
			if (firstflag && (nsuccess == 1))
				break;
		}
	}
	exit((nsuccess == 2) ? 2 : (nsuccess == 0));
}

char *
pfile(pfname)			/* absorb expression from file */
	char *pfname;
{
	FILE *pf;
	struct stat patstat;
	static char *pat;

	if ((pf = fopen(pfname, "r")) == NULL)
		oops("can't read pattern file");
	if (fstat(fileno(pf), &patstat) != 0)
		oops("can't stat pattern file");
	if (patstat.st_size > PATSIZE) {
		if (fgrepflag) {	/* defer to unix version */
			fgrepold++;
			return "dummy";
		} else
			oops("pattern file too big");
	}
	if ((pat = malloc((unsigned) patstat.st_size + 1)) == NULL)
		oops("out of memory to read pattern file");
	if (patstat.st_size !=
	    fread(pat, sizeof(char), patstat.st_size + 1, pf))
		oops("error reading pattern file");
	(void) fclose(pf);

	pat[patstat.st_size] = EOS;
	if (pat[patstat.st_size - 1] == NL)	/* NOP for egrep; helps grep */
		pat[patstat.st_size - 1] = EOS;

	if (nlcount(pat, &pat[patstat.st_size]) > NALT) {
		if (fgrepflag)
			fgrepold++;	/* "what's it all about, alfie?" */
		else
			egrepold++;
	}
	return (pat);
}

egsecute(file)
	char *file;
{
	if (file == NULL)
		fd = 0;
	else if ((fd = open(file, 0)) <= 0) {
		fprintf(stderr, "%s: can't open %s\n", progname, file);
		nsuccess = 2;
		return;
	}
	chimaera(file, patboy);

	if (!boyonly && !flushflag && file != NULL)
		flushmatches();
	if (file != NULL)
		close(fd);
}

chimaera(file, pat)		/* "reach out and boyer-moore search someone" */
	char *file, *pat;	/* -- soon-to-be-popular bumper sticker */
{
	register char *k, *strend, *s;
	register int j, count;
	register int *deltazero = delta0;
	int patlen = altmin;
	char *t;
	char *gotamatch(), *kanji(), *linesave(), *submatch();

	nleftover = boyfound = flushflag = 0;
	nline = 1L;
	prevmatch = 0;
	nmatch = counted = rxcount = 0L;

	while ((count = read(fd, str + nleftover, BUFSIZE - nleftover)) > 0) {

		counted += count;
		strend = linesave(str, count);

		for (k = str + patlen - 1; k < strend;) {
			/*
			 * for a large class of patterns, upwards of 80% of
			 * match time is spent on the next line.  we beat
			 * existing microcode (vax 'matchc') this way. 
			 */
			while ((k += deltazero[*(unsigned char *) k]) < strend);
			if (k < (str + LARGE))
				break;
			k -= LARGE;

			if (altflag) {
				/*
				 * Parallel Boyer-Moore.  Check whether each
				 * of the previous <altmin> chars COULD be
				 * from one of the alternative strings. 
				 */
				s = k - 1;
				j = altmin;
				while (altset[--j][(unsigned char)
					     cmap[*(unsigned char *) s--]]);
				/*
				 * quick test fails. in this life, compare
				 * 'em all.  but, a "reverse trie" would
				 * attenuate worst case (linear w/delta2?). 
				 */
				if (--j < 0) {
					count = nalt - 1;
					do {
						s = k;
						j = altlen[count];
						t = altpat[count];

						while
							(cmap[*(unsigned char *) s--]
							 == t[--j]);
						if (j < 0)
							break;
					}
					while (count--);
				}
			} else {
				/* One string -- check it */
				j = patlen - 1;
				s = k - 1;
				while (cmap[*(unsigned char *) s--] == pat[--j]);
			}
			/*
			 * delta-less shortcut for literati. short shrift for
			 * genetic engineers? 
			 */
			if (j >= 0) {
				k++;	/* no match; restart next char */
				continue;
			}
			k = submatch(file, pat, str, strend, k, count);
			if (k == NULL)
				return;
		}
		if (nflag) {
			if (prevmatch)
				nline = prevnline + nlcount(prevloc, k);
			else
				nline = nline + nlcount(str, k);
			prevmatch = 0;
		}
		strncpy(str, linetemp, nleftover);
	}
	if (cflag) {
		/* Bug from old grep: -c overrides -h.  We fix the bug. */
		if (!hflag)
			printf("%s:", file);
		printf("%ld\n", nmatch);
	}
}

char *
linesave(str, count)		/* accumulate partial line at end of buffer */
	char str[];
	register int count;
{
	register int j;

	count += nleftover;
	if (count != BUFSIZE && fd != 0)
		str[count++] = NL;	/* insurance for broken last line */
	str[count] = EOS;
	for (j = count - 1; str[j] != NL && j >= 0;)
		j--;
	/*
	 * break up these lines: long line (> BUFSIZE), last line of file, or
	 * short return from read(), as from tee(1) input 
	 */
	if (j < 0 && (count == (BUFSIZE - nleftover))) {
		str[count++] = NL;
		str[count] = EOS;
		linetemp[0] = EOS;
		nleftover = 0;
		return (str + count);
	} else {
		nleftover = count - j - 1;
		strncpy(linetemp, str + j + 1, nleftover);
		return (str + j);
	}
}

/*
 * Process partial match. First check for mis-aligned Kanji, then match line
 * against full compiled r.e. if statistics do not warrant handing off to
 * standard egrep. 
 */
char *
submatch(file, pat, str, strend, k, altindex)
	char file[], pat[], str[];
	register char *strend, *k;
	int altindex;
{
	register char *s;
	char *t, c;

	t = k;
	s = ((altflag) ? k - altlen[altindex] + 1 : k - altmin + 1);
#ifndef NOKANJI
	c = ((altflag) ? altpat[altindex][0] : pat[0]);
	if (c & NONASCII)
		if ((s = kanji(str, s, k)) == NULL)
			return (++k);	/* reject false kanji */
#endif
	do;
	while (*s != NL && --s >= str);
	k = s + 1;		/* now at line start */

	if (boyonly)
		return (gotamatch(file, k));

	incount = counted - (strend - k);
	if (boyfound++ == FIRSTFEW)
		execstrategy(file);

	s = t;
	do
		rxcount++;
	while (*s++ != NL);
	*--s = EOS;
	/*
	 * "quick henry -- the flit" (after theodor geisel) 
	 */
	if (regexec(rspencer, ((iflag) ? fold(k) : k)) == 1) {
		*s = NL;
		if (gotamatch(file, k) == NULL)
			return (NULL);
	}
	*s = NL;
	return (s + 1);
}

/*
 * EUC code disambiguation -- scan backwards to first 7-bit code, while
 * counting intervening 8-bit codes.  If odd, reject unaligned Kanji pattern. 
 * SS2/3 checks are for intermixed Japanase Katakana or Kanji2. 
 */
char *
kanji(str, s, k)
	register char *str, *s, *k;
{
	register int j = 0;

	for (s--; s >= str; s--) {
		if (*s == SS2 || *s == SS3 || (*s & NONASCII) == 0)
			break;
		j++;
	}
#ifndef CHINESE
	if (*s == SS2)
		j -= 1;
#endif  CHINESE
	return ((j & 01) ? NULL : k);
}

/*
 * Compute "Boyer-Moore" delta table -- put skip distance in delta0[c] 
 */
gosper(pattern)
	char *pattern;		/* ... HAKMEM lives ... */
{
	register int i, j;
	unsigned char c;

	/* Make one-string case look like simple alternatives case */
	if (!altflag) {
		nalt = 1;
		altmin = altlen[0] = strlen(pattern);
		strcpy(altpat[0], pattern);
	}
	/* For chars that aren't in any string, skip by string length. */
	for (j = 0; j < 256; j++) {
		delta0[j] = altmin;
		cmap[j] = j;	/* Sneak in initialization of cmap */
	}

	/* For chars in a string, skip distance from char to end of string. */
	/* (If char appears more than once, skip minimum distance.) */
	for (i = 0; i < nalt; i++)
		for (j = 0; j < altlen[i] - 1; j++) {
			c = altpat[i][j];
			delta0[c] = MIN(delta0[c], altlen[i] - j - 1);
			if (iflag && islower((int) c))
				delta0[toupper((int) c)] = delta0[c];
		}

	/* For last char of each string, fall out of search loop. */
	for (i = 0; i < nalt; i++) {
		c = altpat[i][altlen[i] - 1];
		delta0[c] = LARGE;
		if (iflag && islower((int) c))
			delta0[toupper((int) c)] = LARGE;
	}
	if (iflag)
		for (j = 'A'; j <= 'Z'; j++)
			cmap[j] = tolower((int) j);
}

/*
 * Print, count, or stop on full match. Result is either the location for
 * continued search, or NULL to stop. 
 */
char *
gotamatch(file, s)
	register char *file, *s;
{
	char *savematch();
	int squirrel = 0;	/* nonzero to squirrel away FIRSTFEW matches */

	nmatch++;
	nsuccess = 1;
	if (!boyonly && boyfound <= FIRSTFEW && file != NULL)
		squirrel = 1;

	if (sflag)
		return (NULL);	/* -s usurps all flags (unlike some versions) */
	if (cflag) {		/* -c overrides -l, we guess */
		do;
		while (*s++ != NL);
	} else if (lflag) {
		puts(file);
		return (NULL);
	} else {
		if (!hflag)
			if (!squirrel)
				printf("%s:", file);
			else
				sprintf(preamble, "%s:", file);
		if (nflag) {
			if (prevmatch)
				prevnline = prevnline + nlcount(prevloc, s);
			else
				prevnline = nline + nlcount(str, s);
			prevmatch = 1;

			if (!squirrel)
				printf("%ld:", prevnline);
			else
				sprintf(preamble + strlen(preamble),
					"%ld:", prevnline);
		}
		if (!squirrel) {
			do
				putchar(*s);
			while (*s++ != NL);
		} else
			s = savematch(s);

		if (nflag)
			prevloc = s - 1;
	}
	return ((firstflag && !cflag) ? NULL : s);
}

char *
fold(line)
	char *line;
{
	static char fline[BUFSIZE];
	register char *s, *t = fline;

	for (s = line; *s != EOS; s++)
		*t++ = (isupper((int) *s) ? (char) tolower((int) *s) : *s);
	*t = EOS;
	return (fline);
}

strindex(s, t)			/* the easy way, as in K&P, p. 192 */
	char *s, *t;
{
	int i, n;

	n = strlen(t);
	for (i = 0; s[i] != '\0'; i++)
		if (strncmp(s + i, t, n) == 0)
			return (i);
	return (-1);
}

char *
grepxlat(pattern)		/* grep pattern meta conversion */
	char *pattern;
{
	register char *p, *s;
	static char newpat[BUFSIZE];

	for (s = newpat, p = pattern; *p != EOS;) {
		if (*p == '\\') {	/* skip escapes ... */
			*s++ = *p++;
			if (*p)
				*s++ = *p++;
		} else if (*p == '[') {	/* ... and char classes */
			while (*p != EOS && *p != ']')
				*s++ = *p++;
		} else if (strchr("+?|()", *p) != NULL) {
			*s++ = '\\';	/* insert protection */
			*s++ = *p++;
		} else
			*s++ = *p++;
	}
	*s = EOS;
	return (newpat);
}

/*
 * Test for simple alternation.  Result is NULL if it's not so simple, or is
 * a pointer to the first string if it is. Warning:  sscanf size is a
 * fixpoint, beyond which the speedup linearity starts to break down.  In the
 * wake of the elegant aho/corrasick "trie"-based fgrep, generalizing
 * altpat[] to arbitrary size is not useful. 
 */
char *
alternate(regexpr)
	char *regexpr;
{
	register i, j, min;
	unsigned char c;
	char oflow[100];

	if (fgrepflag && strchr(regexpr, '|'))
		return (NULL);

	i = strlen(regexpr);
	for (j = 0; j < i; j++)
		if (regexpr[j] == NL)
			regexpr[j] = '|';

	if (!fgrepflag) {
		if (strchr(regexpr, '|') == NULL || regexpr[0] == '|')
			return (NULL);
		if (strpbrk(regexpr, "^$.[]()?+*\\") != NULL
		    || strindex(regexpr, "||") >= 0)
			return (NULL);
	}
	oflow[0] = EOS;
	nalt = sscanf(regexpr, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]",
		      altpat[0], altpat[1], altpat[2], altpat[3], altpat[4], altpat[5], altpat[6], oflow);

	if (nalt < 1 || oflow[0] != EOS)
		return (NULL);

	altmin = NMUSH;
	for (j = 0; j < nalt; j++) {
		min = altlen[j] = strlen(altpat[j]);
		if (min > ALTSIZE)
			return (NULL);
		altmin = MIN(altmin, min);
	}
	if (nalt > 1) {		/* build superimposed "pre-match" sets per
				 * char */
		altflag++;
		for (j = 0; j < nalt; j++)
			for (i = 0; i < altmin; i++) {
				c = altpat[j][altlen[j] - altmin + i];
				altset[i + 1][c] = 1;	/* offset for sentinel */
			}
	}
	return (altpat[0]);
}

/*
 * Grapple with the dfa (std egrep) vs. ndfa (regexp) tradeoff. Criteria to
 * determine whether to use dfa-based egrep:  We do FIRSTFEW matches with
 * regexec().  Otherwise, if Boyer-Moore up to now matched more than
 * PUNTPERCENT of the input, and there is sufficient bulk remaining to
 * justify justify a process exec, do old *grep, presuming that its greater
 * speed at regular expressions will pay us back over this volume.  At
 * FIRSTFEW, dump the saved matches collected by savematch(). They are saved
 * so that a "PUNT" can "rewind" to ignore them.  Stdin is problematic,
 * since it's hard to rewind. 
 */

#define CTHRESH	50000

execstrategy(file)
	char *file;
{
	struct stat stbuf;
	int pctmatch;
	long cremain;

	pctmatch = (100 * rxcount) / incount;
	if (!grepflag && pctmatch > PUNTPERCENT && file != NULL) {
		fstat(fd, &stbuf);
		cremain = stbuf.st_size - incount;
		if (cremain > CTHRESH)
			kernighan(args);
	}
	if (file != NULL)
		flushmatches();
}

nlcount(bstart, bstop)		/* flail interval to totalize newlines. */
	char *bstart, *bstop;
{
	register char *s = bstart;
	register char *t = bstop;
	register int count = 0;

	do {			/* loop unroll for older architectures */
		if (*t == NL)	/* ... ask ames!jaw for sample code */
			count++;
	} while (t-- > s);

	return (count);
}

char *
isolate(regexpr)		/* isolate longest metacharacter-free string */
	char *regexpr;
{
	char *dummyexpr;

	/*
	 * We add (.)* because Henry's regcomp only figures regmust if it
	 * sees a leading * pattern.  Foo! 
	 */
	dummyexpr = malloc((unsigned) strlen(regexpr) + 5);
	sprintf(dummyexpr, "(.)*%s", regexpr);
	if ((rspencer = regcomp(dummyexpr)) == NULL)
		kernighan(args);
	return (rspencer->regmust);
}

char *matches[FIRSTFEW];
static int mcount = 0;

char *
savematch(s)			/* horde matches during statistics gathering */
	register char *s;
{
	char *p;
	char *start = s;
	int msize = 0;
	int psize = strlen(preamble);

	while (*s++ != NL)
		msize++;
	*--s = EOS;

	p = malloc((unsigned) msize + 1 + psize);
	strcpy(p, preamble);
	strcpy(p + psize, start);
	matches[mcount++] = p;

	preamble[0] = 0;
	*s = NL;
	return (s);
}

flushmatches()
{
	int n;

	flushflag = 1;
	for (n = 0; n < mcount; n++)
		printf("%s\n", matches[n]);
	mcount = 0;
}

oops(message)
	char *message;
{
	fprintf(stderr, "%s: %s\n", progname, message);
	exit(2);
}

kernighan(args)			/* "let others do the hard part ..." */
	char *args[];
{
	/*
	 * We may have already run grep on some of the files; remove them
	 * from the arg list we pass on.  Note that we can't delete them
	 * totally because the number of file names affects the output
	 * (automatic -h). 
	 */
	/* better would be fork/exec per punted file -- jaw */

	while (firstfile && optind > firstfile)
		args[firstfile++] = "/dev/null";

	fflush(stdout);

	if (grepflag)
		execvp(GREPSTD, args), oops("can't exec old 'grep'");
	else if (fgrepflag)
		execvp(FGREPSTD, args), oops("can't exec old 'fgrep'");
	else
		execvp(EGREPSTD, args), oops("can't exec old 'egrep'");
}
