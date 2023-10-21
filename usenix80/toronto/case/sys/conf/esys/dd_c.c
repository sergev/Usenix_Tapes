/*
 */

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
	&nulldev,	&nulldev,	&ddstrategy, 	&ddtab,	/* dd */
	&nulldev,	&nulldev,	&rxstrategy,	&rxtab,	/* rx */
	0
};

int	(*cdevsw[])()
{
	&klopen,   &klclose,  &klread,   &klwrite,  &klsgtty,	/* console */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* pc */
	&lcopen,   &lcclose,  &nodev,    &lcwrite,  &nodev,	/* lc */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* dc */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* dh */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* dp */
	&djopen,   &djclose,  &djread,   &djwrite,  &djsgtty,	/* dj */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* dn */
	&nulldev,  &nulldev,  &mmread,   &mmwrite,  &nodev,	/* mem */
	&nulldev,  &nulldev,  &rkread,   &rkwrite,  &nodev,	/* rk */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* rf */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* rp */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* tm */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* hs */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* hp */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* ht */
	&syopen,   &nulldev,  &syread,   &sywrite,  &sysgtty,	/* sys */
	&nulldev,  &nulldev,  &ddread,   &ddwrite,  &nodev,	/* dd */
	0
};

int	rootdev {(0<<8)|0};
int	swapdev	{(0<<8)|0};
int	swplo	4000;	/* cannot be zero */
int	nswap	872;

/*
 * Identification
 */

char	sys_name[]	"Case Esys Unix System";
char	sys_ver[]	"6.3.1";
char	sys_date[]	"16 Jan 79";
