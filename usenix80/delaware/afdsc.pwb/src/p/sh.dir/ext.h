#
#include	"defs.h"
extern	char    shnam[] ;	/*name of shell */
extern	char    globnam[] ;  /* name of glob */
extern	char    bin[32];        /* 256 bits that are set for users bin dir */
extern	char    pwbuf[80];      /* for password file entry and then current dir */
extern	char    dfltfile[30];   /* for default users bin directory */
extern	char    *vbls[52];      /* shell variables */
extern	char    *dolp;
extern	char	pidp[6];
extern	int     ldivr;
extern	int	devdir;
extern	struct ttybf ttybuf;
extern	struct statbf statbuf;
extern	char    **dolv;
extern	int     dolc;
extern	char uprojbuf[10];
extern	char mailbx[80];
extern	char cwkdir[80];
extern	char	daybuf[3];		/* day buffer */
extern	char	monbuf[3];		/* month buffer */
extern	char	yearbuf[3];		/* year buffer */
extern	char	usernm[9] ;
extern	char	ttynm[10] ;
extern	char    *linep;		/* ptr to input line of words */
extern	char	*p_linep;	/* ptr to previous input line */
extern	char    *elinep;	/* ptr to end of input line */
extern	char    **argp;
extern	char    **eargp;
extern	int     *treep;
extern	int     *treeend;
extern	char    peekc;
extern	char    gflg;
extern	int	topshell;
extern	char    error;
extern	char	acctf;
extern	char    uid;
extern	char	gid;
extern	char    setintr;
extern	int	ttys[3];	/* holding area for stty params */
extern	int	oldmode;	/* ttys[2] holding area */
extern	int     sinfil;         /* real standard input during a next */
extern	int     eflag;          /* -e echo flag */
extern	char    *rpromp;        /* real prompt string during a next */
extern	char    *arginp;
extern	int     onelflg;	/* flag for one line of input only */
extern	char    *mesg[];
extern struct stime timeb;
extern char sysnbuf[10];
extern char syscbuf[2];
extern	char    line[LINSIZ];
extern	char	prev_line[LINSIZ];
extern	int	prev_flag;
extern	char    *args[ARGSIZ];
extern	int     trebuf[TRESIZ];
extern	int logtotime[2];
extern	int cputotime[2];
extern	int chgflag ;
extern	char projbuf[6];
extern	char oprbuf[6];
extern	char *projnm ;
extern	char **argv;
extern	int tglob();
extern 	int	trim();
extern char echon;
extern char echoff;
