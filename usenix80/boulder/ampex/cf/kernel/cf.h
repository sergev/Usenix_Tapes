struct key {
	int	lson,		/* block # of left son */
	 	max,		/* largest descendant of this node in the tree */
       	 	base, 		/* 1st chunk number */
		last;		/* last chunk number */
};
#define NKEY ((512-4-6)/sizeof (struct key))-1	/* # that fit in 1 block */
#define MAXDEPTH  2				/* of b-tree */

struct bnode {
	int nfarea;		/* # free areas at this level */
	struct key	*father, 
			keys[NKEY + 1];
} *bnodes[MAXDEPTH+1];

/* info kept on the superblock */
struct {
	int devc,		/* char device */
	    devb,		/* block ... */
	    cfb0,		/* block # of start of c.f.s. */
	    cfsiz,		/* # chunks of c.f.s. */
	    chunksiz,		/* in blocks */
	    bound,		/* for special allocation */
	    flsiz,		/* free list size */
	    infl,		/* inumber of free list file */
	    rootlbn;		/* l.b.n. of b-tree root */
} cf_sb;

/* variables set when superblock or new buffers are read in */
struct filsys *cf_sbp;		/* superblock */
int cf_depth,			/* depth of current search */
    cf_devn;			/* normal device (home of free list) */

struct pointer {int i0, i1;};		/* funny access structures */
struct farea {int i2, i3;};
struct {				/* access to a key */
	struct key k;
	int	rson,
		rmax;
};
struct {int i0, i1; long l2, l4;};	/* to access inode */
#define KSIZ (sizeof (struct key))/2
#define PSIZ (sizeof (struct pointer))/2
#define FSIZ (sizeof (struct farea))/2
#define SBSIZ (sizeof cf_sb)/2
struct inode *cf_ipfl;		/* inode of free list file */
struct buf *cf_buf[MAXDEPTH+1];	/* system buffers temporarily in use */
int cf_lock;			/* for mutually exclusive access */
