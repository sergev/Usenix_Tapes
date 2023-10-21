/* file for cdb etc */

#include "proc.h"

#define	DSP	0
#define	ISP	1
#define	TRACEWARN	30
#define	TRACETERM	50
int	symoff;
int	errflg;
int	symlen;
int	symcnt;
struct symbol	*symbuf;
int	*alpha;
int	*numeric;

#ifndef	BIG_UNIX
stackdecode(r5, r6, ka6)
#endif
#ifdef	BIG_UNIX
stackdecode(r5, r6, aka5, ka6)
#endif
int	r5, r6;
#ifdef	BIG_UNIX
int	aka5;
#endif
int	ka6;
{
	unsigned int	i, j;
	unsigned int	rr5, rr6;
	int	nxtrr5;
	int	intrpt;
	int	callev, narg, argp;
	int	lcls;
	int	inst, adr, callpt;
	struct symbol	*ss;
	struct sybmol	*indss;
	int	callmode;
	int	oldpc;
#ifdef	BIG_UNIX
	int	ka5, nxtka5;
	int	knarg;
	int	paged;
#endif

	printf(S_PTAB, "Stack trace - last called routines appear first\n\n");
	printf(S_PTAB, "Calling routine ( arguments )\n");
	printf(SS_PTAB, "local variables - if any\n\n");

	DEBUG
		printf("stackdecode called with r5=%u (%o), r6=%u (%o), ka6=%u (%o)\n",
			r5, r5, r6, r6, ka6, ka6);

	rr5 = r5;
	rr6 = r6;
#ifdef	BIG_UNIX
	ka5 = aka5;
#endif
	callev = 0;
	errflg = 0;
	oldpc = 0;

	do {
		if (u.u_qsav[1] == rr5)
			printf(S_PTAB, "  <------ u.u_qsav\n");
		if (u.u_ssav[1] == rr5)
			printf(S_PTAB, "  <------ u.u_ssav\n");

		nxtrr5 = get(rr5, DSP, ka6);
		callpt = get(rr5+2, DSP, ka6);
#ifdef	BIG_UNIX
		inst = get(callpt-4, ISP, ka5);
		adr = get(callpt-2, ISP, ka5);
#endif	BIG_UNIX
#ifndef	BIG_UNIX
		inst = get(callpt-4, ISP, txtoff);
		adr = get(callpt-2, ISP, txtoff);
#endif	BIG_UNIX

#ifdef	BIG_UNIX
		paged = 0;
		nxtka5 = ka5;
		if (adr==04730 && inst==KISA5) {
			/* paged kernel transfer */
			paged++;
			printf(S_PTAB, "\t     V\b+--- paged transfer\n");
			inst = get(callpt, ISP, ka5);
			if (inst == 012637)		/* mov (sp)+,*$... */
				knarg = 0;
			else if (inst == 05726)		/* tst (sp)+ */
				knarg = 1;
			else if (inst == 022626) {
				/* cmp (sp)+,(sp)+ */
				inst = get(callpt+8, ISP, ka5);
				if (inst == 011607)	/* mov (sp),pc */
					knarg = 2;
				else
					knarg = (get(callpt+10, ISP, ka5) + 4)/2;
			} else {
				errflg++;
				break;
			}
			callpt = get(rr5+4+(knarg==0 ? 2 : knarg*2), DSP, ka6);
			nxtka5 = get(rr5+4+(knarg==0 ? 0 : (knarg+1)*2), DSP, ka6);
			inst = get(callpt-4, ISP, nxtka5);
			adr = get(callpt-2, ISP, nxtka5);
		}
#endif
		intrpt = 0;
		DEBUG
			printf("stackdecode first test ins=%u (%o), adr=%u (%o), callpt=%u (%o), txtoff=%u (%o)\n",
				inst, inst, adr, adr, callpt, callpt, txtoff, txtoff);
		if (adr == 04730) {
			/* jsr pc,*(r0)+ */
			intrpt++;
#ifdef	BIG_UNIX
			narg = paged ? knarg : 7;
#endif	BIG_UNIX
#ifndef	BIG_UNIX
			narg = 7;
#endif	BIG_UNIX
			adr = oldpc;
		} else {
			if (inst == 04737)			/* jsr pc,*$... */
				narg = 1;
			else if ((inst & ~077) == 04700)	/* jsr pc,... */
				narg = 0;
			else {
				errflg++;
				break;
			}

			callmode = 0;
			if (inst == 04767)		/* jsr pc,... */
				adr =+ callpt;
			else if (inst == 04777) {
				/* jsr pc,*... */
				callmode = 1;
				adr =+ callpt;
#ifdef	BIG_UNIX
				indss = vallook(adr, nxtka5);
#endif	BIG_UNIX
#ifndef	BIG_UNIX
				indss = vallook(adr);
#endif	BIG_UNIX
				adr = get(adr, DSP, ka6);
			} else if (inst == 04775) {
				/* jsr pc,*...(r5) */
				callmode = 2;
				adr = get(adr+nxtrr5, DSP, ka6);
			} else if ((inst & ~07) == 04770) {
				/* jsr pc,*...(?) */
				callmode = 3;
				adr = oldpc;
			}
#ifdef	BIG_UNIX
			if (paged && callmode<3) {
				i = get(adr+4, ISP, ka5);
				if (i != ka5)
					printf(S_PTAB, "-->ka5 discrepancy; vector=%o; stack=%o\n", i, ka5);
				adr = get(adr+6, ISP, ka5);
			}
			inst = get(callpt, ISP, nxtka5);
#endif	BIG_UNIX
#ifndef	BIG_UNIX
			inst = get(callpt, ISP, txtoff);
#endif	BIG_UNIX
			if (inst == 05726)		/* tst (sp)+ */
				narg++;
			else if (inst == 022626)	/* cmp (sp)+,(sp)+ */
				narg =+ 2;
			else if (inst == 062706)	/* add $n,sp */
#ifndef	BIG_UNIX
				narg =+ get(callpt+2, ISP, txtoff)/2;
#endif	BIG_UNIX
#ifdef	BIG_UNIX
				narg =+ get(callpt+2, ISP, nxtka5)/2;
			if (paged && knarg!=narg) {
				printf(S_PTAB, " difference in paged argument count and real argument count\n");
				printf(S_PTAB, "  real = %d.; paged = %d.\n", narg, knarg);
				printf(S_PTAB, " paged count shall be used\n");
				narg = knarg;
			}
#endif	BIG_UNIX
		}

		DEBUG
			printf("narg=%u (%o), rr5=%u (%o)\n", narg, narg, rr5, rr5);


		/*
		 * print call and arguments
		 */
#ifdef	BIG_UNIX
		ss = vallook(adr, ka5);
#endif	BIG_UNIX
#ifndef	BIG_UNIX
		ss = vallook(adr);
#endif	BIG_UNIX
		printf(S_PTAB, "%2d: %8.8s (", callev, ss->s_symbol);
		for (i = 0; i < narg; i++) {
			if (i > 0)
				printf(", ");
			printf("%.1o", get(rr5+4 + 2*i, DSP, ka6));
		}
		switch (callmode) {

		case 0:
			printf(")\n");
			break;

		case 1:
			printf(") indirect call via %8.8s\n", indss->s_symbol);
			break;

		case 2:
			printf(") called via higher arg list\n");
			break;

		case 3:
			printf(") called via array\n");
			adr = ss->s_symval;
			break;
		}

		/*
		 * list locals to called routine
		 */
		lcls = 0;
#ifdef	BIG_UNIX
		if (get(adr, ISP, ka5) == 04567) {
			/* jsr r5,csv */
			inst = get(adr+4, ISP, ka5);
			if (inst == 05746)		/* tst -(sp) */
				lcls = 1;
			else if (inst == 0162706)	/* sub $n,sp */
				lcls = get(adr+6, ISP, ka5) / 2;
		}
#endif	BIG=UNIX
#ifndef	BIG_UNIX
		if (get(adr, ISP, txtoff) == 04567) {
			/* jsr r5,csv */
			inst = get(adr+4, ISP, txtoff);
			if (inst == 05746)		/* tst -(sp) */
				lcls = 1;
			else if (inst == 0162706)	/* sub $n,sp */
				lcls = get(adr+6, ISP, txtoff) / 2;
		}
#endif	BIG_UNIX
		if (lcls > 0) {
			printf(SS_PTAB, "{");
			for (i = 0; i < lcls; i++) {
				if (i > 0)
					printf(", ");
				printf("%.1o", get(rr5-6 - 2*i, DSP, ka6));
			}
			printf("}\n");
		}
		printf(SS_PTAB, "%60tsaved registers:");
		for (i = 2; i <= 5; i++) {
			j = get(rr5 + (i - 5)*2, DSP, ka6);
			if (!(i & 1))
				printf("%78t");
			printf(" r%d = %u (%o);", i, j, j);
			if (i & 1)
				printf("\n");
		}

		if (callev == TRACEWARN)
			WARNING
				warn("stack trace level rather large!");
		if (callev++ == TRACETERM) {
			WARNING
				warn("stack trace looping - terminated!");
			errflg++;
		}

#ifdef	BIG_UNIX
		if (paged)
			ka5 = nxtka5;
#endif	BIG_UNIX
		if (intrpt) {
			i = rr5 + 14 + 4;
#ifdef	BIG_UNIX
			if (paged)
				i =+ 4;
#endif	BIG_UNIX
			if (i >= USERADR+04000)
				break;
		} else {
			if (rr5+4 >= USERADR+04000)
				break;
		}
		rr5 = nxtrr5;
		if (rr5 >= USERADR+04000)
			break;
		oldpc = callpt;

		if (rr5 >= USERADR+04000)
			break;
		DEBUG
			printf("at end of loop new rr5=%u (%o)\n", rr5, rr5);
	} while (!errflg);

	printf("\n\n");
}

/***********************/

get(adr, space, ka56)
unsigned adr;
int	space;
int	ka56;
{
	long	bign;
	int	word;

	DEBUG
		printf("get passed adr=%u (%o), space=%u (%o), ka56=%u (%o)\n", adr, adr, space, space, ka56, ka56);

	bign = ka56.unsign;
	bign =<< 6;	/* 64 bytes/block */
	if (space == DSP) {
		lseek(corefd, bign, 0);
		seek(corefd, adr - USERADR, 1);
	} else {
#ifdef	BIG_UNIX
		if (adr >= PAGEADR) {
			lseek(corefd, bign, 0);
			seek(corefd, adr - PAGEADR, 1);
		} else
#endif	BIG_UNIX
#ifdef	_1170
		if(adr >= FULLPAGE)
			{
			lseek(corefd, bign, 0);
			seek(corefd, adr - FULLPAGE, 1);
			}
		  else
			seek(corefd, adr, 0);
#endif	_1170
#ifdef	BIG_UNIX | _1140
			seek(corefd, adr, 0);
#endif	BIG_UNIX | _1140
	}

	if (read(corefd, &word, 2) != 2)
		errflg++;
	return(word);
}

/****************************/

#ifdef	BIG_UNIX
vallook(value, ka5)
#endif	BIG_UNIX
#ifndef	BIG_UNIX
vallook(value)
#endif	BIG_UNIX
unsigned value;
#ifdef	BIG_UNIX
int	ka5;
#endif	BIG_UNIX
{
#ifdef	BIG_UNIX
	return(getnum(value, ka5));
#endif	BIG_UNIX
#ifndef	BIG_UNIX
	return(getnum(value));
#endif	BIG_UNIX
}

/************************/

numcomp(a, b)
int *a, *b;
{
	register struct symbol	*sa, *sb;

	sa = &symbuf[*a];
	sb = &symbuf[*b];
	if (sa->s_symval < sb->s_symval)
		return(-1);
	if (sa->s_symval > sb->s_symval)
		return(1);
#ifdef	BIG_UNIX
	if (sa->s_symseg < sb->s_symseg)
		return(-1);
	if (sa->s_symseg > sb->s_symseg)
		return(1);
#endif	BIG_UNIX
	return(0);
}

/****************************/

alcomp(a, b)
int	*a, *b;
{
	return(alcom(symbuf[*a].s_symbol, symbuf[*b].s_symbol));
}

/************************/

symsort()
{
	/*
	 *	this procedure verifys the validity of the file
	 *	and having done this creates:
	 *
	 *	symbuf[] - an array of symbol table entry structures
	 *		   read from the namelist file.
	 *	alpha[] -  alpha ordered index into symbuf
	 *	numeric[] - numerically ordered index into symbuf
	 *
	 *	and for 70s notes where the text REALLY is
	 *
	 */

	/* declarations */

	register unsigned	i;

	seek(namefd, 0, 0);
	read(namefd, regbuf, 16);

	if (regbuf[0]!=0407 && regbuf[0]!=0410 && regbuf[0]!=0411)
		crash("namelist file - bad format!");

	symoff = regbuf[1] + regbuf[2];
	symlen = regbuf[4];
	if (symlen == 0)
		crash("namelist stripped!");
	if (regbuf[7] != 1)
		symoff =<< 1;
	symoff =+ 16;
	seek(namefd, symoff, 0);

	/* get core for it */

	symcnt = symlen / 6;

	if ((alpha = alloc(symcnt)) < 0)
		crash("no core for alpha!");
	if ((numeric = alloc(symcnt)) < 0)
		crash("no core for numeric!");

	symcnt = symlen / 12;

	/* get core for symbol buffer */

	if ((symbuf = alloc(symlen)) < 0)
		crash("no core for symbuf!");

	/* now initialise vectors and symbol table */

	for (i = 0; i < symcnt; i++) {
		alpha[i] = i;
		numeric[i] = i;
	}

	if (read(namefd, symbuf, symlen) < symlen)
		crash("cannot read whole symbuf!");

	/* now for a quicksort or two */

	qsort(alpha, symcnt, 2, alcomp);
	qsort(numeric, symcnt, 2, numcomp);
#ifdef	_1170
	if((i = albin("_end\0\0\0\0")) == 0)
		{
		crash("cant find \"_end\"!");
		}
	  else
		{
		txtoff = ((i->s_symval+63) >> 6) & 077777;
		}
	DEBUG
		printf("symsort 70 text offset=%u (%o)\n",
			txtoff, txtoff);
#endif	_1170
#ifdef	BIG_UNIX
	segsort();
#endif

	/* assuming they both worked, that is all */

}

/****************************/

#ifdef	BIG_UNIX
char	segsymbol[] "..seg000";

segsort()
{
	register int	i;
	register struct symbol	*sp;
	
	for (i = 0; i < MAXSEG; i++) {
		segsymbol[7] = (i+1) % 10 + '0';
		segsymbol[6] = (i+1)/10%10 + '0';
		segsymbol[5] = (i+1)/100%10 + '0';
		sp = albin(segsymbol);
		if (sp == 0)
			break;
		segbase[i] = sp->s_symval;
	}
	maxseg = i;
}
#endif

/****************************/

albin(string)
char	*string;
{
	/*
	 *	given a "string" (8-chars, null padded) this routine
	 *	will return a pointer to a structure of type "symbol"
	 *	whose symbol field matches the passed "string".
	 *	Otherwise will return null (0);
	 */

	register	i, j, k;
	int	r;

	i = 0;
	j = symcnt;

	while (i != j-1) {
		k = (i+j) / 2;
		DEBUG
			printf("albin loop: i=%u, j=%u, k=%u\n", i, j, k);
		DEBUG
			printf("alcom(%8.8s, %8.8s) ", string, symbuf[alpha[k]].s_symbol);
		r = alcom(string, symbuf[alpha[k]].s_symbol);
		if (r < 0)
			j = k;
		else
			i = k;
		DEBUG
			printf("=> %d\n", r);
	}
	if (alcom(string, symbuf[alpha[i]].s_symbol) == 0)
		return(&symbuf[alpha[i]]);

	return(0);
}

/***************************/

#ifndef	BIG_UNIX
numbin(a)
#endif
#ifdef	BIG_UNIX
numbin(a, ka5)
#endif
unsigned a;
#ifdef	BIG_UNIX
int	ka5;
#endif
{
	/*
	 *	given a value "a" will return a pointer to a structure
	 *	of type "symbol" whose "value" field is
	 *	exactly equal to the value passed.
	 *	otherwise a pointer to the structure
	 *	with the next lowest value is returned.
	 *	if no lower structure exists, null is returned (0).
	 *
	 *	A non-zero value of ka5 will cause us to look
	 *	in the appropriate page for symbols.
	 */

	register	i, j;
	int	k;
#ifdef	BIG_UNIX
	int	seg;
#endif
	int	r;
	register struct symbol	*sp;

#ifdef	BIG_UNIX
	seg = 0;
	if (a<PAGEADR || a>=PAGEADR+020000)
		ka5 = 0;
	if (ka5 != 0) {
		for (i = 0; i < maxseg; i++)
			if (ka5 == segbase[i]) {
				seg = i+1;
				break;
			}
	}

	DEBUG
		printf("numbin: a=%u, seg=%u, ka5=%u\n", a, seg, ka5);
#endif

	i = 0;
	j = symcnt;

	while (i != j-1) {
		k = (i+j) / 2;
		sp = &symbuf[numeric[k]];
		DEBUG {
			printf("numbin loop: i=%u, j=%u, k=%u\n", i, j, k);
#ifndef	BIG_UNIX
			printf("\t\ts_symval=%u\n", sp->s_symval);
#endif
#ifdef	BIG_UNIX
			printf("\t\ts_symval=%u, s_symseg=%u\n", sp->s_symval, sp->s_symseg);
#endif
		}
		if (a < sp->s_symval)
			j = k;
#ifdef	BIG_UNIX
		else if (a > sp->s_symval)
			i = k;
		else if (seg == 0)
			i = k;
		else if (seg < sp->s_symseg)
			j = k;
#endif
		else
			i = k;
	}

	sp = &symbuf[numeric[i]];
	if (a == sp->s_symval)
#ifdef	BIG_UNIX
		if (seg==0 || seg==sp->s_symseg)
#endif
			return(sp);

	if (i == 0)
		return(0);
	/* above should be checked by calling program */

#ifdef	BIG_UNIX
	if (seg != 0)
		/*
		 * It is a far better thing I do to find an even
		 * more distant symbol that is at least in the
		 * same page of memory.
		 */
		for (j = i; j != 0; j--) {
			DEBUG
				printf("numbin: segment test: s_symseg=%u, s_symval=%u\n",
					symbuf[numeric[j]].s_symseg,
					symbuf[numeric[j]].s_symval);
			if (seg == symbuf[numeric[j]].s_symseg) {
				sp = &symbuf[numeric[j]];
				break;
			}
		}
#endif

	return(sp);
}

/***************************/

alcom(a, b)
{
	register	r;
	register char	*p, *q;

	p = a;
	q = b;

	for (r = 0; r < 8; r++) {
		if (*p < *q)
			return(-1);
		if (*p++ > *q++)
			return(1);
	}
	return(0);
}

/*************************/

#ifdef	BIG_UNIX
getnum(adr, ka5)
#endif	BIG_UNIX
#ifndef	BIG_UNIX
getnum(adr)
#endif	BIG_UNIX
unsigned adr;
#ifdef	BIG_UNIX
int	ka5;
#endif	BIG_UNIX
{
	register struct symbol	*p;

#ifdef	BIG_UNIX
	p = numbin(adr, ka5);
#endif	BIG_UNIX
#ifndef	BIG_UNIX
	p = numbin(adr);
#endif	BIG_UNIX
	if (p == 0)
		p = &nullnum;

	return(p);
}

/*********************/

getname(string)
char	*string;
{
	return(albin(string));
}
