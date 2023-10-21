/* pathalias -- by steve bellovin, as told to peter honeyman */
#ifndef lint
static char	*sccsid = "@(#)printit.c	8.2 (down!honey) 86/01/29";
#endif

#include "def.h"
extern int fdlink, fdnode;

/* use lots of char bufs -- profiling indicates this costs about 5 kbytes */

/* in practice, even the longest paths are < 100 bytes */
#define PSIZE 512

int  Cflag = 0;

main(argc, argv)
int argc;
char **argv;
{
	link *l;
	char pbuf[PSIZE];

	ProgName = "printit";

	if (argc == 2) Cflag++;

	/* Open temp file */
	fdnode = open(NTEMPFILE, 2);
	fdlink = open(LTEMPFILE, 2);
	if (fdnode < 0) {
		fprintf(stderr, "printit: temporary node file open error\n");
		badmagic(1);
	}
	if (fdlink < 0) {
		fprintf(stderr, "printit: temporary link file open error\n");
		badmagic(1);
	}

	/* print home */
	Home = 1;
	if (Cflag)
		printf("%ld\t", (long) tmpgetnode(Home)->n_cost);
	printf("%s\t%%s\n", tmpgetnode(Home)->n_name);
	
	strcpy(pbuf, "%s");
	for (l = tmpgetlink(tmpgetnode(Home)->n_link); l->l_offset;
						 l = tmpgetlink(l->l_next)) {
		if (l->l_flag & LTREE) {
			preorder(l->l_offset, pbuf);
			strcpy(pbuf, "%s");
		}
	}
}

/* preorder traversal of shortest path tree */
preorder(ol, ppath)
register int ol;
char *ppath;
{
	register node *n;
	link	*l;
	char npath[PSIZE];

	setpath(ol, ppath, npath);
	l = getlink(ol);
	n = getnode(l->l_to);
	freelink(l);
	if ((n->n_flag & NNET) || ISADOMAIN(n))
		printnet(n->n_offset, npath, n->n_cost);
	else
		printhost(n->n_offset, npath, n->n_cost);
	freenode(n);
	for (l = getlink(n->n_link); l->l_offset; l = getlink(l->l_next)) {
		if (l->l_flag & LTREE)
			preorder(l->l_offset, npath);
		freelink(l);
	}
	freelink(l);
}

setpath(ol, ppath, npath) 
int ol;
register char *ppath, *npath;
{
	register node *next;
	node *ntmp;
	link *l;
	char netchar;
	extern char *hostpath();

	l = getlink(ol);
	next = getnode(l->l_to);
	/*
	 * for magic @-% conversion.
	 * assume that gateways to domains want no @'s
	 */
	ntmp = getnode(next->n_parent);
	if (ntmp->n_flag & ATSIGN || ISADOMAIN(next)) {
		next->n_flag |= ATSIGN;
		putnode(next);
		next = getnode(next->n_offset);
	}
	freenode(ntmp);

	/* special handling for aliases , domains, and nets */
	if ((l->l_flag & LALIAS) || (next->n_flag & NNET)
							|| ISADOMAIN(next)) {
		strcpy(npath, ppath);
		freenode(next);
		freelink(l);
		return;
	}
		
	netchar = NETCHAR(l);
	if (netchar == '@')
		if (next->n_flag & ATSIGN)
			netchar = '%';	/* shazam?  shaman? */
		else {
			next->n_flag |= ATSIGN;
			putnode(next);
			next = getnode(next->n_offset);
		}

	freenode(next);
	/* remainder should be a sprintf -- foo on '%' as an operator */
	for ( ; *npath = *ppath; ppath++) {
		if (*ppath == '%') {
			switch(ppath[1]) {
			case 's':
				ppath++;
				npath = hostpath(npath, l->l_offset, netchar);
				break;

			case '%':
				*++npath = *++ppath;
				npath++;
				break;

			default:
				fprintf(stderr, "%s: %%%c found in setpath\n",
						ProgName, ppath[1]);
				badmagic(1);
				break;
			}
		} else
			npath++;
	}
	freelink(l);
}

char *
hostpath(path, ol, netchar)
register char *path;
register int ol;
char netchar;
{
	register node *prev;
	node	*ntmp;
	link	*l;

	l = getlink(ol);
	ntmp = getnode(l->l_to);
	prev = getnode(ntmp->n_parent);
	if (NETDIR(l) == LLEFT) {
		/* host!user */
		strcpy(path, ntmp->n_name);
		path += strlen(path);
		while (ISADOMAIN(prev)) {
			strcpy(path, prev->n_name);
			path += strlen(path);
			freenode(prev);
			prev = getnode(prev->n_parent);
		}
		*path++ = netchar;
		if (netchar == '%')
			*path++ = '%';
		*path++ = '%';
		*path++ = 's';
	} else {
		/* %s@host */
		*path++ = '%';
		*path++ = 's';
		*path++ = netchar;
		if (netchar == '%')
			*path++ = '%';
		strcpy(path, ntmp->n_name);
		path += strlen(path);
		while (ISADOMAIN(prev)) {
			strcpy(path, prev->n_name);
			path += strlen(path);
			freenode(prev);
			prev = getnode(prev->n_parent);
		}
	}
	freenode(prev);
	freenode(ntmp);
	freelink(l);
	return(path);
}

STATIC
printhost(on, path, cost)
int on;
char *path;
Cost cost;
{
	node	*n;

	n = getnode(on);
	/* skip private hosts */
	if ((n->n_flag & ISPRIVATE) == 0) {
		if (Cflag)
			printf("%ld\t", (long) cost);
		fputs(n->n_name, stdout);
		putchar('\t');
		puts(path);
	}
	freenode(n);
}

STATIC
printnet(on, path, cost)
int	on;
char	*path;
Cost	cost;
{
	node	*n;
	node	*ntmp;
	node	*parent;

	n = getnode(on);
	/* print domains only */
	if (!ISADOMAIN(n)) {
		freenode(n);
		return;
	}
	/* print top-level domain */
	ntmp = getnode(n->n_parent);
	if (!ISADOMAIN(ntmp)) {
		if (n->n_flag & ISPRIVATE)
			fprintf(stderr,
			 "%s: warning: private top-level domain %s ignored\n",
			  ProgName, n->n_name);
		else
			printdomain(n->n_offset, path, cost);
		freenode(ntmp);
		freenode(n);
		return;
	}
	/* remainder is for subdomains */

	/* don't print if it has a non-private ancestor */
	for (parent = getnode(n->n_parent); parent->n_offset;
					 parent = getnode(parent->n_parent)) {
		if (ISADOMAIN(parent) && (parent->n_flag & ISPRIVATE) == 0) {
			freenode(parent);
			freenode(ntmp);
			freenode(n);
			return;
		}
	freenode(parent);
	}
	freenode(parent);

	/* don't print if this domain is also private */
	if (n->n_flag & ISPRIVATE) {
		freenode(ntmp);
		freenode(n);
		return;
	}

	/* ancestors all private, this domain not private */
	freenode(ntmp);
	freenode(n);
	printdomain(on, path, cost);
}

STATIC
printdomain(on, path, cost)
int on;
char *path;
Cost cost;
{
	node	*n, *ntmp;

	n = getnode(on);
	ntmp = getnode(n->n_parent);

	/* skip subdomains, since the gateway to the parent suffices */
	if (ISADOMAIN(ntmp)) {
		freenode(ntmp);
		freenode(n);
		return;
	}
	freenode(ntmp);
	if (Cflag)
		printf("%ld\t", (long) cost);
	do {
		fputs(n->n_name, stdout);
		freenode(n);
		n = getnode(n->n_parent);
	} while (ISADOMAIN(n));
	putchar('\t');
	puts(path);
	freenode(n);
}
