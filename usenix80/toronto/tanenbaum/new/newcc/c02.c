#
/* C compiler

Copyright 1972 Bell Telephone Laboratories, Inc. 

*/

#include "c0h.c"

extdef()
{
	register o, elsize;
	int type, sclass;
	register struct hshtab *ds;

	if(((o=symbol())==EOF) || o==SEMI)
		return;
	peeksym = o;
	type = INT;
	sclass = EXTERN;
	xdflg = FNDEL;
	if ((elsize = getkeywords(&sclass, &type)) == -1 && peeksym!=NAME)
		goto syntax;
	if (type==STRUCT)
		blkhed();
	do {
		defsym = 0;
		decl1(EXTERN, type, 0, elsize);
		if ((ds=defsym)==0)
			return;
		funcsym = ds;
		ds->hflag = FNDEL;
		printf(".globl	_%.8s\n", ds->name);
		xdflg = 0;
		if ((ds->type&XTYPE)==FUNC) {
			if ((peeksym=symbol())==LBRACE || peeksym==KEYW) {
				funcblk.type = decref(ds->type);
				cfunc(ds->name);
				return;
			}
		} else 
			cinit(ds);
	} while ((o=symbol())==COMMA);
	if (o==SEMI)
		return;
syntax:
	error("External definition syntax");
	errflush(o);
	statement(0);
}

cfunc(cs)
char *cs;
{
	register savdimp;

	savdimp = dimp;
	printf(".text\n_%.8s:\n", cs);
	if (stflg)
		printf("~~%.8s:\n", cs);
	declist(ARG);
	regvar = 5;
	retlab = isn++;
	if ((peeksym = symbol()) != LBRACE)
		error("Compound statement required");
	statement(1);
	printf("L%d:jmp	cret\n", retlab);
	dimp = savdimp;
}

cinit(ds)
struct hshtab *ds;
{
	register basetype, nel, ninit;
	int o, width, realwidth;

	nel = 1;
	basetype = ds->type;
	/*
	 * If it's an array, find the number of elements.
	 * "basetype" is the type of thing it's an array of.
	 */
	while ((basetype&XTYPE)==ARRAY) {
		if ((nel = dimtab[ds->ssp&0377])==0)
			nel = 1;
		basetype = decref(basetype);
	}
	realwidth = width = length(ds) / nel;
	/*
	 * Pretend a structure is kind of an array of integers.
	 * This is a kludge.
	 */
	if (basetype==STRUCT) {
		nel =* realwidth/2;
		width = 2;
	}
	if ((peeksym=symbol())==COMMA || peeksym==SEMI) {
		printf(".comm\t_%.8s,%o\n",ds->name,(nel*width+ALIGN)&~ALIGN);
		return;
	}
	ninit = 0;
	printf(".data\n_%.8s=.\n", ds->name);
	if ((o=symbol())==LBRACE) {
		do
			ninit = cinit1(ds, basetype, width, ninit, nel);
		while ((o=symbol())==COMMA);
		if (o!=RBRACE)
			peeksym = o;
	} else {
		peeksym = o;
		ninit = cinit1(ds, basetype, width, 0, nel);
	}
	/*
	 * Above we pretended that a structure was a bunch of integers.
	 * Readjust in accordance with reality.
	 * First round up partial initializations.
	 */
	if (basetype==STRUCT) {
		if (o = 2*ninit % realwidth)
			printf(".=.+%o\n", realwidth-o);
		ninit = (2*ninit+realwidth-2) / realwidth;
		nel =/ realwidth/2;
	}
	/*
	 * If there are too few initializers, allocate
	 * more storage.
	 * If there are too many initializers, extend
	 * the declared size for benefit of "sizeof"
	 */
	if (ninit<nel)
		printf(".=.+%o\n", (nel-ninit)*realwidth);
	else if (ninit>nel) {
		if ((ds->type&XTYPE)==ARRAY)
			dimtab[ds->ssp&0377] = ninit;
		nel = ninit;
	}
	/*
	 * If it's not an array, only one initializer is allowed.
	 */
	if (ninit>1 && (ds->type&XTYPE)!=ARRAY)
		error("Too many initializers");
	if (((nel&width)&ALIGN))
		printf(".even\n");
}

cinit1(ds, type, awidth, aninit, nel)
struct hshtab *ds;
{
	float sf;
	register struct tnode *s;
	register width, ninit;

	width = awidth;
	ninit = aninit;
	if ((peeksym=symbol())==STRING && type==CHAR) {
		peeksym = -1;
		if (ninit)
			bxdec();
		printf("_%.8s=L%d\n", ds->name, cval);
		if (nel>nchstr) {
			strflg++;
			printf(".=.+%o\n", nel-nchstr);
			strflg = 0;
			nchstr = nel;
		}
		return(nchstr);
	}
	if (peeksym==RBRACE)
		return(ninit);
	initflg++;
	s = tree();
	initflg = 0;
	switch(width) {

	case 1:
		printf(".byte ");
		if (s->op != CON)
			goto bad;

	case 2:
		if (s->op==CON) {
			printf("%o\n", s->value);
			break;
		}
		if (s->op==FCON || s->op==SFCON) {
			if (type==STRUCT) {
				ninit =+ 3;
				goto prflt;
			}
			goto bad;
		}
		rcexpr(block(1,INIT,0,0,s), regtab);
		break;

	case 4:
		sf = fcval;
		printf("%o;%o\n", sf);
		goto flt;

	case 8:
	prflt:
		printf("%o;%o;%o;%o\n", fcval);
	flt:
		if (s->op==FCON || s->op==SFCON)
			break;

	default:
	bad:
		bxdec();

	}
	return(++ninit);
}

bxdec()
{
	error("Inconsistent external initialization");
}

statement(d)
{
	register o, o1, o2;
	int o3, o4;
	struct tnode *np;

stmt:
	switch(o=symbol()) {

	case EOF:
		error("Unexpected EOF");
	case SEMI:
	case RBRACE:
		return;

	case LBRACE:
		if (d) {
			o2 = blkhed() - 6;
			if (proflg) {
				printf("mov\t$L%d,r0\n", isn);
				printf("jsr\tpc,mcount\n");
				printf(".bss\nL%d:.=.+2\n.text\n", isn++);
			}
			printf("jsr	r5,csv\n");
			if (o2==2)
				printf("tst	-(sp)\n");
			else if (o2 != 0)
				printf("sub	$%o,sp\n", o2);
		}
		while (!eof) {
			if ((o=symbol())==RBRACE)
				return;
			peeksym = o;
			statement(0);
		}
		error("Missing '}'");
		return;

	case KEYW:
		switch(cval) {

		case GOTO:
			if (o1 = simplegoto())
				branch(o1);
			else 
				dogoto();
			goto semi;

		case RETURN:
			doret();
			goto semi;

		case IF:
			np = pexpr();
			o2 = 0;
			if ((o1=symbol())==KEYW) switch (cval) {
			case GOTO:
				if (o2=simplegoto())
					goto simpif;
				cbranch(np, o2=isn++, 0);
				dogoto();
				label(o2);
				goto hardif;

			case RETURN:
				if (nextchar()==';') {
					o2 = retlab;
					goto simpif;
				}
				cbranch(np, o1=isn++, 0);
				doret();
				label(o1);
				o2++;
				goto hardif;

			case BREAK:
				o2 = brklab;
				goto simpif;

			case CONTIN:
				o2 = contlab;
			simpif:
				chconbrk(o2);
				cbranch(np, o2, 1);
			hardif:
				if ((o=symbol())!=SEMI)
					goto syntax;
				if ((o1=symbol())==KEYW && cval==ELSE) 
					goto stmt;
				peeksym = o1;
				return;
			}
			peeksym = o1;
			cbranch(np, o1=isn++, 0);
			statement(0);
			if ((o=symbol())==KEYW && cval==ELSE) {
				o2 = isn++;
				branch(o2);
				label(o1);
				statement(0);
				label(o2);
				return;
			}
			peeksym = o;
			label(o1);
			return;

		case WHILE:
			o1 = contlab;
			o2 = brklab;
			label(contlab = isn++);
			cbranch(pexpr(), brklab=isn++, 0);
			statement(0);
			branch(contlab);
			label(brklab);
			contlab = o1;
			brklab = o2;
			return;

		case BREAK:
			chconbrk(brklab);
			branch(brklab);
			goto semi;

		case CONTIN:
			chconbrk(contlab);
			branch(contlab);
			goto semi;

		case DO:
			o1 = contlab;
			o2 = brklab;
			contlab = isn++;
			brklab = isn++;
			label(o3 = isn++);
			statement(0);
			label(contlab);
			contlab = o1;
			if ((o=symbol())==KEYW && cval==WHILE) {
				cbranch(tree(), o3, 1);
				label(brklab);
				brklab = o2;
				goto semi;
			}
			goto syntax;

		case CASE:
			o1 = conexp();
			if ((o=symbol())!=COLON)
				goto syntax;
			if (swp==0) {
				error("Case not in switch");
				goto stmt;
			}
			if(swp>=swtab+swsiz) {
				error("Switch table overflow");
			} else {
				swp->swlab = isn;
				(swp++)->swval = o1;
				label(isn++);
			}
			goto stmt;

		case SWITCH:
			o1 = brklab;
			brklab = isn++;
			np = pexpr();
			chkw(np);
			rcexpr(block(1,RFORCE,0,0,np), regtab);
			pswitch();
			brklab = o1;
			return;

		case DEFAULT:
			if (swp==0)
				error("Default not in switch");
			if ((o=symbol())!=COLON)
				goto syntax;
			label(deflab = isn++);
			goto stmt;

		case FOR:
			o1 = contlab;
			o2 = brklab;
			contlab = isn++;
			brklab = isn++;
			if (o=forstmt())
				goto syntax;
			label(brklab);
			contlab = o1;
			brklab = o2;
			return;
		}

		error("Unknown keyword");
		goto syntax;

	case NAME:
		if (nextchar()==':') {
			peekc = 0;
			o1 = csym;
			if (o1->hclass>0) {
				error("Redefinition");
				goto stmt;
			}
			o1->hclass = STATIC;
			o1->htype = ARRAY;
			if (o1->hoffset==0)
				o1->hoffset = isn++;
			label(o1->hoffset);
			if ((peeksym=symbol())==RBRACE)
				return;
			goto stmt;
		}
	}

	peeksym = o;
	rcexpr(tree(), efftab);

semi:
	if ((o=symbol())==SEMI)
		return;
syntax:
	error("Statement syntax");
	errflush(o);
	goto stmt;
}

#define	forsps	150

forstmt()
{
	int l, savxpr[forsps];
	int *st, *ss;
	register int *sp1, *sp2, o;

	if ((o=symbol()) != LPARN)
		return(o);
	if ((o=symbol()) != SEMI) {		/* init part */
		peeksym = o;
		rcexpr(tree(), efftab);
		if ((o=symbol()) != SEMI)
			return(o);
	}
	label(contlab);
	if ((o=symbol()) != SEMI) {		/* test part */
		peeksym = o;
		rcexpr(block(1,CBRANCH,tree(),brklab,0), cctab);
		if ((o=symbol()) != SEMI)
			return(o);
	}
	if ((peeksym=symbol()) == RPARN) {	/* incr part */
		peeksym = -1;
		statement(0);
		branch(contlab);
		return(0);
	}
	l = contlab;
	contlab = isn++;
	st = tree();
	if ((o=symbol()) != RPARN)
		return(o);
	ss = space;
	if (space-treebase > forsps) {
		error("Expression too large");
		space = &treebase[forsps];
	}
	sp2 = savxpr;
	for (sp1=treebase; sp1<space;)
		*sp2++ = *sp1++;
	statement(0);
	space = ss;
	sp2 = savxpr;
	for (sp1=treebase; sp1<space;)
		*sp1++ = *sp2++;
	label(contlab);
	rcexpr(st, efftab);
	branch(l);
	return(0);
}

pexpr()
{
	register o, t;

	if ((o=symbol())!=LPARN)
		goto syntax;
	t = tree();
	if ((o=symbol())!=RPARN)
		goto syntax;
	return(t);
syntax:
	error("Statement syntax");
	errflush(o);
	return(0);
}

pswitch()
{
	int *sswp, swlab;
	register int *swb, *wswp, dl;

	swb = sswp = swp;
	if (swp==0)
		swb = swp = swtab;
	branch(swlab=isn++);
	dl = deflab;
	deflab = 0;
	statement(0);
	branch(brklab);
	label(swlab);
	putchar('#');		/* switch is pseudo-expression */
	if (deflab==0)
		deflab = brklab;
	wswp = swp;
	putw(wswp-swb, obuf);
	putw(deflab, obuf);
	putw(4, obuf);	/* table 4 is switch */
	putw(line, obuf);
	while (swb < wswp)
		putw(*swb++, obuf);
	label(brklab);
	deflab = dl;
	swp = sswp;
}

blkhed()
{
	register pl;
	register struct hshtab *cs;

	autolen = 6;
	declist(0);
	pl = 4;
	while(paraml) {
		parame->hoffset = 0;
		cs = paraml;
		paraml = paraml->hoffset;
		if (cs->htype==FLOAT)
			cs->htype = DOUBLE;
		cs->hoffset = pl;
		cs->hclass = AUTO;
		if ((cs->htype&XTYPE) == ARRAY) {
			cs->htype =- (ARRAY-PTR);	/* set ptr */
			cs->ssp++;		/* pop dims */
		}
		pl =+ rlength(cs);
	}
	for (cs=hshtab; cs<hshtab+hshsiz; cs++) {
		if (cs->name[0] == '\0')
			continue;
		/* check tagged structure */
		if (cs->hclass>KEYWC && (cs->htype&TYPE)==RSTRUCT) {
			cs->lenp = dimtab[cs->lenp&0377]->lenp;
			cs->htype = cs->htype&~TYPE | STRUCT;
		}
		if (cs->hclass == STRTAG && dimtab[cs->lenp&0377]==0)
			error("Undefined structure: %.8s", cs->name);
		if (cs->hclass == ARG)
			error("Not an argument: %.8s", cs->name);
		if (stflg)
			prste(cs);
	}
	osleft = ossiz;
	space = treebase;
	rcexpr(block(1, SETREG, regvar), regtab);
	return(autolen);
}

blkend() {
	register struct hshtab *cs;

	for (cs=hshtab; cs<hshtab+hshsiz; cs++) {
		if (cs->name[0]) {
			if (cs->hclass==0 && (cs->hflag&FNUND)==0) {
				error("%.8s undefined", cs->name);
				cs->hflag =| FNUND;
			}
			if((cs->hflag&FNDEL)==0) {
				cs->name[0] = '\0';
				hshused--;
			}
		}
	}
}

prste(acs)
{
	register struct hshtab *cs;
	register rflg;

	rflg = 0;
	cs = acs;
	switch (cs->hclass) {
	case REG:
		rflg = 'r';

	case AUTO:
		printf("~%.8s=%c%o\n", cs->name, rflg, cs->hoffset);
		return;

	case STATIC:
		printf("~%.8s=L%d\n", cs->name, cs->hoffset);
		return;
	}
}

errflush(ao)
{
	register o;

	o = ao;
	while(o>RBRACE)	/* ; { } */
		o = symbol();
	peeksym  = o;
}

declist(sclass)
{
	register sc, elsize, offset;
	int type;

	offset = 0;
	sc = sclass;
	while ((elsize = getkeywords(&sclass, &type)) != -1) {
		offset = declare(sclass, type, offset, elsize);
		sclass = sc;
	}
	return(offset);
}

getkeywords(scptr, tptr)
int *scptr, *tptr;
{
	register skw, tkw, longf;
	int o, elsize, isadecl;

	isadecl = 0;
	longf = 0;
	tkw = -1;
	skw = *scptr;
	elsize = 0;
	while ((o=symbol())==KEYW) {
		switch(cval) {

		case LONG:
			longf++;
			break;
	
		case AUTO:
		case STATIC:
		case EXTERN:
		case REG:
			if (skw && skw!=cval)
				error("Conflict in storage class");
			skw = cval;
			break;
	
		case STRUCT:
			o = STRUCT;
			elsize = strdec(&o, skw==MOS);
			cval = o;
		case INT:
		case CHAR:
		case FLOAT:
		case DOUBLE:
			if (tkw>=0)
				error("Type clash");
			tkw = cval;
			isadecl++;
			if (skw==MOS)
				goto bbreak1;
			break;
	
		default:
			goto bbreak;
		}
		isadecl++;
	}
    bbreak:
	peeksym = o;
    bbreak1:
	if (isadecl==0)
		return(-1);
	if (tkw<0)
		tkw = INT;
	if (skw==0)
		skw = AUTO;
	if (longf) {
		if (tkw==FLOAT)
			tkw = DOUBLE;
		else if (tkw==INT)
			tkw = LONG;
		else
			error("Misplaced 'long'");
	}
	*scptr = skw;
	*tptr = tkw;
	return(elsize);
}

strdec(tkwp, mosf)
int *tkwp;
{
	register elsize, o;
	register struct hshtab *ssym;
	struct hshtab *ds;

	mosflg = 1;
	ssym = 0;
	if ((o=symbol())==NAME) {
		ssym = csym;
		if (ssym->hclass==0) {
			ssym->hclass = STRTAG;
			ssym->lenp = dimp;
			chkdim();
			dimtab[dimp++] = 0;
		}
		if (ssym->hclass != STRTAG)
			redec();
		mosflg = mosf;
		o = symbol();
	}
	mosflg = 0;
	if (o != LBRACE) {
		if (ssym==0) {
		syntax:
			decsyn(o);
			return(0);
		}
		if (ssym->hclass!=STRTAG)
			error("Bad structure name");
		if ((elsize = dimtab[ssym->lenp&0377])==0) {
			*tkwp = RSTRUCT;
			elsize = ssym;
		}
		peeksym = o;
	} else {
		ds = defsym;
		mosflg = 0;
		elsize = declist(MOS);
		elsize = (elsize+ALIGN) & ~ ALIGN;
		defsym = ds;
		if ((o = symbol()) != RBRACE)
			goto syntax;
		if (ssym) {
			if (dimtab[ssym->lenp&0377])
				error("%.8s redeclared", ssym->name);
			dimtab[ssym->lenp&0377] = elsize;
		}
	}
	return(elsize);
}

chkdim()
{
	if (dimp >= dimsiz) {
		error("Dimension/struct table overflow");
		exit(1);
	}
}
