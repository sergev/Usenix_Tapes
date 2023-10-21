/* header file for cpm[12].c */

# define IOBUF	1024	/* floppy I/O buffer size */
# define CMD	200	/* storage for interactive commands */
# define ARG	40	/* max args for interactive mode */
# define SLOTS	64	/* number of cpm directory slots */
# define GROUPS	243	/* number of groups for storage */
# define MDEL	0xe5	/* marked for deletion */
# define CNTLZ	0x1a
# define USAGE	"usage:  cpm -drive command -options file1 file2 ...\n"
# define SREV	"serious revision of program is indicated ...\n"

# define loop while(1)
# define min(a, b) (a < b ? a : b)
# define err(s) fputs(s, stderr)
# define cgetc() (--conv.ccnt >= 0 ? *conv.cptr++ & 0377 : cfilbuf())
# define cputc(x) (--conv.ccnt >= 0 ? (int)(*conv.cptr++ = (unsigned)(x)) \
	: cflsbuf((unsigned)(x)))

struct fcb {			/* image of a directory slot */
	int	et;		/* entry type */
	char	fn[12];		/* file name */
	int	ex;		/* extent */
	int	rc;		/* record count */
	int	dm[16];		/* disk map */
};
struct acon {			/* for ascii I/O conversions */
	struct fcb* cdir;	/* pointer to dir slot */
	char	cmod;		/* 'r' for read, 'w' for write */
	char	ccvt;		/* 'a' for ascii, 'b' for binary */
	int	ccnt;		/* number of chars left in cbuf */
	char*	cptr;		/* pointer to next char */
	char	cbuf[IOBUF];	/* packing buffer */
};
