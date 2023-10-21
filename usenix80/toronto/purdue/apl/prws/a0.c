#

/*
 * this is  a special version of a0.c, not the original.
 * it contains various low level routines needed by prws.
 */

#include "apl.h"
int	chartab[];

putto(col)
{
	while(col > column+8)
		putchar('\t');
	while(col > column)
		putchar(' ');
}

fix(d)
data d;
{
	register i;

	i = floor(d+0.5);
	return(i);
}

error(s)
char *s;
{
	register c;
	register char *cp;
	char bbb[2];

	intflg = 0;
	if(ifile) {
		close(ifile);
		ifile = 0;
	}
	cp = s;
	while(c = *cp++) {
		if(c >= 'A' && c <= 'Z') {
			switch(c) {

			case 'I':
				c = "\ninterrupt";
				break;

			case 'L':
				c = "L";
				break;

			case 'C':
				c = "conformability";
				break;

			case 'S':
				c = "syntax";
				break;

			case 'R':
				c = "rank";
				break;

			case 'X':
				c = "index";
				break;

			case 'Y':
				c = "character";
				break;

			case 'M':
				c = "memory";
				break;

			case 'D':
				c = "domain";
				break;

			case 'T':
				c = "type";
				break;

			case 'E':
				c = "error";
				break;

			case 'P':
				c = "programmer";

			case 'B':
				c = "botch";
				break;

			default:
				bbb[0] = c;
				bbb[1] = 0;
				c = bbb;
			}
			printf(c);
			continue;
		}
		putchar(c);
	}
	putchar('\n');
	ibeam36 = gsip->funlc;
	gsip->funlc = 0;
	while(sp > stack)
		pop();
	reset();
}

printf(f, a)
char *f;
{
	register char *s, *cp;
	register *p;

	s = f;
	p = &a;
	while(*s) {
		if(s[0] == '%' && s[1] == 'd') {
			putn(*p++);
			s =+ 2;
		} else if(s[0] == '%' && s[1] == 'o') {
			puto(*p++);
			s =+ 2;
		} else if(s[0] == '%' && s[1] == 's') {
			cp = *p++;
			s =+ 2;
			while(*cp)
				putchar(*cp++);
		} else
			putchar(*s++);
	}
}

putn(n)
{
	register a;

	if(n < 0) {
		n = -n;
		if(n < 0) {
			printf("32768");
			return;
		}
		putchar('-');	/* apl minus sign, was '"' */
	}
	if(a=n/10)
		putn(a);
	putchar(n%10 + '0');
}

puto(n)
{
	if(n&0177770)
		puto( (n>>3) & 017777);
	putchar('0'+ ( n&7 ) );
}

getchar()
{
	int c;

	c = 0;
	if(read(ifile, &c, 1) == 1){
		if(echoflg == 1 && ifile == 0)
			write(1, &c, 1);
		if(protofile && ifile == 0)
			write(protofile, &c, 1);
	}
	return(c);
}

fuzz(d1, d2)
data d1, d2;
{
	double f1, f2;

	f1 = d1;
	if(f1 < 0.)
		f1 = -f1;
	f2 = d2;
	if(f2 < 0.)
		f2 = -f2;
	if(f2 > f1)
		f1 = f2;
	f1 =* thread.fuzz;
	if(d1 > d2) {
		if(d2+f1 >= d1)
			return(0);
		return(1);
	}
	if(d1+f1 >= d2)
		return(0);
	return(-1);
}

pop()
{

	dealloc(*--sp);
}

erase(np)
struct nlist *np;
{
	register *p;

	p = np->itemp;
	if(p) {
		switch(np->use) {
		case NF:
		case MF:
		case DF:
			for(; *p>0; (*p)--)
				free(p[*p]);

		}
		free(p);
		np->itemp = 0;
	}
	np->use = 0;
}

dealloc(p)
struct item *p;
{

	switch(p->type) {

	case DA:
	case CH:
	case QQ:
	case QD:
	case QC:
		free(p);
	}
}

newdat(type, rank, size)
{
	register i;
	register struct item *p;

	if(rank > MRANK)
		error("max R");
	i = sizeof *p + rank * SINT;
	if(type == DA)
		i =+ size * SDAT; else
	if(type == CH)
		i =+ size;
	p = alloc(i);
	p->rank = rank;
	p->type = type;
	p->size = size;
	p->index = 0;
	if(rank == 1)
		p->dim[0] = size;
	p->datap = &p->dim[rank];
	return(p);
}

dupdat(ap)
struct item *ap;
{
	register struct item *p1, *p2;
	register i;

	p1 = ap;
	p2 = newdat(p1->type, p1->rank, p1->size);
	for(i=0; i<p1->rank; i++)
		p2->dim[i] = p1->dim[i];
	copy(p1->type, p1->datap, p2->datap, p1->size);
	return(p2);
}

copy(type, from, to, size)
char *from, *to;
{
	register i;
	register char *a, *b;
	int s;

	if((i = size) == 0)
		return(0);
	a = from;
	b = to;
	if(type == DA)
		i =* SDAT; else
	if(type == IN)
		i =* SINT;
	s = i;
	do
		*b++ = *a++;
	while(--i);
	return(s);
}

fetch1()
{
	register struct item *p;

	p = fetch(sp[-1]);
	sp[-1] = p;
	return(p);
}

fetch2()
{
	register struct item *p;

	sp[-2] = fetch(sp[-2]);
	p = fetch(sp[-1]);
	sp[-1] = p;
	return(p);
}

fetch(ip)
struct item *ip;
{
	register struct item *p, *q;
	register i;
	struct nlist *n;
	int c;

	p = ip;

loop:
	switch(p->type) {

	case DU:
		if(lastop != PRINT)
			error("no fn result");

	case DA:
	case CH:
		p->index = 0;
		return(p);

	case LV:
		if(p->use != DA)
			error("used before set\n");
		p = p->itemp;
		i = p->type;
		if(i == LBL)
			i = DA;		/* treat label as data */
		q = newdat(i, p->rank, p->size);
		copy(IN, p->dim, q->dim, p->rank);
		copy(i, p->datap, q->datap, p->size);
		return(q);

	default:
		error("fetch B");
	}
}

topfix()
{
	register struct item *p;
	register i;

	p = fetch1();
	if(p->type != DA || p->size != 1)
		error("topval C");
	i = fix(p->datap[0]);
	pop();
	return(i);
}

bidx(ip)
struct item *ip;
{
	register struct item *p;

	p = ip;
	idx.type = p->type;
	idx.rank = p->rank;
	copy(IN, p->dim, idx.dim, idx.rank);
	size();
}

size()
{
	register i, s;

	s = 1;
	for(i=idx.rank-1; i>=0; i--) {
		idx.del[i] = s;
		s =* idx.dim[i];
	}
	idx.size = s;
	return(s);
}

colapse(k)
{
	register i;

	if(k < 0 || k >= idx.rank)
		error("collapse X");
	idx.dimk = idx.dim[k];
	idx.delk = idx.del[k];
	for(i=k; i<idx.rank; i++) {
		idx.del[i] = idx.del[i+1];
		idx.dim[i] = idx.dim[i+1];
	}
	idx.size =/ idx.dimk;
	idx.rank--;
}

forloop(co, arg)
int (*co)();
{
	register i;

	if(idx.rank == 0) {
		(*co)(arg);
		return;
	}
	for(i=0;;) {
		while(i < idx.rank)
			idx.idx[i++] = 0;
		(*co)(arg);
		while(++idx.idx[i-1] >= idx.dim[i-1])
			if(--i <= 0)
				return;
	}
}

access()
{
	register i, n;

	n = 0;
	for(i=0; i<idx.rank; i++)
		n =+ idx.idx[i] * idx.del[i];
	return(n);
}

data
getdat(ip)
struct item *ip;
{
	register struct item *p;
	register i;
	data d;

	p = ip;
	i = p->index;
	while(i >= p->size) {
		if(i == 0)
			error("getdat B");
		i =- p->size;
	}
	if(p->type == DA) {
		d = p->datap[i];
	} else
	if(p->type == CH) {
		d = p->datap->c[i];
	} else
		error("getdat B");
	i++;
	p->index = i;
	return(d);
}

putdat(ip, d)
data d;
struct item *ip;
{
	register struct item *p;
	register i;

	p = ip;
	i = p->index;
	if(i >= p->size)
		error("putdat B");
	if(p->type == DA) {
		p->datap[i] = d;
	} else
	if(p->type == CH) {
		p->datap->c[i] = d;
	} else
		error("putdat B");
	i++;
	p->index = i;
}

aplmod(n)
{
	char m[6];

	if(n){
		gtty(0, m);
		if(m[2] == '\b' || m[3] == '\b')
			printf("warning: erase char is ^H\n");
	}
}

s2vect(ap)
struct item *ap;
{
	register struct item *p, *q;

	p = ap;
	q = newdat(p->type, 1, 1);
	q->datap = p->datap;
	q->dim[0] = 1;
	return(q);
}

struct nlist *
nlook(name)
char *name;
{
	register struct nlist *np;

	for(np = nlist; np->namep; np++)
		if(equal(np->namep, name))
			return(np);
	return(0);
}

checksp()
{
	if(sp >= &stack[STKS])
		error("stack overflow");
}

char *ty[] {
0,"DA","CH","LV","QD","QQ","IN","EL","NF","MF","DF","QC","QV","DU"
};

dstack()
{
	register struct item **p;
	register i,n;

	p = sp;
	n = (p - stack) / 2;
	while(--p > stack){
		printf("\tSP[%d]:\ttype = ", n--);
		if((i=(*p)->type) >= 0 && i <= DU && ty[i])
			printf(ty[i]);
		else
			printf("%d", (*p)->type);
		printf(",  rank = %d\n", (*p)->rank);
	}
	putchar('\n');
}

concat(s1,s2)
char *s1, *s2;
{
	register i,j;
	char *p,*q;

	i = lsize(s1) - 1;
	j = lsize(s2) - 1;
	p = q = alloc(i+j);
	p =+ copy(CH, s1, p, i);
	copy(CH, s2, p, j);
	return(q);
}

opn(file, rw)
{
	register fd, (*p)();
	extern open(), creat();

	p = (rw > 2 ? creat : open);
	if((fd = (*p)(file,rw)) < 0){
		printf("can't open file %s\n", file);
		error("");
	}
	return(fd);
}

equal(a, b)
char *a, *b;
{
	register char *c1, *c2;

	c1 = a;
	c2 = b;
	while(*c1++ == *c2)
		if(*c2++ == 0)
			return(1);
	return(0);
}

lsize(s)
char *s;
{
	register i;
	register char *p;

	i=1;
	p=s;
	while (*p++) i++;
	return(i);
}

