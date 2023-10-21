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

extern int	*addr1;
extern int	*addr2;
extern int	circfl;
extern int	col;
extern int	colread;
extern int	count[2];
extern int	curdepth;
extern int	*dol;
extern int	*dot;
extern int	*endcore;
extern int	errfunc();
extern int	fakend;
extern int	fchanged;
 		/* fchanged=0 means internal file = external file
 		 fchanged=1 means internal file = temp files
 		 fchanged>1 means internal file differs from temp
 		 files by less than fchanged lines */
extern int	*fendcore;
extern int     gflag;
extern int     hiding;
extern int	iblock;
extern int	ichanged;
extern int	imedmode;
extern int	io;
extern int     istty;
extern int	listf;
extern int     margin;
extern int	names[26];
extern int	ninbuf;
extern int	nleft;
extern int     nostty;		/* flag whether to do stty or not (1=not do) */
extern int	oblock;
extern int	onhup();
extern int	onquit;
extern int	peekc;
extern int	pflag;
extern int	*stackptr;
extern int	deepstk[10];
extern int	savin;
extern int     savvflag;
extern int     start;
extern int	tfile;
extern int	tline;
extern int	tlpfile;
extern int     ttmode[3];
extern int     ttnorm;		/* saves ttmode[2] during 'o' commands */
extern int	vflag;
extern int	yflag;
extern int	bflag;
extern int	*zero;
extern int	zflag;
extern int     zlength;
extern char	*braelist[NBRA];
extern char	*braslist[NBRA];
extern char	BUFERR[16];
extern char    colbuf[GBSIZE];
extern char    *colbufp;
extern char    colperm[10];
extern char    delimit;
extern char	expbuf[ESIZE+4];
extern char	file[FNSIZE];
extern char    *fr;
extern char	genbuf[LBSIZE];
extern char	globuf[GBSIZE];
extern char    *globfile;				/* ptr to global cmd file */
extern char	*globp;
extern char    *hup;
extern char	ibuff[512];
extern char	IOERR[12];
extern char	JOINERR[23];
extern char	*linebp;
extern char	linebuf[LBSIZE];
extern char	*loc1;
extern char	*loc2;
extern char	*locs;
extern char    nextfile[FNSIZE];
extern char	*nextip;
extern char	obuff[512];
extern char    *prompt;
extern char	rhsbuf[LBSIZE/2];
extern char	savedfile[FNSIZE];
extern char	*tfname;
	 		/* tfname = "etmp.Exxxxx" where xxxxx is process id
	 		   in decimal, and E is either 'e' for text temporary
	 		   (maintained by append and delete routines)
	 		   or 'E' for line index temporary (maintained by
	 		   flushio routine)
	 		   if for some reason etmp.Exxxxx can't be created on entry,
	 		   /tmp/Exxxxx is tried.
	 		*/
extern char	TMPERR[16];
extern char    *to;
extern char    *whichfile;
