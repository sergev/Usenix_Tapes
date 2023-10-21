/* pathalias -- by steve bellovin, as told to peter honeyman */
#ifndef lint
static char	*sccsid = "@(#)addlink.c	8.1 (down!honey) 86/01/19";
#endif lint

#include "def.h"

static long	Trace[NTRACE];
static int	Tracecount;

int
addlink(ofrom, oto, cost, netchar, netdir)
int	ofrom;
register int	oto;
Cost	cost;
char	netchar;
char	netdir;
{
	register link	*l, *ltmp;
	node	*ntmp;
	node	*from;
	int	prev = 0;

/*	if (Tflag)
		ltrace(from, to, cost, netchar, netdir);
*/
	/* maintain uniqueness for dead links (only) */
	from = getnode(ofrom);
	freenode(from);
	for (l = getlink(from->n_link); l->l_offset && l->l_flag & LDEAD;
						    l = getlink(l->l_next)) {
		if (oto == l->l_to) {
			/* what the hell, use cheaper cost */
			if (cost < l->l_cost) {
				l->l_cost = cost;
				netbits(l, netchar, netdir);
			}
			putlink(l);
			return(l->l_offset);
		}
		prev = l->l_offset;
		freelink(l);
	}
	freelink(l);

	/* allocate and link in the new link struct */
	l = newlink();
	if (cost != INF)	/* ignore back links */
		Lcount++;
	if (prev) {
		ltmp = getlink(prev);
		l->l_next = ltmp->l_next;
		ltmp->l_next = l->l_offset;
		putlink(ltmp);
	} else {
		ntmp = getnode(ofrom);
		l->l_next = ntmp->n_link;
		ntmp->n_link = l->l_offset;
		putnode(ntmp);
	}

	l->l_to = oto;
	l->l_cost = cost;
	if (netchar == 0) {
		netchar = DEFNET;
		netdir = DEFDIR;
	}
	netbits(l, netchar, netdir);

	putlink(l);
	return(l->l_offset);
}

int
addgateway(ofrom, oto, cost, netchar, netdir)
int	ofrom;
int	oto;
Cost	cost;
char	netchar;
char	netdir;
{
	register link	*l;

	l = getlink(addlink(ofrom, oto, cost, netchar, netdir));
	l->l_flag |= LGATEWAY;
	putlink(l);
	return(l->l_offset);
}

deadlink(s) 
char	*s;
{
	char	*t, c;
	link	*l;
	node	*n;

	t = index(s, '!');
	if (t) {
		c = *t;
		*t = 0;
		l = getlink(addlink(addnode(s), addnode(t + 1), INF / 2, c,
		     DEFDIR));
		l->l_flag |= LDEAD;
		putlink(l);
	} else 
		n = getnode(addnode(s));
		n->n_flag |= NDEAD;
		putnode(n);
}

netbits(l, netchar, netdir)
link	*l;
char	netchar, netdir;
{
	char	*nptr;

	if ((nptr = index(Netchars, netchar)) == 0) {
		fprintf(stderr, "%s: unknown network operator: %c\n",
							ProgName, netchar);
		badmagic(1);
	}
	l->l_flag &= ~(LNETCHARS|LDIR);
	l->l_flag |= (nptr - Netchars) | dirbits(netdir);
}

#ifdef FORGET_IT
tracelink(arg) 
char	*arg;
{
	char	*bang;
	link	*l;

	if (Tracecount >= NTRACE)
		return(-1);
	l = newlink();
	bang = index(arg, '!');
	if (bang) {
		*bang = 0;
		l->l_to = addnode(bang+1);
	} else 
		l->l_to = 0;

	l->l_from = (link *) addnode(arg);
	Trace[Tracecount++] = l;
	return(0);
}
#endif

#ifdef FORGET_IT
STATIC
ltrace(from, to, cost, netchar, netdir)
node	*from, *to;
Cost	cost;
char	netchar;
char	netdir;
{
	link	*l;
	int	i;

	for (i = 0; i < Tracecount; i++) {
		l = Trace[i];
		/* overkill -- you asked for it! */
		if ((l->l_to == 0
		  && (from == (node *) l->l_from || to == (node *) l->l_from))
		 || (from == (node *) l->l_from && to == l->l_to)
		 || (to == (node *) l->l_from && from == l->l_to)) {
			ltrprint(from, to, cost, netchar, netdir);
			return;
		}
	}
}
#endif

#ifdef FORGET_IT
/* print a trace item */
STATIC
ltrprint(from, to, cost, netchar, netdir)
node	*from, *to;
Cost	cost;
char	netchar;
char	netdir;
{
	char	buf[256], *bptr = buf;

	strcpy(bptr, from->n_name);
	bptr += strlen(bptr);
	*bptr++ = ' ';
	if (netdir == LRIGHT)			/* @% */
		*bptr++ = netchar;
	strcpy(bptr, to->n_name);
	bptr += strlen(bptr);
	if (netdir == LLEFT)			/* !: */
		*bptr++ = netchar;
	sprintf(bptr, "(%ld)", cost);
	yyerror(buf);
}
#endif

#ifdef FORGET_IT
atrace(n1, n2)
node	*n1, *n2;
{
	link	*l;
	int	i;
	char	buf[256];

	for (i = 0; i < Tracecount; i++) {
		l = Trace[i];
		if (l->l_to == 0 && ((node *) l->l_from == n1 || (node *) l->l_from == n2)) {
			sprintf(buf, "%s = %s", n1->n_name, n2->n_name);
			yyerror(buf);
			return;
		}
	}
}
#endif
