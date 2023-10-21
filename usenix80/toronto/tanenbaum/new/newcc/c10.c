#
/*

	    	C compiler, part 2

	Copyright 1972 Bell Telephone Laboratories, Inc.

*/

#include "c1h.c"

char	maprel[] {	EQUAL, NEQUAL, GREATEQ, GREAT, LESSEQ,
			LESS, GREATQP, GREATP, LESSEQP, LESSP
};

char	notrel[] {	NEQUAL, EQUAL, GREAT, GREATEQ, LESS,
			LESSEQ, GREATP, GREATQP, LESSP, LESSEQP
};

struct tconst czero { CON, INT, 0, 0};
struct tconst cone  { CON, INT, 0, 1};
struct tconst fczero { SFCON, DOUBLE, 0, 0 };

struct table *tabtab[]
{
	regtab,
	efftab,
	cctab,
	sptab,
	0
};
struct	table	*cregtab;

int	nreg	3;
int	isn	10000;
int	namsiz	8;
int	*treebase;

/*
 * cells used for long functions.
 */
struct	tname	lleft { NAME, LONG, 0, STATIC, 0, 0};
struct	tnode	lassgn { ASSIGN, LONG, 0, &lleft, 0 };
struct	tnode	llocv { AMPER, LONG+PTR, 0, &lleft };

main(argc, argv)
char *argv[];
{
	int treespace[ossiz];
	struct table *table;
	register *sp, c, *tree;
	extern fout;

	if (argc<4) {
		error("Arg count");
		exit(1);
	}
	if(fopen(argv[1], ascbuf)<0) {
		error("Missing temp file");
		exit(1);
	}
	if ((fout = creat(argv[3], 0666)) < 0) {
		error("Can't create %s", argv[3]);
		exit(1);
	}
	treebase = getw(ascbuf);
	if (treebase < treespace) {
		error("Tree space botch");
		exit(1);
	}
	spacemax = &treespace[ossiz];
	while ((c=getc(ascbuf)) > 0) {
		if(c=='#') {
			sp = treebase;
			c = getw(ascbuf);
			tree = getw(ascbuf);
			table = tabtab[getw(ascbuf)];
			line = getw(ascbuf);
			while(--c >= 0)
				*sp++ = getw(ascbuf);
			if (table==0)		/* is switch */
				pswitch(treebase, sp, tree);
			else {
				spacep = sp;
				tree = optim(tree);
				nstack = 0;
				rcexpr(tree, table, 0);
			}
		} else
			putchar(c);
	}
	if (nfloat)
		printf(".globl	fltused\n");
	/*
	 * tack on the string file.
	 */
	close(ascbuf[0]);
	if (fopen(argv[2], ascbuf)<0) {
		error("Missing temp file");
		exit(1);
	}
	while ((c=getc(ascbuf)) >= 0)
		putchar(c);
	flush();
	exit(nerror!=0);
}

char *match(atree, table, nrleft)
struct tnode *atree;
struct table *table;
{
	int op, d1, d2, t1, t2, dope;
	struct tnode *p2;
	register struct tnode *p1, *tree;
	register struct optab *opt;

	if ((tree=atree)==0)
		return(0);
	if (table==lsptab)
		table = sptab;
	op = tree->op;
	dope = opdope[op];
	if ((dope&LEAF) == 0)
		p1 = tree->tr1;
	else
		p1 = tree;
	t1 = p1->type;
	d1 = dcalc(p1, nrleft);
	if ((dope&BINARY)!=0) {
		p2 = tree->tr2;
		if(opdope[p1->op]&CNVRT && (opdope[p2->op]&CNVRT)==0) {
			tree->tr1 = p1->tr1;
			if (opt = match(tree, table, nrleft))
				return(opt);
			tree->tr1 = p1;
		} else if (opdope[p2->op]&CNVRT && (opdope[p1->op]&CNVRT)==0) {
			tree->tr2 = p2->tr1;
			if (opt = match(tree, table, nrleft))
				return(opt);
			tree->tr2 = p2;
		}
		t2 = p2->type;
		d2 = dcalc(p2, nrleft);
	}
	for (; table->op!=op; table++)
		if (table->op==0)
			return(0);
	for (opt = table->tabp; opt->tabdeg1!=0; opt++) {
		if (d1 > (opt->tabdeg1&077)
		 || (opt->tabdeg1 >= 0100 && (p1->op != STAR)))
			continue;
		if (notcompat(p1, opt->tabtyp1, op)) {
			continue;
		}
		if ((opdope[op]&BINARY)!=0 && p2!=0) {
			if (d2 > (opt->tabdeg2&077)
			 || (opt->tabdeg2 >= 0100) && (p2->op != STAR) )
				continue;
			if (notcompat(p2,opt->tabtyp2, 0))
				continue;
		}
		return(opt);
	}
	return(0);
}

rcexpr(atree, atable, reg)
struct tnode *atree;
struct table *atable;
{
	register r;
	int modf, nargs, recurf;
	register struct tnode *tree;
	register struct table *table;

	table = atable;
	recurf = 0;
	if (reg<0) {
		recurf++;
		reg = ~reg;
		if (reg>=020) {
			reg =- 020;
			recurf++;
		}
	}
	if((tree=atree)==0)
		return(0);
	switch (tree->op)  {

	case SETREG:
		lleft.nloc = 0;
		nreg = tree->type-1;
		return;

	case CBRANCH:
		cbranch(tree->btree, tree->lbl, tree->cond, 0);
		return(0);

	case INIT:
		if (tree->tr1->op == AMPER)
			tree->tr1 = tree->tr1->tr1;
		if (tree->tr1->op==NAME)
			pname(tree->tr1);
		else if (tree->tr1==CON)
			psoct(tree->tr1->value);
		else
			error("Illegal initialization");
		putchar('\n');
		return(0);

	case RFORCE:
		/*
		 * Long function returns:
		 * stash value in temp, return pointer.
		 */
		if (tree->tr1->type==LONG) {
			lassgn.tr2 = tree->tr1;
			if (lleft.nloc==0) {
				lleft.nloc = isn++;
				printf(".bss\nL%d:.=.+4\n.text\n", lleft.nloc);
			}
			rcexpr(&lassgn, efftab, 0);
			tree->tr1 = &llocv;
		}
		if((r=rcexpr(tree->tr1, table, reg)) != 0)
			printf("mov%c	r%d,r0\n", isfloat(tree->tr1), r);
		return(0);

	case COMMA:
		rcexpr(tree->tr1, efftab, reg);
		atree = tree = tree->tr2;
		break;

	case NAND:
		if (table==cctab) {
			tree->op = TAND;
			tree->tr2 = optim(block(1, COMPL, INT, 0, tree->tr2));
		}
		break;

	case STAR:
		if (table==efftab && tree->type==LONG)
			return(rcexpr(tree->tr1, table, reg));
		break;

	case CALL:
		r = 0;
		nargs = 0;
		modf = 0;
		if (tree->tr1->op!=NAME) {	/* get nargs right */
			nargs++;
			nstack++;
		}
		tree = tree->tr2;
		if(tree->op) {
			while (tree->op==COMMA) {
				r =+ comarg(tree->tr2, &modf);
				tree = tree->tr1;
				nargs++;
			}
			r =+ comarg(tree, &modf);
			nargs++;
		}
		tree = atree;
		tree->op = CALL2;
		if (modf && tree->tr1->op==NAME && tree->tr1->class==EXTERN)
			tree->op = CALL1;
		cexpr(tree, regtab, reg);
		popstk(r);
		nstack =- nargs;
		if (table==efftab || table==regtab)
			return(0);
		r = 0;
		goto fixup;

	case TIMES:
	case DIVIDE:
	case ASTIMES:
	case ASDIV:
		tree = pow2(tree);
	}
/*
	if (table==cctab && opdope[tree->op]&RELAT
	 && tree->tr2->op==CON && tree->tr2->value==0)
		atree = tree = tree->tr1;
*/
	if (table!=cctab && table!=cregtab && recurf<2
	 && (opdope[tree->op]&LEAF)==0) {
		if (r=delay(&atree, table, reg)) {
			tree = atree;
			table = efftab;
			reg = r-1;
		}
	}
	if (recurf==0 && reorder(&atree, table, reg)) {
		if (table==cctab && atree->op==NAME)
			return(reg);
	}
	tree = atree;
	if (table==efftab && tree->op==NAME)
		return(reg);
	if ((r=cexpr(tree, table, reg))>=0)
		return(r);
	if (table!=regtab)  {
		if((r=cexpr(tree, regtab, reg))>=0) {
	fixup:
			modf = isfloat(tree);
			if (table==sptab || table==lsptab) {
				printf("mov%c	r%d,%c(sp)\n", modf, r,
					table==sptab? '-':0);
				nstack++;
			}
			if (table==cctab)
				printf("tst%c	r%d\n", modf, r);
			return(r);
		}
	}
	if (tree->op>0 && tree->op<RFORCE && opntab[tree->op])
		error("No code table for op: %s", opntab[tree->op]);
	else
		error("No code table for op %d", tree->op);
	return(reg);
}

cexpr(atree, table, areg)
struct tnode *atree;
struct table *table;
{
	int c, r;
	register struct tnode *p, *p1, *tree;
	struct table *ctable;
	struct tnode *p2;
	char *string;
	int reg, reg1, rreg, flag, opd;
	char *opt;

	tree = atree;
	reg = areg;
	p1 = tree->tr2;
	c = tree->op;
	opd = opdope[c];
	if ((opd&RELAT||c==LOGAND||c==LOGOR||c==EXCLA) && table!=cctab) {
		cbranch(tree, c=isn++, 1, reg);
		rcexpr(&czero, table, reg);
		branch(isn, 0);
		label(c);
		rcexpr(&cone, table, reg);
		label(isn++);
		return(reg);
	}
	if(c==QUEST) {
		if (table==cctab)
			return(-1);
		cbranch(tree->tr1, c=isn++, 0, reg);
		flag = nstack;
		rreg = rcexpr(p1->tr1, table, reg);
		nstack = flag;
		branch(r=isn++, 0);
		label(c);
		reg = rcexpr(p1->tr2, table, rreg);
		if (rreg!=reg)
			printf("mov%c	r%d,r%d\n",
			    isfloat(tree),reg,rreg);
		label(r);
		return(rreg);
	}
	reg = oddreg(tree, reg);
	reg1 = reg+1;
	if ((r = chkleaf(tree, table, reg)) >= 0)
		return(r);
	if ((tree->op==PLUS||tree->op==ASPLUS) &&
	    (p1=tree->tr2)->op == CON && p1->value == -1) {
		p1->value = 1;
		tree->op =+ (MINUS-PLUS);
	}
	if (table==cregtab)
		table = regtab;
	if (table!=cctab || c==INCAFT || c==DECAFT
	 || (opt = match(tree, efftab, nreg-reg)) == 0)
		if ((opt=match(tree, table, nreg-reg))==0)
			return(-1);
	string = opt->tabstring;
	p1 = tree->tr1;
	p2 = 0;
	if (opdope[tree->op]&BINARY)
		p2 = tree->tr2;
loop:
	if ((c = *string++) & 0200) {
		c =& 0177;
		putchar('\t');
	}
	switch (c) {

	case '\0':
		if (!isfloat(tree))
			if (tree->op==DIVIDE || tree->op==ASDIV)
				reg--;
		return(reg);

	/* A1 */
	case 'A':
		p = p1;
		goto adr;

	/* A2 */
	case 'B':
		p = p2;
		goto adr;

	adr:
		c = 0;
		if (*string=='\'') {
			c = 1;
			string++;
		} else if (*string=='+') {
			c = 2;
			string++;
		}
		pname(p, c);
		goto loop;

	/* I */
	case 'M':
		if ((c = *string)=='\'')
			string++;
		else
			c = 0;
		prins(tree->op, c, instab);
		goto loop;

	/* B1 */
	case 'C':
		if ((opd&LEAF) != 0)
			p = tree;
		else
			p = p1;
		goto pbyte;

	/* BF */
	case 'P':
		p = tree;
		goto pb1;

	/* B2 */
	case 'D':
		p = p2;
	pbyte:
		if (p->type==CHAR)
			putchar('b');
	pb1:
		if (isfloat(p))
			putchar('f');
		goto loop;

	/* BE */
	case 'L':
		if (p1->type==CHAR || p2->type==CHAR)
			putchar('b');
		p = tree;
		goto pb1;

	/* F */
	case 'G':
		p = p1;
		flag = 01;
		goto subtre;

	/* S */
	case 'K':
		p = p2;
		flag = 02;
		goto subtre;

	/* H */
	case 'H':
		p = tree;
		flag = 04;

	subtre:
		ctable = regtab;
		c = *string++ - 'A';
		if ((c&02)!=0)
			ctable = sptab;
		if ((c&04)!=0)
			ctable = cctab;
		if ((flag&01) && ctable==regtab && (c&01)==0
		  && (tree->op==DIVIDE||tree->op==MOD
		   || tree->op==ASDIV||tree->op==ASMOD))
			ctable = cregtab;
		if ((c&01)!=0) {
			p = p->tr1;
			if(collcon(p) && ctable!=sptab) {
				if (p->op==STAR)
					p = p->tr1;
				p = p->tr1;
			}
		}
		if (table==lsptab && ctable==sptab)
			ctable = lsptab;
		if (c&010)
			r = reg1;
		else
			if (opdope[p->op]&LEAF || p->degree < 2)
				r = reg;
			else
				r = areg;
		rreg = rcexpr(p, ctable, r);
		if (ctable!=regtab && ctable!=cregtab)
			goto loop;
		if (c&010)
			reg1 = rreg;
		else if (rreg!=reg)
			if (oddreg(tree, 0)==0 && (flag&04 ||
			      flag&01
			  && xdcalc(p2, nreg-rreg-1) <= (opt->tabdeg2&077)
			 ||   flag&02
			  && xdcalc(p1,nreg-rreg-1) <= (opt->tabdeg1&077))) {
				reg = rreg;
				reg1 = rreg+1;
			} else
				printf("mov%c\tr%d,r%d\n",
				    isfloat(tree), rreg, reg);
		goto loop;

	/* R */
	case 'I':
		r = reg;
		if (*string=='-') {
			string++;
			r--;
		}
		goto preg;

	/* R1 */
	case 'J':
		r = reg1;
	preg:
		if (r>nreg)
			error("Register overflow: simplify expression");
		printf("r%d", r);
		goto loop;

	case '-':		/* check -(sp) */
		if (*string=='(') {
			nstack++;
			if (table!=lsptab)
				putchar('-');
			goto loop;
		}
		break;

	case ')':		/* check (sp)+ */
		putchar(')');
		if (*string=='+')
			nstack--;
		goto loop;

	/* #1 */
	case '#':
		p = p1->tr1;
		goto nmbr;

	/* #2 */
	case '"':
		p = p2->tr1;

	nmbr:
		if(collcon(p)) {
			if (p->op==STAR) {
				printf("*");
				p = p->tr1;
			}
			if ((p = p->tr2)->op == CON) {
				if (p->value)
					psoct(p->value);
			} else if (p->op==AMPER)
				pname(p->tr1, 0);
		}
		goto loop;

	case 'T':		/* "tst R" if 1st op not in cctab */
		if (dcalc(p1, 5)>12 && !match(p1, cctab, 10))
			printf("tst	r%d\n", reg);
		goto loop;
	case 'V':	/* adc or sbc as required for longs */
		switch(tree->op) {
		case PLUS:
		case ASPLUS:
		case INCBEF:
		case INCAFT:
			printf("adc");
			break;

		case MINUS:
		case ASMINUS:
		case NEG:
		case DECBEF:
		case DECAFT:
			printf("sbc");
		}
		if (*string=='\'')
			string++;
		else
			printf("	r%d\n", reg);
		goto loop;
	}
	putchar(c);
	goto loop;
}

reorder(treep, table, reg)
struct tnode **treep;
struct table *table;
{
	register r, r1;
	register struct tnode *p;

	p = *treep;
	if (opdope[p->op]&LEAF)
		return(0);
	r1 = 0;
	while(sreorder(&p->tr1, table, reg))
		r1++;
	if (opdope[p->op]&BINARY) 
		while(sreorder(&p->tr2, table, reg))
			r1++;
	r = 0;
	while (sreorder(treep, table, reg))
		r++;
	*treep = optim(*treep);
	return(r);
}

sreorder(treep, table, reg)
struct tnode **treep;
struct table *table;
{
	register struct tnode *p, *p1;

	p = *treep;
	if (opdope[p->op]&LEAF)
		return(0);
	if (p->op==PLUS)
		if (reorder(&p->tr2, table, reg))
			*treep = p = optim(p);
	p1 = p->tr1;
	if (p->op==STAR || p->op==PLUS) {
		if (reorder(&p->tr1, table, reg))
			*treep = p = optim(p);
		p1 = p->tr1;
	}
	if (p1->op==NAME) switch(p->op) {
		case ASLSH:
		case ASRSH:
		case ASSIGN:
			if (p1->class != REG || isfloat(p->tr2))
				return(0);
			if (p->op==ASSIGN) switch (p->tr2->op) {
			case TIMES:
			case DIVIDE:
				if (!ispow2(p->tr2))
					break;
				p->tr2 = pow2(p->tr2);
			case PLUS:
			case MINUS:
			case AND:
			case NAND:
			case OR:
			case EXOR:
			case LSHIFT:
			case RSHIFT:
				p1 = p->tr2->tr2;
				if (xdcalc(p1) > 12
				 || p1->op==NAME
				 &&(p1->nloc==p->tr1->nloc
				  || p1->regno==p->tr1->nloc))
					return(0);
				p1 = p->tr2;
				p->tr2 = p1->tr1;
				if (p1->tr1->op!=NAME
				 || p1->tr1->class!=REG
				 || p1->tr1->nloc!=p->tr1->nloc)
					rcexpr(p, efftab, reg);
				p->tr2 = p1->tr2;
				p->op = p1->op + ASPLUS - PLUS;
				*treep = p;
				return(1);
			}
			goto OK;

		case ASTIMES:
		case ASDIV:
			if (!ispow2(p))
				return(0);
		case ASPLUS:
		case ASMINUS:
		case ASSAND:
		case ASSNAND:
		case ASOR:
		case ASXOR:
		case DECBEF:
		case INCBEF:
		OK:
			if (table==cctab||table==cregtab)
				reg =+ 020;
			rcexpr(optim(p), efftab, ~reg);
			*treep = p1;
			return(1);
	}
	return(0);
}

delay(treep, table, reg)
struct tnode **treep;
{
	register struct tnode *p, *p1;
	register r;

	p = *treep;
	if (table!=efftab && (p->op==INCAFT||p->op==DECAFT)
	 && p->tr1->op==NAME) {
		return(1+rcexpr(p->tr1, table, reg));
	}
	p1 = 0;
	if (opdope[p->op]&BINARY)
		p1 = sdelay(&p->tr2);
	if (p1==0)
		p1 = sdelay(&p->tr1);
	if (p1) {
		r = rcexpr(optim(p), table, reg);
		*treep = p1;
		return(r+1);
	}
	return(0);
}

sdelay(ap)
struct tnode **ap;
{
	register struct tnode *p, *p1;

	p = *ap;
	if ((p->op==INCAFT||p->op==DECAFT) && p->tr1->op==NAME) {
		*ap = ncopy(p->tr1);
		return(p);
	}
	if (p->op==STAR || p->op==PLUS)
		if (p1=sdelay(&p->tr1))
			return(p1);
	if (p->op==PLUS)
		return(sdelay(&p->tr2));
	return(0);
}

ncopy(ap)
struct tname *ap;
{
	register struct tname *p;

	p = ap;
	if (p->class!=REG)
		return(p);
	return(block(3, NAME, p->type, p->elsize, p->tr1,
	    p->offset, p->nloc));
}

chkleaf(atree, table, reg)
struct tnode *atree;
{
	struct tnode lbuf;
	register struct tnode *tree;

	tree = atree;
	if (tree->op!=STAR && dcalc(tree, nreg-reg) > 12)
		return(-1);
	lbuf.op = LOAD;
	lbuf.type = tree->type;
	lbuf.degree = tree->degree;
	lbuf.tr1 = tree;
	return(rcexpr(&lbuf, table, reg));
}

comarg(atree, flagp)
int *flagp;
{
	register struct tnode *tree;
	register retval;

	tree = atree;
	if (nstack || isfloat(tree) || tree->type==LONG) {
		rcexpr(tree, sptab, 0);
		retval = arlength(tree->type);
	} else {
		(*flagp)++;
		rcexpr(tree, lsptab, 0);
		retval = 0;
	}
	return(retval);
}
