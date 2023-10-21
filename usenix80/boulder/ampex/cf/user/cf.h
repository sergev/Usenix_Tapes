#include "param.h"
#include "/usr/sys/user.h"
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
			keys[NKEY];
} *bnodes[MAXDEPTH+1];

/* info kept on the superblock */
struct {
	int devc,		/* contiguous device */
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
#define ISIZ sizeof(struct inode)
int	cf_ipfl;	/* in this mode, it's a file descriptor */
struct bnbuf {
	struct bnode b;
	char padd[512 - sizeof(struct bnode)];
};

struct	filsys {
	int	s_isize;	/* size in blocks of I list */
	int	s_fsize;	/* size in blocks of entire volume */
	int	s_nfree;	/* number of in core free blocks (0-100) */
	int	s_free[100];	/* in core free blocks */
	int	s_ninode;	/* number of in core I nodes (0-100) */
	int	s_inode[100];	/* in core free I nodes */
	char	s_flock;	/* lock during free list manipulation */
	char	s_ilock;	/* lock during I list manipulation */
	char	s_fmod;		/* super block modified flag */
	char	s_ronly;	/* mounted read-only flag */
	int	s_time[2];	/* current date of last update */
	int 	s_devc,		/* contiguous device */
		s_devb,
		s_cfb0,		/* block # of start of c.f.s. */
		s_cfsiz,	/* # chunks */
	    	s_chunksiz,	/* in blocks */
	    	s_bound,	/* for special allocation */
	    	s_flsiz,	/* number of bnodes in use */
	    	s_infl,		/* inumber of free list file */
	    	s_rootlbn,	/* l.b.n. of b-tree root */
	   	pad[44];
};

struct inodestat {		/* inode structure for "stat" call */
	int   dev;
        int   inumber;       /* +2 */
        int   flags;         /* +4: see below */
        char  nlinks;        /* +6: number of links to file */
        char  uid;           /* +7: user ID of owner */
        char  gid;           /* +8: group ID of owner */
        char  size0;         /* +9: high byte of 24-bit size */
        int   size1;         /* +10: low word of 24-bit size */
        int   addr[8];       /* +12: block numbers or device number */
        int   actime[2];     /* +28: time of last access */
        int   modtime[2];    /* +32: time of last modification */
} ;

struct ubuf {
	int	bn,
		b_addr[256];
} ;
struct ubuf *cf_buf[MAXDEPTH+1];
