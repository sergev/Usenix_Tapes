#
#include <acct2.h>
#include <site.h>
#include <error.h>
#include <error.h>
#include <sysid.h>
#define HANGUP  1
#define INTR    2
#define QUIT    3
#define LINSIZ 1000
#define ARGSIZ 50
#define TRESIZ 100

#define QUOTE 0200
#define FAND 1
#define FCAT 2
#define FPIN 4
#define FPOU 8
#define FPAR 16
#define FINT 32
#define FPRS 64
#define TCOM 1		/* shell-executed command */
#define TPAR 2		/* parameter */
#define TFIL 3
#define TLST 4
#define DTYP 0
#define DLEF 1		/* redirected standard input (<) */
#define DRIT 2		/* redirected standard output (>) */
#define DERR 3	/* redirected error output */
#define DFLG 4
#define DSPR 5
#define DCOM 6		/* command name */
#define ENOMEM  12
#define ENOEXEC 8
#define echo(xchar) if(eflag) putc(xchar)
#define DEVDIR "/dev"
#define MAILBOX "/mailbox"

#define promp   vbls['M'-'A']   /* prompt is now $M */
#define ttyname vbls['T'-'A']	/* tty name is now $T */
#define username vbls['U'-'A']	/* user name is now $U */
#define day vbls['D'-'A']	/* day is now $D */
#define month vbls['X'-'A']	/* month is now $X */
#define year vbls['Y'-'A']	/* year is now $Y */
#define homedir vbls['H'-'A']  /* home directory is now $H */
#define dayweek vbls['W'-'A']	/* day of the week is now $W */
#define curwkdir vbls['K'-'A']	/* current work directory is $K */
#define oprno vbls['R'-'A']	/* opr nr is now $R */
#define projno vbls['J'-'A']	/* project nr is now $J */
#define sysname vbls['S'-'A']	/* system name is $S */
#define sysclass vbls['C'-'A']	/* system classification is $C */
struct ttybf {
	int	dinode;
	char	tt1;
	char	tt2;
	char	yy;
	char	dname;
	char	dend;
	char	filler[9];
};
struct statbf {
	int	fill1;
	int	sinode;
	int	fill2[3];
	int	size1;
	int	fill3[12];
};
struct stime {
        int proct[2];
        int cputim[2];
        int systim[2];
};
