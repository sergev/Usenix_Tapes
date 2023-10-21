#
/*
 *  C compiler
 */

#include "nc1h.c"

max(a, b)
{
	if (a>b)
		return(a);
	return(b);
}

degree(at)
struct tnode *at;
{
	register struct tnode *t, *t1;

	if ((t=at)==0 || t->op==0)
		return(0);
	if (t->op == CON)
		return(-3);
	if (t->op == AMPER)
		return(-2);
	if (t->op==ITOL && (t1 = isconstant(t)) && t1->value>= 0)
		return(-2);
	if ((opdope[t->op] & LEAF) != 0) {
		if (t->type==CHAR || t->type==FLOAT)
			return(1);
		return(0);
	}
	return(t->degree);
}


prins(op, c, itable)
struct instab *itable;
{
	register struct instab *insp;
	register char *ip;

	for (insp=itable; insp->op != 0; insp++) {
		if (insp->op == op) {
			ip = c? insp->str2: insp->str1;
			if (ip==0)
				break;
			printf("%s", ip);
			return;
		}
	}
	error("No match' for op %d", op);
}


isfloat(at)
struct tnode *at;
{
	register struct tnode *t;

	t = at;
	if ((opdope[t->op]&RELAT)!=0)
		t = t->tr1;
	if (t->type==FLOAT || t->type==DOUBLE) {
		nfloat = 1;
		return('f');
	}
	return(0);
}


ispow2(atree)
{
	register int d;
	register struct tnode *tree;

	tree = atree;
	if (!isfloat(tree) && tree->tr2->op==CON) {
		d = tree->tr2->value;
		if (d>1 && (d&(d-1))==0)
			return(d);
	}
	return(0);
}

pow2(atree)
struct tnode *atree;
{
	register int d, i;
	register struct tnode *tree;

	tree = atree;
	if (d = ispow2(tree)) {
		for (i=0; (d=>>1)!=0; i++);
		tree->tr2->value = i;
		d = tree->op;
		tree->op = d==TIMES? LSHIFT:
			  (d==DIVIDE? RSHIFT:
			  (d==ASTIMES? ASLSH: ASRSH));
		tree = optim(tree);
	}
	return(tree);
}

branch(lbl, aop, c)
{
	register op;

	/*if(op=aop)
		prins(op, c, branchtab);
	else
		*/printf("	jmp");
	printf("\tL%d\n", lbl);
}

label(l)
{
	printf("L%d	equ	*\n", l);
}

error(s, p1, p2, p3, p4, p5, p6)
{
	register f;
	extern fout;

	nerror++;
	flush();
	f = fout;
	fout = 1;
	printf("%d: ", line);
	printf(s, p1, p2, p3, p4, p5, p6);
	putchar('\n');
	flush();
	fout = f;
}

/*
 * Read in an intermediate file.
 */
getree()
{
	struct tnode *expstack[20];
	register struct tnode **sp;
	register t, op;
	static char s[9];
	struct swtab *swp;

	int *xxx;
	spacep = treespace;
	sp = expstack;
	for (;;) {
		if (sp >= &expstack[20])
			error("Stack botch");
		op = getw(ascbuf);
		if ((op&0177400) != 0177000) {
			error("Intermediate file error");
			diag("op = %o\n",op);
			flush();
			exit1(1,159);
		}
		switch(op =& 0377) {

	case EOF:
		return;

	case BDATA:
		printf("	fcb\t");
		seq(',');
		break;

	case WDATA:
		printf("	fdb\t");
		seq(',');
		break;

	case PROG:
		break;

	case DATA:
		break;

	case BSS:
		break;

	case SYMDEF:
		printf("**\n");  /* signal to optimizer  */
		outname(s);
		break;

	case RETRN:
		printf("	jmp	cret\n");
		break;

	case CSPACE:
		t = outname(s);
		printf("_%s	rmb	@%o\n",t, getw(ascbuf));
		break;

	case SSPACE:
		printf("	rmb	@%o\n", getw(ascbuf));
		break;

	case EVEN:
		break;

	case SAVE:
		t = getw(ascbuf);   /*  -6;  */
		autosize = t + 6;
		printf("	ldx	#@%o\n", t-1);
		printf("	jsr	csv\n");
		xvalid = 0;
		break;

	case PROFIL:
		error("profiling not allowed");
		exit1(1,220);

	case SNAME:
		t = outname(s);
		printf("~%s	equ	L%d\n", t, getw(ascbuf));
		break;

	case ANAME:
		t = outname(s);
		printf("~%s	equ	@%o\n", t, getw(ascbuf));
		break;

	case RNAME:
		t = outname(s);
		printf("~%s	equ	%d\n", t, (getw(ascbuf) - 2) * 2);
		break;

	case SWIT:
		t = getw(ascbuf);
		line = getw(ascbuf);
		swp = treespace;
		while (swp->swlab = getw(ascbuf)) {
			swp->swval = getw(ascbuf);
			swp++;
		}
		pswitch(treespace, swp, t);
		break;

	case EXPR:
		line = getw(ascbuf);
		if (sp != &expstack[1]) {
			error("Expression input botch");
			exit1(1,252);
		}
		/*diag("sp=%o\n",*(sp-1));
		for (xxx=treespace; xxx<treespace+50; xxx++)
			diag("%o:%o\n",xxx,*xxx);*/
		nstack = 0;
		printf("*  line %d\n",line);
		ccode(optim(*--sp));
		spacep = treespace;
		break;

	case NAME:
		t = getw(ascbuf);
		if (t==EXTERN) {
			t = getw(ascbuf);
			*sp = block(6, NAME, t, 0, EXTERN, 0, 0,0,0,0);
			outname(&(*sp)->nloc);
			sp++;
			break;
		}
		*sp = block(3, NAME, 0, 0, t, 0, 0);
		(*sp)->type = getw(ascbuf);
		(*sp)->nloc = getw(ascbuf);
		sp++;
		break;

	case FCON:
	case SFCON:
		error("no floating point\n");
		exit1(1,277);

	case CON:
		t = getw(ascbuf);
		*sp++ = block(1, op, t, 0, getw(ascbuf));
		break;

	case FSEL:
		t = getw(ascbuf);
		sp[-1] = block(2, op, t, 0, sp[-1], getw(ascbuf));
		break;

	case NULL:
		*sp++ = block(0, 0, 0, 0);
		break;

	case CBRANCH:
		t = getw(ascbuf);
		sp[-1] = block(1, CBRANCH, t, getw(ascbuf), sp[-1]);
		break;

	case LABEL:
		label(getw(ascbuf));
		xvalid = 0;
		break;

	case NLABEL:
		printf("_%s	equ	*\n", outname(s));
		xvalid = 0;
		break;

	case RLABEL:
		t = outname(s);
		printf("_%s	equ	*\n", t);
		xvalid = 0;
		break;

	case BRANCH:
		branch(getw(ascbuf), 0);
		xvalid = 0;
		break;

	case SETREG:
		nreg = getw(ascbuf)-1;
		break;

	default:
		if (opdope[op]&BINARY) {
			if (sp < &expstack[1]) {
				error("Binary expression botch");
				exit1(1,323);
			}
			t = *--sp;
			*sp++ = block(2, op, getw(ascbuf), 0, *--sp, t);
		} else {
			sp[-1] = block(1, op, getw(ascbuf), 0, sp[-1]);
		}
		break;
	}
	}
}

outname(s)
{
	register char *p, c;
	register n;

	p = s;
	n = 0;
	while (c = getc(ascbuf)) {
		*p++ = c;
		n++;
	}
	while (n++ < 8)
		*p++ = 0;
	return(s);
}

seq(c)
{
	register o;
	register int count;

	if (getw(ascbuf) == 0)
		return;
	count = 0;
	for (;;) {
		printf("@%o", getw(ascbuf));
		if ((o = getw(ascbuf)) != 1)
			break;
		count++;
		if (count > 5)
			{
			printf("\n	fcb	");
			count = 0;
			}
		else
			printf("%c",c);
	}
	printf("\n");
}
exit1(i,j)
{
 diag("dying at %d\n",j);
 flush();
 exit(i);
}
