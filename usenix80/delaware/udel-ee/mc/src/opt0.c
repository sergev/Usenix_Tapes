#include "opt.h"
#include "optab.h"
struct optab *ophash[OPHS];

char	revbr[] { BCS, BCC, BNE, BLT, BLE, BLS, BGT, BHI, BGE, BPL,
		  BEQ, BMI, BVS, BVC };
int	isn	30000;

/*
 *	The main routine:
 *
 *	The optimizer can be called with a command string of the
 *	following form.
 *
 *		opt [-] [+] [input] [output]
 *
 *	All of the arguments are optional.  If an input file is not
 *	specified then the standard input will be used.  Similarly
 *	if an output file is not specified then the standard output will
 *	be used.  The '-' switch is used to obtain a list of the number of
 *	things changed in the program, and the '+' switch is used to debug the
 *	optimizer.  The '+' switch only works if compiled with DEBUG defined.
 *
 */

main(argc, argv)
char **argv;
{
	register int niter, maxiter, isend;
	extern end;
	extern fin, fout;
	int nflag;

	nflag = 0;
#ifdef DEBUG
	if (argc>1 && argv[1][0]=='+') {
		argc--;
		argv++;
		debug++;
	}
#endif
	if (argc>1 && argv[1][0]=='-') {
		argc--;
		argv++;
		nflag++;
	}
	if (argc>1) {
		if ((fin = open(argv[1], 0)) < 0) {
			printf("opt: can't find %s\n", argv[1]);
			exit(1);
		}
	} else
		fin = dup(0);
	if (argc>2) {
		if ((fout = creat(argv[2], 0666)) < 0) {
			fout = 1;
			printf("opt: can't create %s\n", argv[2]);
			exit(1);
		}
	} else
		fout = dup(1);
	lasta = firstr = lastr = sbrk(2);
	maxiter = 0;
	opsetup();
	flush();
	do {
		isend = input();
		movedat();
		refcount();
		ckldx();
		niter = 0;
		do {
			refcount();
			do {
				iterate();
				niter++;
			} while (nchange);
			redcode();
			comjump();
		} while (nchange);
		brasw();
		output();
		flush();
		if (niter > maxiter)
			maxiter = niter;
		lasta = firstr;
	} while (isend);
	flush();
	fout = 2;
	if (nflag) {
		printf("%d iterations\n", maxiter);
		printf("%d jumps to jumps\n", nbrbr);
		printf("%d inst. after jumps\n", iaftbr);
		printf("%d jumps to next location\n", njp1);
		printf("%d redundant labels\n", nrlab);
		printf("%d cross-jumps\n", nxjump);
		printf("%d common seqs before jmp's\n", ncomj);
		printf("%d skips over jumps\n", nskip);
		printf("%d jmp's changed to bra's\n", jmpbr);
		printf("%d redundant codes removed\n", redop);
		printf("%dK core\n", ((lastr+01777)>>10)&077);
		flush();
	}
	exit(0);
}

/*
 *	Function:	input()
 *
 *	Arguments:	none
 *
 *	Returns:	0 if eof reached.
 *			1 if more data in the file.
 *
 *	Desc:
 *
 *	     This function will read one subroutine of a 6800 program
 *	and load it into the internal linked list.
 *
 */

input()
{
	register struct node *p, *lastp;
	register int op;

	lastp = &first;
	for (;;) {
		op = getline();
		switch (op.op) {
	
		case LABEL:
			p = alloc(sizeof first);
			if (line[0] == 'L') {
				p->combop = LABEL;
				p->labno = getnum(line+1);
				p->code = 0;
			} else {
				p->combop = DLABEL;
				p->labno = 0;
				p->code = copy(line,0);
			}
			break;
	
		case BRA:
		case CBR:
		case JMP:
		case FDB:
			p = alloc(sizeof first);
			p->combop = op;
			if (*curlp=='L' && (p->labno = getnum(curlp+1)))
			{
				p->code = 0;
				if (op==FDB)
					p->combop = JSW;
				else if (op == JMP)
					{
					p->combop = BRA;
					jmpbr++;
					}
			}
			else {
				p->labno = 0;
				p->code = copy(curlp,0);
			}
			break;

		case COMMEN:
			goto skip;

		case EQU:
			if (equstr(curlp,"*"))
				goto skip;

		default:
			p = alloc(sizeof first);
			p->combop = op;
			p->labno = 0;
			if (p->op == LDA && equstr(curlp,"#@0"))
				{
				p->code = 0;
				p->op = CLR;
				}
			else p->code = copy(curlp,0);
			break;

		}
		p->forw = 0;
		p->back = lastp;
		lastp->forw = p;
		lastp = p;
		p->ref = 0;
		if (op==EROU)
			return(1);
skip:		if (op==END)
			return(0);
	}
}

/*
 *	Function:	getline
 *
 *	Arguments:	none
 *
 *	Returns:	Type of op-code on the line just read.
 *
 *	Desc:
 *
 *	     Reads one line from the input file and stores it in
 *	the global character array line[].
 *
 */

getline()
{
	register char *lp;
	register c;
	int label;
	char get();

	lp = line;
	while ((c=get()) == '\n');
	if (c == '*')
	{
		while ((c=get()) != '\n' && c!=0)
			*lp++ = c;
		*lp++ = 0;
		curlp = 0;
		if (line[0] == '*')
			return(EROU);
		else
			return(COMMEN);
	}
	if (c == '~')
		{
		while ((c=get()) != '\n' && c != 0);
		return(COMMEN);
		}
	if (c==0)
	{
		line[0] = 0;
		return(END);
	}
	if (c!=' ' && c!='\t')
		label=1;
	else
		label=0;
	do {
		if ((c==' ' || c=='\t') && label)
		{
			*lp++ = 0;
			unget(c);
			return(LABEL);
		}
		if (c=='\n') {
			*lp++ = 0;
			if (label == 0)
				return(oplook());
			else
				return(LABEL);
		}
		*lp++ = c;
	} while (c=get());
	*lp++ = 0;
	return(END);
}

char	gbuf[100];
int	gfill	0;

/*
 *	Function:	get
 *
 *	Arguments:	none
 *
 *	Returns:	One character from the input file.
 *
 *	     This function works like the UNIX getchar() routine.
 *	This function can also be used with the unget() function.
 *
 */

char get()
{
	if (gfill > 0)
		return(gbuf[--gfill]);
	else
		return(getchar());
}

/*
 *	Function:	unget(c)
 *
 *	Arguments:	A single character variable.
 *
 *	Returns:	Nothing.
 *
 *	Desc:
 *
 *	     This function is used to place a character back into the input
 *	stream so that the next call to get() will re-read the character.
 *
 */
unget(c)
char c;
{
	gbuf[gfill++] = c;
}
/*
 *	Function:	getnum(ap)
 *
 *	Arguments:	Pointer to character string.
 *
 *	Returns:	The integer value contained in the string.
 *
 */

getnum(ap)
char *ap;
{
	register char *p;
	register n, c;

	p = ap;
	n = 0;
	while ((c = *p++) >= '0' && c <= '9')
		n = n*10 + c - '0';
	if (*--p != 0)
		return(0);
	return(n);
}

/*
 *	Function:	output
 *
 *	Arguments:	none
 *
 *	Returns:	nothing
 *
 *	Desc:
 *
 *	     Converts the internal linked list into external form and
 *	writes it on the output file.
 *
 */

output()
{
	register struct node *t;
	register struct optab *op;
	int areg, breg;

	areg = breg = 0;
	t = &first;
	while (t = t->forw) switch (t->op) {

	case END:
#ifdef DEBUG
		if (debug)
			printf("case END:	");
#endif
		return;

	case LABEL:
#ifdef DEBUG
		if (debug)
			printf("case LABEL:	");
#endif
		if (t->forw && t->forw->op != LABEL && t->forw->op != DLABEL)
			printf("L%d", t->labno);
		else
			printf("L%d\tequ\t*\n", t->labno);
#ifdef DEBUG
		if (debug)
			printf("\n");
#endif
		continue;

	case DLABEL:
#ifdef DEBUG
		if (debug)
			printf("case DLABEL:	");
#endif
		if (t->forw && t->forw->op != LABEL && t->forw->op != DLABEL)
			printf("%s", t->code);
		else
			printf("%s\tequ\t*\n", t->code);
#ifdef DEBUG
		if (debug)
			printf("\n");
#endif
		continue;

	case JSW:
#ifdef DEBUG
		if (debug)
			printf("case JSW:	");
#endif

		printf("\tfdb\tL%d\n", t->labno);
		continue;

	case EROU:
#ifdef DEBUG
		if (debug)
			printf("case EROU:	");
#endif
		printf("**\n");
		continue;

	default:
#ifdef DEBUG
		if (debug)
			printf("case Default:	");
#endif
		if ((areg = t->subop) == AREG)
			t->subop = 0;
		if ((breg = t->subop) == BREG)
			t->subop = 0;
#ifdef DEBUG
		if (areg==AREG && debug) printf("\na reg. used	");
		if (breg==BREG && debug) printf("\nb reg. used	");
		if (debug)
			printf("Op-code is number %d\n\t",t->combop);
#endif
		for (op = optab; op->opstring!=0; op++) 
			if (op->opcode == t->combop) {
				printf("\t%s", op->opstring);
				if (areg==AREG)
					printf("a");
				if (breg==BREG)
					printf("b");
				break;
			}
		if (t->code) {
			printf("\t%s\n", t->code);
		} else if (t->op==BRA || t->op==CBR || t->op==JMP)
			printf("\tL%d\n", t->labno);
		else
			printf("\n");
		continue;



	case 0:
#ifdef DEBUG
		if (debug)
			printf("case UNKNOWN:	");
#endif
		if (t->code)
			printf("%s", t->code);
		printf("\n");
		continue;
	}
}


/*
 *	Function:	copy(x, y)
 *
 *	Arguments:	One or two character pointers.
 *
 *	Returns:	A pointer to a copy of x concatenated to y.
 *
 */

copy(ap, ap2)
char *ap, *ap2;
{
	register char *p, *np;
	char *onp;
	register n;

	p = ap;
	n = 0;
	if (*p==0)
		return(0);
	do
		n++;
	while (*p++);
	if (ap2) {
		p = ap2;
		while (*p++)
			n++;
	}
	onp = np = alloc(n);
	p = ap;
	while (*np++ = *p++);
	if (ap2) {
		p = ap2;
		np--;
		while (*np++ = *p++);
	}
	return(onp);
}

/*
 *	Function:	oplook
 *
 *	Arguments:	none
 *
 *	Returns:	Type of op-code
 *
 *	Desc:
 *
 *	     This function returns the type of op-code that is in
 *	the line buffer line[].
 *
 */

oplook()
{
	register struct optab *optp;
	register char *lp, *op;
	static char tmpop[32];
	struct optab **ophp;

	op = tmpop;
	lp = line;
	while (*lp==' ' || *lp=='\t')
		lp++;
	while (*lp && *lp!=' ' && *lp!='\t')
		*op++ = *lp++;
	*op++ = 0;
	while (*lp=='\t' || *lp==' ')
		lp++;
	curlp = lp;
	ophp = &ophash[(((tmpop[0]<<3)+(tmpop[1]<<1))&077777) % OPHS];
#ifdef DEBUG
	if (debug)
		printf("Searching op-table for '%s'\n",tmpop);
#endif
	while (optp = *ophp) {
		op = optp->opstring;
#ifdef DEBUG
		if (debug)
			printf("Scanning '%s'\n",op);
#endif
		lp = tmpop;
		while (*lp == *op++)
			if (*lp++ == 0)
				return(optp->opcode);
		if (*lp=='b' && *(lp+1)==0 && *(op-1)==0)
			return(optp->opcode + (BREG<<8));
		if (*lp=='a' && *(lp+1)==0 && *(op-1)==0)
			return(optp->opcode + (AREG<<8));
		ophp++;
		if (ophp >= &ophash[OPHS])
			ophp = ophash;
	}
	curlp = line;
	return(0);
}

/*
 *	Function:	refcount
 *
 *	Arguments:	none
 *
 *	Returns:	nothing
 *
 *	Desc:
 *
 *		This function counts the number of references made to
 *	each label in the program and sets the refc field appropriately.
 *	Any redundant labels are removed.
 *
 */

refcount()
{
	register struct node *p, *lp;
	static struct node *labhash[LABHS];
	register struct node **hp;

	for (hp = labhash; hp < &labhash[LABHS];)
		*hp++ = 0;
	for (p = first.forw; p!=0; p = p->forw)
		if (p->op==LABEL) {
			labhash[p->labno % LABHS] = p;
			p->refc = 0;
		}
	for (p = first.forw; p!=0; p = p->forw) {
		if (p->op==BRA || p->op==CBR || p->op==JSW) {
			p->ref = 0;
			lp = labhash[p->labno % LABHS];
			if (lp==0 || p->labno!=lp->labno)
			for (lp = first.forw; lp!=0; lp = lp->forw) {
				if (lp->op==LABEL && p->labno==lp->labno)
					break;
			}
			if (lp) {
				hp = nonlab(lp)->back;
				if (hp!=lp) {
					p->labno = hp->labno;
					lp = hp;
				}
				p->ref = lp;
				lp->refc++;
			}
		}
	}
	for (p = first.forw; p!=0; p = p->forw)
		if (p->op==LABEL && p->refc==0
		 && (lp = nonlab(p))->op && lp->op!=JSW)
			decref(p);
}

/*
 *	Function:	iterate
 *
 *	Arguments:	none
 *
 *	Returns:	nothing
 *
 *	Desc:
 *
 *	     This is the primary optimization routine in the program.
 *	It looks for and fixes the following things:
 *
 *	1. Jumps to jumps.
 *	2. Branches over branches.
 *	3. Instructions after jumps.
 *	4. Jumps to the next instruction.
 *
 */

iterate()
{
	register struct node *p, *rp, *p1;

	nchange = 0;
	for (p = first.forw; p!=0; p = p->forw) {
		if ((p->op==BRA || p->op==CBR || p->op==JSW)
		    && p->ref) {
			rp = nonlab(p->ref);
			if (rp->op==BRA && rp->labno && p!=rp) {
				nbrbr++;
				p->labno = rp->labno;
				decref(p->ref);
				rp->ref->refc++;
				p->ref = rp->ref;
				nchange++;
			}
		}
		if (p->op==CBR && (p1 = p->forw)->op==BRA) {
			rp = p->ref;
			do
				rp = rp->back;
			while (rp->op==LABEL);
			if (rp==p1) {
				decref(p->ref);
				p->ref = p1->ref;
				p->labno = p1->labno;
				p1->forw->back = p;
				p->forw = p1->forw;
				p->subop = revbr[p->subop];
				nchange++;
				nskip++;
			}
		}
		if (p->op==BRA) {
			while (p->forw && p->forw->op!=LABEL &&
				p->forw->op!=DLABEL
				&& p->forw->op!=EROU && p->forw->op!=END) {
				nchange++;
				iaftbr++;
				if (p->forw->ref)
					decref(p->forw->ref);
				p->forw = p->forw->forw;
				p->forw->back = p;
			}
		}
		if (p->op == BRA || p->op == CBR)
		{
			rp = p->forw;
			while (rp && rp->op==LABEL) {
				if (p->ref == rp) {
					rmnode(p, 0);
					p = p->back;
					decref(rp);
					nchange++;
					njp1++;
					break;
				}
				rp = rp->forw;
			}
			xjump(p);
		}
	}
}

/*
 *	Function:	xjump(p)
 *
 *	Arguments:	Pointer to a node containing a jump or branch.
 *
 *	Returns:	nothing
 *
 *	Desc:
 *
 *	     This routine looks for jumps that have equivalent sections
 *	of code prior to the jump as to the code prior to the label being
 *	jumped to.
 *
 */

xjump(ap)
{
	register int *p1, *p2, *p3;
	int nxj;

	nxj = 0;
	p1 = ap;
	if ((p2 = p1->ref)==0)
		return(0);
	for (;;) {
		while ((p1 = p1->back) && p1->op==LABEL);
		while ((p2 = p2->back) && p2->op==LABEL);
		if (!equop(p1, p2) || p1==p2)
			return(nxj);
		p3 = insertl(p2);
		p1->combop = BRA;
		p1->ref = p3;
		p1->labno = p3->labno;
		p1->code = 0;
		nxj++;
		nxjump++;
		nchange++;
	}
}

/*
 *	Function:	insertl(ap)
 *
 *	Arguments:	Pointer to a node.
 *
 *	Returns:	Pointer to a unique label inserted before ap.
 *
 */

insertl(ap)
struct node *ap;
{
	register struct node *lp, *op;

	op = ap;
	if (op->op == LABEL) {
		op->refc++;
		return(op);
	}
	if (op->back->op == LABEL) {
		op = op->back;
		op->refc++;
		return(op);
	}
	lp = alloc(sizeof first);
	lp->combop = LABEL;
#ifdef DEBUG
	if (debug)
		printf("Inserting label L%d\n", isn);
#endif
	lp->labno = isn++;
	lp->ref = 0;
	lp->code = 0;
	lp->refc = 1;
	lp->back = op->back;
	lp->forw = op;
	op->back->forw = lp;
	op->back = lp;
	return(lp);
}


/*
 *	Function:	comjump
 *
 *	Arguments:	none
 *
 *	Returns:	nothing
 *
 *	Desc:
 *
 *	     This function looks for pairs of equivalent jumps in a
 *	program and calls the backjmp subroutine for each pair.
 *	Depends on jumpsw to patch up out of range BRA's.
 *
 */

comjump()
{
	register struct node *p1, *p2, *p3;

	for (p1 = first.forw; p1!=0; p1 = p1->forw)
		if (p1->op==BRA &&(p2 = p1->ref)&& p2->refc >1)
			for (p3 = p1->forw; p3!=0; p3 = p3->forw)
			    if (p3->op==BRA && p3->ref== p2)
				backjmp(p1, p3);
}

/*
 *	Function:	backjmp
 *
 *	Arguments:	Pointers to two nodes containing jump instructions.
 *
 *	Returns:	nothing
 *
 *	Desc:
 *
 *	     This subroutine looks for common sequences of code before
 *	two jump statements.  It deletes one such sequence.
 *
 */

backjmp(ap1, ap2)
struct node *ap1, *ap2;
{
	register struct node *p1, *p2, *p3;

	p1 = ap1;
	p2 = ap2;
	for(;;) {
		while ((p1 = p1->back) && p1->op==LABEL);
		p2 = p2->back;
		if (equop(p1, p2)) {
			p3 = insertl(p1);
			p2->back->forw = p2->forw;
			p2->forw->back = p2->back;
			p2 = p2->forw;
			decref(p2->ref);
			p2->labno = p3->labno;
			p2->ref = p3;
			nchange++;
			ncomj++;
		} else
			return;
	}
}

/*
 *	Function:	movedat
 *
 *	Arguments:	none
 *
 *	Returns:	nothing
 *
 *	Desc:
 *
 *	     This function moves all the data declaring statements in a
 *	subroutine to the beginning of the subroutine.
 *
 */

movedat()
{
	register struct node *p1, *p2;
	struct node *p3;
	struct node data;
	struct node *datp;

#ifdef DEBUG
	if (debug)
	{
		printf("Entering movedat\n");
		flush();
		check();
	}
#endif
	if (first.forw == 0)
		return;
	datp = &data;
	for (p1 = first.forw; p1!=0; p1 = p1->forw) {
	  if (p1->op == RMB || p1->op==EQU || p1->op==FCB
	      || p1->op==FDB || p1->op==ORG || p1->op==JSW)
	  {
	    p2 = p1;
	    if (p1->op!=EQU && p1->op!=ORG)
	      while (p1->back && (p1->back->op==LABEL || p1->back->op==DLABEL))
		p1 = p1->back;

	    if (p1->op == EQU && p1->back && (p1->back->op==LABEL ||
		p1->back->op==DLABEL))
		   p1 = p1->back;


	  p3 = p1->back;
	  p1->back->forw = p2->forw;
	  p2->forw->back = p3;
	  p2->forw = 0;
	  datp->forw = p1;
	  p1->back = datp;
	  p1 = p3;
	  datp = p2;
	 }
	}
	if (data.forw) {
		datp->forw = first.forw;
		first.forw->back = datp;
		data.forw->back = &first;
		first.forw = data.forw;
	}
#ifdef DEBUG
	if (debug)
	{
		printf("movedat completed\n");
		flush();
		check();
	}
#endif
}

/*
 *	Function:	equop(p1,p2)
 *
 *	Arguments:	Pointers to two nodes
 *
 *	Returns:	1 if the nodes are equivalent but not the same node
 *			0 if the nodes are different
 *
 */

equop(ap1, p2)
struct node *ap1, *p2;
{
	register char *cp1, *cp2;
	register struct node *p1;

	p1 = ap1;
	if (p1->combop != p2->combop)
		return(0);
	if (p1->op>0 && p1->op<= BSS)
		return(0);
	cp1 = p1->code;
	cp2 = p2->code;
	if (p1->op == BRA || p1->op == CBR || p1->op == JSR)
	    if (cp1 == 0)
		if (p1->ref == p2->ref) return(1);
		else return(0);
	if (cp1==0 && cp2==0)
		return(1);
	if (cp1==0 || cp2==0)
		return(0);
	return(equstr(cp1, cp2));
}

/*
 *	Function:	decref(ap)
 *
 *	Arguments:	Pointer to a node containing a LABEL
 *
 *	Returns:	nothing
 *
 *	Desc:
 *
 *	     This function will decrement a reference to a label and
 *	remove it from the linked list if it's reference count goes
 *	to zero.
 *
 */

decref(ap)
{
	register struct node *p, *lp;

	p = ap;
	if (--p->refc <= 0 && (lp = nonlab(p))->op && lp->op!=FCB
	    && lp->op!=FDB && lp->op!=RMB && lp->op!=JSW && lp->op!=EQU) {
		nrlab++;
#ifdef DEBUG
		if (debug)
			printf("Deleting Label 'L%d'\n", p->labno);
#endif
		if (p->op != LABEL)
			{
			write(2, "Optimizer: Fatal Error\n", 23);
			abort();
			}
		p->back->forw = p->forw;
		p->forw->back = p->back;
	}
}

/*
 *	Function:	nonlab(ap)
 *
 *	Arguments:	Pointer to a node containg a label.
 *
 *	Returns:	Pointer to the first non-label node following ap.
 *
 */

nonlab(ap)
struct node *ap;
{
	register struct node *p;

	p = ap;
	while (p && p->op==LABEL)
		p = p->forw;
	return(p);
}

/*
 *	Function:	alloc(an)
 *
 *	Arguments:	Amount of memory to allocate.
 *
 *	Returns:	A pointer to the block of memory allocated.
 *
 */

alloc(an)
{
	register int n;
	register char *p;

	n = an;
	n++;
	n =& ~01;
	if (lasta+n >= lastr) {
		if (sbrk(2000) == -1) {
			write(2, "Optimizer: out of space\n", 24);
			abort();
		}
		lastr =+ 2000;
	}
	p = lasta;
	lasta =+ n;
	return(p);
}

#ifdef DEBUG
/*
 *	Function:	check
 *
 *	Arguments:	none
 *
 *	Returns:	nothing
 *
 *	Desc:
 *
 *	     This function checks the backward pointers in the linked
 *	list by traversing the forward pointers.  If any inconsistancies
 *	are found the abort() routine will be called.
 *
 *	Note:	This function is called only if the debug flag is used.
 *
 */

check()
{
	register struct node *p, *lp;

	lp = &first;
	for (p=first.forw; p!=0; p = p->forw) {
		if (p->back != lp)
			abort();
		lp = p;
	}
}
#endif

/*
 *	Function:	equstr(p1,p2)
 *
 *	Arguments:	Two pointers to character strings.
 *
 *	Returns:	1 if the strings are equal
 *			0 if they are different
 *
 */

equstr(ap1, ap2)
char *ap1, *ap2;
{
	register char *p1, *p2;

	p1 = ap1;
	p2 = ap2;
	do {
		if (*p1++ != *p2)
			return(0);
	} while (*p2++);
	return(1);
}

/*
 *	Function:	opsetup
 *
 *	Arguments:	none
 *
 *	Returns:	nothing
 *
 *	Sets up the hash table for use with the opcode table in
 *	looking up instruction mnemonics.
 */

opsetup()
{
register struct optab *optp, **ophp;
register char *p;

for (optp = optab; p = optp->opstring; optp++)
	{
	ophp = &ophash[(((*p++<<3)+(*p<<1))&077777) % OPHS];
	while (*ophp++)
		if (ophp >= &ophash[OPHS])
			ophp = ophash;
	*--ophp = optp;
	}
}
