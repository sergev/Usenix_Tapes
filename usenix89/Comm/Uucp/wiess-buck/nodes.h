#include <stdio.h>
#include <ctype.h>

#define HOME "polyof"	/* name of your uucp site */
#define NHASH 6817	/* size of the hash table */
#define NAMESIZE 16	/* maximum length of site name */
#define MAXSITE 6800	/* maximum number of sites */
#define MAXNEIGH 32768	/* max neighbors allowed */

#define PATH "/usr1/uumap/"	/* tells where to get the site data */
/* where should we put the data files? */
#define HASHPATH "/usr1/steve/uucp/hashtab"
#define SITEPATH "/usr1/steve/uucp/sitetab"
#define NEIGHPATH "/usr1/steve/uucp/neightab"
#define WEIGHPATH "/usr1/steve/uucp/weighttab"
#define TREEPATH "/usr1/steve/uucp/uucptree"
#define MODE 00664	/* mode of data files */

#define BIG 1000000000
#define TMPVAL "333"	/* temp value used till we find cost to/from a NET */
#define NETVALS "1"	/* cost to travel within a NET (a string) */
#define NETVAL 1	/* cost to travel within a NET (a number) */

/* a site structure  - [MAXSITE * sizeof(struct site) = lots_of_mem] */
struct site {
	char s_name[NAMESIZE];  /* site name */
	int s_neigh;		/* # of neighbors */
	char s_isnet;		/* is this a network? */
	union noff {
		int s_noff;	/* offset into neighbor list */
		struct nptr *s_next;	/* ptr to head of list */
	} s_un;
	short  s_next; /* ptr to next guy in bucket */
	short  s_parent; /* parent node */
	int    s_cost;	/* cost to talk to home */
};

/* neighbor structure - [even more mem] */
struct nptr {
	struct nptr *n_next;	/* pointer to next neighbor */
	short n_index;	/* index into NEIGHPATH when we create it */
	int n_freq;	/* cost for site to talk to this neighbor */
};

/*NOBASE*/
struct site site[MAXSITE];	/* list of sites */
short hashtab[NHASH];		/* hash table */
short neightab[MAXNEIGH];	/* image of neighbor file */
short weighttab[MAXNEIGH];	/* frequency nieghbor is spoken to */

struct nptr *new_neigh();

int total_neigh;	/* total number of numbers encountered */

#define strchr index
