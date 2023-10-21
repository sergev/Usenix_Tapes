#
#define	CBRA	1
#define	CCHR	2
#define	CCL	6
#define	CDOL	10
#define	CDOT	4
#define	CEOF	11
#define	CKET	12
#define	COPY     1
#define CTLA	001
#define CTLC	003
#define CTLH	010
#define CTLX	030
#define ECHO    010
#define	EOF	-1
#define EOT     004
#define	ERROR	errfunc("?")
#define	ESIZE	128
#define FLUSHLIM 20 /* number of changed lines before temp updated */
#define	FNSIZE	64
#define	GBSIZE	256
#define	LBSIZE	512
#define LEFT 0
#define LENGTH  80
#define	NBRA	9
#define	NCCL	8
#define NOCOPY 0
#define NP	014
#define	READ	0
#define RIGHT   1
#define	SIGHUP	1
#define	SIGINTR	2
#define	SIGQUIT	3
#define	STAR	01
#define	WRITE	1

int	*addr1;
int	*addr2;
int	circfl;
int	col;
int	colread;
int	count[2];
int	curdepth;
int	*dol;
int	*dot;
int	*endcore;
int	errfunc();
int	fakend;
int	fchanged;
 		/* fchanged=0 means internal file = external file
 		 fchanged=1 means internal file = temp files
 		 fchanged>1 means internal file differs from temp
 		 files by less than fchanged lines */
int	*fendcore;
int     gflag;
int	hiding	0;
int	iblock	-1;
int	ichanged;
int	imedmode;
int	io;
int	istty	0;
int	listf;
int     margin;
int	names[26];
int	ninbuf;
int	nleft;
int     nostty;		/* flag whether to do stty or not (1=not do) */
int	oblock	-1;
int	onhup();
int	onquit;
int	peekc;
int	pflag;
int	*stackptr;
int	deepstk[10];
int	savin;
int     savvflag;
int     start;
int	tfile	-1;
int	tline;
int	tlpfile;
int     ttmode[3];
int     ttnorm;		/* saves ttmode[2] during 'o' commands */
int	vflag	1;
int	yflag;
int	bflag;
int	*zero;
int	zflag	0;
int	zlength	23;
char	*braelist[NBRA];
char	*braslist[NBRA];
char	BUFERR[16]	"Buffer overflow";
char    colbuf[GBSIZE];
char    *colbufp;
char	colperm[10]	"\\([^\t]*\\)";
char    delimit;
char	expbuf[ESIZE+4];
char	file[FNSIZE];
char    *fr;
char	genbuf[LBSIZE];
char	globuf[GBSIZE];
char    *globfile;				/* ptr to global cmd file */
char	*globp;
char	*hup	"ed-hupaXXXXXX";
char	ibuff[512];
char	IOERR[12]	"Write error";
char	JOINERR[23]	"Too many chars to join";
char	*linebp;
char	linebuf[LBSIZE];
char	*loc1;
char	*loc2;
char	*locs;
char    nextfile[FNSIZE];
char	*nextip;
char	obuff[512];
char    *prompt;
char	rhsbuf[LBSIZE/2];
char	savedfile[FNSIZE];
char	*tfname;
	 		/* tfname = "etmp.Exxxxx" where xxxxx is process id
	 		   in decimal, and E is either 'e' for text temporary
	 		   (maintained by append and delete routines)
	 		   or 'E' for line index temporary (maintained by
	 		   flushio routine)
	 		   if for some reason etmp.Exxxxx can't be created on entry,
	 		   /tmp/Exxxxx is tried.
	 		*/
char    *to;
char	TMPERR[16]	"Temp file error";
char    *whichfile;
