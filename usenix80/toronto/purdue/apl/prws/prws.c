#
/*
 * prws -- print apl workspace contents
 *
 * % prws [-r] [-v] [-h] [-n] [-f] [-d] file ...
 *
 */

#include "../apl.h"

#define VFLAG	01
#define FFLAG	02
#define HFLAG	04
#define NFLAG	08

int ffile;

int vflg;	/* variables only */
int fflg;	/* functions only */
int nflg;	/* names only */
int hflg;	/* no header info */
int oflg;	/* enable true overprinting */
int rflg;	/* RSTS compatability mode */

main(argc, argp)
char **argp;
{
	sp = stack;
	setexit();
	close(ffile);

	while(argc-- > 1){
		argp++;
		if(**argp == '-'){
			switch(argp[0][1]){
			case 'd':
			case 'v':	vflg++;	break;
			case 'f':	fflg++;	break;
			case 'r':	rflg++;
			case 'h':	hflg++;	break;
			case 'n':	nflg++;	break;
			case 'o':	oflg++;	break;
			}
		} else {
			doit(*argp);
		}
	}
}


doit(arg)
char *arg;
{
	register i;
	register struct item *ip;
	register struct item *p;
	char c, name[NAMS];
	int b[18], iz, dconv;
	struct { int word; };

	printf("%s ", arg);
	if((ffile = open(arg, 0)) < 0){
		printf("-- cannot open\n");
		return;
	}
	iz = 0;

/* Check for correct magic number */

	read(ffile,&iz,2);
	if((iz|1) != (MAGIC|1)){
barf:
		close(ffile);
		error("-- bad file format");
	}
	printf("-- created on ");
	fdat(ffile);
	if(iz > MAGIC){
		hpr("[single precision version]\n");
		dconv = 2;
	} else if(iz < MAGIC){
		hpr("[double precision version]\n");
		dconv = 1;
	} else
		dconv = 0;
	read(ffile,&thread,sizeof thread);
	if(!hflg){
		printf("origin = %d\n", thread.iorg);
		printf("digits = %d\n", thread.digits);
		printf("width  = %d\n", thread.width);
		printf("\n\n");
	}
	while(read(ffile,&iz,2) == 2) {
		i = iz.c[1];
/* read name of vbl or fn */
		read(ffile,name,i);
		switch(iz.c[0]) {
default:
			goto barf;

case DA:
			read(ffile,&iz,2);
			p=alloc(iz);
			read(ffile,p,iz);
			p->datap =  &p->dim[p->rank]; /*make absolute*/
			/*
			 * convert data type if neccessary
			 */
			*sp++ = ip = convrt(dconv, p);
			if(!fflg){
				printf("%s  {  ", name);
				if(ip->rank){
					for(i=0; i<ip->rank; i++)
						printf(" %d", ip->dim[i]);
					if(rflg && ip->type == CH)
						printf(" ''");
					printf(" R\n");
				}
				if(!nflg){
					ex_printfint();
					printf("\n\n");
				}
			}
			break;
case NF:
case MF:
case DF:
			if(nflg)
				printf("%s\n", name);
			prfn(ffile);
			break;
		}
	}
	close(ffile);
	printf("\n\n");
}

prfn(fd)
{
	register lnum, nl;
	char c;

	lnum = 0;
	nl = 0;
	if(!(nflg||vflg))
		putchar('\t');
	if(rflg)
		printf("G  ");
	while(1) {
		read(fd,&c,1);
		switch(c){
		case '\0':
			if(!(nflg|vflg)){
				if(rflg)
					printf("\n\tG");
				printf("\n\n");
			}
			return;

		case '\n':
			nl++;
			continue;

		default:
			if(vflg || nflg)
				continue;
			while(nl){
				printf("\n[%d]\t", ++lnum);
				nl--;
			}
			putchar(c);
		}
	}
}

int lastc;
int colnum	0;
int olmax	0;
char oline[200];

putchar(c)
char c;
{
	register i, cn;

	if(!oflg && lastc == '\b'){
		oline[olmax - 2] = lastc = c;
		return;
	}
	lastc = c;
	if(c < ' ' || c >= 0177){
		switch(c){
		default:
			printf("\\%o", c&0377);
			return;
		case '\n':
			colnum = 0;
			if(olmax){
				write(1, "\n", 1);
				write(1, oline, olmax);
			}
			olmax = 0;
			break;
		case '\b':
			if(oflg)
				break;
			if(olmax == 0)
				for(i=0; i<200; i++)
					oline[i] = ' ';
			olmax = colnum;
			return;
		case '\t':
			colnum =+ 8 - colnum%8;
			break;
		}
	}
	write(1, &c, 1);
	colnum++;
}

fdat(f)
{
	int b[18];
	register *p;

	fstat(f,b);
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
	putchar('\n');
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

hpr(f,a,b,c,d,e)
{
	if(hflg == 0)
		printf(f,a,b,c,d,e);
}
