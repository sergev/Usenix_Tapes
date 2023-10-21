/*	defines and structures for mgtsum and mgtrpt	*/

#define	PRHOLD "/usr/adm/pr.hold"
#define	SH2HOLD	"/usr/adm/sh2.hold"
#define	LOGHOLD	"/usr/adm/log.hold"
#define MGDATA	"/usr/adm/mgt.data"

struct	mrec	{
int	mtime[2];
int	nlog;
int	maxu;
int	pmaxu;
int	ncmd;
int	nsf;
unsigned	int	nproc;
float	ahours;
float	pavusr;
float	loghr;
float	ploghr;
float	cpsys;
float	cpusr;
float	pcpsys;
float	pcpusr;
float	pcputil;
float	pavrt;
float	prt90;
float	turn;
float	pturn;
float	avturn;
float	pavturn;
float	usf;
int	reboot;
int	unavail;
int	crash;
int	tproc;
int	tblk;
int	ccnt;
int	core;
int	proccor;
int	swapout;
int	rproc;
int	swappr;
int	coreleft;
int	cormap;
int	swapleft;
int	swapmap;
int	filemap;
int	inmap;
int	textmap;
int	stkmap;
int	callmap;
char	filler2[8];
};

struct sh2 {
char	chname[14];
char	shf2;
char	uid2;
int	date2[2];
int	realt2[2];
int	bcput2[2];
int	bsyst2[2];
};
