#include    "beasts.h"
/*
**      BSNOOP -- Aid for guessing game, "beasts"
** Compile: cc -O -q bsnoop.c -lS -o bsnoop
**      (c) P. Langston 1979
*/

char    *whatsccs   "@(#)bsnoop.c	1.4  last mod 6/13/80 -- (c) psl 1979";
char    *h_sccs     H_SCCS;

int     maxbn, bfh;
int     maxqn, qfh;
int     locked  = 0;
struct  bstr    b;
struct  qstr    q;

char    *instr(), *ask();

main(argc, argv)
char    *argv[];
{
	register char *cp, *ap;

	maxbn = binit();
	maxqn = qinit();
	printf("%d beasts, %d questions\n", maxbn, maxqn);
	for (;;) {
	    cp = ask("Op? ");
	    if (cp == 0)
		exit(0);
	    if ((ap = instr(" ", cp)) != 0)
		ap++;
	    switch (*cp) {
	    case 'c':
		check(ap);
		break;
	    case 'l':
		list(ap);
		break;
	    case 'm':
		mod(ap);
		break;
	    default:
		printf("Ops are check, list, and mod.  (^D to end)\n");
	    }
	}
}

check(com)
char    *com;
{
	register char *cp;

	for (cp = com; ; cp = ask("beasts or questions? ")) {
	    if (cp == 0)
		continue;
	    if (*cp == 'b')
		return(bcheck());
	    if (*cp == 'q')
		return(qcheck());
	}
}

qcheck()
{
	register int q1n, q2n, bn;
	char yesno[N_QUEST];

	for (q1n = maxqn; --q1n >= 0; yesno[q1n] = 0);
	for (bn = 0; bn < maxbn; bn++) {
	    getbeast(bn, &b);
	    for (q1n = maxqn; --q1n >= 0; ) {
		if (getbit(b.b_ans, q1n) == NO)
		    yesno[q1n] |= 1;
		else
		    yesno[q1n] |= 2;
	    }
	}
	for (q1n = maxqn; --q1n >= 0; ) {
	    if (yesno[q1n] != 3) {
		getquest(q1n, &q);
		printf("%s (#%d) is useless; all answers are `%s'\n",
		 q.q_text, q1n, yesno[q1n] == 1? "no" : "yes");
	    }
	}
	for (q1n = maxqn; --q1n > 0; ) {
	    for (q2n = 0; q2n < q1n; q2n++) {
		for (bn = 0; bn < maxbn; bn++) {
		    getbeast(bn, &b);
		    if (getbit(b.b_ans, q1n) != getbit(b.b_ans, q2n))
			break;
		}
		if (bn >= maxbn) {
		    getquest(q1n, &q);
		    printf("%s (#%d)", q.q_text, q1n);
		    getquest(q2n, &q);
		    printf(" :: %s (#%d)\n", q.q_text, q2n);
		}
	    }
	}
}

bcheck()
{
	register int b1n, b2n, qn;
	struct bstr b2;

	for (b1n = maxbn; --b1n > 0; ) {
	    getbeast(b1n, &b);
	    for (b2n = 0; b2n < b1n; b2n++) {
		getbeast(b2n, &b2);
		for (qn = maxqn; --qn >= 0; )
		    if (getbit(b.b_ans, qn) != getbit(b2.b_ans, qn))
			break;
		if (qn < 0)
		    printf("%s (#%d) :: %s (%d)\n",
		     b.b_name, b1n, b2.b_name, b2n);
	    }
	}
}

list(com)
char    *com;
{
	register char *cp;

	for (cp = com; ; cp = ask("beasts, answers or questions? ")) {
	    if (cp == 0)
		continue;
	    if (*cp == 'b')
		return(blist());
	    if (*cp == 'a')
		return(alist());
	    if (*cp == 'q')
		return(qlist());
	}
}

blist()
{
	register int bn;

	for (bn = 0; getbeast(bn, &b) != -1; bn++)
	    printf("%3d  %s\n", bn, b.b_name);
}

alist()
{
	register int bn, i;

	if (maxqn > 10) {
	    for (i = 0; i < maxqn; i++)
		if (i > 9)
		    printf(" %d", i / 10);
		else
		    printf("  ");
	    printf("\n");
	}
	for (i = 0; i < maxqn; i++)
	    printf(" %d", i % 10);
	printf("\n");
	for (bn = 0; getbeast(bn, &b) != -1; bn++) {
	    for (i = 0; i < maxqn; i++)
		printf(" %c", getbit(b.b_ans, i) == YES? 'Y' : 'N');
	    printf(" %s\n", b.b_name);
	}
}

qlist()
{
	register int qn;

	for (qn = 0; qn < maxqn; qn++) {
	    getquest(qn, &q);
	    printf("%2d :%3d : %s\n", qn, q.q_uid & 0377, q.q_text);
	}
}

mod(com)
char    *com;
{
	register char *cp, *ap;

	for (cp = com; ; cp = ask("beasts or questions? ")) {
	    if (cp == 0)
		continue;
	    ap = instr(" ", cp);
	    if (ap)
		ap++;
	    if (*cp == 'b')
		return(bmod(ap));
	    if (*cp == 'q')
		return(qmod(ap));
	}
}

qmod(com)
char    *com;
{
	register char *cp;
	register int i;
	int qn, bn, delete;
	char buf[16];

	if ((cp = com) == 0)
	    cp = ask("Which #? (- to del) ");
	qn = atoi(cp);
	if (qn < 0) {
	    delete = 1;
	    qn = -qn;
	} else
	    delete = 0;
	lock();
	if (getquest(qn, &q) == -1) {
	    printf("question #%d doesn't exist\n", qn);
	    unlock();
	    return;
	}
	printf("%2d :%3d : %s\n", qn, q.q_uid & 0377, q.q_text);
	if (delete) {
	    cp = ask("Delete this question? ");
	    if (*cp == 'y') {
		qdelete(qn);
		unlock();
		return;
	    }
	}
	printf("new text : ");
	i = read(0, q.q_text, 128);
	if (i > 1) {
	    q.q_text[i - 1] = '\0';
	    putquest(qn, &q);
	    printf("%2d :%3d : %s (modified)\n",
	     qn, q.q_uid & 0377, q.q_text);
	}
	if (*ask("Modify beast answers? ") == 'y') {
	    for (bn = 0; bn < maxbn && getbeast(bn, &b) != -1; bn++) {
		printf("%d) %s <%c>? ",
		 bn, b.b_name, getbit(b.b_ans, qn) == YES? 'y' : 'n');
		cp = ask("");
		if (*cp == 'y')
		    setbit(b.b_ans, qn);
		else if (*cp == 'n')
		    clrbit(b.b_ans, qn);
		putbeast(bn, &b);
	    }
	}
	unlock();
}

qdelete(oqn)
{
	register int qn, bn;

	maxqn = maxqn - 1;
	for (qn = oqn; qn < maxqn; qn++) {
	    getquest(qn + 1, &q);
	    putquest(qn, &q);
	}
	q.q_text[0] = '\0';
	putquest(maxqn, &q);
	for (bn = 0; bn < maxbn; bn++) {
	    getbeast(bn, &b);
	    for (qn = oqn; qn < maxqn; qn++) {
		if (getbit(b.b_ans, qn + 1) == YES)
		    setbit(b.b_ans, qn);
		else
		    clrbit(b.b_ans, qn);
	    }
	    clrbit(b.b_ans, maxqn);
	    putbeast(bn, &b);
	}
}

bmod(com)
char    *com;
{
	register char *cp;
	register int i;
	int bn, qn, delete;
	char buf[128];

	if ((cp = com) == 0)
	    cp = ask("Which #? (- to del) ");
	bn = atoi(cp);
	if (bn < 0) {
	    delete = 1;
	    bn = -bn;
	} else
	    delete = 0;
	lock();
	if (getbeast(bn, &b) == -1) {
	    printf("beast #%d doesn't exist\n", bn);
	    unlock();
	    return;
	}
	printf("%2d : %s\n", bn, b.b_name);
	if (delete) {
	    cp = ask("Delete this beast? ");
	    if (*cp == 'y') {
		bdelete(bn);
		unlock();
		return;
	    }
	}
	printf("new name : ");
	i = read(0, buf, sizeof buf);
	if (i > 1) {
	    if (i > sizeof b.b_name)
		i = sizeof b.b_name;
	    buf[i - 1] = '\0';
	    sprintf(b.b_name, "%s", buf);
	}
	if (*ask("Modify answers? ") == 'y') {
	    for (qn = 0; qn < maxqn && getquest(qn, &q) != -1; qn++) {
		printf("%d) %s <%c>",
		 qn, q.q_text, getbit(b.b_ans, qn) == YES? 'y' : 'n');
		cp = ask("? ");
		if (*cp == 'y')
		    setbit(b.b_ans, qn);
		else if (*cp == 'n')
		    clrbit(b.b_ans, qn);
	    }
	}
	putbeast(bn, &b);
	unlock();
}

bdelete(obn)
{
	register int bn;

	maxbn = maxbn - 1;
	for (bn = obn; bn < maxbn; bn++) {
	    getbeast(bn + 1, &b);
	    putbeast(bn, &b);
	}
	b.b_name[0] = '\0';
	putbeast(maxbn, &b);
}

qinit()                /* open file for existing questions & return number */
{
	register int n;
	struct qstr q;

	if ((qfh = open(QUESTFILE, 2)) < 0) {
	    perror(QUESTFILE);
	    exit(3);
	}
	for (n = 0; read(qfh, &q, sizeof q) == sizeof q; n++)
	    if (q.q_text[0] == '\0')
		break;
	return(n);
}

binit()                   /* open file for existing beasts & return number */
{
	register int n;
	struct bstr b;

	if ((bfh = open(BEASTFILE, 2)) < 0) {
	    perror(BEASTFILE);
	    exit(3);
	}
	for (n = 0; read(bfh, &b, sizeof b) == sizeof b; n++)
	    if (b.b_name[0] == '\0')
		break;
	return(n);
}

char    *
instr(pat, string)  /* return pointer to pat in string, ret 0 if not there */
char    *string;
{
	register char *sp, *pp, *bp;

	sp = string;
	for (;;) {
	    pp = pat;
	    while (*sp && *sp != *pp)
		sp++;
	    if (*sp == '\0')
		return(0);
	    bp = sp;
	    while (*bp++ == *pp++)
		if (*pp == '\0')
		    return(sp);
	    sp++;
	}
}

char    *
ask(prompt)
char    *prompt;
{
	register int i;
	static char buf[4][64];
	static int n;

	printf("%s", prompt);
	n = (n + 1) & 3;
	i = read(0, buf[n], 128);
	if (i <= 0)
	    return(0);
	return(buf[n]);
}

getbeast(bn, bp)                        /* read beast #bn into struct @ bp */
struct  bstr    *bp;
{
	seek(bfh, bn * sizeof *bp, 0);
	if (read(bfh, bp, sizeof *bp) != sizeof *bp)
	    return(-1);
	return(0);
}

putbeast(bn, bp)                                    /* write out beast #bn */
struct  bstr    *bp;
{
	if (locked == 0) {
	    printf("Oops!  Tried to putbeast() without lock.\n");
	    exit(3);
	}
	seek(bfh, bn * sizeof *bp, 0);
	write(bfh, bp, sizeof *bp);
}

getquest(qn, qp)                       /* read question #qn into area @ qp */
struct  qstr    *qp;
{
	seek(qfh, qn * sizeof *qp, 0);
	if (read(qfh, qp, sizeof *qp) != sizeof *qp)
	    return(-1);
	return(0);
}

putquest(qn, qp)                                    /* write out quest #qn */
struct  qstr    *qp;
{
	if (locked == 0) {
	    printf("Oops!  Tried to putquest() without lock.\n");
	    exit(3);
	}
	seek(qfh, qn * sizeof *qp, 0);
	write(qfh, qp, sizeof *qp);
}

getbit(map, n)
int     map[];
{
	int word, shift;

	word = n >> 4;
	shift = n & 017;
	return((map[word] >> shift) & 1);
}

setbit(map, n)
int     map[];
{
	int word, shift;

	word = n >> 4;
	shift = n & 017;
	map[word] |= 1 << shift;
}

clrbit(map, n)
int     map[];
{
	int word, shift;

	word = n >> 4;
	shift = n & 017;
	map[word] &= ~(1 << shift);
}

lock()
{
	if (locked == 0) {
	    while (link(LOCKNODE, LOCKFILE) == -1) {
		printf("Waiting for lock file\n");
		sleep(5);
	    }
	    locked = 1;
	}
}

unlock()
{
	if (locked == 0)
	    printf("Tried to unlock with locked = 0\n");
	else
	    unlink(LOCKFILE);
	locked = 0;
}
