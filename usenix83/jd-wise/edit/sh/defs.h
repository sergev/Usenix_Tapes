#
/*
 *	UNIX shell
 */

/* error exits from various parts of shell */
#define ERROR	1
#define SYNBAD	2
#define SIGFAIL 3
#define SIGFLG	0200

/* command tree */
#define FPRS	020
#define FINT	040
#define FAMP	0100
#define FPIN	0400
#define FPOU	01000
#define FPCL	02000
#define FCMD	04000
#define COMMSK	017

#define TCOM	0
#define TPAR	1
#define TFIL	2
#define TLST	3
#define TIF	4
#define TWH	5
#define TUN	6
#define TSW	7
#define TAND	8
#define TORF	9
#define TFORK	10
#define TFOR	11

/* execute table */
#define SYSSET	1
#define SYSCD	2
#define SYSEXEC	3
#define SYSLOGIN 4
#define SYSTRAP	5
#define SYSEXIT	6
#define SYSSHFT 7
#define SYSWAIT	8
#define SYSCONT 9
#define SYSBREAK 10
#define SYSEVAL 11
#define SYSDOT	12
#define SYSRDONLY 13
#define SYSTIMES 14
#define SYSXPORT 15
#define SYSNULL 16
#define SYSREAD 17
#define SYSTST	18
#define	SYSUMASK	19
#define SYSLOGOUT 20 /* Explicit logout feature - dhj 12/6/79 */

/* used for input and output of shell */
#define INIO 10
#define OTIO 11

/*io nodes*/
#define USERIO	10
#define IOUFD	15
#define IODOC	16
#define IOPUT	32
#define IOAPP	64
#define IOMOV	128
#define IORDW	256
#define INPIPE	0
#define OTPIPE	1

/* arg list terminator */
#define ENDARGS	0

#include	"mac.h"
#include	"mode.h"
#include	"name.h"


/* result type declarations */
#define alloc malloc
char*		alloc();
int		addblok();
char*		make();
char*		movstr();
TREPTR		cmd();
TREPTR		makefork();
NAMPTR		lookup();
int		setname();
int		setargs();
DOLPTR		useargs();
float		expr();
char*		catpath();
char*		getpath();
char*		*scan();
char*		mactrim();
char*		macro();
char*		execs();
int		await();
int		post();
char*		copyto();
int		exname();
char*		staknam();
int		printnam();
int		printflg();
int		prs();
int		prc();
int		getenv();
char*		*setenv();

#define attrib(n,f)	(n->namflg |= f)
#define round(a,b)	(((int)((ADR(a)+b)-1))&~((b)-1))
#define closepipe(x)	(close(x[INPIPE]), close(x[OTPIPE]))
#define eq(a,b)		(cf(a,b)==0)
#define max(a,b)	((a)>(b)?(a):(b))
#define assert(x)	;

/* temp files and io */
UFD		output;
int		ioset;
IOPTR		iotemp;		/* files to be deleted sometime */
IOPTR		iopend;		/* documents waiting to be read at NL */

/* substitution */
int		dolc;
char*		*dolv;
DOLPTR		argfor;
ARGPTR		gchain;

/* stack */
#define		BLK(x)	((BLKPTR)(x))
#define		BYT(x)	((BYTPTR)(x))
#define		STK(x)	((STKPTR)(x))
#define		ADR(x)	((char*)(x))

/* stak stuff */
#include	"stak.h"

/* string constants */
char		atline[];
char		readmsg[];
char		colon[];
char		minus[];
char		nullstr[];
char		sptbnl[];
char		unexpected[];
char		endoffile[];
char		synmsg[];

/* name tree and words */
SYSTAB		reserved;
int		wdval;
int		wdnum;
ARGPTR		wdarg;
int		wdset;
char		reserv;

/* prompting */
char		stdprompt[];
char		supprompt[];
char		profile[];

/* built in names */
NAMNOD		fngnod;
NAMNOD		ifsnod;
NAMNOD		homenod;
NAMNOD		mailnod;
NAMNOD		pathnod;
NAMNOD		ps1nod;
NAMNOD		ps2nod;

/* special names */
char		flagadr[];
char*		cmdadr;
char*		exitadr;
char*		dolladr;
char*		pcsadr;
char*		pidadr;

char		defpath[];

/* names always present */
char		mailname[];
char		homename[];
char		pathname[];
char		fngname[];
char		ifsname[];
char		ps1name[];
char		ps2name[];

/* transput */
char		tmpout[];
char*		tmpnam;
int		serial;
#define		TMPNAM 7
FILE		standin;
#define input	(standin->fdes)
#define eof	(standin->feof)
int		peekc;
char*		comdiv;
char		devnull[];

/* flags */
#define		noexec	01
#define		intflg	02
#define		prompt	04
#define		setflg	010
#define		errflg	020
#define		ttyflg	040
#define		forked	0100
#define		oneflg	0200
#define		rshflg	0400
#define		waiting	01000
#define		stdflg	02000
#define		execpr	04000
#define		readpr	010000
#define		keyflg	020000
int		flags;

/* error exits from various parts of shell */
#include	<setjmp.h>
jmp_buf		subshell;
jmp_buf		errshell;

/* fault handling */
#include	"brkincr.h"
unsigned		brkincr;

#define MINTRAP	0
#define MAXTRAP	17

#define INTR	2
#define QUIT	3
#define MEMF	11
#define ALARM	14
#define KILL	15
#define TRAPSET	2
#define SIGSET	4
#define SIGMOD	8

int		fault();
char		trapnote;
char*		trapcom[];
char		trapflg[];

/* name tree and words */
char*		*environ;
char		numbuf[];
char		export[];
char		readonly[];

/* execflgs */
int		exitval;
char		execbrk;
int		loopcnt;
int		breakcnt;

/* messages */
char		mailmsg[];
char		coredump[];
char		badopt[];
char		badparam[];
char		badsub[];
char		nospace[];
char		notfound[];
char		badtrap[];
char		baddir[];
char		badshift[];
char		illegal[];
char		restricted[];
char		execpmsg[];
char		notid[];
char		wtfailed[];
char		badcreate[];
char		piperr[];
char		badopen[];
char		badnum[];
char		arglist[];
char		txtbsy[];
char		toobig[];
char		badexec[];
char		notfound[];
char		badfile[];

address	end[];

#include	"ctype.h"

