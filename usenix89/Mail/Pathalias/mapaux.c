/* pathalias -- by steve bellovin, as told to peter honeyman */
#ifndef lint
static char	*sccsid = "@(#)mapaux.c	8.2 (down!honey) 86/01/26";
#endif lint

#include "def.h"

void	pack();

void
pack()
{
	int	hole, next;
	node	*n;

	/* find first "hole " */
	for (hole = Tabsize - 1; hole >= 0 && Table[hole] != 0; --hole)
		;

	/* repeatedly move next filled entry into last hole */
	for (next = hole - 1; next >= 0; --next) {
		if (Table[next] != 0) {
			Table[hole] = Table[next];
			n = getnode(Table[hole]);
			n->n_tloc = hole;
			putnode(n);
			Table[next] = 0;
			while (Table[--hole] != 0)	/* find next hole */
				;
		}
	}
	Hashpart = hole + 1;
}

FILE	*Gstream;

dumpgraph()
{
	int	i;
	node	*n;

	if ((Gstream = fopen(Graphout, "w")) == NULL) {
		fprintf(stderr, "%s: ", ProgName);
		perror(Graphout);
	}

	untangle();	/* untangle net cycles for proper output */

	for (i = Hashpart; i < Tabsize; i++) {
		n = getnode(Table[i]);
		if (n->n_offset == 0) {
			freenode(n);
			continue;	/* impossible, but ... */
		}
		/* a minor optimization ... */
		if (n->n_link == 0) {
			freenode(n);
			continue;
		}
		/* pathparse doesn't need these */
		if (n->n_flag & NNET) {
			freenode(n);
			continue;
		}
		dumpnode(n->n_offset);
		freenode(n);
	}
}

dumpnode(ofrom)
int	ofrom;
{
	link	*l;
	node	*from;
	node	*to;
	char	netbuf[BUFSIZ], *nptr = netbuf;

	from = getnode(ofrom);
	for (l = tmpgetlink(from->n_link) ; l->l_offset;
						l = tmpgetlink(l->l_next)) {
		if (ofrom == l->l_to)
			continue;	/* oops -- it's me! */

		to = tmpgetnode(l->l_to);
		if ((to->n_flag & NNET) == 0) {
			/* host -> host -- print host>host */
			if (l->l_cost == INF)
				continue;	/* phoney link */
			fputs(from->n_name, Gstream);
			putc('>', Gstream);
			fputs(to->n_name, Gstream);
			putc('\n', Gstream);
		} else {
			/* host -> net -- just cache it for now */
			while (to->n_root && to->n_offset != to->n_root)
				to = tmpgetnode(to->n_root);
			*nptr++ = ',';
			strcpy(nptr, to->n_name);
			nptr += strlen(nptr);
		}
	}

	/* dump nets */
	if (nptr != netbuf) {
		/* nets -- print host@\tnet,net, ... */
		*nptr = 0;
		fputs(from->n_name, Gstream);
		putc('@', Gstream);
		*netbuf = '\t';	/* insert tab by killing initial ',' */
		fputs(netbuf, Gstream);	/* skip initial ',' */
		putc('\n', Gstream);
	}
	freenode(from);
}

/*
 * remove cycles in net definitions. 
 *
 * depth-first search
 *
 * for each net, run dfs on its neighbors (nets only).  if we return to
 * a visited node, that's a net cycle.  mark the cycle with a pointer
 * in the n_root field (which gets us closer to the root of this
 * portion of the dfs tree).
 */
untangle()
{
	int	i;
	node	*n;

	for (i = Hashpart; i < Tabsize; i++) {
		n = tmpgetnode(Table[i]);
		if (n->n_offset == 0 || (n->n_flag & NNET) == 0 || n->n_root)
			continue;
		dfs(n->n_offset);
	}
}

dfs(o)
int	o;
{
	link	*l;
	node	*n, *next;

	n = getnode(o);
	n->n_flag |= INDFS;
	n->n_root = n->n_offset;
	putnode(n);
	n = getnode(n->n_offset);
	for (l = getlink(n->n_link); l->l_offset; l = getlink(l->l_next)) {
		next = getnode(l->l_to);
		if ((next->n_flag & NNET) == 0) {
			freenode(next);
			freelink(l);
			continue;
		}
		if ((next->n_flag & INDFS) == 0) {
			dfs(next->n_offset);
			regetnode(next);
			if (next->n_root != next->n_offset) {
				regetnode(n);
				n->n_root = next->n_root;
				putnode(n);
				n = getnode(n->n_offset);
			}
		}
		else {
			regetnode(n);
			n->n_root = next->n_root;
			putnode(n);
			n = getnode(n->n_offset);
		}
		freenode(next);
		freelink(l);
	}
	freelink(l);
	regetnode(n);
	n->n_flag &= ~INDFS;
	putnode(n);
}

showlinks() 
{
	link	*l;
	node	*n;
	int	i;
	FILE	*estream;

	if ((estream = fopen(Linkout, "w")) == 0)
		return;

	for (i = Hashpart; i < Tabsize; i++) {
		n = getnode(Table[i]);
		if (n->n_offset == 0) {	/* impossible, but ... */
			freenode(n);
			continue;
		}
		l = tmpgetlink(n->n_link);
		if (l->l_offset) {
			fprintf(estream, "%s\t%s(%d)", n->n_name,
				tmpgetnode(l->l_to)->n_name,
				l->l_cost ? l->l_cost : DEFCOST);
			for (l = tmpgetlink(l->l_next); l->l_offset;
			 			 l = tmpgetlink(l->l_next))
				fprintf(estream, ",\n\t%s(%d)",
				   tmpgetnode(l->l_to)->n_name,
				      l->l_cost ? l->l_cost : DEFCOST);
			fputc('\n', estream);
		}
		freenode(n);
	}
	(void) fclose(estream);
}

/*
 * n is node in heap, newp is candidate for new parent.
 * choose between newp and n->n_parent for parent.
 * return 0 to use newp, non-zero o.w.
 */
#define NEWP 0
#define OLDP 1
tiebreaker(o, onewp)
int	o, onewp;
{
	register char	*opname, *npname, *name;
	node	*n, *newp, *oldp;
	int	metric;

	n = getnode(o);
	newp = getnode(onewp);
	oldp = getnode(n->n_parent);

	/*
	 * given the choice, avoid gatewayed nets,
	 * thereby placating the FCC or some such.
	 */
	if (GATEWAYED(newp) && !GATEWAYED(oldp)) {
		freenode(n);
		freenode(newp);
		freenode(oldp);
		return(OLDP);
	}
	if (!GATEWAYED(newp) && GATEWAYED(oldp)) {
		freenode(n);
		freenode(newp);
		freenode(oldp);
		return(NEWP);
	}

	/* look at real parents, not nets */
	while (oldp->n_flag & NNET) {
		freenode(oldp);
		oldp = getnode(oldp->n_parent);
	}
	while (newp->n_flag & NNET) {
		freenode(newp);
		newp = getnode(newp->n_parent);
	}

	/* use fewer hops, if possible */
	metric = height(oldp->n_offset) - height(newp->n_offset);
	if (metric < 0) {
		freenode(oldp);
		freenode(newp);
		freenode(n);
		return(OLDP);
	}
	if (metric > 0) {
		freenode(oldp);
		freenode(newp);
		freenode(n);
		return(NEWP);
	}

	/* compare names */
	opname = oldp->n_name;
	npname = newp->n_name;
	name = n->n_name;

	/* find point of departure */
	while (*opname == *npname && *npname == *name) {
		if (*name == 0) {
			fprintf(stderr, "%s: error in tie breaker\n", ProgName);
			badmagic(1);
		}
		opname++;
		npname++;
		name++;
	}

	/* use longest match, if appl. */
	if (*opname == *name || *opname == 0) {
		freenode(oldp);
		freenode(newp);
		freenode(n);
		return(OLDP);
	}
	if (*npname == *name || *npname == 0) {
		freenode(oldp);
		freenode(newp);
		freenode(n);
		return(NEWP);
	}

	/* use shorter host name, if appl. */
	metric = strlen(opname) - strlen(npname);
	if (metric < 0) {
		freenode(oldp);
		freenode(newp);
		freenode(n);
		return(OLDP);
	}
	if (metric > 0) {
		freenode(oldp);
		freenode(newp);
		freenode(n);
		return(NEWP);
	}

	/* use larger lexicographically (no reason) */
	metric = strcmp(opname, npname);
	freenode(oldp);
	freenode(newp);
	freenode(n);
	if (metric < 0)
		return(NEWP);
	return(OLDP);
}

height(o)
register int	o;
{
	node	*n;
	register int i = 0;

	n = getnode(o);
	freenode(n);
	n = getnode(n->n_parent);
	while (n->n_offset != 0) {
		if ((n->n_flag & NNET) == 0)
			i++;	/* should count domains too ... */
		freenode(n);
		n = getnode(n->n_parent);
	}
	freenode(n);
	return(i);
}
