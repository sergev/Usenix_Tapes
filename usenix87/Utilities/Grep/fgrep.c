/*
#define DEBUG	/* compile code to display tree structure */
/*
 * fgrep -- print all lines containing any of a set of keywords
 *
 *	status returns:
 *		0 - ok, and some matches
 *		1 - ok, but no matches
 *		2 - some error
 */

static char sccsid[] = "@(#)fgrep.c	2.0";

/*
 * fgrep employs a generalisation of the Knuth-Morris-Pratt string matching
 * algorithm (ref: Baase, S: Computer Algorithms, 1978), which builds a
 * "flowchart" tree representing the patterns to be searched for. The tree
 * is constructed so that possible character matches in a particular position
 * in the composite pattern are linked together (using the "link" field of
 * struct words), and share a common parent tree. For example, the patterns
 * "roar" and "road" are represented by three nodes containing "r", "o" and
 * "a", linked by success pointers ("succ"), and two nodes "r" and "d"
 * joined by "link" pointers. The succ pointer of "a" points to the head
 * of this list. All unused pointers are NULL.
 * 
 * The feature of this algorithm is that it is never necessary to back up
 * in the subject when matching fails. If a mismatch occurs, a failure link
 * is followed to an earlier point in the pattern which is known still to be
 * matched. For example, if the fourth character of "poppy" fails, we
 * should continue from the second character, as the first remains matched.
 * Thus "popoppy" will be matched with 8 comparisons.
 *
 * For exact matching (-x), a newline character is appended to each pattern.
 * No failure links are computed, as any mismatch causes total failure.
 *
 * Exception conditions - pattern building:
 *  1.	If -x is not specified, the null pattern is represented by a NULL tree
 *	and always matches; otherwise it is stored as a single newline.
 *  2.	If a pattern is an initial substring of another, the longer string
 *	is ignored (since the shorter matches both).
 *  3.	If a pattern is a non-initial substring of another, the longer string
 *	is truncated at the end of the common substring. Thus "pattern" and
 *	"at" become "pat" and "at". The longer string should really be deleted,
 *	but this is a non-trivial task as it may share a common prefix with
 *	some other pattern (such as "pad"). The redundant string is harmless.
 *  4.	Patterns longer than BUFSIZ are truncated.
 *
 * Exception conditions - matching:
 *  1.	The length of lines able to be matched is unlimited, but only the first
 *	BUFSIZ characters are retained for printing.
 *  2.	If the last non-empty line in a file does not contain a newline,
 * 	one is added if the line is printed. It is not possible to exactly
 *	match such a line with the -x option.
 *
 * Author:	Geoff Whale
 * 		University of New South Wales, Sydney
 *		May, 1984.
 */



#include <stdio.h>
#include <ctype.h>

#define INCRSIZ 64
#define QSIZE 128

#if u370
#define BLKSIZE 4096
#else
#define BLKSIZE BUFSIZ
#endif

struct words {
	char 	key;
	struct	words *succ;
	struct	words *link;
	struct	words *fail;
} w[INCRSIZ],		/* first block, others obtained through walloc() */
*tree = NULL,		/* root of the KMP flowchart tree */
*smax = w,		/* last element used */
*slim = &w[INCRSIZ];	/* limit for smax, then malloc some more */

struct qpage {
	struct words *queue[QSIZE];
	struct qpage *nextpage;
};

int	bflag, cflag, lflag, fflag, nflag, sflag, vflag, xflag, yflag, eflag;
#ifdef DEBUG
char *usage = "[ -bcdlnsvxy ] [ -f stringfile ] [ -e ] strings [ files... ]";
char *optlist = "bcde:f:lmnsvxy";
int  dflag;
#else
char *usage = "[ -bclnsvxy ] [ -f stringfile ] [ -e ] strings [ files... ]";
char *optlist = "bce:f:lmnsvxy";
#endif DEBUG

int	numfiles;		/* prepend filename to matches if >1 */
int	anymatches, notfound;	/* for exit status */
FILE	*stringfile = NULL;
char	*argptr;
extern	char *optarg;
extern	int optind;
extern char *malloc(), *calloc();

#define match(ec,lc)	(ec == lc || (yflag && isupper(lc) && ec == lc + 'a' - 'A'))

main(argc, argv)
char **argv;
{
	register int  c;
	int  errflg = 0;

	while(( c = getopt(argc, argv, optlist)) != EOF)
	    switch(c) {

		case 'b':	/* block number prepended */
			bflag++;
			continue;

		case 'c':	/* count of lines only */
			cflag++;
			continue;

#ifdef DEBUG
		case 'd':	/* debug/display option */
			dflag++;
			continue;
#endif DEBUG

		case 'e':	/* expression follows */
			eflag++;
			argptr = optarg;
			continue;

		case 'f':	/* string file follows */
			fflag++;
			stringfile = fopen(optarg, "r");
			if (stringfile == NULL) {
			    fprintf(stderr, "fgrep: can't open %s\n", optarg);
			    exit(2);
			}
			continue;

		case 'l':	/* list file names only */
			lflag++;
			continue;

		case 'n':	/* line number prepended */
			nflag++;
			continue;

		case 's':	/* return status only */
			sflag++;
			continue;

		case 'v':	/* print lines not matching */
			vflag++;
			continue;

		case 'x':	/* exact match only */
			xflag++;
			continue;

		case 'y':	/* lower case in expression matches upper case too */
			yflag++;
			continue;

		case 'm':	/* grep option selecting fgrep */
			continue;

		case '?':
			errflg++;
	}

	argc -= optind;
	if (errflg || ((argc <= 0) && !fflag && !eflag)) {
		fprintf(stderr, "usage: fgrep %s\n",usage);
		exit(2);
	}
	if ( !eflag  && !fflag ) {
		argptr = argv[optind];
		optind++;
		argc--;
	}
	cgotofn();

	if (stringfile != NULL)
	    fclose(stringfile);

	if (!xflag)
	    cfail();
#ifdef DEBUG
	if (dflag)
	    display(tree);
#endif
	numfiles = argc;
	argv = &argv[optind];
	if (argc <= 0) {
	    if (lflag)
		exit(2);	/* can't list name of standard input */
	    execute((char *) NULL);
	}
	else
	    while ( --argc >= 0 ) {
		execute(*argv);
		argv++;
	    }
	exit (notfound? 2 : !anymatches? 1 : 0);
}


char *
getstring(buf, len)
    char *buf;
    int  len;
	/* Returns next string in buf, NULL if input is exhausted.
	 * If xflag, \n precedes the null terminator.
	 * Long strings (> len-2 chars) are silently truncated.
	 */
{
	register char  *sp;
	register int  c;
	char  *lim;

	lim = &buf[len-2];
	sp = buf;
	do {
	    if (stringfile != NULL) {
		if ((c = getc(stringfile)) == EOF)
		    break;
	    }
	    else if ((c = *argptr++) == '\0') {
		argptr--;	/* so next call on getstring sees the null */
		break;
	    }
	    if (sp < lim)
		*sp++ = c;
	} while (c != '\n');

	if (sp == buf && (!xflag || tree != NULL))
		/* true EOF, and empty pattern list processed if xflag */
	    return(NULL);

	if (c == '\n' && xflag == 0)
	    sp--;
	else if (c != '\n' && xflag == 1)
	    *sp++ = '\n';
	*sp = '\0';
	return(buf);
}

cgotofn()
{
	register struct words  *t;
	register char  *sp;
	register int  c;
	char  buf[BUFSIZ+2];	/* enough for newline and null + BUFSIZ chars */
	struct words  *last, *walloc();

	tree = NULL;
	while ((sp = getstring(buf, sizeof(buf))) != NULL) {
		/* match prefix of buf against previous patterns */
	    last = NULL;
	    for (t = tree; (c = *sp++) != '\0' && t != NULL; last = t, t = t->succ) {
		while (t->key != c && t->link != NULL)
		    t = t->link;
		if (t->key != c)
		    break;
	    }

	    if (c != '\0' && (t != NULL || tree == NULL)) {
		/* first pattern (non-empty) or buf is neither initial substring
		 * nor initial superstring of previous pattern; add new tail
		 * to common prefix.
		 */
		if (t == NULL)
		    t = tree = walloc();
		else
		    t = t->link = walloc();
		t->key = c;
		while ((c = *sp++) != '\0') {
		    t = t->succ = walloc();
		    t->key = c;
		}
	    }
	    else if (c == '\0')
		/* current pattern is initial substring of previous pattern */
		if (last != NULL)
		    last->succ = NULL;	/* delete suffix of longer string */
		else {
		    tree = NULL;	/* null string: matches all */
		    return;
		}
	}
}

overflo()
{
	fprintf(stderr, "fgrep: word list too large\n");
	exit(2);
}

struct words *
walloc()
{
	if (smax >= slim) {
	    if ((smax = (struct words *) calloc(INCRSIZ, sizeof(struct words))) == NULL)
		overflo();
	    slim = &smax[INCRSIZ];
	}
	return(smax++);
}

struct qpage *
qalloc()
{
	register struct qpage *qp;

	if ((qp = (struct qpage *) malloc(sizeof(struct qpage))) == NULL)
	    overflo();
	qp->nextpage = NULL;
	return(qp);
}

cfail()
	/* Performs a breadth-first traversal of the tree, setting failure
	 * links at each node above the first level. */
{
	struct words	**front, **rear;
	struct qpage	*firstpage, *lastpage;
	register struct words	*t, *pfail, *q;
	register char	c;

	if (tree == NULL)
	    return;
	firstpage = lastpage = qalloc();
	front = rear = firstpage->queue;
	*rear++ = tree;

	while (rear != front) {
	    t = *front++;
	    if (front >= &(firstpage->queue[QSIZE])) {
		free( (char *) firstpage);
		firstpage = firstpage->nextpage;
		front = firstpage->queue;
	    }

		/* now set failure links for the children of t and its siblings,
		 * scheduling their childrens' link setting at the same time.
		 */
	    do {
		if ((q = t->succ) != NULL) {
		    pfail = t->fail;
		    c = t->key;

		    while (pfail != NULL && pfail->key != c)
			if (pfail->link != NULL)
			    pfail = pfail->link;
			else
			    pfail = pfail->fail;
		    if (pfail == NULL)
			pfail = tree;
		    else
			pfail = pfail->succ;

		    if (pfail == NULL)
			/* current pattern (path from root) ends in a suffix
			 * which is another complete pattern; delete any
			 * remaining subtree.
			 */
			t->succ = NULL;
		    else {
			*rear++ = q;
		        if (rear >= &(lastpage->queue[QSIZE])) {
			    lastpage->nextpage = qalloc();
			    lastpage = lastpage->nextpage;
			    rear = lastpage->queue;
			}
			do
			    q->fail = pfail;
			while ((q = q->link) != NULL);
		    }
		}
	    } while ((t = t->link) != NULL);
	}
	free( (char *) lastpage);
}

#define BUF2  (&buf[BUFSIZ])
#define BUF3  (&buf[2*BUFSIZ])
#define ENDBUF3  (&buf[3*BUFSIZ])
execute(filename)
    char *filename;
{
	register char  *p;
	register struct words *t;
	register int  c;
	long  lcount, totalccount, nmatches;
	int  ccount, pcount, f;
	int  found, finished, matched;
	char buf[3*BUFSIZ];	/* large buffer + auxiliary buffer */
	char  *start, *lim, *oldp;

	if (filename != NULL) {
	    if ((f = open(filename, 0)) < 0) {
		if(!sflag)
		    fprintf(stderr, "fgrep: can't open %s\n", filename);
		else
		    exit(2);
		notfound = 1;
		return;
	    }
	}
	else
	    f = fileno(stdin);

	lcount = 0L;  totalccount = 0L;  nmatches = 0L;
	ccount = 0;  p = buf;
	do {	/* process one line */
	    t = tree;  lcount++;
	    found = finished = (t == NULL);
	    if ((start = p) == BUF3)
		start = buf;
	    do {	/* process one character */
		if (ccount-- == 0) {	/* buffer empty */
		    if (p == BUF3)
			p = buf;	/* wrap-around from second half of main buffer */
		    if (p > BUF2) {
			if (start > p && start < BUF3)
			    copyblock(&start, BUF3);
			ccount = read(f, p, (unsigned) (BUF3 - p));
		    }
		    else {	/* in first half or at start of second half
				 * of main buffer */
			if (start > p && start < &p[BUFSIZ])
			    copyblock(&start, BUF3);
			ccount = read(f, p, BUFSIZ);
		    }

		    if (ccount <= 0) {
			c = EOF;
			break;
		    }

		    totalccount += ccount--;
		}
		c = *p++;

		if (!finished) {
		    while (t != NULL) {
			while ( !(matched = match(t->key,c)) && t->link != NULL)
				/* try alternatives at this level */
			    t = t->link;
			if (matched)
			    break;
			t = t->fail;	/* chain back in pattern */
			finished = xflag;
		    }

		    if (t == NULL)	/* all alternatives tried, restart from
					 * first pattern character */
			t = tree;
		    else if ((t = t->succ) == NULL)
			found = finished = 1;
		}
	    } while (c != '\n');

	    if (vflag)
		found = !found;

	    if (found && p != start) {
		anymatches++;
		if (sflag)
		    exit(0);	/* matched: no need to look further */
		if (cflag)
		    nmatches++;
		else if (lflag) {
		    printf("%s\n", filename);
		    close(f);
		    return;
		}
		else {
		    if (numfiles > 1)
			printf("%s:", filename);
		    if (bflag)
			printf("%ld:", (totalccount - ccount) / BLKSIZE);
		    if (nflag)
			printf("%ld:", lcount);
		    oldp = p;  pcount = 0;
		    if (start < p)	/* no wrap-around or long line */
			lim = p;
		    else if (start == BUF3)	/* saved long line */
			lim = ENDBUF3;
		    else {	/* print tail of BUF2 first */
			for (p = start; p < BUF3; p++)
			    putchar(*p);
			pcount = p - start;
			lim = oldp;  start = buf;
		    }
		    if (pcount + (lim-start) > BUFSIZ)
			/* limit all lines to BUFSIZ chars to be consistent */
			lim = &start[BUFSIZ - pcount];
		    for (p = start; p < lim; p++)
			putchar(*p);
		    if (p[-1] != '\n')	/* line truncated or non-terminated last line */
			putchar('\n');
		    p = oldp;
		}
	    }
	} while (c != EOF);

	close(f);
	if (cflag) {
	    if (numfiles > 1)
		printf("%s:", filename);
	    printf("%ld\n", nmatches);
	}
}

copyblock (startp, dest)
    char **startp, *dest;
	/* copy BUFSIZ chars from *startp to dest; reset *startp to dest */
{
	register char  *sp, *dp;
	char  *lim;

	lim = *startp + BUFSIZ;  dp = dest;
	for (sp = *startp; sp < lim; sp++)
	    *dp++ = *sp;
	*startp = dest;
}

#ifdef DEBUG
struct qpage  *firstpage, *lastpage;
struct words  **nextfree;

display(root)
    struct words  *root;
{
	if (root != NULL) {
	    firstpage = lastpage = qalloc();
	    nextfree = lastpage->queue;
	    printkeys(root, 0);
	    if (!xflag)
		printfaillinks();
	    do
		free( (char *) firstpage);
	    while ((firstpage = firstpage->nextpage) != NULL);
	}
}

printkeys(t, level)
    register struct words  *t;
    int level;
{
	register int  c, i;

	if ((c = t->key) == '\n')
	    printf("\\n");
	else if (c < ' ' || c > '~')
	    putchar('?');
	else {
	    putchar(c);
	    *nextfree++ = t;
	    if (nextfree >= &(lastpage->queue[QSIZE])) {
		lastpage->nextpage = qalloc();
		lastpage = lastpage->nextpage;
		nextfree = lastpage->queue;
	    }

	    if (t->succ == NULL)
		putchar('\n');
	    else
		printkeys(t->succ, level+1);

	    if (t->link != NULL) {
		for (i=0; i < level; i++)
		    printf("^\b|");
		printkeys(t->link, level);
	    }
	}
}

tlookup(p, keyp, posp)
    struct words  *p;
    int  *keyp, *posp;
{
	register struct words  **wp;
	register struct qpage  *curpage;
	int  count;

	curpage = firstpage;  wp = curpage->queue;  count = 0;
	while (wp != nextfree && *wp != p)
	    if (++wp >= &(curpage->queue[QSIZE])) {
		count += QSIZE;
		curpage = curpage->nextpage;
		wp = curpage->queue;
	    }

	if (wp == nextfree)
	    return(0);	/* not found */
	else {
	    *posp = count + 1 + (wp - curpage->queue);
	    *keyp = (*wp)->key;
	    return(1);
	}
}

printfaillinks()
{
	register struct words  **wp;
	register struct qpage  *curpage;
	register int  count;
	int  key, pos;

	printf("num key  fail link\n");
	curpage = firstpage;  wp = curpage->queue;  count = 0;
	while (wp != nextfree) {
	    printf("%3d%3c   ", ++count, (*wp)->key);
	    if ((*wp)->fail == NULL)
		printf("NULL\n");
	    else if (tlookup((*wp)->fail, &key, &pos))
		printf("%d (%c)\n");
	    else
		printf("unknown\n");

	    if (++wp >= &(curpage->queue[QSIZE])) {
		count += QSIZE;
		curpage = curpage->nextpage;
		wp = curpage->queue;
	    }
	}
}
#endif DEBUG


