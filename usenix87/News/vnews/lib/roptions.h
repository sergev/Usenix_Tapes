/* flags for readnews */
#define pflag	options[0].flag
#define tflag	options[1].flag
#define toptbuf	options[1].buf
#define aflag	options[2].flag
#define datebuf	options[2].buf
#define nflag	options[3].flag
#define sublist	options[3].buf
#define cflag	options[4].flag
#define coptbuf	options[4].buf
#define lflag	options[5].flag
#define rflag	options[6].flag
#define sflag	options[7].flag
#define xflag	options[8].flag
#define hflag	options[9].flag
#define Mflag	options[10].flag
#define fflag	options[11].flag
#define uflag	options[12].flag
#define eflag	options[13].flag
#define oneflag	options[14].flag
#define twoflag	options[15].flag
#define Aflag	options[16].flag

#define UNKNOWN 0001	/* possible modes for news program */
#define MAIL	0004
#define ANY	0007

#define OPTION	0	/* pick up an option string */
#define STRING	1	/* pick up a string of arguments */

struct optable {			/* options table. */
	char	optlet;		/* option character. */
	char	filchar;	/* if to pickup string, fill character. */
	int	flag;		/* TRUE if have seen this opt. */
	int	newstate;	/* STRING if takes arg, else OPTION */
	int	oldmode;	/* OR of legal input modes. */
	int	newmode;	/* output mode. */
	char	*buf;		/* string buffer */
};

extern struct optable options[];
extern int mode;
