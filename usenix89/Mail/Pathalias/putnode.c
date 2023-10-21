/* putnode -- store node and link structrures temporarily in a disk file */

#include "def.h"

long	lseek();
extern int fdnode, fdlink;
extern int nodecount, linkcount;
static node tmpnode;
static node nullnode;
static link nulllink;
static link tmplink;

/* Opens temporary file */

meminit()
{
	nodecount = 0;
	linkcount = 0;
	fdnode = creat(NTEMPFILE, 0600);
	close(fdnode);
	fdnode = open(NTEMPFILE, 2);
	if (fdnode < 0) {
		fprintf(stderr, "meminit: temporary node file open error\n");
		badmagic(1);
	}
	fdlink = creat(LTEMPFILE, 0600);
	close(fdlink);
	fdlink = open(LTEMPFILE, 2);
	if (fdlink < 0) {
		fprintf(stderr, "meminit: temporary link file open error\n");
		badmagic(1);
	}

	nullnode.n_name[0] = '\0';
	nullnode.n_offset = 0;
	nullnode.n_link = 0;
	nullnode.n_net_root = 0;
	nullnode.n_prvparent = 0;
	nullnode.n_cost = 0;
	nullnode.n_tloc = 0;
	nullnode.n_flag = 0;
	write(fdnode, &nullnode, sizeof(nullnode));

	nulllink.l_next_from = 0;
	nulllink.l_offset = 0;
	nulllink.l_to = 0;
	nulllink.l_cost = 0;
	nulllink.l_flag = '\0';
	write(fdlink, &nulllink, sizeof(nulllink));
}

/* Frees memory and stores node in temp file; takes pointer to node */

putnode(n)
register node *n;
{
	if (n->n_offset != 0) {
		lseek(fdnode, (long) (((long) n->n_offset) * sizeof(node)), 0);
		if (write(fdnode, n, sizeof(*n)) == -1) {
			fprintf(stderr, "putnode: temporary file write error\n");
			badmagic(1);
		}
	}
	if (n != &tmpnode) {
		free((char *) n);
		nodecount--;
	}
}

/* Frees memory without rewriting into temp file; takes pointer to node */

freenode(n)
register node *n;
{
	if (n != &tmpnode) {
		free((char *) n);
		nodecount--;
	}
}

/* Frees memory and stores link in temp file; takes pointer to link */

putlink(l)
register link *l;
{
	if (l->l_offset != 0) {
		lseek(fdlink, (long) (((long) l->l_offset) * sizeof(link)), 0);
		if (write(fdlink, l, sizeof(*l)) == -1) {
			fprintf(stderr, "putlink: temporary file write error\n");
			badmagic(1);
		}
	}
	if (l != &tmplink) {
		free((char *) l);
		linkcount--;
	}
}

/* Frees memory without rewriting into temp file; takes pointer to link */

freelink(l)
register link *l;
{
	if (l != &tmplink) {
		free((char *) l);
		linkcount--;
	}
}

/* Refreshes node from possibly modified disk image */

regetnode(oldnode)
register node *oldnode;
{
	long	offset;

	if (oldnode->n_offset == 0)
		offset = nullnode.n_offset;
	else
		offset = oldnode->n_offset;

	lseek(fdnode, offset * sizeof(node), 0);
	if (read(fdnode, oldnode, sizeof(node)) <= 0) {
		fprintf(stderr, "regetnode: temporary file read error\n");
		badmagic(1);
	}
}

/* Returns pointer to node; takes long constant offset */

node *
getnode(offset)
register int offset;
{
	node	*tnode;

	if (offset == 0)
		offset = nullnode.n_offset;

	lseek(fdnode, (long) (((long) offset) * sizeof(node)), 0);
	if ((tnode = (node * ) calloc(1, sizeof(node))) == 0)
		nomem();
	nodecount++;
	if (read(fdnode, tnode, sizeof(tmpnode)) <= 0) {
		fprintf(stderr, "getnode: temporary file read error\n");
		badmagic(1);
	}
	return(tnode);
}

/* Returns pointer to node; takes long constant offset */
/* Puts node in a temporary location */

node *
tmpgetnode(offset)
register int offset;
{
	if (offset == 0)
		offset = nullnode.n_offset;

	lseek(fdnode, (long) (((long) offset) * sizeof(node)), 0);
	if (read(fdnode, &tmpnode, sizeof(tmpnode)) <= 0) {
		fprintf(stderr, "tmpgetnode: temporary file read error\n");
		badmagic(1);
	}
	return(&tmpnode);
}

/* Returns pointer to link; takes long constant offset */

link *
getlink(offset)
register int offset;
{
	link 	*tlink;

	if (offset == 0)
		offset = nulllink.l_offset;

	lseek(fdlink, (long) (((long) offset) * sizeof(link)), 0);
	if ((tlink = (link * ) calloc(1, sizeof(link))) == 0)
		nomem();
	linkcount++;
	if (read(fdlink, tlink, sizeof(tmplink)) <= 0) {
		fprintf(stderr, "getlink: temporary file read error\n");
		badmagic(1);
	}
	return(tlink);
}

/* Refreshes link from possibly modified disk image */

regetlink(oldlink)
register link *oldlink;
{
	long offset;

	if (oldlink->l_offset == 0)
		offset = nulllink.l_offset;
	else
		offset = oldlink->l_offset;

	lseek(fdlink, offset * sizeof(link), 0);
	if (read(fdlink, oldlink, sizeof(link)) <= 0) {
		fprintf(stderr, "regetlink: temporary file read error\n");
		badmagic(1);
	}
}

/* Returns pointer to link; takes long constant offset */
/* Puts link in a temporary location */

link *
tmpgetlink(offset)
register int offset;
{
	if (offset == 0)
		offset = nulllink.l_offset;

	lseek(fdlink, (long) (((long) offset) * sizeof(link)), 0);
	if (read(fdlink, &tmplink, sizeof(tmplink)) <= 0) {
		fprintf(stderr, "tmpgetlink: temporary file read error\n");
		badmagic(1);
	}
	return(&tmplink);
}

badmagic(n)
{
	if (n) {
		fprintf(stderr, "%s: cannot recover!\n", ProgName);
#ifdef DEBUG
		abort();
#else
		exit(n);
#endif
	}
}
