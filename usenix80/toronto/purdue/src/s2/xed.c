#

/*
 * Editor
 *
 *	Syntactic changes starting 04/77
 *	Purdue University EE 11/70 Unix System
 *		- TGI
 */

#define TTL "XED V5.51\t[ 12/29 01:29 ]\n*** Use at your own risk ***"
#define PROMPT	">"
#define XED		/* enable powerful stuff */

/*
 * conditional compilation
 */

#ifdef XED
#define AGAIN		/* enable "o" "again" command */
#define APLMAP		/* anable Apl character mapping */
#define DEBUG		/* enable -T flag as well as "du" command */
#define DUMB	0	/* enable command to disable spcl chars */
#define EOL		/* enable special eol stuff */
#define EXTMARK		/* extended "k" capability */
#define PAGE		/* enable proper line counting on ":" */
#define PIPE		/* enable ! command to pipe to process */
#define TABS	512/16	/* words for tab stops */
#define USE		/* enable "u" command */
#define XDEL		/* enable undelete stuff */
#define YINT		/* enable special interrupt processing */
#endif

/*
 * stuff normally enabled
 */

#define CLEAR	"\33:\33H\33J\32\14"	/* HP-2640A, Lear ADM-3A */
#define CMDS	"edsav"	/* all commands written if exists */
#define HELP	"/usr2/news/manual/chapter1/xed.doc"
#ifndef DUMB
#define DUMB	1  /* enable command to disable spcl chars */
#endif

/*
 * data #defines
 */

#define BAK	4	/* file.bak - backup() */
#define BLKSIZE	512	/* disk block size (bytes) */
#define BS1	0100000	/* stty() */
#define CBRA	1
#define CCHR	2
#define CCL	6
#define CCOUNT	80 - 1	/* terminal width */
#define CDOL	10
#define CDOT	4
#define CEOF	11
#define CKET	12
#define CMIN	14
#define EOF	-1
#define ESIZE	128	/* regular expression size */
#define FCHAR	020000	/* character special */
#define FILE	0	/* no extension - backup() */
#define FNSIZE	64	/* max size of pathname to file */
#define FTYPE	060000	/* file-type in stat() structure */
#define GBSIZE	256	/* max global command length */
#define HUP	3	/* file.hup - backup() */
#define INT	2	/* file.int - backup() */
#define LBSIZE	512	/* max line length */
#define MODCNT	35	/* default mod count before auto-write */
#define NBRA	5	/* number of \( \) pairs */
#define NCCL	8
#define PAGSIZ	21	/* page size for ":" command */
#define READ	0
#define SHU	5	/* file.shu - backup() */
#define SIGHUP	1	/* Hangup signal */
#define SIGINT	2	/* Interrupt signal */
#define SIGPIP	13	/* Broken pipe for ! stuff */
#define SIGQIT	3	/* Quit signal */
#define SIGTRC	5	/* Trace/BPT for mail stuff */
#define SIGSHU	15	/* System shutdown */
#define STAR	1
#define TABFILL	'\t'	/* fill character for tab expansion */
#define TMP	1	/* file.tmp - backup() */
#define WRITE	1

#define error	errfunc()

#ifdef AGAIN
char	agbuf[GBSIZE];	/* save area for "again" command */
char	agf	0;	/* "again" flag  (executing the command) */
char	*agp	0;	/* "again" command pointer */
#endif

#ifdef APLMAP
int	aplmap	0;	/* Apl character mapping */
#include <aplmap.h>
#endif

#ifdef CMDS
char	cmd	0;	/* file des for command-save file */
char	cmdfil[]	CMDS;	/* command-save file */
#endif

#ifdef DEBUG
char	tflg	0;	/* tracing flag */
#endif

#ifdef DUMB
char	dumbf	DUMB;	/* 1 = disable special chars in patterns */
#endif

#ifdef EOL
char	eol	0;	/* "end-of-line" char for multiple commands */
			/*   per line */
char	prompt3	1;	/* disable prompts for "eol" stuff */
#endif

#ifdef HELP
char	doc	0;	/* "help" file descriptor */
#endif

#ifdef PIPE
char	piperr	0;	/* pipe error flag - unix() */
#endif

#ifdef TABS
char	tabfill	TABFILL;	/* fill character */
int	maxtab	-1;	/* last column number with tab stop */
char	tabc	0;	/* tab character - if 0 no tab processing */
int	tabs[TABS];	/* each bit on = tab stop */
#endif

#ifdef USE
char	alt	0;	/* alternate command input file */
char	altfile[FNSIZE];
char	eflg2	0;	/* another kludge */
#endif

#ifdef XDEL
int	*deleted	0;    /* pointer to deleted line pointers */
int	ndeleted	0;    /* number of lines */
#endif

#ifdef YINT
char	yflg	0;	/* page upon interrupt */
int	yplus	0;	/* page from this line - if zero, from dot */
#endif

char	*braelist[NBRA];
char	*braslist[NBRA];
char	expbuf[ESIZE+4];
char	file[FNSIZE];
char	genbuf[LBSIZE];
char	ibuff[BLKSIZE];
char	line[LBSIZE + 4];
char	linebuf[LBSIZE];
char	obuff[BLKSIZE];
char	rhsbuf[LBSIZE/2];
char	savedfile[FNSIZE];
char	tempfile[FNSIZE];
char	tfname[]  "/tmp/e00000";  /* "buffer" name */
char	ver[]	TTL;

int	names['z' - 'a' + 1];  /* "k" command markers */
#ifdef EXTMARK
int	names2['z' - 'a' + 1];  /* "k" command markers */
#endif
int	parenc[3]	{ 0, 0, 0, };	/* parentheses counts */

char	*e_prompt PROMPT;  /* editor command prompt */
char	*fmtlno	"%7l=";  /* format for line-number output */
char	*globp;		/* global command pointer */
char	*linp	line;	/* line pointer */
char	*linebp;
char	*loc1;
char	*loc2;
char	*locs;
char	*nextip;
char	aflg	0;	/* "apl mode" flag */
char	appflg	0;	/* append flag (if "w>file") */
char	badf	0;	/* bad read on temp file */
char	bflg	0;	/* "back-up" flag -- Generate back-up file */
char	bflg2	0;	/* Secondary "back-up" flag */
char	brcount	1; /* number of lines to output on "newline" command */
char	circfl;		/* reg expr started with ^ */
char	deltflg	0;	/* don't delete .tmp file upon exit */
char	eflg	0;	/* echo input flag */
char	fflg	0;	/* "file" flag */
char	fout	1;	/* putchar() writes on this fildes */
char	gaskf	0;	/* verify mode on global command */
char	globf2	0;	/* kludge for -f */
char	hugef	0;	/* -h is process huge file */
char	hupflag	0;	/* hangup signal has been caught */
char	iflg	0;	/* file.int and exit on interrupt */
char	immflg	0;	/* immediate flag -- q and e */
char	io	0;	/* file descriptor for "r", "w", "e" */
char	lastc	0;	/* peekc set to lastc on interrupt */
char	modcount  MODCNT;  /* number of mods before auto-write */
char	mods	0;	/* number of mods */
char	num_reads  0;	/* indicator to aid text_modified-- */
			/* first read isn't really a modify */
char	parenf	0;	/* count parentheses and brackets */
char	peekc	0;	/* one character pushback */
char	pflag;		/* print line after doing command */
char	pipef	0;	/* for talking to pipes */
char	prompt1	1;  /* flag--enable or disable line-num prompts */
char	prompt2	1;  /* flag--enable or disable ALL prompting */
char	reading	0;	/* waiting on tty read */
char	seekf	0;	/* no seek to EOF on error on fd 0 */
char	shutflg	0;	/* if shutdown signal (15) occurred */
char	tfile	-1;	/* file des for "buffer" */
char	zflg	0;	/* if "stty bs1" not set */
			/* bs1 displays ctrl-z as ^Z on tty */

int	*addr1;		/* lower line bound */
int	*addr2;		/* upper line bound */
int	*dol;		/* last line in file */
int	*dot;		/* "current" line */
int	*dotdot;	/* last different "dot" */
int	*endcore;	/* current end of memory */
int	*fendcore;	/* start of dynamic area */
int	*lastdot;	/* last "dot" */
int	*old_a1;	/* previous address bounds */
int	*old_a2;
int	*zero;		/* anchor line for all other lines */
int	ccount	CCOUNT;  /* terminal width */
int	col	0;	/* column counter for calculating line wraps */
int	iblock	-1;	/* block number of input buffer */
int	ichanged;	/* ibuf has been changed */
int	line_num;	/* integer for line number on output */
int	listf	0;	/* list control chars explicitly */
int	ninbuf;		/* ??? */
int	nleft;		/* byte count remaining in output buffer */
int	num;		/* ??? */
int	oblock	-1;	/* output buffer block number */
int	pcount	PAGSIZ;	/* number of lines to display on ":" command */
int	savf;		/* counter for auto-write stuff */
int	s_cnt	0;	/* counter for "s/str1/str2/nn" */
int	s_tmp	0;	/* scratch var for same */
int	text_modified	0; /* flag--on if text was modified */
int	tline;		/* ??? */
int	_1	~0377;	/* tl =& _1; getline()... */
int	_2	0400;	/* tl =+ _2; getline()... */
int	_3	0377;	/* bno = ... & _3; getblock() */
int	_4	0774;	/* off = ... & _4; getblock() */
int	_5	255;	/* if (bno >= _5)... getblock() */
int	_6	077776;	/* tline =+ ... & _6; */

/*
 * error messages
 *
 *	(there are more than these)
 */

char	*errtext[]	{
	/*  0 */	"syntax is k[a-z]",
	/*  1 */	"illegal command format",
	/*  2 */	"no command",
	/*  3 */	"no tab character",
	/*  4 */	"can't change filename",
	/*  5 */	"file name syntax",
	/*  6 */	"recursive \"use\" command",
	/*  7 */	"null file name illegal",
	/*  8 */	"unrecognized command",
	/*  9 */	"no tabs set",
	/* 10 */	"global command not allowed with huge file",
	/* 11 */	"file name too long",
	/* 12 */	"expanded line too long",
	/* 13 */	0,
	/* 14 */	"can't fork",
	/* 15 */	"can't write to process",
	/* 16 */	"no lines",
	/* 17 */	"backup(FILE) error (?)",
	/* 18 */	"string not found",
	/* 19 */	"  '  must be followed by [a-z]",
	/* 20 */	"address syntax error",
	/* 21 */	"lower address bound > upper one",
	/* 22 */	"address illegal here",
	/* 23 */	"non-existent line number",
	/* 24 */	"bottom of file reached",
	/* 25 */	"command syntax error",
	/* 26 */	"\"advance\" error (?)",
	/* 27 */	"null string illegal",
	/* 28 */	"destination not found",
	/* 29 */	"INTERRUPT!",
	/* 30 */	"line too long",
	/* 31 */	"missing destination address",
	/* 32 */	"I/O error--file not saved!",
	/* 33 */	"file overflows available memory",
	/* 34 */	"file too large (TMPERR)",
	/* 35 */	"I/O error on temp file (TMPERR)",
	/* 36 */	"open error on temp file (TMPERR)",
	/* 37 */	"recursive global command",
	/* 38 */	"global command list too long",
	/* 39 */	"substitute pattern not found",
	/* 40 */	"missing substring",
	/* 41 */	"string2 too long",
	/* 42 */	"substring too long",
	/* 43 */	"substituted string too long",
	/* 44 */	"too many  \\(",
	/* 45 */	"unbalanced  \\(  \\)",
	/* 46 */	"\\n illegal in pattern",
	/* 47 */	"illegal use of  *",
	/* 48 */	"missing  ]",
	/* 49 */	"pattern too complicated",
	/* 50 */	"string too long",
#define MAXERR	50
};

main(argc, argv)
char **argv;
{
	int onintr(), hangup(), mail(), shutdown(), getfile();
	register char *p1, *p2;
	register n;
	int savint;

	signal(SIGQIT, 1);
	savint = signal(SIGINT, 1);
	prompt2 = istty(0);
	while (--argc) {
		argv++;
		if (**argv == '-') {
			while (*++*argv) {
				switch (**argv) {
#ifdef APLMAP
				case 'A':  /* apl char mapping */
					aplmap++;
					errtext[29] = "G interrupt G";
#ifdef XED
					for (n = 0; n < 5; n++)
#endif
#ifndef XED
					for (n = 0; n < 4; n++)
#endif
						ver[n] =| 040;
#endif
				case 'a':  /* apl mode */
					aflg = 1;
					fmtlno = "[ %l ]\t";
#ifdef DUMB
					dumbf++;
#endif
#ifdef XED
					ver[25] = 0;
#endif
					break;
				case 'b':  /* file.bak on entry */
					bflg++;
					bflg2++;
					break;
				case 'c':  /* crt depth in lines */
					++*argv;
					n = argnum(argv);
					if (n >= 0)
						pcount = n;
					break;
				case 'd':  /* don't delete .tmp file */
					deltflg++;
					break;
				case 'e':  /* echo input commands */
					eflg++;
					break;
				case 'f':  /* create mode */
					fflg++;
					break;
#ifdef XED
				case 'h':  /* edit "huge" file */
					hugef = 1;
					_1 = ~0177;
					_2 = 0200;
					_3 = 0777;
					_4 = 0776;
					_5 = 511;
					_6 = 077777;
					break;
#endif
				case 'i':  /* file.int on interrupt */
					iflg++;
					break;
#ifdef EOL
				case 'l':  /* set eol char to "x" */
					if (*++*argv)
						eol = **argv;
					else
						--*argv;
					break;
#endif
				case 'm':  /* mod cnt for autosave */
					++*argv;
					n = argnum(argv);
					if (n >= 0)
						modcount = n;
					break;
				case 'n':  /* no line num */
					prompt1 = 0;
					break;
				case 'o':  /* no seek to EOF on error */
					seekf++;
					break;
				case 'p':  /* force prompts for pipe */
					pipef = prompt2 = 1;
					break;
				case 'q':  /* don't inhibit quits */
					signal(SIGQIT, 0);
					break;
#ifdef DUMB
				case 'r':  /* spcl char meaning */
					dumbf = !DUMB;
					break;
#endif
				case 's':  /* silent mode */
					prompt2 = 0;
					break;
#ifdef TABS
				case 't':  /* tab char */
					if (*++*argv)
						tabc = **argv;
					else
						--*argv;
					break;
				case 'v':  /* tab fill char */
					if (*++*argv)
						tabfill = **argv;
					else
						--*argv;
					break;
#endif
				case 'w':  /* crt width */
					++*argv;
					n = argnum(argv);
					if (--n >= 2)
						ccount = n;
					break;
#ifdef YINT
				case 'y':  /* page on interrupt */
					yflg++;
					break;
#endif
#ifdef DEBUG
				case 'D':  /* trace mode -- debug */
					tflg++;
					break;
#endif
				default:  /* tabs stops/illegals */
					if (!**argv
#ifdef TABS
						    || **argv == ','
#endif
								    )
						break;
#ifdef TABS
					if (**argv < '0' ||
					    **argv > '9') {
#endif
						printf("bad flag: -%c\n",
						    **argv);
						flush_buf();
						exit(1);
#ifdef TABS
					}
					n = argnum(argv);
					if (--n < TABS * 16)
						settab(n);
#endif
				}
			}
		}
		else {
			p1 = *argv;
			p2 = savedfile;
			while (*p2++ = *p1++)
				if (p2 >= &savedfile[FNSIZE - 2]) {
					puts(errtext[11]);
					flush_buf();
					exit(1);
				}
			globf2 = 1;
			if (fflg)
				globp = "a\n";
			else
				globp = "r\n";
		}
	}

	signal(SIGTRC, mail);
	signal(SIGHUP, hangup);
	signal(SIGSHU, shutdown);

#ifdef YINT
	if (iflg)
		yflg = 0;
#endif
	if (!istty(1) && !eflg)
		prompt2 = 0;
	fendcore = sbrk(0);
	n = getpid();
	for (num = 10; num > 5; num--) {
		tfname[num] = n % 10 + '0';
		n =/ 10;
	}
	if (prompt2) {
#ifdef CMDS
		if ((cmd = open(cmdfil, 2)) > 0)
			seek(cmd, 0, 2);
		else
			cmd = 0;
#endif
		puts(ver);	/* XED V0.00 ... */
		flush_buf();
	} else
		modcount = 0;
#ifdef CMDS
	if (cmd && *savedfile) {
		write(cmd, "e,", 2);
		p1 = savedfile;
		while (*p1++);
		write(cmd, savedfile, p1 - savedfile);
		write(cmd, "\n", 1);
	}
#endif
	setexit();
	if ((savint & 01) == 0)
		signal(SIGINT, onintr);
#ifdef YINT
	else
		yflg = 0;
#endif
	init();
	setexit();
	do
		commands(0);
	while (are_you_sure());
	if (fflg)
		backup(FILE);
	if (text_modified == 0 && immflg == 0)
		backup(-TMP);
	flush_buf();
	delexit(0);
}

abort() {
	register char *p;
	register char c;

	setnoaddr();
	peekc = 0;
	p = "ort\n";
	while (*p)
		if ((c = getchar()) != *p++) {
			peekc = c;
			errmsg(25);
		}
	delexit(1);
}

address() {
	register *a1, minus, c;
	int n, relerr, *start;

	minus = 0;
	a1 = 0;
	for (;;) {
		c = getchar();
		if ('0' <= c && c <= '9') {
			peekc = c;
			n = getnum();
			if (a1 == 0) {
				a1 = zero;
				n =+ aflg;
			}
			if (minus < 0)
				n = -n;
			a1 =+ n;
			minus = 0;
			continue;
		}
		relerr = 0;
		if (a1 || minus)
			relerr++;
		switch (c) {
		case ' ':
		case '\t':
			continue;

		case '+':
			minus =+ brcount;
			if (a1 == 0)
				a1 = dot;
			continue;

		case '-':
		case '^':	/* for upwards compatibility */
			minus =- brcount;
			if (a1 == 0)
				a1 = dot;
			continue;

	     /* search:	*/
		case '?':
			minus++;
		case '/':
			compile(c);
			if (a1 == 0)
				a1 = dot;
			if (a1 < zero)
				a1 = zero;
			if (a1 > dol)
				a1 = dol;
			start = a1;
			for (;;) {
				if (minus == 0) {
					if (++a1 > dol)
						a1 = zero;
				} else {
					if (--a1 < zero)
						a1 = dol;
				}
				if (execute(0, a1)) {
					minus = 0;
					relerr = 0;
					break;
				}
				if (a1 == start)
					errmsg(18);
			}
			break;

		case '$':
			a1 = dol;
			break;

		case '.':
			if ((peekc = getchar()) == '.') {
				peekc = 0;
				a1 = dotdot;
			} else
				a1 = dot;
			break;

		case '\'':
			if ((c = getchar()) < 'a' || c > 'z') {
				peekc = c;
				errmsg(19);
			}
		casemark:
			for (a1 = zero + 1; a1 <= dol; a1++)
				if (names[c - 'a'] == (*a1 | 01))
					break;
#ifdef EXTMARK
			if (names2[c - 'a']) {
				if (addr1 || addr2)
					errmsg(20);
				addr1 = a1;
				for (a1 = zero + 1; a1 <= dol; a1++)
					if (names2[c - 'a'] == (*a1 | 01))
						break;
				addr2 = a1;
				return(0);
			}
#endif
			break;

		case '=':
			if ((peekc = getchar()) == '^')
				a1 = old_a1;
			else if (peekc == '$')
				a1 = old_a2;
			else {
				if (addr1 || addr2)
					errmsg(20);
				addr1 = old_a1;
				addr2 = old_a2;
				return(0);
			}
			peekc = 0;
			break;

		default:
			if ('A' <= c && c <= 'Z') {
				c =| 040;
				goto casemark;
			}
/*			if (c > ' ' && !alpha(c))
 *				goto search;
 */
			peekc = c;
			if (a1 == 0)
				if (c == ',' || c == ';')
					if (dot + 1 > dol)
						return(dol);
					else
						return(dot + 1);
				else
					return(0);
			a1 =+ minus;
			if (a1 < zero)
				a1 = dol == zero? zero : zero + 1;
			else if (a1 > dol)
				a1 = dol;
			return(a1);
		}
		if (relerr)
			errmsg(20);
	}
}

advance(alp, aep) {
	register char *lp, *ep, *curlp;

	lp = alp;
	ep = aep;
	for (;;) switch (*ep++) {

	case CCHR:
		if (*ep++ == *lp++)
			continue;
		return(0);

	case CDOT:
		if (*lp++)
			continue;
		return(0);

	case CDOL:
		if (*lp == 0)
			continue;
		return(0);

	case CEOF:
		loc2 = lp;
		if (--s_tmp > 0)
			return(0);
		return(1);

	case CCL:
		if (cclass(ep, *lp++, 1)) {
			ep =+ *ep;
			continue;
		}
		return(0);

	case NCCL:
		if (cclass(ep, *lp++, 0)) {
			ep =+ *ep;
			continue;
		}
		return(0);

	case CBRA:
		braslist[*ep++] = lp;
		continue;

	case CKET:
		braelist[*ep++] = lp;
		continue;

	case CDOT | STAR:
		curlp = lp;
		while (*lp++);
		goto star;

	case CCHR | STAR:
		curlp = lp;
		while (*lp++ == *ep);
		ep++;
		goto star;

	case CCL | STAR:
	case NCCL | STAR:
		curlp = lp;
		while (cclass(ep, *lp++, ep[-1] == (CCL | STAR)));
		ep =+ *ep;
		goto star;

	star:
		do {
			lp--;
			if (lp == locs)
				break;
			if (advance(lp, ep))
				return(1);
		} while (lp > curlp);
		return(0);

	default:
		errmsg(-26);
	}
}

/*
alpha(c)
char c;
{
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
		return(1);
	else
		return(0);
}
 */

append(f, a, single, bkpf)
int (*f)();
{
	register *a1, *a2, *rdot;
	int nline, tl;
	struct { int integer; };

	nline = 0;
	dot = a;
	while ((*f)(single) == 0) {
		if (dol >= endcore) {
			if (sbrk(1024) == -1)
				errmsg(33);
			endcore.integer =+ 1024;
		}
		tl = putline();
		nline++;
		a1 = ++dol;
		a2 = a1 + 1;
		rdot = ++dot;
		while (a1 > rdot)
			*--a2 = *--a1;
		*rdot = tl;
		if (!globp && modcount && bkpf) {
			if (++mods >= modcount) {
				mods = 0;
				if (backup(TMP) && prompt2
#ifdef EOL
					&& prompt3
#endif
						  )
					puts("[file saved]");
			}
		}
		if (single)
			break;
	}
	return(nline);
}

are_you_sure() {
	register c, n, l;

	l = listf;
	listf = 0;
#ifdef USE
	if (alt)
		return(1);
#endif
	if (!text_modified || fflg || immflg || zero == dol)
		return(0);
	while (1) {
		puts2("\ndid you forget to save your text? ");
		for (n = 0; n < 5; n++) {
			flush_buf();
			if ((c = getchar()) == EOF) {
				putchar('\n');
				return(0);
			}
			if (c != '\n')
				skip_rest();
			else {
				listf = l;
				return(1);
			}
			if (c == 'y' || c == 'n') {
				listf = l;
				return(c == 'y'? 1 : 0);
			}
			puts2("yes or no? ");
		}
	}
}

argnum(ap)
char **ap;
{
	register n;
	register char *p;

	p = *ap;
	n = 0;
	while ('0' <= *p && *p <= '9')
		n = n * 10 + *p++ - '0';
	*ap = --p;
	return(n);
}

backup(af) {
	register char *p1, *p2, *t2;
	int *a1, *a2, savint;

	flush_buf();
	p1 = savedfile;
	t2 = p2 = file;
	while (*p2 = *p1++)
		if (*p2++ == '/')
			t2 = p2;
	if (p2 > t2 + 10)
		p2 = t2 + 10;
	switch (af < 0? -af : af) {
		case FILE:	p1 = "";	break;
		case TMP:	p1 = ".tmp";	break;
		case INT:	p1 = ".int";	break;
		case HUP:	p1 = ".hup";	break;
		case BAK:	p1 = ".bak";	break;
		case SHU:	p1 = ".shu";	break;
		default:  errmsg(-17);
	}
	if (af)
		while (*p2++ = *p1++);
	if (af < 0) {
		if (deltflg)
			return(1);
		return(!unlink(file));
	}
	if (dol == zero && !fflg)
		return(1);
	if ((io = creat(file, (af == FILE? 0644 : 0600))) < 0) {
		if (af != TMP) {
			puts2("can't create ");
			puts(file);
		}
		io = 0;
		return(0);
	}
	if (dol == zero) {
		exfile();
		return(1);
	}
	a1 = addr1;
	a2 = addr2;
	addr1 = zero + 1;
	addr2 = dol;
	if (!iflg)
		savint = signal(SIGINT, 1);
	putfile();
	if (!iflg)
		signal(SIGINT, savint);
	exfile();
	addr1 = a1;
	addr2 = a2;
	return(1);
}

blkio(b, buf, iofcn)
int (*iofcn)();
{
	seek(tfile, b, 3);
	if ((*iofcn)(tfile, buf, 512) != 512) {
		badf++;
		errmsg(-35);
	}
}

cclass(aset, ac, af) {
	register char *set, c;
	register n;
	char m;

	set = aset;
	if ((c = ac) == 0)
		return(0);
	n = *set++;
	while (--n) {
		if (set[1] == '-') {
			c = min(set[0], set[2]);
			m = max(set[0], set[2]);
			do{
				if (c == ac)
					return(af);
				c++;
			} while (c != m);
			set =+ 2;
			c = ac;
		} else
			if (*set++ == c)
				return(af);
	}
	return(!af);
}

chktmp() {
	register char *p2, *p1, c;
	char *t2;
	char buf[36], oldc;

	if (*savedfile == '\0')
		return(0);
	t2 = p2 = file;
	p1 = savedfile;
	while (*p2 = *p1++)
		if (*p2++ == '/')
			t2 = p2;
	if (p2 > t2 + 10)
		p2 = t2 + 10;
	p1 = ".tmp";
	while (*p2++ = *p1++);
	if (stat(file, buf) != -1) {
		puts("When you were last editing this file");
		puts("you did not exit the editor normally,");
		puts2("leaving the file:  \"");
		puts2(file);
		puts("\".\nIt contains your file up to the last \"[file saved]\"");
		puts("message.  This file will be deleted if you do");
		puts("not read it into the editor now.  If you read");
		puts("it, then decide not to use it, exit the editor");
		puts2("with \"qi\".\n\nDo you wish to read it? ");
		oldc = peekc;
		peekc = 0;
		flush_buf();
		while ((c = getchar()) != EOF) {
			peekc = c;
			skip_rest();
			if (c == 'y' || c == 'n') {
				peekc = oldc;
				return(c == 'y'? 1 : 0);
			}
			puts2("yes or no? ");
			flush_buf();
		}
		putchar('\n');
		peekc = '\n';
		return(0);
	}
	return(0);
}

clear() {
	register l, i;

	l = listf;
	listf = 0;
/*
 * set up for Lear-Siegler ADM-3A and Hewlett-Packard 2640A
 */
	puts2(CLEAR);	/* clear sequence */
	for (i = 0; i < 5; i++)
		putchar(0);	/* ADM-3A's need padding at 19.2 */
					/* also at 38.4 kbaud */
	putchar('\n');
	listf = l;
}

commands(baseflg) {
	int getfile(), gettty(), onintr();
	register *a1, c;
	register char *p;
	int r;

	for (;;) {
#ifdef AGAIN
	if (agp) {
		*agp++ = '\n';
		*agp = 0;
	}
	agf = agp = 0;
#endif
	immflg = 0;
	if (!globp && (hupflag || shutflg)) {
		if (hupflag)
			backup(HUP);
		else
			backup(SHU);
		delexit(1);
	}
	if (pflag) {
		pflag = 0;
		addr1 = addr2 = dot;
		goto print;
	}
#ifdef USE
	if (!globp && !alt) {
#endif
#ifndef USE
	if (!globp) {
#endif
		if (modcount) {
			if (text_modified > savf)
				mods++;
			savf = text_modified;
			if (mods >= modcount) {
				mods = 0;
#ifdef EOL
				if (backup(TMP) && prompt2 && prompt3)
#endif
#ifndef EOL
				if (backup(TMP) && prompt2)
#endif
					puts("[file saved]");
			}
		}
#ifdef EOL
#ifdef USE
		if (prompt2 && prompt3 && !eflg2)
#endif
#ifndef USE
		if (prompt2 && prompt3)
#endif
#endif
#ifndef EOL
#ifdef USE
		if (prompt2 && !eflg2)
#endif
#ifndef USE
		if (prompt2)
#endif
#endif
			puts2(e_prompt);
	}
	if (!globp) {
		if (dotdot > dol)
			dotdot = dol;
		if (dotdot < zero)
			dotdot = zero;
		if (dot != lastdot) {
			dotdot = lastdot;
			lastdot = dot;
		}
	}
	addr1 = 0;
	addr2 = 0;
	s_tmp = 0;
	s_cnt = 0;
	r = 1;
	do {
		addr1 = addr2;
		if ((a1 = address()) == 0) {
			c = getchar();
			break;
		}
		addr2 = a1;
		if ((c = getchar()) == ';') {
			c = ',';
			dot = a1;
		}
	} while (c == ',' && r-- > 0);
	if (addr1 == 0)
		addr1 = addr2;
	if (!globp && !baseflg) {
		if (addr1) {
			old_a1 = addr1;
			old_a2 = addr2;
		} else
			old_a1 = old_a2 = dot;
	}
	line_num = (addr1? addr1 : dot) - zero;
#ifdef AGAIN
	/* ^Q as again command only 'cause Purdue's ^Q is "raw" */
	if (c == 'o' || c == '\021') { /* again command "o" */
		if (c != '\021' && (peekc = getchar()) != '\n')
			errmsg(1);
		if (c == '\021')
			putchar(lastc = '\n');
		if (*agbuf == 0)
			errmsg(2);
		agf++;
		agp = agbuf;
		c = *agp++;
		peekc = 0;
	} else if (baseflg == 0 && globp == 0) {
		agp = agbuf;
		*agp++ = c;  /* first char not yet saved in buffer */
	}
#endif

	switch (c) {
	case 'a':
		if ((peekc = getchar()) == 'b')
			abort();
		setdot();
#ifdef XED
		if (peekc != ' ' && peekc != '\n') {
			c = peekc;
			peekc = 0;
			if (tack(c, 1))
				text_modified++;
			continue;
		}
#endif
		line_num++;
		num = addr2;
	caseadd:
		if ((c = getchar()) == ' ')
			r = 1;
		else {
			if (c != '\n') {
				peekc = c;
				newline();
			}
			r = 0;
		}
		if (append(gettty, num, r, 1))
			text_modified++;
		continue;

	case 'b':
		setnoaddr();
		while ((c = getchar()) == ' ' || c == '\t');
		peekc = c;
		num = getnum();
		newline();
		if (num > 0)
			brcount = num;
		else
			brcount = 1;
		continue;

	case 'c':
		if ((peekc = getchar()) == 'o') {  /* co == t */
			peekc = 0;
			goto casecopy;
		}
		if (peekc != '\n')
			goto casesub;
		newline();
		setdot();
		nonzero();
		delete();
		text_modified++;
		append(gettty, addr1 - 1, 0, 0);
		continue;

	case 'd':
#ifdef DEBUG
/*		*du	Dump command (testing only)	*/
		if ((peekc = getchar()) == 'u') {
			peekc = 0;
			dump();
			continue;
		}
#endif
		newline();
		setdot();
		nonzero();
		delete();
		text_modified++;
		continue;

	case 'e':
	    eagain:
		if ((peekc = getchar()) != '\n') {
#ifdef EOL
			if (peekc == '=') {  /* e=c - set eol to 'c' */
				peekc = 0;
				if (immflg)
					errmsg(8);
				setnoaddr();
				if ((c = getchar()) == '\\')
					c = getchar();
				if (c != '\n') {
					newline();
					eol = c;
					continue;
				}
				eol = 0;
				continue;
			}
#endif
#ifdef TABS
			if (peekc == 'x') {
				peekc = 0;
				if (immflg)
					errmsg(8);
				if ((c = getchar()) != 'p')
					goto illfnm;
				newline();
				if (!tabc)
					errmsg(3);
				if (maxtab < 0)
					errmsg(9);
				if (exp())
					text_modified++;
				continue;
			}
#endif
			if (peekc == 'i') {
				peekc = 0;
				if (immflg)
					errmsg(8);
				immflg++;
				goto eagain;
			}
			setnoaddr();
			if (fflg)
				errmsg(4);
			if (peekc != ' ' && peekc != ',')
		illfnm:		errmsg(5);
			for (num = 0; tempfile[num] = savedfile[num];
			    num++);
			if (zero == dol || text_modified == 0)
				backup(-TMP);
			savedfile[0] = 0;
			filename();
			if (text_modified && are_you_sure()) {
				for (num = 0; savedfile[num] =
				    tempfile[num]; num++);
				error;
			}
			peekc = '\n';
		} else {
			setnoaddr();
			if (text_modified) {
				r = peekc;
				peekc = 0;
				if (are_you_sure()) {
					peekc = r;
					error;
				}
				peekc = r;
			}
			if (zero == dol || text_modified == 0)
				backup(-TMP);
		}
		if (init())
			continue;
		goto caseread;

	case 'f':
		setnoaddr();
#ifdef TABS
		if ((peekc = getchar()) == '=') {  /* f=c fill char */
			peekc = 0;
			if ((c = getchar()) == '\\')
				c = getchar();
			if (c != '\n') {
				newline();
				tabfill = c;
				continue;
			}
			tabfill = TABFILL;
			continue;
		}
#endif
		if ((c = getchar()) != '\n') {
			if (fflg)
				errmsg(4);
			peekc = c;
			filename();
			for (c = 0; savedfile[c] = file[c]; c++);
		}
		puts(savedfile);
		continue;

	case 'g':
		global(1);
		continue;

	case 'h':
#ifdef HELP
		if ((peekc = getchar()) == 'e') {  /* he[lp] */
			peekc = 0;
			setnoaddr();
			skip_rest();
			if ((doc = open(HELP, 0)) > 0) {
				while ((num = read(doc, linebuf,
				    LBSIZE)) > 0)
					write(1, linebuf, num);
				close(doc);
			}
			doc = 0;
			continue;
		}
#endif
		setdot();
		nonzero();
		header();
		continue;

	case 'i':
		setdot();
#ifdef XED
		if ((peekc = getchar()) != ' ' && peekc != '\n') {
			c = peekc;
			peekc = 0;
			if (tack(c, 0))
				text_modified++;
			continue;
		}
#endif
		nonzero();
		num = addr2 - 1;
		goto caseadd;

#ifdef XED
	case 'j':
		newline();
		setdot();
		nonzero();
		join();
		text_modified++;
		continue;
#endif

	case 'k':
		if ((c = getchar()) == '\n') {
			setnoaddr();
			printmarks();
			continue;
		}
		if ('A' <= c && c <= 'Z')
			c =| 040;
		if (c < 'a' || c > 'z')
			errmsg(0);
		newline();
		setdot();
		nonzero();
#ifdef EXTMARK
		if (addr1 != addr2) {
			names[c - 'a'] = *addr1 | 01;
			names2[c - 'a'] = *addr2 | 01;
		} else {
			names[c - 'a'] = *addr2 | 01;
			names2[c - 'a'] = 0;
		}
#endif
#ifndef EXTMARK
		names[c - 'a'] = *addr2 | 01;
#endif
		continue;

	case 'm':
#ifdef DUMB
		if ((peekc = getchar()) == '\n') {
			setnoaddr();
			dumbf = !dumbf;
			peekc = 0;
			printf("\"$&[^.*\\(\\)\" have %sspecial meaning\n",
			    (dumbf? "no " : ""));
			continue;
		}
#endif
		if ((peekc = getchar()) == 'o')  /* mo == m */
			peekc = 0;
		move(0);
		text_modified++;
		continue;

	case 'n':
		setnoaddr();
		newline();
		prompt1 = !prompt1;
		printf("%sline numbers\n", (prompt1? "" : "no "));
		continue;

	case '\n':
		if (globp || addr2 > dol)
			continue;
		if (addr2 == 0) {
			addr1 = addr2 = dot + 1;
			if (addr2 <= dol)
				line_num++;
			if (brcount != 1) {
				addr2 = dot + brcount;
				if (addr1 > dol)
					continue;
				if (addr2 > dol)
					addr2 = dol;
				if (addr2 < zero)
					addr2 = zero;
#ifdef CLEAR
				if (zflg && addr2-addr1 > pcount/2)
					clear();
#endif
			}
		}
		if (addr2 <= dol) {
			pflag = 0;
			goto print;
		}
		continue;

	case 'l':
		listf++;
	case 'p':
		if ((peekc = getchar()) == 'a') {
	case ':':
	case '*':
			peekc = 0;
			newline();
			pflag = 0;
			if (addr1 == 0)
				addr1 = addr2 = dot + 1;
			nonzero();
			if (addr1 == addr2 || addr2 > dol ||
			    addr2 <= zero)
				addr2 = dol;
			setdot();
#ifdef CLEAR
			if (zflg && addr2 - addr1 > 10)
				clear();
#endif
#ifdef PAGE
			page();
#endif
#ifndef PAGE
			if (addr2 > addr1 + pcount)
				addr2 = addr1 + pcount;
			goto print;
#endif
			listf = 0;
			parenf = 0;
			continue;
		} else if (peekc == 'p' || peekc == 'l') {
			peekc = 0;
			addr1 = zero + 1;
			addr2 = dol;
		}
		newline();
		pflag = 0;
	print:
		setdot();
		nonzero();
		a1 = addr1;
		parenc[0] = 0;
		parenc[1] = 0;
		parenc[2] = 0;
		do {
			col = 0;
			if (prompt1) {
				c = listf;
				listf = 0;
				line_num = a1 - zero;
				printf(fmtlno, line_num - aflg);
				listf = c;
				col = 8;
			}
			puts(getline(*a1++));
		} while (a1 <= addr2);
		dot = addr2;
		listf = 0;
		parenf = 0;
		continue;

	case 'q':
		if ((peekc = getchar()) == 'i') {  /* qi - immediate */
			peekc = 0;
			newline();
			immflg++;
			return;
		}
	casequit:
		setnoaddr();
		newline();
		return;

	case 'r':
	caseread:
		filename();
		if ((io = open(file, 0)) < 0) {
			lastc = '\n';
			puts2("can't read ");
			puts(file);
			io = 0;
			error;	/* errmsg(9) */
		}
		setall();
		ninbuf = 0;
		r = append(getfile, addr2, 0, 0);
		if (prompt2)
			printf("%l line%s\n", r, (r == 1? "" : "s"));
		exfile();
		if (num_reads++ || fflg)
			if (r)
				text_modified++;
		else
			text_modified = 0;
		if ((c == 'e' && bflg == 1) || bflg2 == 1) {
			bflg2 = 0;
			backup(BAK);
		}
		continue;

	case 's':
		if (!globp && (peekc = getchar()) == '\n') {  /* w;q */
			setnoaddr();
			if (text_modified && !fflg)
				if (backup(FILE) == 0)
					error;  /* errmsg(10) */
			peekc = text_modified = 0;
			return;
		}
		if (peekc == 'a') {  /* sa - count before auto-save */
			setnoaddr();
			peekc = 0;
			while ((c = getchar()) == ' ' || c == '\t');
			peekc = c;
			if ((num = getnum()) >= 0)
				modcount = num;
			mods = 0;
			newline();
			continue;
		}
	casesub:
		setdot();
		nonzero();
		substitute(globp);
		text_modified++;
		continue;

	case 't':
#ifdef TABS
		if ((peekc = getchar()) == '=') {  /* t=c tab char */
			setnoaddr();
			peekc = 0;
			if ((c = getchar()) == '\\')
				c = getchar();
			if (c != '\n') {
				newline();
				tabc = c;
				continue;
			}
			tabc = 0;
			continue;
		} else if (peekc == ',') {
			setnoaddr();
			while ((c = getchar()) == ',')
				settab(getnum() - 1);
			if (c != '\n')
				newline();
			continue;
		} else if (peekc == '\n') {
			setnoaddr();
			listabs();
			peekc = 0;
			continue;
		}
#endif
	casecopy:
		move(1);
		text_modified++;
		continue;

#ifdef USE
	case 'u':
		setnoaddr();
		if (alt)
			errmsg(6);
		if ((peekc = getchar()) == 'p')
			peekc = 0;
		else
			eflg2++;
		if ((peekc = getchar()) == '\n' && altfile[0]) {
			peekc = 0;
			goto altname;
		}
		if ((c = getchar()) != ' ' && c != ',') {
			eflg2 = 0;
			errmsg(5);
		}
		while ((c = getchar()) == ' ');
		if (c == '\n') {
			eflg2 = 0;
			errmsg(7);
		}
		altfile[0] = c;
		num = 1;
		while ((altfile[num++] = getchar()) != '\n');
		altfile[--num] = 0;
	altname:
		if ((alt = open(altfile, 0)) < 0) {
			alt = 0;	/* this MUST be 0 */
			lastc = '\n';
			puts2("can't read ");
			puts(altfile);
			eflg2 = 0;
			error;
		}
		continue;
#endif

	case 'v':
		global(0);
		continue;

	case 'w':
	casewrite:
		if (globp)
			setdot();
		else
			setall();
		filename();
		if (dol == zero) {
			puts("[nothing written]");
			if (fflg && close(creat(file, 0644)) < 0) {
				puts2("can't create ");
				puts(file);
				error;
			}
			continue;
		}
		if (appflg)
			io = open(file, 1);
		if (!appflg || io < 0)
			io = creat(file, 0644);
		if (io < 0) {
			io = 0;
			puts2("can't create ");
			puts(file);
			error;
		}
		if (appflg)
			seek(io, 0, 2);	/* append onto file */
		putfile();
		exfile();
		if (addr1 == zero + 1 && addr2 == dol)
			text_modified = 0;
		continue;

#ifdef DEBUG
	case '@':	/* toggle debug flag */
		if (addr1 != addr2 || addr1 != zero)
			errmsg(8);
		newline();
		tflg = !tflg;
		continue;

	case '`':
		setnoaddr();
		newline();
		if (ndeleted == 0)
			errmsg(16);
		printf("deleted = %o  ndeleted = %d\n", deleted, ndeleted);
		{ register n, *bp, nl;
		    int tl;

		    tl = deleted;
		    bp = getblock(tl, READ);
		    nl = nleft / 2;
		    tl =& _1;
		    for (n = 0; n < ndeleted; n++) {
		    	printf("%7d: %6o\n", n + 1, *bp++);
		    	if (--nl == 0) {
			    bp = getblock(tl =+ _2, READ);
			    nl = nleft / 2;
		    	}
		    }
		}
		continue;
#endif

#ifdef XDEL
	case 'x':
		newline();
		r = undelete();
		if (prompt2)
			printf("%l line%s\n", r, (r == 1? "" : "s"));
		text_modified++;
		continue;
#endif

#ifdef YINT
	case 'y':
		if ((c = getchar()) == '+') {
			setdot();
			nonzero();
			newline();
			if (zero == dol)
				yplus = 0;
			else
				yplus = *addr2 | 01;
			yflg = 1;
		} else if (c == '-') {
			setnoaddr();
			newline();
			yflg = 0;
		} else {
			setnoaddr();
			peekc = c;
			newline();
			yflg = 1;
			yplus = 0;
		}
		continue;

#endif
	case '!':
#ifdef XED
	case '|':
#endif
		unix();
		continue;

	case EOF:
#ifdef USE
		if (!baseflg && alt) {
			close(alt);
			alt = 0;
			eflg2 = 0;
			continue;
		}
#endif
		return;

	}
	illcmd:
		errmsg(8);
	}
}

compile(aeof) {
	register eof, c;
	register char *ep;
	char *lastep;
	char bracket[NBRA], *bracketp;
	int nbra;
	int cclcnt;

	lastep = ep = expbuf;
	eof = aeof;
	bracketp = bracket;
	nbra = 0;
	if ((c = getchar()) == eof || c == '\n') {
		if (*ep == 0)
			errmsg(27);
		if (c == '\n')
			peekc = c;
		return;
	}
	circfl = 0;
	if (
#ifdef DUMB
	    !dumbf &&
#endif
			c == '^') {
		c = getchar();
		circfl++;
	}
	if (c == '*') {		/* if first = *, then quote it */
		*ep++ = CCHR;
		*ep++ = c;
		c = 0;
	}
	peekc = c;
	for (;;) {
		if (ep >= &expbuf[ESIZE - 2])
			break;
		c = getchar();
		if (c == eof || c == '\n') {
			*ep++ = CEOF;
			if (c == '\n') {
				pflag++;
				peekc = c;
			}
			return;
		}
		if (
#ifdef DUMB
		    !dumbf &&
#endif
			c != '*')
			lastep = ep;
#ifdef DUMB
		if (dumbf)
			goto defchar;
#endif
		switch (c) {
		case '\\':
			if ((c = getchar()) == '(') {
				if (nbra >= NBRA) {
					c = 44;
					goto cerror;
				}
				*bracketp++ = nbra;
				*ep++ = CBRA;
				*ep++ = nbra++;
				continue;
			}
			if (c == ')') {
				if (bracketp <= bracket) {
					c = 45;
					goto cerror;
				}
				*ep++ = CKET;
				*ep++ = *--bracketp;
				continue;
			}
			*ep++ = CCHR;
			if (c == '\n') {
				c = 46;
				goto cerror;
			}
			*ep++ = c;
			continue;

		case '.':
			*ep++ = CDOT;
			continue;

		case '*':
			if (*lastep == CBRA || *lastep == CKET) {
				c = 47;
				goto cerror;
			}
			*lastep =| STAR;
			continue;

		case '$':
			if ((peekc = getchar()) != eof && peekc != '\n')
				goto defchar;
			*ep++ = CDOL;
			continue;

		case '[':
			*ep++ = CCL;
			*ep++ = 0;
			cclcnt = 1;
			if ((c = getchar()) == '^') {
				c = getchar();
				ep[-2] = NCCL;
			}
			do {
				if (c == '\\')
					c = getchar();
				if (c == '\n') {
					c = 48;
					goto cerror;
				}
				if (c == EOF) {
					c = 25;
					goto cerror;
				}
				*ep++ = c;
				cclcnt++;
				if (ep >= &expbuf[ESIZE - 2]) {
					c = 49;
					goto cerror;
				}
			} while ((c = getchar()) != ']');
			lastep[1] = cclcnt;
			continue;

		case EOF:
			c = 25;
			goto cerror;

		defchar:
		default:
			*ep++ = CCHR;
			*ep++ = c;
		}
	}
	c = 50;
   cerror:
	expbuf[0] = 0;
	errmsg(c);
}

compsub() {
	register seof, c;
	register char *p;

	if ((seof = getchar()) == '\n')
		errmsg(40);
	compile(seof);
	p = rhsbuf;
	for (;;) {
		c = getchar();
		if (c == '\\')
			c = getchar() | 0200;
		if (c == '\n') {
			*p++ = 0;
			peekc = 0;
			pflag++;
			return(0);
		}
		if (c == seof)
			break;
		*p++ = c;
		if (p >= &rhsbuf[LBSIZE / 2 - 2])
			errmsg(41);
	}
	*p = 0;
	p = 0;
	if ((peekc = getchar()) == 'g') {	/* if in 'glob' */
		peekc = 0;
		p = 1;
	} else if ('0' <= peekc && peekc <= '9')
		if ((s_cnt = getnum()) < 0)
			s_cnt = 0;
	newline();
	return(p);
}

delete() {
	register *a1, *a2, *a3;

	a1 = addr1;
	a2 = addr2 + 1;
#ifdef XDEL
	if (!globp)
		saveline();
#endif
	a3 = dol;
	dol =- a2 - a1;
	do
		*a1++ = *a2++;
	while (a2 <= a3);
	a1 = addr1;
	if (a1 > dol)
		a1 = dol;
	dot = a1;
}

delexit(aretcode) {
	unlink(tfname);
	exit(aretcode);
}

dosub() {
	register char *lp, *sp, *rp;
	int c;

	lp = linebuf;
	sp = genbuf;
	rp = rhsbuf;
	while (lp < loc1)
		*sp++ = *lp++;
	while (c = *rp++) {
#ifdef DUMB
		if (!dumbf)
#endif
			if (c == '&') {
				sp = place(sp, loc1, loc2);
				continue;
			} else if (c < 0 && (c =& 0177) >= '1' &&
			    c < NBRA + '1') {
				sp = place(sp, braslist[c - '1'],
				    braelist[c - '1']);
				continue;
			}
		*sp++ = c & 0177;
		if (sp >= &genbuf[LBSIZE - 2])
			errmsg(42);
	}
	lp = loc2;
	loc2 = sp + linebuf - genbuf;
	while (*sp++ = *lp++)
		if (sp >= &genbuf[LBSIZE - 2])
			errmsg(43);
	lp = linebuf;
	sp = genbuf;
	while (*lp++ = *sp++);
}

#ifdef DEBUG
dump() {
	register *i, b, o;

	setdot();
	nonzero();
	newline();
	line_num = addr1 - zero;
	for (i = addr1; i <= addr2; i++) {
		b = (*i >> (8 - hugef)) & _3;
		o = (*i << (1 + hugef)) & _4;
		if (prompt1 && prompt2)
			printf(fmtlno, line_num - aflg);
		printf("%6o: %6o  %4d/%d\n", i, *i, b, o);
		line_num++;
	}
	dot = addr2;
}
#endif

echo(ch) {
#ifdef AGAIN
	static lastchar;
#endif
#ifdef USE
	if (eflg || alt && !eflg2)
#endif
#ifndef USE
	if (eflg)
#endif
		putchar(ch);
#ifdef AGAIN
	if (!agf && agp) {	/* save current command for "again" */
		if (agp >= &agbuf[GBSIZE - 2])
			agp = agbuf[0] = 0;
		else
			*agp++ = ch;
		if (ch == '\n' && lastchar != '\\')
			agp = *agp = 0;
	}
	lastchar = ch;
#endif
	return(ch);
}

errfunc() {
	register c;

	exfile();
/*	puts("?");	*/
	if (!seekf)
		seek(0, 0, 2);
	listf = pflag = 0;
	if (globp) {
		lastc = '\n';
		globp = 0;
	}
	peekc = lastc;
	skip_rest();
	reset();
}

errmsg(n) {
	listf = 0;
	parenf = 0;
	col = 0;
	if (n < 0) {
		puts2("******** ");
		n = -n;
	}
	if (0 <= n && n <= MAXERR)
		puts(errtext[n]);
	error;
}

execute(gf, addr)
int *addr;
{
	register char *p1, *p2, c;

	if (gf) {
		if (circfl)
			return(0);
		p1 = linebuf;
		p2 = genbuf;
		while (*p1++ = *p2++);
		locs = p1 = loc2;
	} else {
		if (addr == zero)
			return(0);
		p1 = getline(*addr);
		locs = 0;
	}
	p2 = expbuf;
	if (circfl) {
		loc1 = p1;
		return(advance(p1, p2));
	}
	/* fast check for first character */
	if (*p2 == CCHR) {
		c = p2[1];
		do {
			if (*p1 != c)
				continue;
			if (advance(p1, p2)) {
				loc1 = p1;
				return(1);
			}
		} while (*p1++);
		return(0);
	}
	/* regular algorithm */
	do {
		if (advance(p1, p2)) {
			loc1 = p1;
			return(1);
		}
	} while (*p1++);
	return(0);
}

exfile() {
#ifdef USE
	if (alt) {
		close(alt);
		alt = 0;
		eflg2 = 0;
	}
#endif
#ifdef HELP
	if (doc) {
		close(doc);
		doc = 0;
	}
#endif
	if (io) {
		close(io);
		io = 0;
	}
}

#ifdef TABS
exp() {
	register *a1, *p, nl;
	int n;

	setdot();
	nonzero();
	n = 0;
	for (a1 = addr1; a1 <= addr2; a1++) {
		getline(*a1);
		if (expand()) {
			n++;
			p = *a1;
			*a1 = putline();
			savemark(p, *a1);
		}
	}
	dot = addr2;
	return(n);
}

expand() {
	register char *p2, c;
	register n;
	char *p1;
	int cnt, tabflg, i;

	p1 = linebuf;
	p2 = genbuf;
	while (*p2++ = *p1++);
	p2 = linebuf;
	p1 = genbuf;
	cnt = col = n = 0;
	while (c = *p1++) {
		if (c == tabc && col < maxtab) {
			n = col;
			c = tabfill;
			if (c == '\t') {
				n = col;
				tabflg = 0;
				while (n < maxtab) {
					if (++n % 8 == 0) {
						*p2++ = c;
						tabflg++;
						if (p2 >=
						    &linebuf[LBSIZE - 2]
						    || n >= TABS * 16)
							goto experr;
					}
					if (tabs[n / 16] >> n
					    % 16 & 01)
						if (*p1 == tabc &&
						    n < maxtab) {
							p1++;
							cnt++;
						} else
							break;
				}
				i = n;
				if (tabflg)
					n =% 8;
				else
					n =- col;
				col = i;
				while (n--) {
					*p2++ = ' ';
					if (p2 >= &linebuf[LBSIZE - 2])
						goto experr;
				}
				cnt++;
				continue;
			}
			do {
				*p2++ = c;
				if (p2 >= &linebuf[LBSIZE - 2]
				    || n >= TABS * 16)
					goto experr;
				n++;
			} while (!(tabs[n / 16] >> n % 16 & 01));
			col = n;
			cnt++;
			continue;
		} else if ('\0' < c && c < ' ') {
			switch (c) {
			case '\b':
				col =- 2;
				break;
			case '\t':
				col =+ 7 - col % 8;
				break;
			case '\r':
				col = -1;
				break;
			default:
				col--;
			}
		}
		if (c) {
			*p2++ = c;
			col++;
			if (p2 >= &linebuf[LBSIZE - 2] ||
			    col >= TABS * 16)
	experr:			errmsg(12);
		}
	}
	*p2++ = 0;
	return(cnt);
}
#endif

filename() {
	register char *p1, *p2;
	register c;

	appflg = 0;
	c = getchar();
	if (c == '\n' || c == EOF) {
    noname:
		p1 = savedfile;
		if (*p1 == 0)
			goto nofil;
		p2 = file;
		while (*p2++ = *p1++);
		return;
	}
	if (c != ' ' && c != ',' && c != '>')
    nofil:	errmsg(5);
	if (c == ',')
		c = getchar();
	while (c == ' ')
		c = getchar();
	if (c == '>') {
		while ((c = getchar()) == '>');
		appflg++;
		if (c == '\n')
			goto noname;
		while (c == ' ')
			c = getchar();
	}
	if (c == '\n')
		errmsg(7);
	p1 = file;
	do {
		*p1++ = c;
		if (p1 >= &file[FNSIZE - 2])
			errmsg(11);
	} while ((c = getchar()) != '\n');
	*p1++ = 0;
	if (savedfile[0] == 0) {
		p1 = savedfile;
		p2 = file;
		while (*p1++ = *p2++);
	}
}

flush_buf() {
	register n;

	if (n = linp - line) {
		write(fout, line, n);
#ifdef CMDS
		if (cmd && fout == 1)
			write(cmd, line, n);
#endif
		linp = line;
	}
}

gask(a1)
int *a1;
{
	register char c;

	col = 0;
	if (prompt1) {
		c = listf;
		listf = 0;
		line_num = a1 - zero;
		printf(fmtlno, line_num - aflg);
		listf = c;
		col = 8;
	}
	puts(linebuf);
	puts2("Ok? ");
	flush_buf();
	if ((c = getchar()) == EOF) {
		putchar('\n');
		return(0);
	}
	if (c == '\n')
		return(1);
	skip_rest();
	return(c == 'n'? 0 : 1);
}

getblock(atl, iof) {
	extern read(), write();
	register bno, off;

	bno = (atl >> (8 - hugef)) & _3;
	off = (atl << (1 + hugef)) & _4;
	if (bno >= _5)
		errmsg(34);
	nleft = 512 - off;
	if (bno == iblock && !badf) {
		ichanged =| iof;
		return(ibuff + off);
	}
	if (bno == oblock)
		return(obuff + off);
	if (iof == READ) {
		if (ichanged) {
			blkio(iblock, ibuff, write);
			ichanged = 0;
		}
		iblock = bno;
		blkio(bno, ibuff, read);
		return(ibuff + off);
	}
	if (oblock >= 0)
		blkio(oblock, obuff, write);
	oblock = bno;
	return(obuff + off);
}

getchar() {
	flush_buf();
	if (lastc = peekc) {
		peekc = 0;
		return(lastc);
	}
	if (globp) {
		if (lastc = *globp++) {
			if (globf2 && *globp == 0)
				globp = globf2 = 0;
			return(lastc);
		}
		globp = 0;
		return(EOF);
	}
#ifdef AGAIN
	if (agf) {	/* "again" */
		if (lastc = *agp++)
			return(lastc);
		agf = agp = 0;
	}
#endif
	reading++;
#ifdef USE
	if (read(alt, &lastc, 1) <= 0) {
#endif
#ifndef USE
	if (read(0, &lastc, 1) <= 0) {
#endif
		reading = 0;
		return(lastc = EOF);
	}
	reading = 0;
	lastc =& 0177;
#ifdef APLMAP
	if (aplmap && lastc > ' ')
		lastc = map_ascii[lastc - (' ' + 1)];
#endif
#ifdef EOL
	if (lastc == '\n')
		prompt3 = 1;
	if (eol && lastc == eol) {
		lastc = '\n';
		prompt3 = 0;
	}
#endif
#ifdef CMDS
	if (cmd)
		write(cmd, &lastc, 1);
#endif
	return(echo(lastc));
}

getcopy() {
	if (addr1 > addr2)
		return(EOF);
	getline(*addr1++);
	return(0);
}

getfile() {
	register c;
	register char *lp, *fp;

	lp = linebuf;
	fp = nextip;
	do {
		if (--ninbuf < 0) {
			if ((ninbuf = read(io,genbuf,LBSIZE) - 1) < 0)
				return(EOF);
			fp = genbuf;
		}
		if (lp >= &linebuf[LBSIZE - 2]) {
			puts("line in file too long");
			flush_buf();
			*lp++ = '>';
			*lp = 0;
			nextip = fp;
			return(0);
			break;
		}
		if ((*lp++ = c = *fp++ & 0177) == 0) {
			lp--;
			continue;
		}
	} while (c != '\n');
	*--lp = 0;
	nextip = fp;
	return(0);
}

getline(tl) {
	register char *bp, *lp;
	register nl;

#ifdef DEBUG
	if (tflg)
		printf("getline:\t%o\n", tl);
#endif
	lp = linebuf;
	bp = getblock(tl, READ);
	nl = nleft;
	tl =& _1;
	while (*lp++ = *bp++)
		if (--nl == 0) {
			bp = getblock(tl =+ _2, READ);
			nl = nleft;
		}
	return(linebuf);
}

getnum() {
	register n;
	register char c;

	n = 0;
	while ('0' <= (c = getchar()) && c <= '9')
		n = n * 10 + c - '0';
	peekc = c;
	return(n);
}

getsub() {
	register char *p1, *p2;

	p1 = linebuf;
	if ((p2 = linebp) == 0)
		return(EOF);
	while (*p1++ = *p2++);
	linebp = 0;
	return(0);
}

gettty(single) {
	register c, gf;
	register char *p;
#ifdef TABS
	int tabf;

	tabf = 0;
#endif
	p = linebuf;
	gf = globp;
#ifdef EOL
	if (prompt1 && prompt2 && prompt3 && !single) {
#endif
#ifndef EOL
	if (prompt1 && prompt2 && !single) {
#endif
		printf(fmtlno, line_num - aflg);
		flush_buf();
	}
	line_num++;
	while ((c = getchar()) != '\n') {
		if (c == EOF) {
			if (gf)
				peekc = c;
			else if (prompt2 && (prompt1 || !zflg))
				putchar('\n');
			return(c);
		}
		if ((c =& 0177) == 0)
			continue;
#ifdef TABS
		if (tabc && c == tabc)
			tabf++;
#endif
		*p++ = c;
		if (p >= &linebuf[LBSIZE-2])
			errmsg(30);
	}
	*p++ = 0;
#ifdef TABS
	if (tabf)
		expand();
#endif
	if (!single && linebuf[0] == '.' && linebuf[1] == 0)
		return(EOF);
	return(0);
}

global(k) {
	register char *gp;
	register c;
	register *a1;
	char globuf[GBSIZE];
	int globpf;

	if (hugef)
		errmsg(10);
	if (globp)
		errmsg(37);
	setall();
	nonzero();
	if ((c = getchar()) == '\n')
		peekc = c;
	compile(c);
	gaskf = 0;
	if ((peekc = getchar()) == 'v') {
		gaskf++;
		peekc = 0;
	}
	globpf = pflag;
	if (peekc == '\n')
		globpf = 0;
	pflag = 0;
	gp = globuf;
	if ((peekc = getchar()) == '\n' || peekc == EOF) {
		*gp++ = 'p';
		peekc = 0;
	} else
		while ((c = getchar()) != '\n') {
			if (c == '\\') {
				c = getchar();
				if (c != '\n')
					*gp++ = '\\';
			}
			if (c == EOF)
				errmsg(25);
			*gp++ = c;
			if (gp >= &globuf[GBSIZE-2])
				errmsg(38);
		}
	*gp++ = '\n';
	*gp++ = 0;
	for (a1 = zero + 1; a1 <= dol; a1++) {
		*a1 =& ~01;
		s_tmp = 0;
		if (a1 >= addr1 && a1 <= addr2 && execute(0, a1) == k)
			if (!gaskf || gask(a1))
				*a1 =| 01;
	}
	for (a1 = zero + 1; a1 <= dol; a1++) {
		if (*a1 & 01) {
			*a1 =& ~01;
			dot = a1;
			globp = globuf;
			pflag = globpf;
			commands(1);
			a1 = zero;
		}
	}
}

hangup() {
	register f;

	signal(SIGHUP, hangup);
	f = fout;
	if (fout != 1) {
		flush_buf();
		fout = 1;
	}
	puts2("\nHANGUP!\n");
	fout = f;
	exfile();
	if (reading) {
		backup(HUP);
		delexit(1);
	}
	hupflag++;
}

header() {
	register colnum, number;
	register char c;
	int lf;

	number = 0;
	if ((peekc = c = getchar()) != '\n') {
		while ((c = getchar()) == ' ' || c == '\t');
		peekc = c;
		number = getnum();
	}
	if (!number)
		number = ccount - (prompt1? 8 : 0);
	newline();
	dot = addr2;
	lf = listf;
	listf = 0;
	if (prompt1)
		putchar('\t');
	for (colnum = 0; colnum < number / 10; colnum++)
		printf("%10d", colnum + 1);
	putchar('\n');
	if (prompt1)
		putchar('\t');
	for (colnum = 1; colnum <= number; colnum++)
#ifdef TABS
		if (colnum < TABS * 16 && (tabs[(colnum - 1) / 16] >>
		    (colnum - 1) % 16 & 01))
			putchar('-');
		else
#endif
			putchar(colnum % 10 + '0');
	putchar('\n');
	listf = lf;
}

init() {
	register char *p;
	register pid, n;

	close(tfile);
	exfile();
	tline = 0;
#ifdef XDEL
	deleted = ndeleted = 0;
#endif
	num_reads = 0;
	text_modified = 0;
#ifdef YINT
	yplus = 0;
#endif
	iblock = -1;
	oblock = -1;
	badf = 0;
	ichanged = 0;
	close(creat(tfname, 0600));
	if ((tfile = open(tfname, 2)) < 0)
		errmsg(-36);
	brk(fendcore);
	addr1 = addr2 = dot = dotdot = lastdot = zero = dol = fendcore;
	endcore = fendcore - 2;
	for (n = 0; n < 26; n++) {  /* kill marks */
		names[n] = 0;
#ifdef EXTMARK
		names2[n] = 0;
#endif
	}
	p = globp;
	globp = 0;
	if (chktmp()) {
		if ((io = open(file, 0)) < 0) {
			lastc = '\n';
			puts2("can't read ");
			puts(file);
			io = 0;
		} else {
			n = append(getfile, addr1, 0, 0);
			if (prompt2)
				printf("%l line%s\n", n, (n == 1?
				    "" : "s"));
			if (n) {
				num_reads++;
				text_modified++;
			}
			exfile();
		}
		return(1);
	}
	globp = p;
	return(0);
}

istty(fd) {
	int buf[18];

	if (pipef || fstat(fd, buf) != -1 && (buf[2]&FTYPE) == FCHAR) {
		gtty(fd, buf);
		zflg = (buf[2] & BS1) == 0? 1 : 0;
		return(1);
	}
	return(0);
}

#ifdef XED
join() {
	register *a1;
	register char *p1, *p2;

	setdot();
	if (addr1 == addr2)
		addr1 = addr2 - 1;
	nonzero();

	p2 = genbuf;
	a1 = addr1;

	while (a1 <= addr2) {
		p1 = getline(*a1++);
		while (*p2++ = *p1++)
			if (p2 >= &genbuf[LBSIZE - 2])
				errmsg(30);
		--p2;
	}
	*p2++ = '\0';

	p1 = genbuf;
	p2 = linebuf;
	while (*p2++ = *p1++);

	*addr1++ = putline();
	delete();
	dot = addr1 - 1;
	if (dot > dol)
		dot = dol;
}
#endif

#ifdef TABS
listabs() {
	register n;

	if (maxtab < 0)
		errmsg(9);
	n = next_tab(-1);
	printf("tabs: %d", n + 1);
	while ((n = next_tab(n)) >= 0)
		printf(",%d", n + 1);
	putchar('\n');
}
#endif

mail() {
	delexit(1);
}

max(a, b) {
	return(a > b? a : b);
}

min(a, b) {
	return(a > b? b : a);
}

move(cflag) {
	register *adt, *ad1, *ad2;
	int getcopy();

	setdot();
	nonzero();
	if ((adt = address()) == 0)
		errmsg(31);
	ad1 = addr1;
	ad2 = addr2;
	newline();
	if (cflag) {
		ad1 = dol;
		append(getcopy, ad1++, 0, 0);
		ad2 = dol;
	}
	ad2++;
	if (adt < ad1) {
		dot = adt + (ad2-ad1);
		if (++adt == ad1)
			return;
		reverse(adt, ad1);
		reverse(ad1, ad2);
		reverse(adt, ad2);
	} else if (adt >= ad2) {
		dot = adt++;
		reverse(ad1, ad2);
		reverse(ad2, adt);
		reverse(ad1, adt);
	} else
		errmsg(28);
}

newline() {
	register c;

	if ((c = getchar()) == '\n')
		return;
	if (c == 'p' || c == 'l' || c == 'b') {
		pflag++;
		if (c == 'l')
			listf++;
		if (c == 'b')
			parenf++;
		if (getchar() == '\n')
			return;
	}
	errmsg(25);
}

#ifdef TABS
next_tab(acol) {
	register n;

	if ((n = acol) < maxtab) {
		do {
			n++;
		} while (!(tabs[n / 16] >> n % 16 & 01));
		return(n);
	}
	return(-1);
}
#endif

nonzero() {
	if (addr1 <= zero)
		errmsg(23);
	if (addr2 > dol)
		errmsg(24);
}

#ifdef PIPE
oil_spilled() {
	if (!iflg)
		signal(SIGINT, 1);
#ifdef PIPE
	signal(SIGPIP, 1);
	piperr++;
#endif
}
#endif

onintr() {
	register char *p1, *p2;
	register *a1;

	signal(SIGINT, onintr);
#ifdef EOL
	prompt3 = 1;
#endif
	if (fout != 1) {
		close(fout);
		fout = 1;
	}
#ifdef YINT
	if (!globp && reading && yflg) {
		globp = ".:\n";
		if (yplus) {
			for (a1 = zero + 1; a1 <= dol; a1++)
				if (yplus == (*a1 | 01)) {
					dot = a1;
					break;
				} else if (a1 == dol)
					yplus = 0;
		} else
			globp++;
		peekc = 0;
		putchar('\n');
		commands(1);
		reset();
	}
#endif
	if (iflg) {
		signal(SIGINT, 1);
		backup(INT);
		delexit(1);
	}
	putchar(lastc = '\n');
	errmsg(29);
}

#ifdef PAGE
page() {
	register cl, n;
	register char *p;
	char *pp;
	int *a, *b, l;

	a = addr1;
	if (addr2 != addr1)
		b = addr2;
	else
		b = dol;
	parenc[0] = 0;
	parenc[1] = 0;
	parenc[2] = 0;
	for (n = pcount; n >= 0 && a <= b; n--) {
		pp = p = getline(*a);
		cl = prompt1? 8 : 0;
		l = 0;
		while (*p) {
			if (*p < ' ')
				switch (*p) {
				case '\b':
					if (listf)
						cl++;
					else {
						cl--;
						if (aflg && !parenf)
							l++;
					}
					break;
				case '\t':
					if (listf)
						cl++;
					else
						cl =+ 8 - cl % 8;
					break;
				case '\r':
					if (listf)
						cl =+ 3;
					else
						cl = 0;
					break;
				case '\7':
				case '\13':
				case '\14':
					if (listf)
						cl =+ 3;
					break;
				default:
					if (listf)
						cl =+ 3;
					else if (zflg)
						cl =+ 2;
				}
			else {
				if (*p == '\177') {
					if (listf)
						cl =+ 4;
					else if (zflg)
						cl++;
				} else
					cl++;
			}
			if (parenf && paren(*p))
				l++;
			if (cl < 0)
				cl = 0;
			else if (cl > ccount) {
				cl = 0;
				if (--n < 0)
					goto done;
			}
			p++;
		}
		if (l)
			if (--n < 0)
				goto done;
		col = 0;
		if (prompt1) {
			l = listf;
			listf = 0;
			line_num = a - zero;
			printf(fmtlno, line_num - aflg);
			col = 8;
			listf = l;
		}
		puts(pp);
		a++;
	}
    done:
	dot = a - 1;
}
#endif

paren(ac) {
	switch (ac) {
	case '(':	return(1);
	case '[':	return(2);
	case '{':	return(3);
	case ')':	return(-1);
	case ']':	return(-2);
	case '}':	return(-3);
	}
	return(0);
}

place(asp, al1, al2) {
	register char *sp, *l1, *l2;

	sp = asp;
	l1 = al1;
	l2 = al2;
	while (l1 < l2) {
		*sp++ = *l1++;
		if (sp >= &genbuf[LBSIZE - 2])
			errmsg(42);
	}
	return(sp);
}

printmarks() {
	register *a1, c;

	for (c = 'a' - 'a'; c <= 'z' - 'a'; c++)
	    for (a1 = zero + 1; a1 <= dol; a1++)
		if (names[c] == (*a1 | 01)) {
		    printf("%c=%l", c + 'a', a1 - zero);
#ifdef EXTMARK
		    if (names2[c]) {
			for (a1 = zero + 1; a1 <= dol; a1++)
			    if (names2[c] == (*a1 | 01)) {
				printf(",%l", a1 - zero);
				break;
			    }
		    }
#endif
		    putchar(' ');
		    break;
		}
	putchar('\n');
}

putchar(ac) {
	register char *lp;
	register c;

	lp = linp;
	c = ac;
#ifdef APLMAP
	if (aplmap && fout == 1 && c > ' ')
		c = map_apl[c - (' ' + 1)];
#endif
	if (listf) {
		if (++col >= ccount) {
			col = 1;
			if (c != '\n') {
				*lp++ = '\\';
				*lp++ = '\n';
			}
		}
		if (c == '\t') {
			c = '>';
			goto esc;
		}
		if (c == '\b') {
			c = '<';
		esc:
			*lp++ = '-';
			*lp++ = '\b';
			*lp++ = c;
			goto out;
		}
		if (c < ' ' && c !=  '\n') {
			*lp++ = '\\';
			*lp++ = (c >> 3) + '0';
			*lp++ = (c & 07) + '0';
			col =+ 2;
			goto out;
		}
		if (c == '\177') {
			*lp++ = '\\';
			*lp++ = '1';
			*lp++ = '7';
			*lp++ = '7';
			col =+ 3;
			goto out;
		}
	}
	*lp++ = c;
out:
	if (lp >= &line[LBSIZE]) {
		linp = line;
		write(fout, line, lp - line);
#ifdef CMDS
		if (cmd && fout == 1)
			write(cmd, line, lp - line);
#endif
		return;
	}
	linp = lp;
}

putfile() {
	int *a1;
	register char *fp, *lp;
	register nib;

	nib = 512;
	fp = genbuf;
	a1 = addr1;
	do {
		lp = getline(*a1++);
		for (;;) {
			if (--nib < 0) {
				wte(io, genbuf, fp - genbuf);
				nib = 511;
				fp = genbuf;
			}
			if ((*fp++ = *lp++) == 0) {
				fp[-1] = '\n';
				break;
			}
		}
	} while (a1 <= addr2);
	wte(io, genbuf, fp - genbuf);
}

putline() {
	register char *bp, *lp;
	register nl;
	int tl;

	lp = linebuf;
	tl = tline;
#ifdef DEBUG
	if (tflg)
		printf("putline:\t%o\n", tl);
#endif
	bp = getblock(tl, WRITE);
	nl = nleft;
	tl =& _1;
	while (*bp = *lp++) {
		if (*bp++ == '\n') {
			*--bp = 0;
			linebp = lp;
			break;
		}
		if (--nl == 0) {
			bp = getblock(tl =+ _2, WRITE);
			nl = nleft;
		}
	}
	nl = tline;
	tline =+ (((lp - linebuf) + 03) >> (1 + hugef)) & _6;
	return(nl);
}

puts(as) {
	register char *sp;
	register ovp, n;

	sp = as;
	ovp = 0;
	while (*sp) {
		if (aflg && !parenf && !listf && *sp == '\b') {
			ovp++;
			sp =+ 2;
			continue;
		}
		if (parenf && !listf && paren(*sp))
			ovp++;
		putchar(*sp++);
	}
	sp = as;
	if (aflg && !parenf && !listf && ovp) {
		putchar('\n');
		if (prompt1)
			putchar('\t');
		for (; *sp; sp++)
			if (*sp == '\t')
				putchar('\t');
			else if (*sp >= ' ')
				putchar(' ');
			else if (*sp == '\b')
				putchar(*++sp);
	}
	if (parenf && !listf && ovp) {
		putchar('\n');
		if (prompt1)
			putchar('\t');
		for (; *sp; sp++)
			if (ovp = n = paren(*sp)) {
				if (n < 0) {
					n = -n - 1;
					putchar('0' + parenc[n]);
					--parenc[n];
				} else
					parenc[--n]++;
				if (parenc[n] > 9)
					parenc[n] =- 10;
				if (parenc[n] < 0)
					parenc[n] =+ 10;
				if (ovp > 0)
					putchar('0' + parenc[n]);
			} else if (*sp == '\t' || *sp >= ' ')
				putchar(*sp == '\t'? '\t' : ' ');
	}
	putchar('\n');
}

puts2(as) {
	register char *sp;

	sp = as;
	while (*sp)
		putchar(*sp++);
	flush_buf();
}

reverse(aa1, aa2) {
	register *a1, *a2, t;

	a1 = aa1;
	a2 = aa2;
	for (;;) {
		t = *--a2;
		if (a2 <= a1)
			return;
		*a2 = *a1;
		*a1++ = t;
	}
}

#ifdef XDEL
saveline() {
	register *bp;
	register *a1, *a2;
	int nl, tl;

	a1 = addr1;
	a2 = addr2;
	ndeleted = a2 - a1 + 1;
	tl = tline;
#ifdef DEBUG
	if (tflg)
		printf("saveline:\t%o\n", tl);
#endif
	bp = getblock(tl, WRITE);
	nl = nleft / 2;
	tl =& _1;
	while (a1 <= a2) {
		*bp++ = *a1++;
		if (--nl == 0) {
			bp = getblock(tl =+ _2, WRITE);
			nl = nleft / 2;
		}
	}
	deleted = tline;
	tline =+ ((a2 - addr1 + 1) << (1 - hugef)) & _6;
}
#endif

savemark(p1, p2) {
	register n;

	/* save "marks" on lines so marked */
	for (n = 0; n <= 'z' - 'a'; n++) {
		if (names[n] == (p1 | 01))
			names[n] = p2 | 01;
#ifdef EXTMARK
		if (names2[n] && names2[n] == (p1 | 01))
			names2[n] = p2 | 01;
#endif
	}
#ifdef YINT
	/* don't lose "y+" line either */
	if (yplus == (p1 | 01))
		yplus = p2 | 01;
#endif
}

setall() {
	if (addr2 == 0) {
		addr1 = dol == zero? zero : zero + 1;
		addr2 = dol;
	}
	setdot();
}

setdot() {
	if (addr2 == 0)
		addr1 = addr2 = dot;
	if (addr1 > addr2)
		errmsg(21);
}

setnoaddr() {
	if (addr2)
		errmsg(22);
}

#ifdef TABS
settab(acs) {
	register *p;

	if (acs < 0) {
		for (p = tabs; p < &tabs[TABS]; p++)
			*p = 0;
		maxtab = -1;
	} else if (acs < TABS * 16) {
		tabs[acs / 16] =| 01 << acs % 16;
		if (acs > maxtab)
			maxtab = acs;
	}
}
#endif

shutdown() {
	register f;

	signal(SIGSHU, shutdown);
	f = fout;
	if (fout != 1) {
		flush_buf();
		fout = 1;
	}
	puts2("\nSystem shutdown...\n");
	fout = f;
	exfile();
	if (reading) {
		backup(SHU);
		delexit(1);
	}
	shutflg++;
}

skip_rest() {
	register c;

	while ((c = getchar()) != EOF && c != '\n');
}

substitute(inglob) {
	register *a1, nl, p;
	int gsubf;
	int getsub();

	gsubf = compsub();		/* 0 or 1 depending on 'g' */
	for (a1 = addr1; a1 <= addr2; a1++) {
		s_tmp = s_cnt;
		if (execute(0, a1) == 0)
			continue;
		inglob =| 01;
		dosub();
		if (gsubf)
			while (*loc2) {
				if (execute(1) == 0)
					break;
				dosub();
			}
		p = *a1;
		*a1 = putline();
		savemark(p, *a1);
		nl = append(getsub, a1, 0, 0);
		a1 =+ nl;
		addr2 =+ nl;
	}
	if (inglob == 0)
		errmsg(39);
}

#ifdef XED
tack(aeof, aflag)
char aeof;
{
	register n;
	register char *p1, *p2;
	int *a;
	char c;

	nonzero();
	setdot();
	p1 = rhsbuf;
	while ((c = getchar()) != aeof && c != '\n') {
		if (c == '\\')
			if ((c = getchar()) == '\n')
				break;
		if (c == EOF)
			return(0);
		*p1++ = c;
		if (p1 >= &rhsbuf[LBSIZE / 2 - 2])
			errmsg(42);
	}
	if (*rhsbuf == 0)
		errmsg(27);
	if (c == '\n')
		pflag++;
	else
		newline();
	if (p1 != rhsbuf)
		*p1 = 0;
	else
		while (*p1)
			p1++;
	n = p1 - rhsbuf;
	for (a = addr1; a <= addr2; a++) {
		getline(*a);
		for (p2 = linebuf; *p2; p2++);
		if (p2 + n >= &linebuf[LBSIZE / 2 - 2])
			errmsg(30);
		if (aflag) {	/* $ */
			p1 = rhsbuf;
			while (*p2++ = *p1++);
		} else {	/* ^ */
			p1 = p2 + n;
			*p1 = *p2;
			while (p2 > linebuf)
				*--p1 = *--p2;
			p1 = linebuf;
			p2 = rhsbuf;
			while (*p2)
				*p1++ = *p2++;
		}
		p2 = *a;
		*a = putline();
		savemark(p2, *a);
	}
	dot = addr2;
	return(addr2 - addr1 + 1);
}
#endif

#ifdef XDEL
undelete() {
	register *a1, *a2, *bp;
	int tl, nl;
	struct { int integer; };

	if ((tl = deleted) == 0)
		errmsg(16);
#ifdef DEBUG
	if (tflg)
		printf("undelete:\t%o\n", tl);
#endif
	setdot();
	a1 = dol + 1;
	a2 = a1 + ndeleted;
	bp = addr2 + 1;
	if (dol + ndeleted > endcore) {
		num = ((ndeleted * 2) + 1023) & ~01777;
		if (sbrk(num) == -1)
			errmsg(33);
		endcore.integer =+ num;
	}
	while (a1 > bp)
		*--a2 = *--a1;
	dol =+ ndeleted;
	a1 = addr2 + 1;
	a2 = a1 + ndeleted;
	bp = getblock(tl, READ);
	nl = nleft / 2;
	tl =& _1;
	while (a1 < a2) {
		*a1++ = *bp++;
		if (--nl == 0) {
			bp = getblock(tl =+ _2, READ);
			nl = nleft / 2;
		}
	}
	dot = a2 - 1;
	return(ndeleted);
}
#endif

unix() {
#ifdef PIPE
	int oil_spilled();
#endif
	register pid, wpid;
	register *a;
	int savint, p[2];
	char c;

#ifdef PIPE
	if (addr2)
		nonzero();
	piperr = 0;
	if (!iflg)
		if (addr2)
			savint = signal(SIGINT, oil_spilled);
		else
#endif
			savint = signal(SIGINT, 1);
#ifdef PIPE
	if (addr2 && pipe(p) < 0) {
		if (!iflg)
			signal(SIGINT, savint);
		errmsg(15);
	}
#endif
	if ((pid = fork()) < 0) {
#ifdef PIPE
		if (addr2) {
			close(p[0]);
			close(p[1]);
		}
#endif
		if (!iflg)
			signal(SIGINT, savint);
		errmsg(14);
	}
	if (pid == 0) {
		if (!iflg) {
			signal(SIGHUP, 0);
			signal(SIGINT, 0);
			signal(SIGQIT, 0);
		}
#ifdef PIPE
		if (addr2) {
			close(0);
			dup(p[0]);
			close(p[0]);
			close(p[1]);
		}
#endif
		execl("/bin/sh", "e!", "-t", 0);
		puts("No shell!");
		flush_buf();
		exit(1);
	}
#ifdef CMDS
	if (cmd && !addr2)
		write(cmd, "<UNIX>\n", 7);
#endif
#ifdef PIPE
	if (addr2) {
		signal(SIGPIP, oil_spilled);
		close(p[0]);
		a = addr1;
		fout = p[1];
		while ((c = getchar()) != EOF && c != '\n')
			putchar(c);
		putchar('\n');
		do {
			if (prompt1) {
				line_num = a - zero;
				printf(fmtlno, line_num - aflg);
			}
			puts(getline(*a++));
		} while (a <= addr2 && !piperr);
		flush_buf();
		fout = 1;
		close(p[1]);
		if (!iflg)
			signal(SIGINT, 1);
	}
#endif
	while ((wpid = wait()) != pid && wpid != -1);
#ifdef CMDS
	if (cmd)
		seek(cmd, 0, 2);
#endif
#ifdef PIPE
	if (addr2) {
		if (piperr)
			puts("broken pipe");
		signal(SIGPIP, 0);
	}
#endif
	if (!iflg)
		signal(SIGINT, savint);
	puts("!");
#ifdef CLEAR
	istty(1);
#endif
}

wte(fd, buf, len)
char *buf;
{
	if (write(fd, buf, len) != len)
		errmsg(-32);
}
