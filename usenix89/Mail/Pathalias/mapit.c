/* pathalias -- by steve bellovin, as told to peter honeyman */
#ifndef lint
static char	*sccsid = "@(#)mapit.c	8.1 (down!honey) 86/01/19";
#endif

#include "def.h"

/* privates */
extern void	reheap(), insert(), heapswap();
extern int	min_node(), rmlink();
extern Cost	costof();
extern int	nodecount, linkcount;

static int	Nheap;
static long	Heaphighwater;
static int	*Heap;


/* transform the graph to a shortest-path tree by marking tree edges */

mapit()
{
	register node *n, *next;
	register link *l;
	int	olnext;
	int	ol;
	int	next_offset, olprev;
	Cost cost;

	/*
	 * re-use the hash table space for the heap.
	 */
	Heap = (int *) Table;

	pack();		/* remove holes in the Table */
	if (Linkout && *Linkout)	/* dump cheapest links */
		showlinks();
	if (Graphout && *Graphout)	/* dump the edge list */
		dumpgraph();

	/* invent and insert a link for Home to get things started */
	l = newlink();
	l->l_to = Home;
	(void) dehash(Home);
	putlink(l);
	insert(l->l_offset);

	/* main mapping loop */
remap:
	Heaphighwater = Nheap;
	while ((ol = min_node()) != 0) {
		l = tmpgetlink(ol);
		l->l_flag |= LTREE;
		n = getnode(l->l_to);
		n->n_flag |= MAPPED;
		putnode(n);
		putlink(l);
		n = tmpgetnode(l->l_to);
 
		/* add children to heap */
		olprev = 0;
		for (l = tmpgetlink(n->n_link); l->l_offset != 0;
						l = tmpgetlink(olnext)) {

			next_offset = l->l_to;
			next = getnode(l->l_to);	/* neighboring node */
			if (next->n_flag & MAPPED) {
				olnext = rmlink(l->l_offset, olprev,
								 n->n_offset);
				freenode(next);
				continue;
			}
			freenode(next);
			cost = costof(n->n_offset, l->l_offset);

			if (skiplink(l->l_offset, n->n_offset, cost)) {
				olnext = rmlink(l->l_offset, olprev,
								n->n_offset);
				continue;
			}

			/*
			 * put this link in the heap, in a place where it may
			 * percolate up, but not down.  if new, or if cost is
			 * being increased, move to end.  otherwise, cost is
			 * same or less, so leave it where it is.  unfortunately,
			 * freeing a link already in the heap is too costly at
			 * this point.
			 *
			 * TODO: avoid heaping aliases and network members.
			 */
			next = getnode(next_offset);
			if (dehash(next_offset) == 0) { /* first time in heap */
				insert(l->l_offset);	/* insert at end */
				regetnode(next);
			}
			else {
				regetnode(next);
				/* replace heaped link by this one */
				if (cost > next->n_cost) {	/* gateway */
					/* move old link to end of heap */
					heapswap((int) next->n_tloc, Nheap);
					regetnode(next);
					next->n_tloc = Nheap;
				}
				Heap[next->n_tloc] = l->l_offset;
			}
			
			next->n_cost = cost;
			next->n_parent = n->n_offset;
			putnode(next);
			reheap(l->l_offset);	/* restore heap property */

			/*
			 * note whether we got here via a gatewayed net.
			 * domains are presumed to require gateways.
			 * aliases inherit parent's gateway status.
			 */
			next = getnode(next_offset);
			next->n_flag &= ~GATEWAYIN;
			n = tmpgetnode(n->n_offset);
			l = tmpgetlink(l->l_offset);
			if (l->l_flag & LALIAS)
				next->n_flag |= (n->n_flag & GATEWAYIN);
			else if (GATEWAYED(n))
				next->n_flag |= GATEWAYIN;
			putnode(next);
			olprev = l->l_offset;	/* this link's a keeper */
			olnext = l->l_next;
		}
	}
	vprintf(stderr, "heap high water mark was %d\n", Heaphighwater);

	/* sanity check on implementation */
	if (Nheap != 0) {
		fprintf(stderr, "%s: null entry found in heap\n", ProgName);
		badmagic(1);
	}

	if (Hashpart < Tabsize) {
		/*
		 * add back links from unreachable hosts to reachable
		 * neighbors, then remap.  asymptotically, this is
		 * quadratic.  in practice, this is done exactly once.
		 */
		backlinks();
		if (Nheap)
			goto remap;
	}
	if (Hashpart < Tabsize) {
		fputs("You can't get there from here:\n", stderr);
		for ( ; Hashpart < Tabsize; Hashpart++) {
			fprintf(stderr, "\t%s",
					tmpgetnode(Table[Hashpart])->n_name);
			if (tmpgetnode(Table[Hashpart])->n_flag
						& (ISPRIVATE|COLLISION))
				fputs(" (private)", stderr);
			putc('\n', stderr);
		}
	}
}

/*
 * can this link be ignored?  if yes, return 1, o.w. 0.
 * a link can be skipped if it is not in the shortest path tree.
 */
STATIC int
skiplink(ol, oparent, cost)
int	ol;			/* new link to this node */
int	oparent;		/* new parent of this node */
Cost	cost;			/* new cost to this node */
{
	link 	*l, *ltmp;
	node	*parent;
	node	*n;		/* this node */
	int	lheap;		/* existing link to this node */

	l = getlink(ol);
	n = getnode(l->l_to);

	/* first time we've reached this node? */
	if (n->n_tloc >= Hashpart) {
		freelink(l);
		freenode(n);
		return(0);
	}

	lheap = Heap[n->n_tloc];
	ltmp = getlink(lheap);

	/* examine links to nets that require gateways */
	if (GATEWAYED(n)) {
		/* if exactly one is a gateway, use it */
		if ((ltmp->l_flag & LGATEWAY) && !(l->l_flag & LGATEWAY)) {
			freenode(n);
			freelink(l);
			freelink(ltmp);
			return(1);	/* old is gateway */
		}
		if (!(ltmp->l_flag & LGATEWAY) && (l->l_flag & LGATEWAY)) {
			freenode(n);
			freelink(l);
			freelink(ltmp);
			return(0);	/* new is gateway */
		}

		/* no gateway or both gateways;  resolve in standard way ... */
	}

	/* examine dup link (sanity check) */
	parent = getnode(oparent);
	if (n->n_parent == oparent && ((ltmp->l_flag & LDEAD)
						|| (l->l_flag & LDEAD))) {
		fprintf(stderr, "%s: dup dead link not eliminated: %s -> %s\n",
			ProgName, parent->n_name, n->n_name);
		badmagic(1);
	}
	freenode(parent);
	freelink(ltmp);

	/*  examine cost */
	if (cost < n->n_cost) {
		freenode(n);
		freelink(l);
		return(0);
	}
	if (cost > n->n_cost) {
		freenode(n);
		freelink(l);
		return(1);
	}

	/* all other things being equal, consult the oracle */
	freelink(l);
	freenode(n);

	return(tiebreaker(n->n_offset, oparent));
}

STATIC Cost
costof(oprev, ol)
register int	oprev;
register int	ol;
{
	link	*l;
	node	*prev;
	register node	*next;
	register Cost	cost;

	l = getlink(ol);
	next = getnode(l->l_to);
	prev = getnode(oprev);

	if (l->l_flag & LALIAS) {
		/* copy left/right bits */
		next->n_flag &= ~(HASLEFT|HASRIGHT);
		next->n_flag |= (prev->n_flag & (HASLEFT|HASRIGHT));
		putnode(next);
		freelink(l);
		freenode(prev);
		return(prev->n_cost);	/* by definition */
	}

		
	cost = prev->n_cost + l->l_cost;	/* basic cost */

	/*
	 * heuristics:
	 *    charge for a dead link.
	 *    charge for getting out of a dead host.
	 *    charge for getting into a gatewayed net (except at a gateway).
	 *    discourage mixing of left and right syntax when next is a host.
	 *    charge for leaving a gatewayed net.
	 *
	 * life was simpler when pathalias computed true shortest paths.
	 */
	if (l->l_flag & LDEAD)		/* dead link */
		cost += INF/2;
	if (DEADHOST(prev))		/* dead host */
		cost += INF/2;
	if (GATEWAYED(next) && !(l->l_flag & LGATEWAY))	/* not gateway */
		cost += INF/2;
	if (!ISANET(next)) {
		/* charge for mixed syntax here */
		if ((NETDIR(l) == LLEFT && (prev->n_flag & HASRIGHT))
		 || (NETDIR(l) == LRIGHT && (prev->n_flag & HASLEFT)))
			cost += DEFCOST;
	}
	/*
	 * if reached by a gatewayed net, discourage further links.
	 * this has some relevance to common carriers and the FCC ...
	 * the penalty inheres to hosts, not aliases, nets, or domains.
	 */
	if ((prev->n_flag & GATEWAYIN) && !ISADOMAIN(prev) && !(prev->n_flag & NNET))
		cost += INF/2;	/* heavyweight, but appropriate */

	/* set left/right bits */
	next->n_flag &= ~(HASLEFT|HASRIGHT);
	if (NETDIR(l) == LLEFT || (prev->n_flag & HASLEFT))
		next->n_flag |= HASLEFT;
	if (NETDIR(l) == LRIGHT || (prev->n_flag & HASRIGHT))
		next->n_flag |= HASRIGHT;

	putnode(next);
	freenode(prev);
	freelink(l);
	return(cost);
}

STATIC int
rmlink(ol, olprev, on)
int	ol, olprev;
int	on;
{
	node	*n;
	link	*l, *lprev;

	l = getlink(ol);
	n = getnode(on);
	if (olprev) {
		lprev = getlink(olprev);
		lprev->l_next = l->l_next;
		putlink(lprev);
		freenode(n);
	}
	else {
		n->n_link = l->l_next;
		putnode(n);
	}
	freelink(l);
	return(l->l_next);
}

/*
 * binary heap implementation of priority queue.
 * TODO: make the heap smaller by giving inserting a placeholder
 * for net members when the net is extracted.  this requires storing the
 * cost of a net in the net node itself -- yuck.  is it worth it?
 */

STATIC void
insert(ol)
int	ol;
{
	link	*l;
	register node	*n;

	l = getlink(ol);
	n = getnode(l->l_to);
	Heap[n->n_tloc] = 0;
	if (Heap[Nheap+1] != 0) {
		fprintf(stderr, "%s: heap error in insert\n", ProgName);
		badmagic(1);
	}
	if (Nheap++ == 0) {
		Heap[1] = l->l_offset;
		n->n_tloc = 1;
		freelink(l);
		putnode(n);
		return;
	}
	if (Vflag && Nheap > Heaphighwater)
		Heaphighwater = Nheap;	/* diagnostics */

	/* insert at the end.  caller must reheap(). */
	Heap[Nheap] = l->l_offset;
	n->n_tloc = Nheap;
	freelink(l);
	putnode(n);
}

/*
 * replace existing link in heap by this one, then
 * "percolate" up the heap by exchanging with the parent
 */
STATIC void
reheap(ol)
int	ol;
{
	link	*l, *ltmp;
	register int	loc, parent;
	register Cost	cost;
	register node	*n, *np;

	l = getlink(ol);
	n = getnode(l->l_to);

	cost = n->n_cost;
	for (loc = n->n_tloc; loc > 1; loc = parent) {
		parent = loc / 2;
		/* sanity check on implementation */
		if (Heap[parent] == 0) {
			fprintf(stderr, "%s: heap error in insert\n", ProgName);
			badmagic(1);
		}
		ltmp = getlink(Heap[parent]);
		np = getnode(ltmp->l_to);
		freelink(ltmp);
		if (cost > np->n_cost) {
			freenode(np);
			freenode(n);
			freelink(l);
			return;
		}
		/* move nets below hosts for output stability */
		if (cost == np->n_cost && ((n->n_flag & NNET)
						|| !(np->n_flag & NNET))) {
			freenode(np);
			freenode(n);
			freelink(l);
			return;
		}
		freenode(np);
		heapswap(loc, parent);
		regetlink(l);
		regetnode(n);
	}
	freenode(n);
	freelink(l);
}

/* extract min (== Heap[1]) from heap */
STATIC int
min_node()
{
	link *ltmp, *ltmp1;
	int orval;
	node *ntmp, *ntmp1;
	register int *regheap;
	register int	i, child;

	if (Nheap == 0)
		return(0);

	regheap = Heap;		/* in register -- heavily used */
	orval = regheap[1];	/* return this one */
			
	/* move last entry into root, percolate down */
	regheap[1] = regheap[Nheap];
	ltmp = getlink(regheap[1]);
	ntmp = getnode(ltmp->l_to);
	ntmp->n_tloc = 1;
	putnode(ntmp);
	freelink(ltmp);
	regheap[Nheap] = 0;
	if (--Nheap == 0)
		return(orval);

	i = 1;
	for (;;) {
		/* swap with smaller child down the tree */
		child = i * 2;	/* lhs child is 2i, rhs is 2i+1. */
		if (child >= Nheap)
			return(orval);

		/* use rhs child if smaller than lhs child */
		ltmp = getlink(regheap[child]);
		ntmp = getnode(ltmp->l_to);
		ltmp1 = getlink(regheap[child+1]);
		ntmp1 = getnode(ltmp1->l_to);
		if (ntmp->n_cost > ntmp1->n_cost
		 || (ntmp->n_cost == ntmp1->n_cost
		  && !ISANET(ntmp1)))
			child++;
		freenode(ntmp1);
		freelink(ltmp1);
		freenode(ntmp);
		freelink(ltmp);

		ltmp = getlink(regheap[child]);
		ntmp = getnode(ltmp->l_to);
		ltmp1 = getlink(regheap[i]);
		ntmp1 = getnode(ltmp1->l_to);
		if (ntmp1->n_cost < ntmp->n_cost) {
			freenode(ntmp1);
			freelink(ltmp1);
			freenode(ntmp);
			freelink(ltmp);
			return(orval);
		}
		/* move nets below hosts for output stability */
		if (ntmp1->n_cost == ntmp->n_cost
		 && (!ISANET(ntmp1) || ISANET(ntmp))) {
			freenode(ntmp1);
			freelink(ltmp1);
			freenode(ntmp);
			freelink(ltmp);
			return(orval);
		}
		freenode(ntmp1);
		freelink(ltmp1);
		freenode(ntmp);
		freelink(ltmp);
		heapswap(i, child);
		i = child;
	}
	/*NOTREACHED*/
}

/* exchange Heap[i] and Heap[j] pointers */
STATIC void
heapswap(i, j)
int	i, j;
{
	register int	temp;
	int	*regheap;
	node	*ntmp;
	link	*ltmp;

	regheap = Heap;	/* heavily used -- put in register */
	temp = regheap[i];
	regheap[i] = regheap[j];
	regheap[j] = temp;
	ltmp = getlink(regheap[j]);
	ntmp = getnode(ltmp->l_to);
	ntmp->n_tloc = j;
	putnode(ntmp);
	freelink(ltmp);
	ltmp = getlink(regheap[i]);
	ntmp = getnode(ltmp->l_to);
	ntmp->n_tloc = i;
	putnode(ntmp);
	freelink(ltmp);
}

/* return 1 if n is already de-hashed (n_tloc < Hashpart), 0 o.w. */
dehash(on)
register int	on;
{
	node	*n, *ntmp;

	n = getnode(on);
	if (n->n_tloc < Hashpart) {
		freenode(n);
		return(1);
	}

	/* swap with entry in Table[Hashpart] */
	ntmp = getnode(Table[Hashpart]);
	ntmp->n_tloc = n->n_tloc;
	putnode(ntmp);
	Table[n->n_tloc] = Table[Hashpart];
	Table[Hashpart] = n->n_offset;

	n->n_tloc = Hashpart++;
	putnode(n);
	return(0);
}

backlinks()
{
	link *l;
	node *n, *parent, *nomap;
	int i, ol;
	long tcost;

	for (i = Hashpart; i < Tabsize; i++) {

		nomap = getnode(Table[i]);
		parent = getnode(0);
		for (l = getlink(nomap->n_link); l->l_offset;
						l = getlink(l->l_next)) {
			n = getnode(l->l_to);
			if (!(n->n_flag & MAPPED)) {
				freenode(n);
				freelink(l);
				continue;
			}
			if (parent->n_offset == 0) {
				freenode(parent);
				parent = n;
				freelink(l);
				continue;
			}
			if (n->n_cost > parent->n_cost) {
				freenode(n);
				freelink(l);
				continue;
			}
			if (n->n_cost == parent->n_cost) {
				nomap->n_parent = parent->n_offset;
				putnode(nomap);
				nomap = getnode(nomap->n_offset);
				if (tiebreaker(nomap->n_offset, n->n_offset))
				{
					freenode(n);
					freelink(l);
					continue;
				}
			}
			freenode(parent);
			parent = n;
			freelink(l);
		}
		freelink(l);
		if (parent->n_offset == 0) {
			freenode(parent);
			freenode(nomap);
			continue;
		}
		(void) dehash(nomap->n_offset);
		ol = addlink(parent->n_offset, nomap->n_offset, INF,
							    DEFNET, DEFDIR);
		regetnode(nomap);
		regetnode(parent);
		nomap->n_parent = parent->n_offset;
		putnode(nomap);
		nomap = getnode(nomap->n_offset);
		tcost = costof(parent->n_offset, ol);
		regetnode(nomap);
		nomap->n_cost = tcost;
		putnode(nomap);
		freenode(parent);
		insert(ol);
		reheap(ol);
	}
	vprintf(stderr, "%d backlinks\n", Nheap);
}
