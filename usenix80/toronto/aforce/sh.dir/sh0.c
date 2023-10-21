#
/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
#include	"defs.h"
char    shnam[] "/bin/sh";	/*name of shell */
char    globnam[] "/sys/prog/glob";  /* name of glob */
char    bin[32];        /* 256 bits that are set for users bin dir */
char    pwbuf[80];      /* for password file entry and then current dir */
char    dfltfile[30];   /* for default users bin directory */
char    *vbls[52];      /* shell variables */
char    *dolp;
char	pidp[6];
int     ldivr;
int	devdir;
struct ttybf ttybuf;
struct statbf statbuf;
struct stime timeb;
char    **dolv;
int     dolc;
char uprojbuf[10];
char mailbx[80];
char cwkdir[80];
char	daybuf[3];		/* day buffer */
char	monbuf[3];		/* month buffer */
char	yearbuf[3];		/* year buffer */
char	usernm[9] "unknown";
char	ttynm[10] "/dev/tty?";
char    *linep;		/* ptr to input line of words */
char	*p_linep;	/* ptr to previous input line */
char    *elinep;	/* ptr to end of input line */
char    **argp;		/* actual end of args */
char    **eargp;		/* max possible end of args */
int     *treep;
int     *treeend;
char    peekc;
char gflg;		/* 1=some global chars are in cmd line */
int	topshell;
char error;
char acctf;		/* accounting file descriptor */
char    uid;
char	gid;
char    setintr;
int	ttys[3];	/* holding area for stty params */
int	oldmode;	/* ttys[2] holding area */
int     sinfil;         /* real standard input during a next */
int     eflag;          /* -e echo flag */
char    *rpromp;        /* real prompt string during a next */
char    *arginp;
int     onelflg;	/* flag for one line of input only */
char    *mesg[] {
        0,
        "Hangup",
        0,
        "Quit",
        "Illegal instruction - recompile with -f",
        "Trace/BPT trap",
        "IOT trap",
        "EMT trap",
        "Floating exception",
        "Killed",
        "Bus error - negative address or subscript",
        "Memory fault - oversize address or subscript",
        "Bad system call",
        0,
        "Sig 14",
        "Sig 15",
        "Sig 16",
        "Sig 17",
        "Sig 18",
        "Sig 19",
};
extern struct stime timeb;
char sysnbuf[10];
char syscbuf[2];
char    line[LINSIZ];
char	prev_line[LINSIZ];
int	prev_flag;
char    *args[ARGSIZ];
int     trebuf[TRESIZ];
int logtotime[2];
int cputotime[2];
int chgflag 1;
char projbuf[6];
char oprbuf[6];
char echon '{';
char echoff '}';
char **argv;
