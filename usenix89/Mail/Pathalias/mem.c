/* pathalias -- by steve bellovin, as told to peter honeyman */
#ifndef lint
static char	*sccsid = "@(#)mem.c	8.1 (down!honey) 86/01/19";
#endif

#include "def.h"

long	lseek();
extern int	fdlink, fdnode;
extern int	nodecount, linkcount;
static nlinks, nnodes;
/* imported */
extern char *sbrk();

link	*
newlink()
{
	link	*rval;
	int	offset;

	if ((rval = (link * ) calloc(1, sizeof(link))) == 0)
		nomem();
	linkcount++;
	nlinks++;
	offset = lseek(fdlink, 0l, 2) / sizeof(link);

	rval->l_next_from = 0;
	rval->l_offset = offset;
	rval->l_to = 0;
	rval->l_cost = 0;
	rval->l_flag = '\0';

	if (write(fdlink, rval, sizeof(link)) == -1) {
		fprintf(stderr, "Temp file write error\n");
		badmagic(1);
	}
	return(rval);
}

node	*
newnode()
{
	node	*rval;
	int offset;

	if ((rval = (node * ) calloc(1, sizeof(node))) == 0)
		nomem();
	nodecount++;
	nnodes++;
	offset = lseek(fdnode, 0l, 2) / sizeof(node);

	rval->n_name[0] = '\0';
	rval->n_offset = offset;
	rval->n_link = 0;
	rval->n_net_root = 0;
	rval->n_prvparent = 0;
	rval->n_cost = 0;
	rval->n_tloc = 0;
	rval->n_flag = 0;

	if (write(fdnode, rval, sizeof(node)) == -1) {
		fprintf(stderr, "Temp file write error\n");
		badmagic(1);
	}
	Ncount++;
	return(rval);
}

#ifndef strclear
void
strclear(dst, len)
register char *dst;
register int len;
{
	while (--len >= 0)
		*dst++ = 0;
}
#endif /*strclear*/

int	*
newtable(size)
long	size;
{
	int	*rval;

	if ((rval = (int *) calloc(1, (unsigned int) (size * sizeof(*rval)))) == 0) 
		nomem();
	return(rval);
}

freetable(t)
int	*t;
{
	free((char *) t);
}

nomem()
{
	fprintf(stderr, "%s: Out of memory (%uk allocated)\n",
			ProgName, allocation());
	fprintf(stderr, "nodecount was %d, linkcount was %d\n", nodecount, linkcount);
	fprintf(stderr, "total nodes found %d, total links found %d\n", nnodes, nlinks);
	badmagic(1);
}

/* data space allocation -- main sets End very early */
allocation()
{
	static char	*dataspace;

	if (dataspace == 0) {	/* first time */
		dataspace = sbrk(0);		/* &end? */
		return(0);
	}
	return((sbrk(0) - dataspace)/1024);
}
