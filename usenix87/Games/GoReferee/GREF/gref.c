#include	"godef.h"
#include	<stdio.h>
#include	<signal.h>
/*
**      GREF -- GO referee program
**      psl 5/84
*/

char	transfile[32];		/* transcript of this game */
int	movenum;
int	iflg	= 0;		/* used by cursinit() for 5620 */
int	quiet	= 0;		/* > 0 ==> no board display */
int     sflg	= 0;		/* "stop mode", pause after each white move */
int     trace   = 0;		/* verbose */
int	tlimit	= 60*60;	/* in seconds, normally 60 * 60 */
int	stlimit	= 10;		/* in seconds, normally 10 */

struct	plyrstr *lpw;		/* to whom last pipe write was directed */

FILE    *tfp;			/* transcript file handle */

extern	char	*letters;		/* from stoc.c */
extern	int	ycptp;			/* from utime.c */
extern	struct	gstr	g[MAXGRPS];	/* from groups.o in gosub.a */
extern	struct	hstr	h[MAXGRPS];	/* from holes.o in gosub.a */

int     plumber(), whatsup();
FILE	*opentrans();
extern	char    *stoc(), *copy(), *lfs();
extern  char	*ctime();
extern  FILE	*fopen();

main(argc, argv)
char    *argv[];
{
	char buf[128];
	int bmv, wmv, secs;
	long now;

	setbuf(stdout, 0);
	while (--argc > 0) {
	    if (argv[argc][0] == '-') {
		switch (argv[argc][1]) {
		case 'i':		/* initialization flag for bdisp() */
		    iflg++;
		    break;
		case 'l':		/* time limit */
		    tlimit = atoi(&argv[argc][2]);
		    break;
		case 'q':		/* quiet (no board) */
		    quiet++;
		    break;
		case 's':		/* stop mode */
		    sflg++;
		    break;
		case 't':		/* trace */
		    trace++;
		    break;
		default:
		    goto oops;
		}
	    } else {
		if (access(argv[argc], 1) != 0) {
		    perror(argv[argc]);
		    goto oops;
		}
		if (p[WHITE].p_plyr == 0)
		    p[WHITE].p_plyr = argv[argc];
		else if (p[BLACK].p_plyr == 0)
		    p[BLACK].p_plyr = argv[argc];
		else
		    goto oops;
	    }
	}
	if (p[BLACK].p_plyr == 0) {
oops:
	    fprintf(stderr,
	     "Usage: %s [-quiet] [-trace] black-program white-program\n",
	     argv[0]);
	    exit(2);
	}
	tfp = opentrans(p[BLACK].p_plyr, p[WHITE].p_plyr);
	utiminit();				      /* set up for timing */
	time(&now);
	fprintf(tfp, "%20.20s`%s' vs. `%s'\n",
	 ctime(&now), p[BLACK].p_plyr, p[WHITE].p_plyr);
	bdinit(&b);                           /* initialize board contents */
	if (!quiet)
	    cursinit();                     /* initialize display of board */
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, whatsup);
	signal(SIGPIPE, plumber);
	forkplyr(BLACK);		      /* set up pipes & fork black */
	forkplyr(WHITE);		      /* set up pipes & fork white */
	send(&p[BLACK], bmv = BEBLACK);        /* send "go first" to black */
	send(&p[WHITE], wmv = BEWHITE);       /* send "go second" to white */
	if (!quiet)
	    bdisp();                                         /* show board */
	for (movenum = 0; ; ) {
	    movenum++;
	    bmv = rec(&p[BLACK]);              /* read black player's move */
	    trans(BLACK, bmv);                        /* update transcript */
	    if (p[BLACK].p_utime >= tlimit) {     /* check for short limit */
		secs = utime(p[BLACK].p_pid) - p[BLACK].p_utime;
		if (secs > stlimit)
		    overtime(BLACK, secs);
	    }
	    if (makemove(BLACK, bmv) == ILLEGAL)         /* carry out move */
		illmove(BLACK, bmv);
	    secs = utime(p[WHITE].p_pid);
	    if (p[WHITE].p_utime < tlimit && secs >= tlimit) {
		send(&p[WHITE], BYOROMI);
		log("White byo-romi");
	    }
	    p[WHITE].p_utime = secs;
	    if (ycptp != p[WHITE].p_lycptp) {
		p[WHITE].p_lycptp = ycptp;
		log("White fork?");
	    }
	    send(&p[WHITE], bmv);		  /* white starts thinking */
	    if (bmv == PASS && wmv == PASS)
		break;
	    p[BLACK].p_ko = 0;
	    if (!quiet)
		bdisp();                                     /* show board */

	    movenum++;
	    wmv = rec(&p[WHITE]);              /* read white player's move */
	    trans(WHITE, wmv);                        /* update transcript */
	    if (p[WHITE].p_utime >= tlimit) {     /* check for short limit */
		secs = utime(p[WHITE].p_pid) - p[WHITE].p_utime;
		if (secs > stlimit)
		    overtime(WHITE, secs);
	    }
	    if (makemove(WHITE, wmv) == ILLEGAL)         /* carry out move */
		illmove(WHITE, wmv);
	    secs = utime(p[BLACK].p_pid);
	    if (p[BLACK].p_utime < tlimit && secs >= tlimit) {
		send(&p[BLACK], BYOROMI);
		log("Black byo-romi");
	    }
	    p[BLACK].p_utime = secs;
	    if (ycptp != p[BLACK].p_lycptp) {
		p[BLACK].p_lycptp = ycptp;
		log("Black fork?");
	    }
	    send(&p[BLACK], wmv);		   /* black starts thinking */
	    if (bmv == PASS && wmv == PASS)
		break;
	    p[WHITE].p_ko = 0;
	    if (!quiet)
		bdisp();                                     /* show board */
	    if (sflg)
		read(0, buf, sizeof buf);
	}
	quit(0);
}

FILE	*
opentrans(bplyr, wplyr)
char	*bplyr, *wplyr;
{
	register char *cp, *bp;
	char bbuf[32], wbuf[32];

	for (cp = bp = bplyr; *cp ; )
	    if (*cp++ == '/')
		bp = cp;
	copy(bp, bbuf);
	bbuf[6] = '\0';
	for (cp = bp = wplyr; *cp ; )
	    if (*cp++ == '/')
		bp = cp;
	copy(bp, wbuf);
	wbuf[6] = '\0';
	sprintf(transfile, "%s-%s", bbuf, wbuf);
	if ((tfp = fopen(transfile, "a")) == (FILE *) NULL) {
	    perror(transfile);
	    exit(3);
	}
	return(tfp);
}

quit(hurry)
{
	score(hurry);                           /* put board in transcript */
	if (!quiet) {
	    bdisp();                                   /* show final board */
	    cursfini();
	}
	exit(0);
}

/*	A player took too long for a move after exceeding the time limit
*/
overtime(pn, secs)
{
	sprintf(fmtbuf, "%s took %d secs", color[pn], secs);
	log(fmtbuf);
	quit(0);
}

/*      Handle strange situations.
*/
whatsup(signum)
{
	char buf[64];
	int i, pnum;

	signal(signum, whatsup);
	if (!quiet)
	    update();
	fprintf(stderr, "\r  Caught signal %d; what's up?   \b\b", signum);
	fgets(buf, sizeof buf, stdin);
	switch (*buf) {
	case 'Q':		/* quick quit */
	    quit(1);
	case 'q':		/* conservative quit */
	    quit(0);
	case 'r':		/* redraw */
	    break;
	case 'm':		/* send message */
	    pnum = buf[1] - '0';
	    if (pnum != BLACK && pnum != WHITE)
		goto syntax;
	    i = copy(&buf[2], buf) - buf;
	    write(p[pnum].p_wpipe, buf, i);
	    break;
	case 's':		/* set stop mode move */
	    sflg = atoi(&buf[1]);
	    fprintf(stderr, "Stop set to %d\n", sflg);
	    sleep(1);
	    break;
	case 't':		/* set trace level */
	    trace = buf[1] - '0';
	    fprintf(stderr, "Trace set to %d\n", trace);
	    sleep(1);
	    break;
	default:
syntax:
	    fprintf(stderr, "Options are:\n");
	    fprintf(stderr, " m#text  - send msg to prog # (1 or 2)\n");
	    fprintf(stderr, " q       - kill progs & quit, update trans.\n");
	    fprintf(stderr, " Q       - kill progs & quit quickly\n");
	    fprintf(stderr, " r       - redraw the screen\n");
	    fprintf(stderr, " s#      - set stop mode after move #\n");
	    fprintf(stderr, " t#      - set trace level to #\n");
	    sleep(3);
	}
	if (!quiet)
	    bdredraw();
}

/*      Set up pipes, fork & exec a player's process.
*/
forkplyr(us)
{
	register struct plyrstr *pp;
	char buf[16], logbuf[128];
	int tpipe[2], fpipe[2], i;

	pp = &p[us];
	if (pipe(tpipe) == -1) {
	    perror("to pipe");
	    exit(3);
	}
	if (pipe(fpipe) == -1) {
	    perror("from pipe");
	    exit(3);
	}
	if (trace > 1) {
	    sprintf(fmtbuf, "EXEC: %s\n", pp->p_plyr);
	    log(fmtbuf);
	}
	switch (pp->p_pid = fork()) {
	case -1:                                                /* failure */
	    perror("fork");
	    exit(3);
	case 0:                                                   /* child */
	    close(0);
	    dup(tpipe[0]);
	    close(1);
	    dup(fpipe[1]);
	    close(2);
	    sprintf(logbuf, "log.%s", pp->p_plyr);
	    if (open(logbuf, 1) == 2)
		lseek(2, (long) 0, 2);
	    else
		creat(logbuf, 0644);
	    sprintf(logbuf, "%s vs. %s\n", p[BLACK].p_plyr, p[WHITE].p_plyr);
	    write(2, logbuf, strlen(logbuf));
	    for (i = 16; --i > 2; close(i));
	    signal(SIGINT, SIG_IGN);
	    signal(SIGQUIT, SIG_IGN);
	    sleep(5);			      /* to allow ycptp to be read */
	    execl(pp->p_plyr, pp->p_plyr, 0);
	    perror(pp->p_plyr);
	    exit(3);
	default:
	    utime(pp->p_pid);			   /* do this to set ycptp */
	    pp->p_lycptp = ycptp;
	    pp->p_rpipe = fpipe[0];
	    pp->p_wpipe = tpipe[1];
	    close(fpipe[1]);
	    close(tpipe[0]);
	}
}

/*      Send a move in the recipient's format
*/
send(pp, mv)
struct	plyrstr	*pp;
{
	register char *mp;

	mp = stoc(mv);
	if (trace > 1) {
	    sprintf(fmtbuf, "SEND(%s): %s\n", pp->p_plyr, mp);
	    log(fmtbuf);
	}
	lpw = pp;
	write(pp->p_wpipe, mp, strlen(mp));
	write(pp->p_wpipe, "\n", 1);
}

/*      Catch broken pipe signals.
*/
plumber()
{
	sprintf(fmtbuf, "%s has apparently died (broken pipe)\n", lpw->p_plyr);
	fputs(fmtbuf, tfp);
	fputs(fmtbuf, stdout);
	signal(SIGPIPE, plumber);
}

/*      Read a move & convert from the sender's format to spot number
*/
rec(pp)
struct	plyrstr	*pp;
{
	register char *bp;
	register int i;
	char buf[1024];
	extern int errno;

	if (trace > 1) {
	    sprintf(fmtbuf, "REC(%s):...", pp->p_plyr);
	    log(fmtbuf);
	}
	for (;;) {
	    bp = buf;
	    if (trace > 1) {
		sprintf(fmtbuf, "About to read from %d", pp->p_rpipe);
		log(fmtbuf);
	    }
	    while ((i = read(pp->p_rpipe, bp, 1)) == 1 && *bp != '\n') {
		if (*bp != ' ' && *bp != '\t')
		    bp++;
		if (trace > 2) {
		    sprintf(fmtbuf, "`%c'", bp[-1]);
		    log(fmtbuf);
		}
		if (bp - buf > 32)
		    --bp;
	    }
	    bp[1] = '\0';
	    if (i <= 0) {
		sprintf(fmtbuf, "Read on %s pipe returns %d, errno=%d\n",
		 pp->p_plyr, i, errno);
		log(fmtbuf);
		sprintf(fmtbuf, "Buffer had `%s'", buf);
		log(fmtbuf);
		update();
		sleep(3);
	    }
	    if (trace > 1) {
		sprintf(fmtbuf, "REC got `%s'", lfs(buf));
		log(fmtbuf);
	    }
	    if (bp == &buf[1])
		continue;
	    i = ctos(buf);
	    if ((i > BSZ2 && i < BAREA - BSZ2) || i == PASS)
		return(i);
	}
}

/*      Somebody screwed up...
*/
illmove(us, sn)
{
	illlog(us, sn);
	sprintf(fmtbuf, "Game over; (%s's fault)", color[us]);
	log(fmtbuf);
	quit(0);
}

/*      Update the transcript file
*/
trans(us, mv)
{
	char *cp, *mp;
	long now;

	if (mv == PASS)
	    mp = "pass";
	else
	    mp = stoc(mv);
	time(&now);
	cp = ctime(&now);
	cp = &cp[11];
	if (us == BLACK)
	    sprintf(fmtbuf, "%3d %-6s        %8.8s", movenum, mp, cp);
	else
	    sprintf(fmtbuf, "%3d        %-6s %8.8s", movenum, mp, cp);
	log(fmtbuf);
}

/*      Make an entry in the transcript
*/
log(str)
char	*str;
{
	static int lastline;

	fputs(str, tfp);
	fputs("\n", tfp);
	fflush(tfp);
	if (!quiet)
	    dislog(str);
}

illlog(us, sn) /* log the illegal move */
{
	if (whyillegal == ILL_NOTEMPTY) {
	    sprintf(fmtbuf, "%s/%s plays in nonempty spot (%s)",
	     color[us], p[us].p_plyr, stoc(sn));
	    log(fmtbuf);
	} else if (whyillegal == ILL_INTOKO) {
	    sprintf(fmtbuf, "%s/%s plays back into a KO (%s)",
	     color[us], p[us].p_plyr, stoc(sn));
	    log(fmtbuf);
	} else if (whyillegal == ILL_SUICIDE) {
	    sprintf(fmtbuf, "%s/%s attempts suicide (%s)",
	     color[us], p[us].p_plyr, stoc(sn));
	    log(fmtbuf);
	}
	update();
}

/*      Collect time & score.  Print them out.
*/
score(hurry)
{
	if (!hurry)
	    sleep(3);        /* pregnant pause (for programs to finish up) */
	if (p[BLACK].p_pid)
	    kill(p[BLACK].p_pid, 9);
	if (p[WHITE].p_pid)
	    kill(p[WHITE].p_pid, 9);
	if (hurry)
	    return;
	while (timeone());
	fprintf(tfp, "\n* * *   F I N A L   B O A R D   * * *\n");
	bdump(tfp);
	fprintf(tfp, "              Black/%-16.16s  White/%-16.16s\n",
	 p[BLACK].p_plyr, p[WHITE].p_plyr);
	fprintf(tfp, "CAPTURES %10d %24d\n",
	 p[BLACK].p_captures, p[WHITE].p_captures);
	fprintf(tfp, "USER TIME %6d:%02d:%02d %18d:%02d:%02d\n",
	 (int) (p[BLACK].p_utime / 3600),
	 (int) ((p[BLACK].p_utime % 3600) / 60),
	 (int) (p[BLACK].p_utime % 60),
	 (int) (p[WHITE].p_utime / 3600),
	 (int) ((p[WHITE].p_utime % 3600) / 60),
	 (int) (p[WHITE].p_utime % 60));
	countem(tfp);
}

countem(fp)
FILE	*fp;
{
	register int gn, hn, i, j, k, wc, bc, sn;
	int ngroups, nholes;
	struct bdstr gb, hb;		/* group & hole index boards */

	ngroups = groups(&b, &gb);
	nholes = holes(&b, &gb, &hb, 2);		/* identify holes */
	    /* (ignoring groups of size 2 or smaller in deciding eyes) */
	/* count eyes for all groups */
	for (gn = ngroups; --gn >= 0; )
	    g[gn].g_eyes = 0;
	for (hn = nholes; --hn >= 0; ) {
	    if (h[hn].h_grps == 1) {
		gn = h[hn].h_g[0];
		i = g[gn].g_eyes++;
		if (i < MAXEYES)
		    g[gn].g_h[i] = hn;
	    }
	}
	/* assign ownership of holes */
	for (hn = nholes; --hn >= 0; ) {
	    if (h[hn].h_who == (BLACK | WHITE))	{
		k = 0;
		if ((i = h[hn].h_grps) > MAXLIBS)
		    i = MAXLIBS;
		while (--i >= 0) {
		    gn = h[hn].h_g[i];
		    if (g[gn].g_eyes > 1)
			k |= g[gn].g_who;
		}
		h[hn].h_who = k;
	    }
	}
	wc = bc = 0;
	fprintf(fp, "  ");
	for (i = 1; i < BSZ1; i++)
	    fprintf(fp, " %c", letters[i]);
	fprintf(fp, "\n");
	for (j = BSZ1; --j > 0; ) {
	    fprintf(fp, "%2d", j);
	    for (i = 1; i < BSZ1; i++) {
		sn = i + j * BSZ2;
		k = b.b_spot[sn].s_occ;
		if (k != EMPTY)
		    fprintf(fp, " %c", ".*O?"[k]);
		else {
		    k = h[hb.b_spot[sn].s_flg].h_who;
		    if (k == BLACK) {
			bc++;
			fprintf(fp, " +");
		    } else if (k == WHITE) {
			wc++;
			fprintf(fp, " -");
		    } else
			fprintf(fp, " .");
		}
	    }
	    fprintf(fp, " %d\n", j);
	}
	fprintf(fp, "  ");
	for (i = 1; i < BSZ1; i++)
	    fprintf(fp, " %c", letters[i]);
	fprintf(fp, "\n");
	fprintf(tfp, "TERRITORY    Black:%d    White:%d\n", bc, wc);
}

bdump(fp)
FILE	*fp;
{
	register int i, j;

	fprintf(fp, "  ");
	for (i = 1; i < BSZ1; i++)
	    fprintf(fp, " %c", letters[i]);
	fprintf(fp, "\n");
	for (j = BSZ1; --j > 0; ) {
	    fprintf(fp, "%2d", j);
	    for (i = 1; i < BSZ1; i++)
		fprintf(fp, " %c", ".*O?"[b.b_spot[i + j * BSZ2].s_occ]);
	    fprintf(fp, " %d\n", j);
	}
	fprintf(fp, "  ");
	for (i = 1; i < BSZ1; i++)
	    fprintf(fp, " %c", letters[i]);
	fprintf(fp, "\n");
}

timeone()	/* wait() and collect utime statistics */
{
	int pid, exstat;
	long before, after;
	struct plyrstr *pp;

	before = kidutime();
	pid = wait(&exstat);
	if (pid == -1)
	    return(0);
	after = kidutime();
	if (pid == p[BLACK].p_pid) {
	    pp = &p[BLACK];
	    p[BLACK].p_pid = 0;
	} else if (pid == p[WHITE].p_pid) {
	    pp = &p[WHITE];
	    p[WHITE].p_pid = 0;
	} else {
	    fprintf(stderr, "Proc %d?\n", pid);
	    return(1);
	}
	pp->p_utime = after - before;
	return(1);
}
