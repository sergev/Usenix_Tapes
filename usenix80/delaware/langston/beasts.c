#include    "beasts.h"
/*
**      BEASTS -- Guessing game (with apologies to Lou Lasher and others)
** Compile: cc -O -q beasts.c -lS -o beasts
**      (c) P. Langston 1979
*/

char    *whatsccs   "@(#)beasts.c	1.4  last mod 3/8/80 -- (c) psl 1979";
char    *h_sccs     H_SCCS;

int     bmap[NBI], nb, bfh;
int     qmap[NQI], qfh;
int     maxbn;                                   /* total number of beasts */
int     maxqn;                                /* total number of questions */
int     locked  = 0;
struct  bstr newb;

char    *instr(), *article(), *artstrip(), *copy(), *cat();

main(argc, argv)
char    *argv[];
{
	char *cp;
	int qn, ans;
	long now;
	struct qstr q;

	printf("\t\tWelcome to the bestiary!\n");
	printf("Think of a beast that you know something about and answer");
	printf(" my questions\nwith `yes' or `no'.  If we're lucky I'll guess");
	printf(" your beast!\n\nHit <RETURN> when you've picked a beast...");
	time(&now);
	srand((int) (now & 077777));
	maxbn = binit();               /* turn on bits for existing beasts */
	maxqn = qinit();            /* turn on bits for existing questions */
	read(0, &q, 10);
	for (nb = maxbn; nb > 1; ) {
	    qn = pickquest();                      /* find a good question */
	    if (qn < 0) {
		printf("Oops!  Ran out of questions before beasts\n");
		exit(1);
	    }
	    clrbit(qmap, qn);
	    getquest(qn, &q);
	    ans = rand();
	    if (ans & 030)
		cp = "your beast";
	    else if (ans & 040)
		cp = "the beast you have in mind";
	    else
		cp = "the beast you're thinking of";
	    ans = askabout(q.q_text, cp);
	    if (ans == YES)
		setbit(newb.b_ans, qn);
	    nb = prune(qn, ans);
	}
	if (nb < 1)
	    printf("Gosh!  I don't have any idea ...\n");
	else {
	    ans = guess();                        /* guess remaining beast */
	    if (ans == YES) {
		switch ((rand() >> 4) % 4) {
		case 0:
		    printf("Aha!  Just as I suspected.\n");
		    break;
		case 1:
		    printf("I knew it all along ...\n");
		    break;
		case 2:
		    printf("Whew!  I was afraid I wasn't going to get it!\n");
		    break;
		case 3:
		    printf("Aren't computers smart?  (Actually the");
		    printf(" programmers should get the credit.)\n");
		    break;
		}
		exit(0);
	    }
	}
	add_beast();
	exit(0);
}

qinit()                 /* set bits for existing questions & return number */
{
	register int n;
	struct qstr q;

	if ((qfh = open(QUESTFILE, 2)) < 0) {
	    perror(QUESTFILE);
	    exit(3);
	}
	for (n = 0; read(qfh, &q, sizeof q) == sizeof q; n++) {
	    if (q.q_text[0] == '\0')
		break;
	    setbit(qmap, n);
	}
	return(n);
}

binit()                    /* set bits for existing beasts & return number */
{
	register int n;
	struct bstr b;

	if ((bfh = open(BEASTFILE, 2)) < 0) {
	    perror(BEASTFILE);
	    exit(3);
	}
	for (n = 0; read(bfh, &b, sizeof b) == sizeof b; n++) {
	    if (b.b_name[0] == '\0')
		break;
	    setbit(bmap, n);
	}
	return(n);
}

pickquest()                 /* pick a question that will eliminate a beast */
{
	register int b1n, b2n, q;
	struct bstr b1, b2;

	b1n = -1;
	for (q = (rand() % 7); --q > 0; )
	    b1n = nxtbeast(b1n, &b1);   /* cycle randomly thru poss beasts */
	if (b1n == -1)
	    b1n = nxtbeast(b1n, &b1);              /* get a possible beast */
	b2n = b1n;
	while ((b2n = nxtbeast(b2n, &b2)) != -1) {
	    for (q = maxqn; --q >= 0; )
		if (getbit(b1.b_ans, q) != getbit(b2.b_ans, q))
		    return(q);
	}
	return(-1);                                /* no way to test b1n ! */
}

prune(q, a)                /* remove any beasts where answer to q is not a */
{
	register int n, bn;
	struct bstr b;

	n = 0;
	for (bn = -1; (bn = nxtbeast(bn, &b)) != -1; ) {
	    if (getbit(b.b_ans, q) == a)
		n++;
	    else
		clrbit(bmap, bn);
	}
	return(n);
}

guess()
{
	char buf[64];
	int bn, i;
	struct bstr b;

	bn = nxtbeast(-1, &b);
	i = b.b_name[0];
	sprintf(buf, "Is your beast %s%s", article(b.b_name), b.b_name);
	return(askyn(buf));
}

add_beast()
{
	register int i, bn, a;
	char buf[128], buf2[128], *cp;
	int qn, newa;
	struct bstr b;
	struct qstr newq, q;

	lock();
	printf("Then");
	do {
	    printf(" what was it? ");
	    i = read(0, buf, sizeof newb.b_name);
	} while (i <= 1);
	buf[i - 1] = '\0';
	copy(artstrip(buf), newb.b_name);
	for (bn = 0; bn < maxbn; bn++) {
	    getbeast(bn, &b);
	    if (compare(artstrip(b.b_name), artstrip(newb.b_name)) != 0)
		continue;
	    printf("Wait a minute!\n");
	    for (qn = 0; qn < maxqn; qn++) {
		if (getbit(qmap, qn))            /* skip unasked questions */
		    continue;
		a = getbit(b.b_ans, qn);
		newa = getbit(newb.b_ans, qn);
		if (newa == a)
		    continue;
		printf("I thought the answer to the question:\n");
		getquest(qn, &q);
		printf("%s? was `%s' for a %s!\n", q.q_text,
		 a == YES? "yes" : "no", b.b_name);
		printf("Please help me out --\n");
		newa = askabout(q.q_text, b.b_name);
		if (newa == a)
		    printf("Humph!\n");
		else {
		    printf("Thanks\n");
		    if (newa == YES)
			setbit(b.b_ans, qn);
		    else
			clrbit(b.b_ans, qn);
		    putbeast(bn, &b);
		}
	    }
	    unlock();
	    exit(1);
	}
	switch ((rand() >> 3) % 4) {
	case 0:
	    printf("Wow!  %s%s, huh?\n", article(newb.b_name), newb.b_name);
	    break;
	case 1:
	    printf("Golly... I've never heard of %s%s!\n",
	     article(newb.b_name), newb.b_name);
	    break;
	case 2:
	    printf("%s%s?  Now I've heard of everything!\n",
	     article(newb.b_name), newb.b_name);
	    break;
	}
	bn = nxtbeast(-1, &b);
	if (bn != -1) {
	    printf("Please give me a general question that would distin");
	    printf("guish between %s%s\n", article(newb.b_name), newb.b_name);
	    printf("and %s%s\n", article(b.b_name), b.b_name);
	    for (;;) {
		i = read(0, buf, sizeof buf);
		if (i > 5)
		    break;
		printf("Huh?\n");
	    }
	    buf[i - 1] = '\0';
	    copy(newb.b_name, copy(article(newb.b_name), buf2));
	    if ((cp = instr(buf2, buf)) != 0) {
		*cp = '\0';
		i = copy(buf2, buf2) - buf2;
		sprintf(newq.q_text, "%sit%s", buf, &cp[i]);
	    } else if ((cp = instr("your beast", buf)) != 0) {
		*cp = '\0';
		sprintf(newq.q_text, "%sit%s", buf, &cp[10]);
	    } else if ((cp = instr("your animal", buf)) != 0) {
		*cp = '\0';
		sprintf(newq.q_text, "%sit%s", buf, &cp[11]);
	    } else
		sprintf(newq.q_text, buf);
	    if ((cp = instr("?", newq.q_text)) != 0)
		*cp = '\0';
	    for (qn = 0; qn < maxqn; qn++) {   /* check if question is new */
		getquest(qn, &q);
		if (compare(q.q_text, newq.q_text) == 0)
		    break;
	    }
	    if (qn >= maxqn) {              /* it really is a new question */
		newq.q_uid = getuid();
		time(&newq.q_date);
		putquest(maxqn, &newq);
		printf("Hmmm ... ");
		qn = maxqn;
	    } else {                               /* it's an old question */
		if (getbit(qmap, qn) == 0) {              /* was it asked? */
		    printf("Gee, thanks a lot!\n");
		    unlock();
		    exit(1);                   /* obviously a gross canard */
		}
	    }
	    a = askabout(newq.q_text, newb.b_name);
	    if (a == YES)
		setbit(newb.b_ans, qn);
	    else
		setbit(b.b_ans, qn);
	    putbeast(bn, &b);
	}
	putbeast(maxbn, &newb);
	printf("Very interesting.  I'll try to remember `%s'\n", newb.b_name);
	unlock();
	exit(0);
}

askyn(text)                           /* ask question and return YES or NO */
char    *text;
{
	char buf[32];
	int i;
	struct qstr qst;

	printf("%s? ", text);
	for (;;) {
	    i = read(0, buf, 32);
	    if (i <= 0)
		exit(1);
	    if (buf[0] == 'y')
		return(YES);
	    if (buf[0] == 'n')
		return(NO);
	    buf[i - 1] = '\0';
	    printf("`%s'?  Please answer yes or no.\n", buf);
	}
}

askabout(quest, beast)
char    *quest, *beast;
{
	register char *cp, *bp;
	char buf[96];

	if ((cp = instr(" it ", quest)) != 0) {
	    *cp = '\0';
	    sprintf(buf, "%s %s%s %s", quest, article(beast), beast, &cp[4]);
	} else if ((cp = instr(" it", quest)) != 0 && cp[3] == '\0') {
	    *cp = '\0';
	    sprintf(buf, "%s %s%s", quest, article(beast), beast);
	} else if ((cp = instr(" its ", quest)) != 0) {
	    *cp = '\0';
	    sprintf(buf, "%s %s%s's %s",
	     quest, article(beast), beast, &cp[5]);
	} else if ((cp = instr(" it's ", quest)) != 0) {
	    *cp = '\0';
	    sprintf(buf, "%s %s%s's %s",
	     quest, article(beast), beast, &cp[6]);
	} else
	    sprintf(buf, "How would you answer `%s' for %s%s",
	     quest, article(beast), beast);
	if (buf[0] >= 'a' && buf[0] <= 'z')
	    buf[0] &= ~040;                         /* capitalize question */
	return(askyn(buf));
}

nxtbeast(last, bp)                      /* read in the next possible beast */
struct  bstr    *bp;
{
	register int next;

	for (next = last + 1; next < maxbn; next++) {
	    if (getbit(bmap, next) == 1) {
		if (getbeast(next, bp) == -1) {
		    printf("Oops!  Couldn't read beast #%d\n", next);
		    exit(3);
		}
		return(next);
	    }
	}
	return(-1);
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

char    *
article(string)
char    *string;
{
	register char *sp, *cp;
	register int i;

	for (sp = string; *sp && *sp == ' '; sp++);
	if (*sp == '\0')
	    return("NOTHING?");
	if (artstrip(sp) != sp)                   /* article already there */
	    return("");
	i = *sp;
	if (i == 'a' || i == 'e' || i == 'i' || i == 'o' || i == 'u')
	    return("an ");
	else
	    return("a ");
}

char    *
artstrip(string)         /* return pointer to string with article stripped */
char    *string;
{
	register char *sp, *cp;
	register int i;

	for (sp = string; *sp && *sp == ' '; sp++);
	if (*sp == '\0')
	    return(string);
	cp = instr(" ", sp);
	if (cp != 0) {
	    if (instr("a ", sp) == sp
	     || instr("an ", sp) == sp
	     || instr("the ", sp) == sp
	     || instr("your ", sp) == sp)
		return(&cp[1]);
	}
	return(sp);
}

char    *
instr(pat, string)  /* return pointer to pat in string, ret 0 if not there */
char    *string;           /* NOTE: this routine makes `string' lower case */
{
	register char *sp, *pp, *bp;

	for (sp = string; *sp; sp++)                /* make all lower case */
	    if (*sp >= 'A' && *sp <= 'Z')
		*sp |= 040;
	for (sp = string; ; ) {
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

compare(ap, bp)
char    *ap, *bp;
{
	register char ac, bc;

	do {
	    ac = *ap++;
	    bc = *bp++;
	    if (ac >= 'A' && ac <= 'Z')
		ac |= 040;                  /* use all lower case */
	    if (bc >= 'A' && bc <= 'Z')
		bc |= 040;
	    if (ac != bc)
		return(ac - bc);
	} while (ac != '\0');
	return(0);
}

char    *
copy(from,to)
char *from, *to;
{
	register char *f, *t;

	f = from;
	t = to;
	while (*t++ = *f++);
	return(--t);
}

lock()
{
	register int try;

	if (locked == 0) {
	    for (try = 0; link(LOCKNODE, LOCKFILE) == -1; try++) {
		if (try++ == 0)
		    printf("Waiting for lock file ...\n");
		if (try > 10) {
		    printf("Lock file appears stuck.  Try again later?\n");
		    exit(3);
		}
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
