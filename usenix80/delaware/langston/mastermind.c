#
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *
		Created by Peter Langston

		   All rights reserved

Compile: cc -O -n -q mastermind.c ; mv a.out mastermind

Copyright (c) 1977 by Peter S. Langston - New  York,  N.Y.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#define BITMAPSIZE 4095

char *whatline "@(#)mastermind.c	1.2   Mastermind last changed 1/29/79 -- psl games";

char	*symbol	"abcdefghijklmnopqrstuvwxyz", maxsym;
char    patbuf[64], gesbuf[64];
int     npos, nsym, hguesses, cguesses;
int     poss[BITMAPSIZE], maxmov, kibflg 1;
float   maxpmov;
int	bitz[] {
	01, 02, 04, 010, 020, 040, 0100, 0200, 0400, 01000,
	02000, 04000, 010000, 020000, 040000, 0100000
};

char    *fmts[] {
	"How about %s? ",
	"I'll try %s -- ",
	"What would you say to %s? ",
	"Perhaps your pattern is %s? ",
};

struct	stack {
	int	s_guess;
	char	s_b;
	char	s_w;
} stack[100];

main(argc, argv)
char    *argv[];
{
	register int i, gess;
	char pat[64], p[64], *cp, *pp, b, w;
	struct stack *sp;

	greet();
	if (argc > 1) {
	    if (argv[1][0] == 'q')
		kibflg = 0;
	    else if (argv[1][0] == 'v')
		kibflg = 2;
	    else
		printf("Usage: %s [ quiet | verbose ]\n", argv[0]);
	}
	maxpmov = 16. * BITMAPSIZE;
	time(pat);
	srand(pat[3]);
	maxmov = getsize();
playagain:
	for (i = (maxmov + 15) >> 4; --i >= 0; )
		poss[i] = 0;
	hguesses = cguesses = 0;

	makepat(pat);
	printf("Okay, I have my pattern; your ");
	for (sp = stack; ; sp++) {
		hguesses++;
		do {
			cp = getchrs("guess : ");
			if (*cp == '?') {
			    printf("My pattern was %s\n", pat);
			    hguesses =+ 10;
			    goto gave_up;
			}
			sp->s_guess = encode(cp);
		} while (sp->s_guess < 0);
		copy(pat, p);
		b = blacks(p, cp);
		w = whites();
		sp->s_b = b;
		sp->s_w = w;
		if (kibflg)
			kibbitz(sp);
		printf("%d in the right-position and", b);
		printf(" %d in the wrong position (%d, %d)\n", w, b, w);
		if (b == npos)
			break;
	}
	printf("You got it in %d guess%s, ", hguesses, esplur(hguesses));
	if (hguesses < npos + nsym)
		printf("nice going!\n");
	else
		printf("big deal...\n");
gave_up:
	getchrs("My turn to guess.  Think of a pattern and then hit return...");
	for (sp = stack; ; sp++) {
		sp->s_guess = trymove();
		if (sp->s_guess == -1)
			cheater(sp);
		cguesses++;
		getbw(sp);
		if (sp->s_b == npos)
			break;
		zap(sp);
	}
	printf("I got it, and in only %d guess%s!  Wow!\nPlay again",
	 cguesses, esplur(cguesses));
	cp = getchrs(cguesses >= hguesses ? "? " : ", sucker? ");
	if (*cp == 'y')
		goto playagain;
	printf("Okay, don't be so touchy, geeeeeez!\n");
	exit(0);
}

getsize()
{
	register int i, j;
	double log(), exp(), lsym, lmaxp, d;

	lmaxp =  log(maxpmov);
tryagain:
	npos = getint("Number of positions? ");
	i = lmaxp / log(2.);
	if (npos < 1 || npos > i) {
	    printf("Number of positions must be in the range 1 : %d\n", i);
	    goto tryagain;
	}
	d = exp(lmaxp / npos);
	i = d > 26? 26 : d;
	printf("Number of symbols? (max %d) ", i);
	nsym = getint("");
	if (nsym < 1 || nsym > i) {
	    printf("Number of symbols must be in the range 1 to %d\n", i);
	    goto tryagain;
	}
	lsym = log(lsym = nsym);
	if (exp(npos * lsym) > maxpmov || nsym > 26) {
	    printf("Oops!  Too many -- try something a little smaller.\n");
	    goto tryagain;
	}
	maxsym = 'a' + nsym - 1;
	printf("For this game we will use the letters a - %c\n", maxsym);
	return(i = exp(npos * lsym));
}

makepat(p)
char *p;
{
	register int i;

	for (i=0; i<npos; i++)
		*p++ = symbol[rand() % nsym];
	*p = 0;
}

trymove()
{
	register int i, j;

	i = j = rand() % maxmov + 1;
	do {
		if (getbit(i) == 0)
			return(i);
		if (i == 0) {
			i = maxmov;
			printf("hm%s... ", &"mmmmmmmmmmmmmmmm"[rand()%16]);
		}
	} while (--i != j);
	return(-1);
}

getbit(i)
{
	return(poss[(i >> 4) & 4095] & bitz[i & 15]);
}

setbit(i)
{
	poss[(i >> 4) & 4095] =| bitz[i & 15];
}

getbw(sp)
struct	stack *sp;
{
	char *cp, *pp;

	for (;;) {
		printf(fmts[(rand()>>3) & 3], decode(sp->s_guess));
		pp = cp = getchrs("");
		sp->s_b = atoip(&cp);
		if (*cp++ != ',') {
			printf("%s?\tformat is #-right-position, #-wrong-position\n",
			 pp);
			continue;
		}
		sp->s_w = atoi(cp);
		if (sp->s_b + sp->s_w <= npos)
			return;
		printf("Remember that the total of those two numbers can't exceed %d!\n", npos);
	}
}

zap(sp)
struct	stack *sp;
{
	register char *pp, *ppi;
	register int i;
	char buf[64], *gp;
	int b, w;

	gp = decode(sp->s_guess);
	b = sp->s_b;
	w = sp->s_w;
	for (pp = &buf[npos]; pp > buf; )
		*--pp = 'a';
	i = -1;
	ppi = &buf[npos - 1];
lop:
	if (getbit(++i) == 0)
		if (blacks(gp, buf) != b || whites() != w)
			setbit(i);
	pp = ppi;
	while ((*pp)++ == maxsym) {
		*pp = 'a';
		if (--pp < buf)
			return;
	}
	goto lop;
}

decode(i)                               /* turn code into chars */
{
	register char *cp;
	register int n;
	static char cpt[64];

	n = i;
	cp = &cpt[npos];
	*cp = '\0';
	while (cp > cpt) {
		*--cp = 'a' + (n % nsym);
		n =/ nsym;
	}
	return(cp);
}

encode(pp)                              /* turn chars into code */
char    *pp;
{
	register char *cp;
	register int n;

	cp = pp;
	for (n = 0; cp[n] != 0; n++);
	if (n != npos)
	    return(-1);
	for (n = 0; *cp; )
	    n = n * nsym + *cp++ - 'a';
	return(n);
}

blacks(p, cp)
char *p, *cp;
{
	register char *ipp, *igp, *pp;
	char *gp;
	int nb;

	ipp = p;
	igp = cp;
	pp = patbuf;
	gp = gesbuf;
	nb = 0;
	do {
		if (*ipp++ == *igp++) {
			nb++;
			*pp++ = *gp++ = 1;
		} else {
			*pp++ = ipp[-1];
			*gp++ = igp[-1];
		}
	} while (*ipp && *igp);
	return(nb);
}

whites()
{
	register char c, d, *ccp;
	char *pp, *gp;
	int nw;

	pp = patbuf;
	gp = gesbuf;
	nw = 0;
	while (c = *pp++) {
		if (c < 'a')
			continue;
		ccp = gp;
		while (d = *ccp++) {
			if (c == d) {
				*--ccp = ++nw;
				break;
			}
		}
	}
	return(nw);
}

getint(cp)
char *cp;
{
	char buf[20];

	printf(cp);
	read(0, buf, 20);
	return(atoi(buf));
}

getchrs(cp)
char *cp;
{
	static char buf[80];

	printf(cp);
	buf[read(0, buf, 80)-1] = 0;
	return(buf);
}

cheater(osp)
struct	stack *osp;
{
	/* most likely the human has cheated... prove it */
	register char *cp, *gpat;
	char p[64];
	int b, w;
	struct stack *sp;

	cp = getchrs("Gosh, I give up; what was your pattern? ");
	for (sp = stack; sp < osp; sp++) {
		copy(cp, p);
		gpat = decode(sp->s_guess);
		b = blacks(p, gpat);
		w = whites();
		if (b == sp->s_b && w == sp->s_w)
			continue;
		printf("Perhaps you remember saying that my guess of %s", gpat);
		printf(" was a %d, %d?\n", sp->s_b, sp->s_w);
		printf("Well, it should have been a %d, %d!\n", b, w);
	}
	printf("I think I'll play with someone else for a while...\n");
	exit(1);
}

kibbitz(osp)
struct	stack *osp;
{
	char p[64], pp[64], *gpat;
	int b, w;
	struct stack *sp;

	copy(decode(osp->s_guess), pp);;
	for (sp = stack; sp < osp; sp++) {
	    copy(pp, p);
	    gpat = decode(sp->s_guess);
	    b = blacks(p, gpat);
	    w = whites();
	    if (b == sp->s_b && w == sp->s_w)
		continue;
	    printf("Dumb guess...\n");
	    if (kibflg > 1) {
		printf("If my pattern were %s then your guess of %s",
		 pp, gpat);
		printf(" would have been a %d, %d!\n", b, w);
	    }
	    break;
	}
	return;
}

greet()
{
	register char *cp;

	printf("\n\tWelcome to Mastermind\n");
	cp = getchrs("Instructions? ");
	if (*cp == 'n')
		return;
	printf("\nThe game of Mastermind involves discovering a hidden");
	printf(" pattern by making guesses\nand learning how close those");
	printf(" guesses come to the pattern.  It is an analog to\nthe");
	printf(" \"Scientific Method\" where the guesses are the experiments");
	printf(" performed and\nthe replies are the experimental results");
	printf(" which confirm or refute hypotheses.\n\n");
	printf("In this version the patterns and guesses are coded as");
	printf(" strings of characters;\ne.g.    guess : abcc\n\n");
	printf("The replies are coded as a number pair representing");
	printf(" the number of symbols in\nthe right position");
	printf(" followed by the number of symbols in the wrong position;\n");
	printf("e.g. if the pattern were \"ebab\" the reply to a guess of");
	printf(" \"abcc\" would be 1,1\nindicating 1 in the right");
	printf(" position (-b--) and 1 in the wrong position (--a-).\n");
	printf("(Note that 4,0 would then be a win in a 4 position game.)\n\n");
	printf("First the computer picks a pattern and you guess,");
	printf(" (use \"?\" for your guess if\nyou want to give up),");
	printf(" and then you pick a pattern and the computer guesses.\n\n");
	printf("When the computer thinks you have wasted a guess it says");
	printf(" \"Dumb guess\".\nIf you run \"mastermind verbose\"");
	printf(" it will tell you why.\nIf you run \"mastermind quiet\"");
	printf(" it will refrain from all kibbitzing.\n\n");
	printf("Good luck...\n\n");
}

/*
** Atoip - convert from octal or decimal string to integer.
**	- use pointer to pointer to chars and update it.
*/

atoip(ptrptr)
char **ptrptr;
{
	register num, base;
	register char *cp;
	int neg;

	cp = *ptrptr;
	num = 0;
	base = 10;
	neg = 0;
loop:
	while (*cp == ' ' || *cp == '\t')
		cp++;
	if (*cp == '-') {
		neg++;
		cp++;
		goto loop;
	}
	if (*cp == '0') {
		base = 8;
		cp++;
	}
	while (*cp >= '0' && *cp <= '9')
		num = num * base + *cp++ - '0';
	*ptrptr = cp;
	return(neg ? -num : num);
}

splur(n)
{
	return(n==1? "" : "s");
}

esplur(n)
{
	return(n==1? "" : "es");
}

iesplur(n)
{
	return(n==1? "y" : "ies");
}

copy(from, to)
char *from, *to;
{
	register char *f, *t;

	f = from;
	t = to;
	while (*t++ = *f++);
	return(--t);
}

min(a, b)
{
     return(a < b? a : b);
}
