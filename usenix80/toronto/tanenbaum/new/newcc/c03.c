#
/* C compiler

Copyright 1972 Bell Telephone Laboratories, Inc. 

*/

#include "c0h.c"

decref(at)
{
	register t;

	t = at;
	if ((t & ~TYPE) == 0) {
		error("Illegal indirection");
		return(t);
	}
	return((t>>TYLEN) & ~TYPE | t&TYPE);
}

incref(t)
{
	return(((t&~TYPE)<<TYLEN) | (t&TYPE) | PTR);
}

cbranch(tree, lbl, cond)
struct tnode *tree;
{
	rcexpr(block(1,CBRANCH,tree,lbl,cond),cctab);
}

rcexpr(tree, table)
int table;
struct tnode *tree;
{
	register c, *sp;

	if (tree == 0)
		return;
	putchar('#');
	c = space-treebase;
	sp = treebase;
	putw(c, obuf);
	putw(tree, obuf);
	putw(table, obuf);
	putw(line, obuf);
	while(c--)
		putw(*sp++, obuf);
}

branch(lab) {
	printf("jbr\tL%d\n", lab);
}

label(l) {
	printf("L%d:", l);
}

plength(ap)
struct tname *ap;
{
	register t, l;
	register struct tname *p;

	p = ap;
	if (p==0 || ((t=p->type)&~TYPE) == 0)		/* not a reference */
		return(1);
	p->type = decref(t);
	l = length(p);
	p->type = t;
	return(l);
}

length(acs)
struct tnode *acs;
{
	register t, n;
	register struct tnode *cs;

	cs = acs;
	t = cs->type;
	n = 1;
	while ((t&XTYPE) == ARRAY) {
		t = decref(t);
		n = dimtab[cs->ssp&0377];
	}
	if ((t&~TYPE)==FUNC)
		return(0);
	if (t>=PTR)
		return(2*n);
	switch(t&TYPE) {

	case INT:
		return(2*n);

	case CHAR:
		return(n);

	case FLOAT:
	case LONG:
		return(4*n);

	case DOUBLE:
		return(8*n);

	case STRUCT:
		return(n * dimtab[cs->lenp&0377]);

	case RSTRUCT:
		error("Bad structure");
		return(0);
	}
	error("Compiler error (length)");
}

rlength(cs)
struct tnode *cs;
{
	return((length(cs)+ALIGN) & ~ALIGN);
}

simplegoto()
{
	register struct hshtab *csp;

	if ((peeksym=symbol())==NAME && nextchar()==';') {
		csp = csym;
		if (csp->hclass==0 && csp->htype==0) {
			csp->htype = ARRAY;
			if (csp->hoffset==0)
				csp->hoffset = isn++;
		}
		if ((csp->hclass==0||csp->hclass==STATIC)
		 &&  csp->htype==ARRAY) {
			peeksym = -1;
			return(csp->hoffset);
		}
	}
	return(0);
}

nextchar()
{
	while (ctab[peekc]==SPACE)
		peekc = getchar();
	return(peekc);
}

chconbrk(l)
{
	if (l==0)
		error("Break/continue error");
}

dogoto()
{
	register struct tnode *np;

	*cp++ = tree();
	build(STAR);
	chkw(np = *--cp);
	rcexpr(block(1,JUMP,0,0,np), regtab);
}

doret()
{
	register struct tnode *t;

	if (nextchar() != ';') {
		t = tree();
		*cp++ = &funcblk;
		*cp++ = t;
		build(ASSIGN);
		cp[-1] = cp[-1]->tr2;
		build(RFORCE);
		rcexpr(*--cp, regtab);
	}
	branch(retlab);
}

putchar(c)
{
	register int *b;

	b = obuf;
	if (strflg)
		b = sbuf;
	putc(c, b);
}
