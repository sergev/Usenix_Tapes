/*
 */

extern	nulldev(), nodev();
extern	mmread(), mmwrite();
extern	rkstrategy(), rkread(), rkwrite(), rktab;
extern	rx2open(), rx2strategy(), rx2read(), rx2write(), rx2sgtty(), rx2tab;
extern	klopen(), klclose(), klread(), klwrite(), klsgtty();
extern	syopen(), syread(), sywrite(), sysgtty();
extern	dhopen(), dhclose(), dhread(), dhwrite(), dhsgtty();
#ifdef	PTY
extern	ptcopen(), ptcclose(), ptcread(), ptcwrite(), ptysgtty();
extern	ptsopen(), ptsclose(), ptsread(), ptswrite();
#endif

int	(*bdevsw[])()
{
	&nulldev,	&nulldev,	&rkstrategy, 	&rktab,	/* rk */
	&nodev,		&nodev,		&nodev,		0,	/* rp */
	&nodev,		&nodev,		&nodev,		0,	/* rf */
	&nodev,		&nodev,		&nodev,		0,	/* tm */
	&nodev,		&nodev,		&nodev,		0,	/* tc */
	&nodev,		&nodev,		&nodev,		0,	/* hs */
	&nodev,		&nodev,		&nodev,		0,	/* hp */
	&nodev,		&nodev,		&nodev,		0,	/* ht */
	&nodev,		&nodev,		&nodev,		0,	/* dd */
	&rx2open,	&nulldev,	&rx2strategy,	&rx2tab, /* rx */
	0
};

int	(*cdevsw[])()
{
	&klopen,   &klclose,  &klread,   &klwrite,  &klsgtty,	/*  0 console */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/*  1 pc */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/*  2 lx */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/*  3 dc */
 	&dhopen,   &dhclose,  &dhread,   &dhwrite,  &dhsgtty,   /*  4 dh */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/*  5 dp */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/*  6 dj */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/*  7 dn */
	&nulldev,  &nulldev,  &mmread,   &mmwrite,  &nodev,	/*  8 mem */
	&nulldev,  &nulldev,  &rkread,   &rkwrite,  &nodev,	/*  9 rk */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* 10 rf */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* 11 rp */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* 12 tm */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* 13 hs */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* 14 hp */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* 15 ht */
	&syopen,   &nulldev,  &syread,   &sywrite,  &sysgtty,	/* 16 sys */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* 17 dd */
	&rx2open,  &nulldev,  &rx2read,  &rx2write, &rx2sgtty,	/* 18 rx */
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

int	rootdev	{(9<<8)|0};
int	swapdev	{(9<<8)|0};
int	swplo	300;	/* cannot be zero */
int	nswap	194;

/*
 * Identification
 */

char	sys_name[]	"Case CWRUnet Unix System";
char	sys_ver[]	"6.5.0";
char	sys_date[]	"3 Jun 79";
