/*
 */

extern	nulldev(), nodev();
extern	mmread(), mmwrite();
extern	rkopen(), rkclose(), rkstrategy(), rkread(), rkwrite(), rktab;
extern	klopen(), klclose(), klread(), klwrite(), klsgtty();
extern	syopen(), syread(), sywrite(), sysgtty();
extern	dzopen(), dzclose(), dzread(), dzwrite(), dzsgtty();
extern	lxopen(), lxclose(), lxwrite(), lxsgtty();
#ifdef	PTY
extern	ptcopen(), ptcclose(), ptcread(), ptcwrite(), ptysgtty();
extern	ptsopen(), ptsclose(), ptsread(), ptswrite();
#endif

int	(*bdevsw[])()
{
	&rkopen,	&nulldev,	&rkstrategy, 	&rktab,	/* rk */
	&nodev,		&nodev,		&nodev,		0,	/* rp */
	&nodev,		&nodev,		&nodev,		0,	/* rf */
	&nodev,		&nodev,		&nodev,		0,	/* tm */
	&nodev,		&nodev,		&nodev,		0,	/* tc */
	&nodev,		&nodev,		&nodev,		0,	/* hs */
	&nodev,		&nodev,		&nodev,		0,	/* hp */
	&nodev,		&nodev,		&nodev,		0,	/* ht */
	0
};

int	(*cdevsw[])()
{
	&klopen,   &klclose,  &klread,   &klwrite,  &klsgtty,	/*  0 console */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/*  1 pc */
	&lxopen,   &lxclose,  &nodev,    &lxwrite,  &lxsgtty,	/*  2 lx */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/*  3 dc */
	&dzopen,   &dzclose,  &dzread,   &dzwrite,  &dzsgtty,   /*  4 dz */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/*  5 dp */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/*  6 dj */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/*  7 dn */
	&nulldev,  &nulldev,  &mmread,   &mmwrite,  &nodev,	/*  8 mem */
	&rkopen,   &nulldev,  &rkread,   &rkwrite,  &nodev,	/*  9 rk */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* 10 rf */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* 11 rp */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* 12 tm */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* 13 hs */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* 14 hp */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* 15 ht */
	&syopen,   &nulldev,  &syread,   &sywrite,  &sysgtty,	/* 16 sys */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* 17 dx */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* 18 spare */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* 19 spare */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* 20 spare */
#ifndef	PTY
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* 21 spare */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* 22 spare */
#endif
#ifdef	PTY
	&ptcopen,  &ptcclose, &ptcread,  &ptcwrite, &ptysgtty,	/* 21 ptyc */
	&ptsopen,  &ptsclose, &ptsread,  &ptswrite, &ptysgtty,	/* 22 ptys */
#endif
	0
};

int	rootdev	{(0<<8)|0};
int	swapdev	{(0<<8)|0};
int	swplo	4000;	/* cannot be zero */
int	nswap	872;

/*
 * Identification
 */

char	sys_name[]	"Case Ecmp Unix System";
char	sys_ver[]	"6.6.0";
char	sys_date[]	"10 Jun 79";
