/*
	mkconf -- makes configuration files c.c and l.s from parts list
		  also outputs mch0.s which indicates dump device

	last edit: 19-Jun-1981	D A Gwyn

1) XY11, DUP11, DZ11, CR11, CSS1, MTX512, DR11C, A/D, TC160 support added;
2) PC11, DC11, DJ11, DP11, RS11 support removed;
3) memory parity now trap #10;
4) added "tty" pseudo-device;
5) brought up to 7th Edition level;
6) modified "jsr r0,call" usage slightly (see "m40.s").
*/

#include	<stdio.h>

static char	*btab[] =	/* BLOCK DEVICE TABLE */
	{
	"rk" ,		/* RK11 is block major device 0 */
	"rp" ,		/* RP11 is block major device 1 */
	"rf" ,		/* RF11 is block major device 2 */
	"tm" ,		/* TM11 is block major device 3 */
	"tc" ,		/* TC11 is block major device 4 */
	"cs" ,		/* CSS1 is block major device 5 */
	"hp" ,		/* RJP11 is block major device 6 */
	"ht" ,		/* TU11 is block major device 7 */
	"ct" ,		/* TC160 is block major device 8 */
	NULL
	};

static char	*ctab[] =	/* CHARACTER DEVICE TABLE */
	{
	"console" ,	/* KL11/DL11 is character major device 0 */
	"cr" ,		/* CR11 is character major device 1 */
	"lp" ,		/* LP11 is character major device 2 */
	"mp" ,		/* MTX512 is character major device 3 */
	"dh" ,		/* DH11 is character major device 4 */
	"ad" ,		/* A/D is character major device 5 */
	"dr" ,		/* DR11C is character major device 6 */
	"dn" ,		/* DN11 is character major device 7 */
	"mem" ,		/* memory special device 8 */
	"rk" ,		/* RK11 is character major device 9 */
	"rf" ,		/* RF11 is character major device 10 */
	"rp" ,		/* RP11 is character major device 11 */
	"tm" ,		/* TM11 is character major device 12 */
	"cs" ,		/* CSS1 is character major device 13 */
	"hp" ,		/* RJP11 is character major device 14 */
	"ht" ,		/* TU11 is character major device 15 */
	"dup" ,		/* DUP11 is character major device 16 */
	"tty" ,		/* terminal special device 17 */
	"xy" ,		/* XY11 is character major device 18 */
	"dz" ,		/* DZ11 is character major device 19 */
	"ct" ,		/* TC160 is character major device 20 */
	NULL
	};

static struct tab
	{
	char		*name ;		/* name in parts list */
	int		count ;		/* # of this type of device */
	int		vect ;		/* vector address or
					   300 if no vector or
					   300 + floating vector size */
	int		key ;		/* characteristics: */
#define	CHAR		01	/* character special device */
#define	BLOCK		02	/* block special device */
#define	INTR		04	/* has interrupt vector */
#define	EVEN		010	/* floating vector must be 0xx0 */
#define	KL		020	/* device is KL11/DL11 */
#define	ROOT		040	/* user selected as root device */
#define	SWAP		0100	/* user selected as swap device */
#define	PIPE		0200	/* user selected as pipe device */
	char		*lsv ;		/* l.s code at vector */
	char		*lsla ;		/* l.s C linkage, part A */
	char		*lslb ;		/* l.s C linkage, part B */
	char		*bents ;	/* l.s block device entries */
	char		*cents ;	/* l.s char device entries */
	char		*extb ;		/* extern decls for bents */
	char		*extc ;		/* extern decls for cents */
	}	table[] =
	{
	"console" ,
	-1 ,	60 ,	CHAR+INTR+KL ,
	"\tklin; br4\n\tklou; br4\n" ,
	".globl\t_klrint\nklin:\tjsr\tr0,call\n\tjmp\t_klrint\n" ,
	".globl\t_klxint\nklou:\tjsr\tr0,call\n\tjmp\t_klxint\n" ,
	"" ,
	"\tklopen,  klclose, klread,  klwrite, klsgtty," ,
	"" ,
	"extern\tklopen(), klclose(), klread(), klwrite(), klsgtty();" ,

	"mem" ,
	-1 ,	300 ,	CHAR ,
	"" ,
	"" ,
	"" ,
	"" ,
	"\tnulldev, nulldev, mmread,  mmwrite, nodev," ,
	"" ,
	"extern\tmmread(), mmwrite();" ,

	"tty" ,
	-1 ,	300 ,	CHAR ,
	"" ,
	"" ,
	"" ,
	"" ,
	"\tsyopen,  nulldev, syread,  sywrite, sysgtty," ,
	"" ,
	"extern\tsyopen(), syread(), sywrite(), sysgtty();" ,

	"mp" ,
	0 ,	300 ,	CHAR ,
	"" ,
	"" ,
	"" ,
	"" ,
	"\tmpopen,  mpclose, mpread,  mpwrite, mpsgtty," ,
	"" ,
	"extern\tmpopen(), mpclose(), mpread(), mpwrite(), mpsgtty();" ,

/*
 * 070 PC11 (removed)
 */

	"clock" ,
	-2 ,	100 ,	INTR ,
	"\tkwlp; br6\n" ,
	".globl\t_clock\n" ,
	"kwlp:\tjsr\tr0,call\n\tjmp\t_clock\n" ,
	"" ,
	"" ,
	"" ,
	"" ,

/*
 * 104 KW11-P
 * 110 RSTS/E detect vect 0
 */

	"parity" ,
	-1 ,	114 ,	INTR ,
	"\ttrap; br7+10.\t\t/ memory parity\n" ,
	"" ,
	"" ,
	"" ,
	"" ,
	"" ,
	"" ,

	"xy" ,
	0 ,	120 ,	CHAR+INTR ,
	"\txyou; br4\n" ,
	"" ,
	".globl\t_xyintr\nxyou:\tjsr\tr0,call\n\tjmp\t_xyintr\n" ,
	"" ,
	"\txyopen,  xyclose, nodev,   xywrite, nodev," ,
	"" ,
	"extern\txyopen(), xyclose(), xywrite();" ,

/*
 * 124 DR11-B
 * 130 AD01
 * 134 AFC11
 */

	"ad" ,				/* (not a general DR11-C) */
	0 ,	130 ,	CHAR+INTR ,
	"\tadain; br6\n\tadbin; br6\n" ,
	".globl\t_adeint\nadain:\tjsr\tr0,call\n\tjmp\t_adeint\n" ,
	".globl\t_adintr\nadbin:\tjsr\tr0,call\n\tjmp\t_adintr\n" ,
	"" ,
	"\tadopen,  adclose, adread,  nodev,   adsgtty," ,
	"" ,
	"extern\tadopen(), adclose(), adread(), adsgtty();" ,

	"dr" ,				/* (not a general DR11-C) */
	0 ,	140 ,	CHAR+INTR ,
	"\tdrain; br6\n\tdrbin; br6\n" ,
	".globl\t_drinta\ndrain:\tjsr\tr0,call\n\tjmp\t_drinta\n" ,
	".globl\t_drintb\ndrbin:\tjsr\tr0,call\n\tjmp\t_drintb\n" ,
	"" ,
	"\tdropen,  drclose, drread,  drwrite, drsgtty," ,
	"" ,
	"extern\tdropen(), drclose(), drread(), drwrite(), drsgtty();" ,

/*
 * 140 AA11 (display)
 * 144 AA11 (light pen), RSTS/E crash-dump
 * 150 RH11/RH70 (alt RM02/03)
 * 154 SQUID
 * 160 RL11/RL211
 * 164 unused
 * 170 LP/LS/LV11 #1 (user reserved)
 * 174 LP/LS/LV11 #2 (user reserved)
 */

	"lp" ,
	0 ,	200 ,	CHAR+INTR ,
	"\tlpou; br4\n" ,
	"" ,
	".globl\t_lpint\nlpou:\tjsr\tr0,call\n\tjmp\t_lpint\n" ,
	"" ,
	"\tlpopen,  lpclose, nodev,   lpwrite, nodev," ,
	"" ,
	"extern\tlpopen(), lpclose(), lpwrite();" ,

	"rf" ,
	0 ,	204 ,	BLOCK+CHAR+INTR ,
	"\trfio; br5\n" ,
	".globl\t_rfintr\n" ,
	"rfio:\tjsr\tr0,call\n\tjmp\t_rfintr\n" ,
	"\tnulldev,\tnulldev,\trfstrategy,\t&rftab," ,
	"\tnulldev, nulldev, rfread,  rfwrite, nodev," ,
	"extern\trfstrategy();\nextern struct buf\trftab;" ,
	"extern\trfread(), rfwrite();" ,

/*
 * 204 RS11 (removed)
 * 210 RC11, RK611/RK711, LP20 #2
 */

	"tc" ,
	0 ,	214 ,	BLOCK+INTR ,
	"\ttcio; br6\n" ,
	".globl\t_tcintr\n" ,
	"tcio:\tjsr\tr0,call\n\tjmp\t_tcintr\n" ,
	"\tnulldev,\ttcclose,\ttcstrategy,\t&tctab," ,
	"" ,
	"extern\ttcclose(), tcstrategy();\nextern struct buf\ttctab;" ,
	"" ,

	"ct" ,
	0 ,	214 ,	BLOCK+CHAR+INTR ,
	"\tctio; br5\n" ,
	".globl\t_ctintr\n" ,
	"ctio:\tjsr\tr0,call\n\tjmp\t_ctintr\n" ,
	"\tctopen,\t\tctclose,\tctstrategy,\t&cttab," ,
	"\tctopen,  ctclose, ctread,  ctwrite, nodev," ,
	"extern\tctopen(), ctclose(), ctstrategy();\nextern struct buf\tcttab;" ,
	"extern\tctread(), ctwrite();" ,

	"rk" ,
	0 ,	220 ,	BLOCK+CHAR+INTR ,
	"\trkio; br5\n" ,
	".globl\t_rkintr\n" ,
	"rkio:\tjsr\tr0,call\n\tjmp\t_rkintr\n" ,
	"\tnulldev,\tnulldev,\trkstrategy,\t&rktab," ,
	"\tnulldev, nulldev, rkread,  rkwrite, nodev," ,
	"extern\trkstrategy();\nextern struct buf\trktab;" ,
	"extern\trkread(), rkwrite();" ,

	"tm" ,
	0 ,	224 ,	BLOCK+CHAR+INTR ,
	"\ttmio; br5\n" ,
	".globl\t_tmintr\n" ,
	"tmio:\tjsr\tr0,call\n\tjmp\t_tmintr\n" ,
	"\ttmopen,\t\ttmclose,\ttmstrategy,\t&tmtab," ,
	"\ttmopen,  tmclose, tmread,  tmwrite, nodev," ,
	"extern\ttmopen(), tmclose(), tmstrategy();\nextern struct buf\ttmtab;" ,
	"extern\ttmread(), tmwrite();" ,

	"ht" ,
	0 ,	224 ,	BLOCK+CHAR+INTR ,
	"\thtio; br5\n" ,
	".globl\t_htintr\n" ,
	"htio:\tjsr\tr0,call\n\tjmp\t_htintr\n" ,
	"\thtopen,\t\thtclose,\thtstrategy,\t&httab," ,
	"\thtopen,  htclose, htread,  htwrite, nodev," ,
	"extern\thtopen(), htclose(), htstrategy();\nextern struct buf\thttab;" ,
	"extern\thtread(), htwrite();" ,

	"cr" ,
	0 ,	230 ,	CHAR+INTR ,
	"\tcrin; br6\n" ,
	"" ,
	".globl\t_crintr\ncrin:\tjsr\tr0,call\n\tjmp\t_crintr\n" ,
	"" ,
	"\tcropen,  crclose, crread,  nodev,   nodev," ,
	"" ,
	"extern\tcropen(), crclose(), crread();" ,

/*
 * 234 UDC11, ICS/ICR11, IP11/IP300, RSTS/E statistics ptr
 * 240 PIRQ
 * 244 FPP/FIS exception
 * 250 KT error
 */

	"rp" ,
	0 ,	254 ,	BLOCK+CHAR+INTR ,
	"\trpio; br5\n" ,
	".globl\t_rpintr\n" ,
	"rpio:\tjsr\tr0,call\n\tjmp\t_rpintr\n" ,
	"\tnulldev,\tnulldev,\trpstrategy,\t&rptab," ,
	"\tnulldev, nulldev, rpread,  rpwrite, nodev," ,
	"extern\trpstrategy();\nextern struct buf\trptab;" ,
	"extern\trpread(), rpwrite();" ,

	"hp" ,
	0 ,	254 ,	BLOCK+CHAR+INTR ,
	"\thpio; br5\n" ,
	".globl\t_hpintr\n" ,
	"hpio:\tjsr\tr0,call\n\tjmp\t_hpintr\n" ,
	"\thpopen,\t\tnulldev,\thpstrategy,\t&hptab," ,
	"\thpopen,  nulldev, hpread,  hpwrite, nodev," ,
	"extern\thpopen(), hpstrategy();\nextern struct buf\thptab;" ,
	"extern\thpread(), hpwrite();" ,

	"cs" ,
	0 ,	260 ,	BLOCK+CHAR+INTR ,
	"\tcsio; br5\n" ,
	".globl\t_csintr\n" ,
	"csio:\tjsr\tr0,call\n\tjmp\t_csintr\n" ,
	"\tnulldev,\tnulldev,\tcsstrategy,\t&cstab," ,
	"\tnulldev, nulldev, csread,  cswrite, nodev," ,
	"extern\tcsstrategy();\nextern struct buf\tcstab;" ,
	"extern\tcsread(), cswrite();" ,

/*
 * 260 TA11
 * 264 RX11/RX211
 * 270 LP/LS/LV11 #3 (user reserved)
 * 274 LP/LS/LV11 #4 (user reserved)
 */

/* Floating Vector Devices: */

/*
 * DC11
 */

	"kl" ,
	0 ,	308 ,	INTR+KL ,
	"\tklin; br4+%d.\n\tklou; br4+%d.\n" ,
	"" ,
	"" ,
	"" ,
	"" ,
	"" ,
	"" ,

/*
 * DP11
 * DM11-A
 */

	"dn" ,		/* one vector per 4 devices */
	0 ,	304 ,	CHAR+INTR ,
	"\tdnou; br4+%d.\n\tdnou; br4+%d.\n" ,
	"" ,
	".globl\t_dnint\ndnou:\tjsr\tr0,call\n\tjmp\t_dnint\n" ,
	"" ,
	"\tdnopen,  dnclose, nodev,   dnwrite, nodev," ,
	"" ,
	"extern\tdnopen(), dnclose(), dnwrite();" ,

	"dhdm" ,
	0 ,	304 ,	INTR ,
	"\tdmin; br4+%d.\n" ,
	"" ,
	".globl\t_dmint\ndmin:\tjsr\tr0,call\n\tjmp\t_dmint\n" ,
	"" ,
	"" ,
	"" ,
	"" ,

/*
 * DR11-A
 * DR11-C
 * PA611 (reader)
 * PA611 (punch)
 * LPD11
 * DT11
 * DX11
 */

	"dl" ,
	0 ,	308 ,	INTR+KL ,
	"\tklin; br4+%d.\n\tklou; br4+%d.\n" ,
	"" ,
	"" ,
	"" ,
	"" ,
	"" ,
	"" ,

/*
 * DJ11
 */

	"dh" ,
	0 ,	308 ,	CHAR+INTR+EVEN ,
	"\tdhin; br5+%d.\n\tdhou; br5+%d.\n" ,
	".globl\t_dhrint\ndhin:\tjsr\tr0,call\n\tjmp\t_dhrint\n" ,
	".globl\t_dhxint\ndhou:\tjsr\tr0,call\n\tjmp\t_dhxint\n" ,
	"" ,
	"\tdhopen,  dhclose, dhread,  dhwrite, dhsgtty," ,
	"" ,
	"extern\tdhopen(), dhclose(), dhread(), dhwrite(), dhsgtty();" ,

/*
 * GT40/VSV11
 * LPS11
 * DQ11
 * KW11-W
 * DU11
 */

	"dup" ,
	0 ,	308 ,	CHAR+INTR ,
	"\tduin; br6+%d.\n\tduou; br6+%d.\n" ,
	".globl\t_durint\nduin:\tjsr\tr0,call\n\tjmp\t_durint\n" ,
	".globl\t_duxint\nduou:\tjsr\tr0,call\n\tjmp\t_duxint\n" ,
	"" ,
	"\tduopen,  duclose, duread,  duwrite, nodev," ,
	"" ,
	"extern\tduopen(), duclose(), duread(), duwrite();" ,

/*
 * DV11
 * DV modem control
 * LK11-A
 * DWUN
 * DMC11
 */

	"dz" ,
	0 ,	308 ,	CHAR+INTR ,
	"\tdzin; br5+%d.\n\tdzou; br5+%d.\n" ,
	".globl\t_dzrint\ndzin:\tjsr\tr0,call\n\tjmp\t_dzrint\n" ,
	".globl\t_dzxint\ndzou:\tjsr\tr0,call\n\tjmp\t_dzxint\n" ,
	"" ,
	"\tdzopen,  dzclose, dzread,  dzwrite, dzsgtty," ,
	"" ,
	"extern\tdzopen(), dzclose(), dzread(), dzwrite(), dzsgtty();" ,

/*
 * KMC11
 * LPP11
 * VMV21
 * VMV31
 * VTV01
 * DWR70
 * RL11/RL211
 * RX211
 * TS11
 * LPA11-K
 * IP11/IP300
 * KW11-C
 * RX11
 * DR11-B
 */

	NULL
	};

static char	*lsh[] =	/* l.s heading */
	{
	"/ low core" ,
	"" ,
	NULL
	};

static char	*lsid[] =	/* l.s I & D space only */
	{
	".data" ,
	NULL
	};

static char	*lsl[] =	/* l.s low core & trap vectors */
	{
	"ZERO:" ,
	"" ,
	"br4 = 200" ,
	"br5 = 240" ,
	"br6 = 300" ,
	"br7 = 340" ,
	"" ,
	". = ZERO+0" ,
	"\tbr\t1f" ,
	"\t4" ,
	"" ,
	"/ trap vectors" ,
	"\ttrap; br7+0.\t\t/ bus timeout" ,
	"\ttrap; br7+1.\t\t/ illegal instruction" ,
	"\ttrap; br7+2.\t\t/ bpt-trace" ,
	"\ttrap; br7+3.\t\t/ iot" ,
	"\ttrap; br7+4.\t\t/ power failure" ,
	"\ttrap; br7+5.\t\t/ emt" ,
	"\ttrap; br7+6.\t\t/ trap (sys call)" ,
	"" ,
	". = ZERO+40" ,
	".globl\tstart, dump" ,
	"1:\tjmp\tstart" ,
	"\tjmp\tdump" ,
	"" ,
	NULL
	};

static char	*lsp[] =	/* l.s processor vectors */
	{
	"" ,
	". = ZERO+240" ,
	"\ttrap; br7+7.\t\t/ programmed interrupt" ,
	"\ttrap; br7+8.\t\t/ floating point error" ,
	"\ttrap; br7+9.\t\t/ segmentation violation" ,
	NULL
	};

static char	*lsf[] =	/* l.s floating vectors */
	{
	"" ,
	"/ floating vectors" ,
	". = ZERO+300" ,
	NULL
	};

static char	*lsc[] =	/* l.s C linkages */
	{
	"" ,
	". = ZERO+1000\t\t\t/ no code in interrupt space" ,
	"" ,
	"//////////////////////////////////////////////////////" ,
	"/\t\tinterface code to C" ,
	"//////////////////////////////////////////////////////" ,
	"" ,
	".text" ,
	".globl\tcall, trap" ,
	NULL
	};

static char	*ccp[] =	/* c.c preamble */
	{
	"/* c.c - configuration tables */" ,
	"" ,
	"#include\t\"../h/param.h\"" ,
	"#include\t\"../h/buf.h\"" ,
	"#include\t\"../h/conf.h\"" ,
	"" ,
	"#define\tNULL\t0" ,
	"" ,
	"extern\tnulldev(), nodev();" ,
	NULL
	};

static char	*ccb[] =	/* c.c block device table */
	{
	"" ,
	"struct bdevsw\tbdevsw[] =" ,
	"\t{" ,
	NULL
	};

static char	*ccc[] =	/* c.c character device table */
	{
	"\tNULL" ,
	"\t};" ,
	"" ,
	"struct cdevsw\tcdevsw[] =" ,
	"\t{" ,
	NULL
	};

static char	*cce[] =	/* c.c end tables */
	{
	"\tNULL" ,
	"\t};" ,
	"" ,
	NULL
	};

static int	idspace = 1 ;		/* separate I & D space */
static int	rootmaj = -1 , rootmin ;	/* root device */
static int	swapmaj = -1 , swapmin ;	/* swap device */
static int	pipemaj = -1 , pipemin ;	/* pipe device */
static unsigned	swplo = 4000 , nswap = 872 ;	/* swap blocks */

extern int	strcmp();
static int	input(), equal();


main()
	{
	register struct tab	*p ;	/* -> table[] */
	register char		*q ;
	int			i , n ;
	int			ev ;	/* for floating-vector assignment */
	int			nkl ;	/* # of KLs */
	int			flagf ;	/* no floating vectors */
	int			flagb ;	/* need processor vectors */

	setbuf( stdin , NULL );		/* to run in shell script */
	while( input() )
		;			/* read parts list */

/*
 * pass 1 -- create interrupt vectors
 */

	freopen( "l.s" , "w" , stdout );

	puke( lsh );			/* heading */
	if ( idspace )
		puke( lsid );		/* I & D space only */
	puke( lsl );			/* low core & trap vectors */

	ev = nkl = 0 ;
	flagf = flagb = 1 ;
	for ( p = table ; p->name != NULL ; ++ p )
		if ( p->count != 0 && p->key & INTR )
			{
			if ( p->vect > 240 && flagb )
				{
				flagb = 0 ;
				puke( lsp );	/* processor vectors */
				}

			if ( p->vect >= 300 )
				{
				if ( flagf )
					{
					ev = 0 ;
					flagf = 0 ;
					puke( lsf ) ;	/* floating vectors */
					}

				if ( p->key & EVEN && (ev & 07) != 0 )
					{
					printf( "\t.=.+4\n" );
					ev += 4 ;	/* align */
					}
				printf( "/%s %o\n" , p->name , 0300 + ev );
				}
			else
				printf( "\n. = ZERO+%d\n" , p->vect );

			n = p->count ;
			if ( n < 0 )	/* forced count */
				n = - n ;
			for ( i = 0 ; i < n ; ++ i )
				{
				if ( p->key & KL )
					{
					printf( p->lsv , nkl , nkl );
					++ nkl ;
					}
				else	/* encode subdevice */
					printf( p->lsv , i , i );

				/* display assignments for checking */
				if ( p->vect < 300 )
					fprintf( stderr , "%s @ %d\n" ,
					p->name , p->vect + 4 * i );
				else
					fprintf( stderr , "%s @ %o\n" ,
					p->name , 0300 + ev );
				ev += p->vect - 300 ;
				}
			}

	if ( flagb )
		puke( lsp );		/* processor vectors */

	puke( lsc );			/* C linkages */
	for ( p = table ; p->name != NULL ; ++ p )
		if ( p->count != 0 && p->key & INTR )
			printf( "\n%s%s" , p->lsla , p->lslb );	/* linkage */

/*
 * pass 2 -- create configuration table
 */

	freopen( "c.c" , "w" , stdout );

	puke( ccp );			/* preamble */
	for ( p = table ; p->name != NULL ; ++ p )
		if ( p->count != 0 )
			{		/* external declarations */
			if ( p->key & BLOCK && *p->extb != '\0' )
				printf( "%s\n" , p->extb );
			if ( p->key & CHAR && *p->extc != '\0' )
				printf( "%s\n" , p->extc );
			}

	puke( ccb );			/* block device table */
	for ( i = 0 ; (q = btab[i]) != NULL ; ++ i )
		{
		for ( p = table ; p->name != NULL ; ++ p )
			if ( equal( q , p->name ) &&
			     p->key & BLOCK && p->count != 0 )
				{
				printf( "%s\t/* %s = %d */\n" ,
				p->bents , q , i );
				if ( p->key & ROOT )
					rootmaj = i ;
				if ( p->key & SWAP )
					swapmaj = i ;
				if ( p->key & PIPE )
					pipemaj = i ;
				goto newb ;
				}
		printf( "\tnodev,\t\tnodev,\t\tnodev,\t\tNULL,\t/* %s = %d */\n" , q , i );
    newb:	;
		}

	puke( ccc );			/* character device table */
	for ( i = 0 ; (q = ctab[i]) != NULL ; ++ i )
		{
		for ( p = table ; p->name != NULL ; ++ p )
			if ( equal( q , p->name ) &&
			     p->key & CHAR && p->count != 0 )
				{
				printf( "%s\t/* %s = %d */\n" ,
				p->cents , q , i );
				goto newc ;
				}
		printf( "\tnodev,   nodev,   nodev,   nodev,   nodev,\t/* %s = %d */\n" , q , i );
    newc:	;
		}
	puke( cce );			/* end tables */

	if ( rootmaj < 0 )
		fprintf( stderr , "No root device given\n" );
	printf( "int\trootdev = (%d << 8) | %d ;\n" , rootmaj , rootmin );

	if ( swapmaj < 0 )
		{
		swapmaj = rootmaj ;
		swapmin = rootmin ;
		}
	printf( "int\tswapdev = (%d << 8) | %d ;\n" , swapmaj , swapmin );

	if ( pipemaj < 0 )
		{
		pipemaj = rootmaj ;
		pipemin = rootmin ;
		}
	printf( "int\tpipedev = (%d << 8) | %d ;\n" , pipemaj , pipemin );

	if ( swplo == 0 )
		fprintf( stderr , "swplo cannot be zero\n" );
	printf( "unsigned\tswplo = %u ;\t/* cannot be zero */\n" , swplo );

	if ( nswap == 0 )
		fprintf( stderr , "nswap cannot be zero\n" );
	printf( "unsigned\tnswap = %u ;\n" , nswap );

/*
 * pass 3 -- output dump device
 */

	freopen( "mch0.s" , "w" , stdout );
	{
	int	dumpht = 0 ;

	for ( i = 0 ; table[i].name != NULL ; ++ i )
		if ( equal( table[i].name , "ht" ) && table[i].count != 0 )
			dumpht = 1 ;

	printf( "HTDUMP = %d\nTUDUMP = %d\n" , dumpht , 1 - dumpht );
	}

	exit( 0 );
	}


static
puke( s )				/* output several lines */
	register char	*s[];
	{
	register char	*c ;

	while ( (c = *s++) != NULL )
		puts( c );		/* appends newline */
	}


static int
input()					/* input line from parts list */
	{
	char			line[100], keyw[32], dev[32];
	register struct tab	*p ;
	int			count , n ;
	long			num ;

	if ( fgets( line , 100 , stdin ) == NULL )
		return( 0 );		/* EOF */

	count = -1 ;
	n = sscanf( line , "%d%s%s%D" , &count , keyw , dev , &num );
	if ( count == -1 && n > 0 )
		{
		count = 1 ;
		++ n ;
		}
	if ( n < 2 )
		goto badl ;

	if ( equal( keyw , "root" ) )
		{
		if ( n < 4 )
			goto badl ;
		for ( p = table ; p->name != NULL ; ++ p )
			if ( equal( p->name , dev ) )
				{
				p->key |= ROOT ;
				rootmin = num ;
				return ( 1 ) ;
				}
		fprintf( stderr , "Can't find root\n" );
		return ( 1 ) ;
		}

	if ( equal( keyw , "swap" ) )
		{
		if ( n < 4 )
			goto badl ;
		for ( p = table ; p->name != NULL ; ++ p )
			if ( equal( p->name , dev ) )
				{
				p->key |= SWAP ;
				swapmin = num ;
				return ( 1 ) ;
				}
		fprintf( stderr , "Can't find swap\n" );
		return ( 1 ) ;
		}

	if ( equal( keyw , "pipe" ) )
		{
		if ( n < 4 )
			goto badl ;
		for ( p = table ; p->name != NULL ; ++ p )
			if ( equal( p->name , dev ) )
				{
				p->key |= PIPE ;
				pipemin = num ;
				return ( 1 ) ;
				}
		fprintf( stderr , "Can't find pipe\n" );
		return ( 1 ) ;
		}

	if ( equal( keyw , "swplo" ) )
		{
		if ( n < 3 || sscanf( dev , "%D" , &num ) <= 0 )
			goto badl ;
		swplo = num ;
		return ( 1 ) ;
		}

	if ( equal( keyw , "nswap" ) )
		{
		if ( n < 3 || sscanf( dev , "%D" , &num ) <= 0 )
			goto badl ;
		nswap = num ;
		return ( 1 ) ;
		}

	if ( equal( keyw , "noidspace" ) )
		{
		idspace = 0 ;
		return ( 1 ) ;
		}

	for ( p = table ; p->name != NULL ; ++ p )
		if ( equal( p->name , keyw ) )
			{
			if ( p->count < 0 )
				{
				fprintf( stderr ,
				"%s: no more, no less\n" , keyw );
				return ( 1 ) ;
				}
			p->count += count ;
			if ( p->vect < 300 && p->count > 1 )
				{
				p->count = 1 ;
				fprintf( stderr , "%s: only one\n" , keyw );
				}
			return ( 1 ) ;
			}
	if ( equal( keyw , "done" ) )
		return ( 0 ) ;

	fprintf( stderr , "%s: cannot find\n" , keyw );
	return ( 1 ) ;

    badl:
	fprintf( stderr , "Bad line: %s" , line );
	return ( 1 ) ;
	}


static int
equal( a , b )				/* test strings for equality */
	char	*a , *b ;
	{
	return ( strcmp( a , b ) == 0 ) ;
	}
