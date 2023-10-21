#
/*
 * Strings for switch code.
 */

#include "nc1h.c"
char dirsw[]
{"	cmpb	#@%o\n\
	beq	L%d\n\
	bls	L%d\n\
	jmp	L%d\n\
L%d	cmpa	#@%o\n\
	bls	L%d\n\
	jmp	L%d\n\
L%d	asla\n\
	rolb\n\
	ldx	#L%d\n\
	stx	c0\n\
	adda	c0+1\n\
	adcb	c0\n\
	staa	c0+1\n\
	stab	c0\n\
	ldx	c0\n\
	ldx	x\n\
	jmp	x\n\
L%d	equ	*\n"};

char simpsw[]
{"	stab	L%d\n\
	staa	L%d+1\n\
	ldx	#L%d-2\n\
L%d	inx\n\
	inx\n\
	cmpa	1,x\n\
	bne	L%d\n\
	cmpb	x\n\
	bne	L%d\n\
	inx\n\
	inx\n\
	stx	c0\n\
	ldaa	c0+1\n\
	ldab	c0\n\
	adda	L%d+1\n\
	adcb	L%d\n\
	staa	c0+1\n\
	stab	c0\n\
	ldx	c0\n\
	ldx	x\n\
	jmp	x\n\
L%d	fdb	L%d-L%d\n\
L%d	equ	*\n"};

pswitch(afp, alp, deflab)
struct swtab *afp, *alp;
{
	int tlab, ncase, i, j, tabs, worst, best, range;
	register struct swtab *swp, *fp, *lp;
	int poctab[swsiz];

	fp = afp;
	lp = alp;
	if (fp==lp) {
		printf("	jmp	L%d\n",deflab);
		return;
	}
	tlab = isn++;
	if (sort(fp, lp))
		return;
	ncase = lp-fp;
	lp--;
	range = lp->swval - fp->swval;
	/* direct switch */
	if (range>0 && range <= 3*ncase) {
		if (fp->swval)
			printf("\tsuba\t#@%o\n\tsbcb\t#@%o\n",
					fp->swval & 0377,
					(fp->swval >> 8) & 0377);
		printf(dirsw,(range>>8)&0377,tmplabel,tmplabel+1,deflab,
			tmplabel,range&0377,tmplabel+1,deflab,tmplabel+1,
			tmplabel+2,tmplabel+2);
		tmplabel =+ 3;
		isn++;
		for (i=fp->swval; i<=lp->swval; i++) {
			if (i==fp->swval) {
				printf("	fdb	L%d\n", fp->swlab);
				fp++;
			} else
				printf("	fdb	L%d\n", deflab);
		}
	}
	/* simple switch */
	else	{
		i = isn++;
		j = isn++;
		printf(simpsw,j,j,i,isn,isn,isn,tmplabel,tmplabel,tmplabel,j,i,i);
		tmplabel++;
		isn++;
		for (; fp<=lp; fp++)
			printf("	fdb	@%o\n", fp->swval);
		printf("L%d	rmb	2\n", j);
		for (fp = afp; fp<=lp; fp++)
			printf("	fdb	L%d\n", fp->swlab);
		printf("	fdb	L%d\n", deflab);
	}
	/* hash switch */
/*
	best = 077777;
	for (i=ncase/4; i<=ncase/2; i++) {
		for (j=0; j<i; j++)
			poctab[j] = 0;
		for (swp=fp; swp<=lp; swp++)
			poctab[lrem(0, swp->swval, i)]++;
		worst = 0;
		for (j=0; j<i; j++)
			if (poctab[j]>worst)
				worst = poctab[j];
		if (i*worst < best) {
			tabs = i;
			best = i*worst;
		}
	}
	i = isn++;
	printf(hashsw, tabs, isn, i, i, isn+tabs+1, isn+1, isn);
	isn++;
	for (i=0; i<=tabs; i++)
		printf("L%d\n", isn+i);
	for (i=0; i<tabs; i++) {
		printf("L%d:..\n", isn++);
		for (swp=fp; swp<=lp; swp++)
			if (lrem(0, swp->swval, tabs) == i)
				printf("%o\n", ldiv(0, swp->swval, tabs));
	}
	printf("L%d:", isn++);
	for (i=0; i<tabs; i++) {
		printf("L%d\n", deflab);
		for (swp=fp; swp<=lp; swp++)
			if (lrem(0, swp->swval, tabs) == i)
				printf("L%d\n", swp->swlab);
	}
*/
}

sort(afp, alp)
struct swtab *afp, *alp;
{
	register struct swtab *cp, *fp, *lp;
	int intch, t;

	fp = afp;
	lp = alp;
	while (fp < --lp) {
		intch = 0;
		for (cp=fp; cp<lp; cp++) {
			if (cp->swval == cp[1].swval) {
				error("Duplicate case (%d)", cp->swval);
				return(1);
			}
			if (cp->swval > cp[1].swval) {
				intch++;
				t = cp->swval;
				cp->swval = cp[1].swval;
				cp[1].swval = t;
				t = cp->swlab;
				cp->swlab = cp[1].swlab;
				cp[1].swlab = t;
			}
		}
		if (intch==0)
			break;
	}
	return(0);
}
