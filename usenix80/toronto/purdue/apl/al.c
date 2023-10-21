#
/*
 *	monadic epsilon and encode /rww
 */
#include	"apl.h"

ex_meps()
{
        struct item *p;
        register i,j;
        char *a,*b,*c;
        int dim0,dim1;
	int xpcp;

	p = fetch1();
	if ( p->rank > 2 || p->type != CH )
		error("execute C");
	/*get out if nothing to do, apr 2-23-77 */
	if (p->size == 0){
		return;
	}
	b = p->datap;
	dim0 = p->rank < 2 ? 1       : p->dim[0];
	dim1 = p->rank < 2 ? p->size : p->dim[1];
	a = alloc ( dim1+1 );
	xpcp = pcp;
	for ( i=0; i<dim0 ; i++) {
		copy(CH, b, a, dim1);
		a[dim1] = '\n';
		c = compile(a,1);
		if(c != 0){
			execute(c);
			free(c);
		} else {
			free(a);
			error("");
		}
		b =+ dim1;
		if(i < dim0-1)
			pop();
	}
	free(a);
	pcp = xpcp;
	p = *--sp;
	pop();
	*sp++ = p;
}

ex_menc()
{
	struct item *p;

	p = fetch1();
	if ( p->type == DA )
		menc1();	/*
	else
		return (char argument unchanged);	*/
}


ex_list()	/* List a function on the terminal */
{
	register char lastc;
	register struct item *n;
	register line;
	char c;


	/* Check for valid function */

	n = *--sp;
	if (n->type != LV)
		error("fnlist B");


	/* If a function, locate it in workspace file and
	 * print on the terminal in formatted form.
	 */

	switch(n->use){
	default:
		error("fnlist T");

	case NF:
	case MF:
	case DF:
		SEEKF(wfile, n->label, 0);
		line = 0;
		lastc = 0;
		putchar('\n');

		while(READF(wfile, &c, 1) > 0){

			if (!c){
				putchar('\n');
				return;
			}

			switch(lastc){
			case '\n':
				printf("[%d]", ++line);
			case 0:
				putchar('\t');
			}
			putchar(lastc=c);
		}
		error("workspace eof");
	}
}


ex_crp()         /* dredge up a function and put it into an array*/
{
	char name[NAMS];
	char *c, *c2;
	struct nlist *np;
	struct item *p;
	int len, dim0, dim1;
	register i;
	register char *dp;

	p = fetch1();
	if ( p->size == 0 || p->rank >1 || p->size >= NAMS )
		error("Lcr C");
			/* set up the name in search format     */
	copy(CH, p->datap, name, p->size);
	name[p->size] = '\0';
	np = nlook(name);
			/* if not found then domain error       */
	if ( !np->namep )
		error("Lcr D");
	switch(np->use){
	default:
		error("Lcr D");
	case MF:
	case DF:
	case NF:	/* only allow functions */
		;
	}
			/* set up new array                     */
	dim0 = 0;
	dim1 = 0;
	ifile = DUPF(wfile);
	SEEKF( ifile, np->label, 0);    /* look up function     */
			/* compute max width and height         */
	while ( c2 = c = rline(0) ){
	       while ( *c2++ != '\n' ){}
		dim0++;
		len = c2 - c - 1;
		dim1 = dim1 < len ? len : dim1;
		free(c);
	}
	pop();                /* release old variable         */
			/* create new array and put function in */
	p = newdat ( CH, 2, dim0*dim1 );
	p->rank = 2;
	p->dim[0] = dim0;
	p->dim[1] = dim1;
	dp = p->datap;
	SEEKF( ifile, np->label, 0);
	while ( c2 = c = rline(0) ){
	       for ( i=0; i<dim1; i++)
			if ( *c != '\n' )
				*dp++ = *c++;
			else
				*dp++ = ' ';    /* fill w/blanks*/
		free(c2);
	}
			/* put the new array on the stack       */
	*sp++ = p;
				/* reset the current file               */
	CLOSEF(ifile);
	ifile = 0;
}

menc1()                 /* change numbers into characters       */
{
	struct item *p, *q;
	register i,j,numsz;
	data *dp;
	int total,param[4];

			/* zeroize size information vector      */
	for ( i=0; i<4; i++ )
		param[i] = 0;
			/* pick up the argument                 */
	p = fetch1();
	if(p->rank > 2)
		error("format R");
	dp = p->datap;
			/* find the maximum # of chars in any # */
	for(i=0; i<p->size; i++)
		epr1(*dp++, param);
	numsz = param[1] + param[2] + !!param[2] + param[3] + 1;
			/* rowsize is max # size x last dim     */
	rowsz = p->rank ? p->dim[p->rank-1] : 1;
	rowsz =* numsz;
			/* row size x # of rows (incl blank)    */
	total = p->size * numsz;
	for( j=i=0; i<p->rank; i++ )
		if ( p->dim[i] != 1)
			if ( j++ > 1 )
				total =+ rowsz;
			/* make new data and fill with blanks   */
	if(p->rank == 2){
		q = newdat(CH, 2, total);
		q->dim[0] = total/rowsz;
		q->dim[1] = rowsz;
	} else {
		/* rank = 0 or 1 */
		q = newdat( CH, 1, total);
		q->dim[0] = rowsz;
	}
	mencptr = q->datap;
	for ( i=0; i<total; i++)
		*mencptr++ = ' ';
	mencptr = q->datap;
			/* use putchar() to fill up the array   */
	mencflg = 2;
	ex_hprint();
	mencflg = 0;
			/* put it on the stack                  */
/*	pop();		/* done by ex_hprint() */
	*sp++ = q;
}


ex_run()
{
	register struct item *p;
	register data *dp;
	register int *p2;
	char ebuf[100];
	int i;
	int *run();

	p = fetch1();
	if(p->type != CH || p->rank != 1)
		error("Lrun D");
	copy(CH, p->datap, ebuf, p->size);
	ebuf[p->size] = 0;
	p2 = run(ebuf);
	p = newdat(DA, 1, 0);
	pop();
	*sp++ = p;
}

int *run(s)
char *s;
{
	register p;
	static int a[3];
	int oldint, oldquit;

	oldint = signal(2, 1);
	oldquit = signal(3, 1);
	if(a[0]=fork()){
		while((p = wait0(a+1)) != -1)
			if(p == a[0])
				break;
	} else {
		execl("/bin/sh", "-", "-c", s, 0);
		WRITEF(1, "can't find shell\n", 17);
		exit();
	}
	a[2] = (a[1]>>8)&0377;
	a[1] =& 0377;
	signal(2, oldint);
	signal(3, oldquit);
	return(a);
}

ex_dfmt()
{
	error("dfmt B");
}

ex_mfmt()
{
	ex_menc();
}

ex_nc()
{
	register struct nlist *np;
	register struct item *p;
	register char *q;
	int i;
	char buf[40];

	p = fetch1();
	if(p->type != CH)
		error("Lnc T");
	if(p->size >= 40 || p->rank > 1)
		error("Lnc D");
	copy(CH, p->datap, buf, p->size);
	buf[p->size] = 0;
	np = nlook(buf);
	i = 0;
	if(np != 0)
	switch(np->use){
	case 0:
		i = 0; break;
	case MF:
	case NF:
	case DF:
		i = 3; break;
	case DA:
	case CH:
	case LV:
		i = 2; break;
	default:
		printf("unknown Lnc type = %d\n", np->use);
		i = 4;
	}
	p = newdat(DA, 0, 1);
	p->datap[0] = i;
	pop();
	*sp++ = p;
}
