extern
	mmopen(),	mmclose(),
	nodev(),	nodev(),	nodev(),	
	nulldev(),	nulldev(),	swstrategy(),	swtab(),	
	nodev(),	nodev(),	nodev(),	
	nodev(),	nodev(),	nodev(),	
	tcopen(),        tcclose(),       tcstrategy(),    tctab(), 
	hsopen(),	nulldev(),	hsstrategy(), 	hstab(),	
	hpopen(),	nulldev(),	hpstrategy(), 	hptab(),	
	htopen(),	htclose(),	htstrategy(), 	httab(),	
	siopen(),	nulldev(),	sistrategy(),	sitab(),
	klopen(),   klclose(),  klread(),   klwrite(),  klsgtty(),	
	nodev(),    nodev(),    nodev(),    nodev(),    nodev(),     
	tcopen(),   tcclose(),  tcread(),   tcwrite(),  nodev(),	
	dhopen(),   dhclose(),  dhread(),   dhwrite(),  dhsgtty(),	
	nodev(),    nodev(),    nodev(),    nodev(),    nodev(),	
	nodev(),    nodev(),    nodev(),    nodev(),    nodev(),	
	nodev(),    nodev(),    nodev(),    nodev(),    nodev(),	
	nulldev(),  nulldev(),  mmread(),   mmwrite(),  nodev(),	
	nodev(),    nodev(),    nodev(),    nodev(),    nodev(),	
	nodev(),    nodev(),    nodev(),    nodev(),    nodev(),	
	nodev(),    nodev(),    nodev(),    nodev(),    nodev(),	
	nodev(),    nodev(),    nodev(),    nodev(),    nodev(),	
	hsopen(),   nulldev(),  hsread(),   hswrite(),  nodev(),	
	hpopen(),   nulldev(),  hpread(),   hpwrite(),  nodev(),	
	htopen(),   htclose(),  htread(),   htwrite(),  nodev(),	
        syopen(),   nulldev(),  syread(),   sywrite(),  sysgtty(),   
	mxopen(),   mxclose(),  mxread(),   mxwrite(),  mxsgtty(),	
	ptsopen(),  ptsclose(), ptsread(),  ptswrite(), ptsgtty(),  
	ptcopen(),  ptcclose(), ptcread(),  ptcwrite(),   
	xxopen(),   xxclose(),  xxread(),   nodev(),    nodev(),     
	tblopen(),  nulldev(),  tblread(),  tblwrite(), nodev(),     
	epopen(),   epclose(),  nodev(),    epwrite(),  nodev(),     
	siopen(),		siread(),   siwrite(),
	sjopen(),		sjread(),	sjwrite(),
	;
/*
 */

int	(*bdevsw[])()
{
	&nodev,		&nodev,		&nodev,		0,	/* rk */
	&nulldev,	&nulldev,	&swstrategy,	&swtab,	/* swap */
	&nodev,		&nodev,		&nodev,		0,	/* rf */
	&nodev,		&nodev,		&nodev,		0,	/* tm */
	&tcopen,        &tcclose,       &tcstrategy,    &tctab, /* tc */
	&hsopen,	&nulldev,	&hsstrategy, 	&hstab,	/* hs */
	&hpopen,	&nulldev,	&hpstrategy, 	&hptab,	/* hp */
	&htopen,	&htclose,	&htstrategy, 	&httab,	/* ht */
	&siopen,	&nulldev,	&sistrategy,	&sitab,	/* si-0 */
	&sjopen,        &nulldev,       &sjstrategy,    &sjtab, /* si-1 */
	0
};

int	(*cdevsw[])()
{
	&klopen,   &klclose,  &klread,   &klwrite,  &klsgtty,	/* console */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,     /* pc */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,     /* lp */
	&tcopen,   &tcclose,  &tcread,   &tcwrite,  &nodev,	/* tc */
	&dhopen,   &dhclose,  &dhread,   &dhwrite,  &dhsgtty,	/* dh */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* free */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* free */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* dn */
	&mmopen,  &mmclose,  &mmread,   &mmwrite,  &nodev,	/* mem */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* rk */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* rf */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* rp */
	&nodev,    &nodev,    &nodev,    &nodev,    &nodev,	/* tm */
	&hsopen,   &nulldev,  &hsread,   &hswrite,  &nodev,	/* hs */
	&hpopen,   &nulldev,  &hpread,   &hpwrite,  &nodev,	/* hp */
	&htopen,   &htclose,  &htread,   &htwrite,  &nodev,	/* ht */
        &syopen,   &nulldev,  &syread,   &sywrite,  &sysgtty,   /* tty */
	&mxopen,   &mxclose,  &mxread,   &mxwrite,  &mxsgtty,	/* mx */
	&ptsopen,  &ptsclose, &ptsread,  &ptswrite, &ptsgtty,  /* pseudo ttys */
	&ptcopen,  &ptcclose, &ptcread,  &ptcwrite, &nodev,  /*  "      " */
	&xxopen,   &xxclose,  &xxread,   &nodev,    &nodev,     /* pump */
	&tblopen,  &nulldev,  &tblread,  &tblwrite, &nodev,     /* sys tables */
	&epopen,   &epclose,  &nodev,    &epwrite,  &nodev,     /* ep */
	&siopen,   &nulldev,  &siread,   &siwrite,  &nodev,	/* si-0 */
	&sjopen,   &nulldev,  &sjread,   &sjwrite,  &nodev,     /* si-1 */
	&nulldev,  &nulldev,  &nulldev,  &ilwrite,  &nodev,     /* ingres */
	0
};

/*
 * devname is device name which prdev()/prf.c uses
 * when printing out deverrors, etc.
 * Unfortunately, this restricts the placement of entries in cdevsw.
 * "raw" block devices (which appear in cdevsw) must have a minor
 * number greater than the highest entry in bdevsw for prdev() to 
 * print out right devname in a deverror.
 * Also many devices (like hs and ht) use all kinds of weird bits
 * set in minor dev number to select funny things, O well.
 */

char *devname[]{
	0, "swap", 0, "rtc", "tc","hs","hp","mt","si","sj",
	0, 0, 0, "rhs","rhp","rmt",0,0,0,0,0,0,0,"rsi","rsj"};

/*  the following variables are now defined in "/usr/sys/conf/low.s"
 *  so they may be patched at boot-up time incase of a hardware
 *  failure of the root/swap devices
int	rootdev	{(6<<8)|0};	/* hp0 - RP04 
int	swapdev	{(5<<8)|8};	/* hs0 - RS04 
int	swplo	1;	/* cannot be zero 
int	nswap	2047;
 */
