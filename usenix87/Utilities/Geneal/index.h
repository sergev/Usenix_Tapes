/* index.h - definitions for index */
/* Written by Jim McBeath (jimmc) at SCI */
/* last edit 19-Jan-85 08:53:10 by jimmc (Jim McBeath) */
/* last edit 31-Aug-85 13:00:00 by tlr (Terry L. Ridder) */

#define TBLSIZ 100

struct tblblock {
    int repnum;			/* number of indexes represented by this blk */
    int count;			/* count of entries in this level */
    int tdata[TBLSIZ];		/* the actual data for this level */
    };

struct toplevel {
    int numlevs;		/* how deep the table is at it's deepest */
    struct tblblock *data;	/* pointer to the top level */
    };

/* end */
