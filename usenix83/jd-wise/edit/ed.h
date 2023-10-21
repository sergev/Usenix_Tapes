#include <signal.h>
#include <setjmp.h>
#ifndef major
#include <sys/types.h>
#endif
#include <sys/stat.h>
#include "address.h"
#include "options.h"

#define	FNSIZE	64
#define	LBSIZE	512
#define	ESIZE	128
#define	GBSIZE	256
#define	NBRA	5
#define	EOF	(-1)
#define EOL	(-2)

#define	CBRA	1
#define	CCHR	2
#define	CDOT	4
#define	CCL	6
#define	NCCL	8
#define	CDOL	10
#define	CEOF	11
#define	CKET	12
#define CBACK	14

#define	STAR	01

#define	READ	0
#define	WRITE	1
#define FILEMODE 0660

int	peekc;
int	lastc;
/*char	savedfile[FNSIZE];*/
char	file[FNSIZE];
char	linebuf[LBSIZE];
char	rhsbuf[LBSIZE/2];
char	expbuf[ESIZE+4];
int	circfl;
/*
int	*zero;
#define one zero+1
int	*dot;
int	*dol;
unsigned nlall;
*/
/*
int	*endcore;
int	*fendcore;
*/
struct address	addr1;
struct address	addr2;
char	genbuf[LBSIZE];
char	globbuf[LBSIZE+4];
int	count[2];
char	*nextip;
char	*linebp;
int	ninbuf;
int	io;
int	scriptfd;

#ifdef undef	/* these are now defined in options.h */
int	allflag;
int	crashpid;	/* for crash recovery */
int	dbflag;
int	indflag;	/* auto-indent */
int	jsflag;		/* seperate joined lines with spaces */
int	owriteflag;	/* default overwrite mode */
int	rflag;
int	sbflag;		/* beep on shell return */
int	sflag;
int	wflag;
int	xflag;
#endif

int	hflag;
int	invflag;
int	redrawflag;
int	lflag;
/*int	mmodflag;*/
int	newflag;
int	owrite;		/* current overwrite mode */
int	pflag;
int	proflag;
int	statflag;
int	txflag;
int	updflag;

int	(*oldalrm)();
int	(*oldhup)();
int	(*oldquit)();
int	listf;
char	*globp;
char	*loc1;
char	*loc2;
char	*locs;
/*
char	ibuff[512];
char	obuff[512];
*/
int	errfunc();
struct	mark{
	struct window *mk_window;
	int mk_addr;
	}marks[26];
/*
int	names[26];
*/
int	highmark;
char	namet[26];
char	*braslist[NBRA];
char	*braelist[NBRA];
int	filemode;
struct stat statbuf;
jmp_buf	savej;
int	jumpset;
long	lseek();
char	*malloc();
char	*realloc();
