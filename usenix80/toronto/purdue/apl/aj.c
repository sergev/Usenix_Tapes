#include "apl.h"

clear()
{
	register struct nlist *n;
	extern freelist[], end;

	for(n=nlist; n->namep; n++) {
		n->itemp = n->use = 0;
		n->namep = 0;
		}
		freelist[0] = 0;
		freelist[1] = -1;	/*empty free list*/
		brk(&end);	/*shrink core*/
		gsip = 0;	/* reset state indicator */
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

isize(ip)
struct item *ip;
{
	register struct item *p;
	register i;

	p=ip;
	i=sizeof *p + p->rank*SINT;
	if(p->type == DA)
		i =+ p->size*SDAT; else
	if(p->type == CH)
		i =+ p->size;
	return(i);
}

wsload(ffile)
{
	int b[18];
	char name[NAMS];
	int iz;
	register i;
	register struct nlist *n;
	register struct item *p;
	char c;
	int dconv;
	struct {
		int word;
	};

	iz = 0;
/* Check for correct magic number */
	READF(ffile,&iz,2);
	if((iz|1) != (MAGIC|1)){
barf:
		CLOSEF(ffile);
		error("bad ws file format");
	}
	if(iz > MAGIC){
		printf("single data converted to double\n");
		dconv = 2;
	} else if(iz < MAGIC){
		printf("double data converted to single\n");
		dconv = 1;
	} else
		dconv = 0;
	READF(ffile,&thread,sizeof thread);
	while(READF(ffile,&iz,2) == 2) {
		i = iz.c[1];
/* read name of vbl or fn */
		READF(ffile,name,i);
		for(n=nlist; n->namep; n++)
			if(equal(name, n->namep)){
				erase(n);
				goto hokay;
			}
		n->namep = alloc(i);
		copy(CH,name,n->namep,i);
hokay:
		n->use = iz.c[0];
		n->type = LV;
		switch(n->use) {
default:
			goto barf;

case DA:
			READF(ffile,&iz,2);
			p=alloc(iz);
			READF(ffile,p,iz);
			p->datap =  &p->dim[p->rank]; /*make absolute*/
				/*
				 * convert data type if neccessary
				 */
			n->itemp = convrt(dconv, p);
			continue;
case NF:
case MF:
case DF:
			n->itemp = 0;
			FSTATF(wfile,b);
			n->label=b[5];
			SEEKF(wfile,b[5],0);
			do {
				if(READF(ffile,&c,1) != 1)
					error("wsload eof");
				WRITEF(wfile,&c,1);
			} while(c != 0);
		}
	}
	fdat(ffile);
	CLOSEF(ffile);
}

wssave(ffile)
{
	register struct nlist *n;

	nsave(ffile, 0);
	for(n=nlist; n->namep; n++)
		nsave(ffile, n);
	fdat(ffile);
	CLOSEF(ffile);
}

vsave(fd)
{
	register struct nlist *n;

	nsave(fd, 0);
	while(n = getnm())
		nsave(fd, n);
	fdat(fd);
	CLOSEF(fd);
}

nsave(ffile, an)
struct nlist *an;
{
	int iz;
	register struct nlist *n;
	register i;
	register struct item *p;
	char c;
	struct {
		int word;
	};

	 n = an;
	if(n == 0){
		iz = MAGIC;
		WRITEF(ffile,&iz,2);
		WRITEF(ffile,&thread,sizeof thread);
		return(0);
	}

	if(n->use == 0 || (n->use == DA && n->itemp == 0))
		return(0);
	iz.c[0] = n->use;
	iz.c[1] = i = lsize(n->namep);
	WRITEF(ffile,&iz,2);
	WRITEF(ffile,n->namep,i);

	switch(n->use) {
default:
		CLOSEF(ffile);
		error("save B");
case DA:
		p = n->itemp;
		iz = i = isize(p);
		p -> label =- (&p)->word;
		WRITEF(ffile,&iz,2);
		WRITEF(ffile,p,i);
		p->label =+ (&p)->word;
		break;
case NF:
case MF:
case DF:
		SEEKF(wfile,n->label,0);
		do {
			READF(wfile,&c,1);
			WRITEF(ffile,&c,1);
		} while(c != 0);
	}
	return(0);
}

getnm()
{
	char name[100];
	register char *p;
	register struct nlist *n;
	register c;

	while(1){
		printf("variable name? ");
		c = READF(1, name, 100);
		if(c <= 1)
			return(0);
		name[c-1] = 0;
		for(n=nlist; n->namep; n++)
			if(equal(name, n->namep))
				return(n);
		printf("%s does not exist\n", name);
	}
}

listdir()
{
	register f;
	struct {
	int in;
	char nam[14];
	} dir;

	if((f = OPENF(".",0)) < 0)
		error("directory B");
	while(READF(f,&dir,sizeof dir) == sizeof dir)
		if(dir.in != 0 && dir.nam[0] != '.') {
			if(column+10 >= thread.width)
				printf("\n\t");
			printf(dir.nam);
			putchar('\t');
		}
	putchar('\n');
	CLOSEF(f);
}

fdat(f)
{
	int b[18];
	register *p;

	FSTATF(f,b);
	p = localtime(&b[16]);

	printf("  ");
	pr2d(p[2]);
	putchar('.');
	pr2d(p[1]);
	putchar('.');
	pr2d(p[0]);
	putchar(' ');
	pr2d(p[4]+1);
	putchar('/');
	pr2d(p[3]);
	putchar('/');
	pr2d(p[5]);
}

pr2d(i)
{
	putchar(i/10+'0');
	putchar(i % 10 + '0');
}

convrt(m, p)
struct item *p;
{
	register i;
	register float *f;
	register double *d;
	struct item *q;

	switch(m){
	case 0:
		return(p);

	case 1:		/* apl to apl2 */
		q = newdat(DA, p->rank, p->size);
		f = q->datap;
		d = p->datap;
		for(i=0; i<p->size; i++)
			*f++ = *d++;
		break;

	case 2:		/* apl2 to apl */
		q = newdat(DA, p->rank, p->size);
		f = p->datap;
		d = q->datap;
		for(i=0; i<p->size; i++)
			*d++ = *f++;
		break;
	}
	for(i=0; i<p->rank; i++)
		q->dim[i] = p->dim[i];
	free(p);
	return(q);
}
