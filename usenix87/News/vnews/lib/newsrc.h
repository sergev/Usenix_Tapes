#define MAXUNSUB 200	/* max number discussion unsubscribed from */
#define MAXRCJUNK 128	/* maximum number of comment lines */

#define NG_UNSUB 01

struct ngentry {		/* newsgroup entry */
	char *ng_name;		/* name of newsgroup */
	char *ng_bits;		/* which articles have been read */
	struct ngentry *ng_next;/* next newgroup in .newsrc order */
	char ng_unsub;		/* set if we have unsubscribed */
};

#define ng_num(ngp)	((ngp) - ngtable)
#define numtong(num)	(&ngtable[num])
#ifdef DEBUG
#define isunread(i)	(i > 0 && i <= maxartno ? _isunread(i) : abort())
#define clrunread(i)	(i > 0 && i <= maxartno ? _clrunread(i) : abort())
#define setunread(i)	(i > 0 && i <= maxartno ? _setunread(i) : abort())
#define _isunread(i)	(bitmap[(i-1) >> 3] & (1 << (i-1) % 8))
#define _setunread(i)	(bitmap[(i-1) >> 3] |= (1 << (i-1) % 8))
#define _clrunread(i)	(bitmap[(i-1) >> 3] &= ~(1 << (i-1) % 8))
#else
#define isunread(i)	(bitmap[(i-1) >> 3] & (1 << (i-1) % 8))
#define setunread(i)	(bitmap[(i-1) >> 3] |= (1 << (i-1) % 8))
#define clrunread(i)	(bitmap[(i-1) >> 3] &= ~(1 << (i-1) % 8))
#endif


struct ngentry *findgroup();
struct ngentry *prevgrp(), *nextgrp();


extern struct ngentry ngtable[MAXGROUPS];
extern struct ngentry *curng, *firstng, *lastng;
extern int ndunsub;
extern long dunsub[MAXUNSUB];
extern char *rcjunk[MAXRCJUNK];
extern char **nextrcjunk;
extern int minartno;
extern int maxartno;
extern char *bitmap;
