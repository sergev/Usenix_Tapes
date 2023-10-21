#
/*
Name:
	SH - UNIX Shell Command Interpreter

Function:
	Interpret, coordinate, and execute UNIX commands.

Algorithm:
	Build tree structure of command line syntax.  Execute each level of
	interpretation by forking shell and overlaying with command.
	Write accounting data for each command.

Parameters:
	-p x y	project is x, opr is y.
	-e	echo variables as command is executed.
	-c	take 1 command line from std input
	-t	take commands from command line.

Returns:

Files and Programs:

Installation Procedures:

History:
	Modified by John Levine, Yale U., 1976
		Added variables $a-$Z
		Set default values for variables
			$P	decimal process id of most recent execution
			$$	octal process id of this shell
			$M	prompt
			$N	number of arguments
		Added private bin directory search and newbin command
		Changed wait command to address specific process id
		Added -e for echo  of commands as executed
		Added -t -c for one line input
		Added logout command
		Added next command and start_up file handling
		Added cd as chdir command and automatic homing
	Modified by Walter Lazear, AFDSC, Jun 1977
		Removed 'home' command (Yale dependent)
		Added prev command for command editting
		Changed chdir homing to print home directory
		Set default for variables
			$T	original standard input tty
			$U	user's login name
			$D	login day/month (numeric)
			$H	users home directory
			$C	command just executed
			$W	day of the week
			$O	old working directory
			$K	current working directory
			$J	current project id
		Added logo, logoff, and bye command
		Added set command operator (:) for file input
	Modified by Walter Lazear, Sept 1977
		Added -p project id flag
		Changed accounting records for project
		Added newproj command
		Added session cost printing
		Added chg, nochg, and cost commands
		Changed login and newgrp to log out of shell
		Set default for $J as project
		Changed exits to 'batch' mode processing
	Modified by Walter Lazear, Dec 1978
		Changed -p flag to accept OPR
		Added disk quota processing.
		Streamlined acct records.
		Made newproj and cost separate programs.
		Added 'c' to prev.
		Changed batch to not wait (hung rotary).
		Added check for mail at logout.
		Set default for variables
			$R	current project opr
			$D	login day
			$X	login month (numeric)
			$Y	login year (00-99)
*		Removed $C (current command) and $O (old directory)
		Modified 'texec' to use /bin shell files.
		Added mail check every 20 prompts.
		Fixed prev's scan to look inside quotes.
		Prevented previous line from being wiped out by empty cmd
		Redirected error output (fd 2) with ^.
		Syncronized cost and quota in batch.

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
