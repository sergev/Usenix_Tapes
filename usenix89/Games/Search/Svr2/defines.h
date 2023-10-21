/*
 *
 * search
 *
 * multi-player and multi-system search and destroy.
 *
 * Original by Greg Ordy	1979
 * Rewrite by Sam Leffler	1981
 * Socket code by Dave Pare	1983
 * Ported & improved
 *      by Matt Crawford	1985
 *
 * defines and macros used by search code.
 *
 * Copyright (c) 1979
 *
 * $Header: defines.h,v 2.2 85/08/06 22:25:30 matt Exp $
 * $Log:	defines.h,v $
 * Revision 2.2  85/08/06  22:25:30  matt
 * Change definition of score and error log files for flexibility.
 * 
 * Revision 2.1  85/04/10  17:30:57  matt
 * Major de-linting and minor restructuring.
 * 
 * Revision 1.2  85/02/11  14:59:08  matt
 * raised NPLAYER to 15
 */

#define DEFAULT_IN_PORT 525	/* default internet port number */

/*
 * Configuration definitions
 */
#define	NPLAYER 16		/* # players supported */
#define	MAXMAG	'3'		/* upper bound on magnification */
#define	NBURST  2		/* # concurrent visible shrapnel */
#define	NALIEN  20		/* # of aliens in game */
#define	NBASE	3		/* # of starbases */
#define	NSHANK  12		/* # of shanker aliens */
#define	NWAND	4		/* # of X-aliens */
#define	NTORP	26		/* # of available torpedoes (recycled) */
#define ITCHECK	128		/* # of iterations between new player check */
#define QSIZE	8		/* size of input q for buffering tty reads */
/*
 * Energy costs for various items
 */
#define	SHRAPCOST  20		/* cost of being hit by a piece of shrapnel */
#define	TPCOST  10		/* energy cost to shoot a torpedo */
#define	INCOST  1		/* cost for staying invisible */

#define	SCABLETTER  27
#define	TSIZE	37		/* lifetime of a torpedo */
/*
 * State bits
 */
#define	ALIVE	0		/* player/alien/torpedo alive */
#define	DEAD	7		/* player/alien/torpedo dead */

#define OFF	17
#define ON	23
/*
 * Alien types -- normal, shankers, X-aliens
 */
#define	NORM	11
#define	SHANK	13
#define	WANDER	17
/*
 * Alien monikers -- shown on screen
 */
#define	NAMEAL	'#'		/* normal type */
#define	NAMESH	'@'		/* shanker */
#define	NAMEWD	'X'		/* X-alien */
#define	NAMEP	'%'		/* planet component */
/*
 * Input directional keyboard definitions
 */
#define	NO	'8'
#define	NE	'9'
#define	EA	'6'
#define	SE	'3'
#define	SO	'2'
#define	SW	'1'
#define	WE	'4'
#define	NW	'7'
/*
 * Other sundry commands
 */
#define	FIRE	'0'
/*
 * Union type qualifiers -- used with varargs
 */
#define PLAYER	1
#define ALIEN	2
#define PLANET	3
#define SBASE	4
#define TORPEDO	5

#define		onscreen(ox,oy,x,y)     (ox >= (x - XWIND*mf) && \
ox <= (x + XWIND * mf) && \
oy >= (y - YWIND * mf) && \
oy <= (y + YWIND * mf))

#define NULL	0
#define NULLCH	((char *)0)
#define NULLINT	((int *)0)
#define NOTHING	((thing *)0)
#define NOBODY	((t_player *)0)

#define	TRUE	1
#define FALSE	0

#define isbase(p)	((p) < (thing *)&sbase[NBASE] && \
			 (p) >= (thing *)sbase)
#define isplayer(p)	((p) < (thing *)&player[NPLAYER] && \
			 (p) >= (thing *)player)
#define isalien(p)	((p) < (thing *)&alien[NALIEN] && \
			 (p) >= (thing *)alien)
#define istorp(p)	((p) < (thing *)&torps[NTORP] && \
			 (p) >= (thing *)torps)

/*#define clear(p)	((void) strcat(p->outputq, p->CL))*/
/*#define move(x, y, p)	((void) strcat(p->outputq, tgoto((p),(x),(y)) ))*/


#define POINTFILE	LIBDIR/points"
#define ERRLOG		LIBDIR/errlog"

/*
 * Coordinate definitions for the titles of screen fields --
 *   probably should be defined relative to the screen dimensions.
 */
#define	POSTITLE	55,1
#define	EGYTITLE	56,4
#define	HRTITLE		53,7
#define	H1TITLE		53,8
#define	H2TITLE		53,9
#define	H3TITLE		53,10
#define	PTTITLE		56,12
#define	STTITLE		11,16
#define	INTITLE		4,1
#define	VLTITLE		5,4
#define	TMTITLE		7,7
#define	MFTITLE		5,10
#define	MSTITLE		11,18
#define PLTITLE		23,19
#define XAXISTITLE	CENTX,7
#define YAXISTITLE	34


/*
 * Positions of data fields on screen
 */
#define POS1DX	55
#define POS1DY	2
#define	POS1DATA	POS1DX,POS1DY
#define POS2DX	60
#define POS2DY	2
#define	POS2DATA	POS2DX,POS2DY
#define EGYDATAX	56
#define EGYDATAY	5
#define	EGYDATA		EGYDATAX,EGYDATAY
#define H1DATAX		55
#define H1DATAY		8
#define	H1DATA		H1DATAX,H1DATAY
#define H2DATAX		55
#define H2DATAY		9
#define	H2DATA		H2DATAX,H2DATAY
#define H3DATAX		55
#define H3DATAY		10
#define	H3DATA		H3DATAX,H3DATAY
#define PTDATAX		57
#define PTDATAY		13
#define	PTDATA		PTDATAX,PTDATAY
#define STDATAX		19
#define STDATAY		16
#define	STDATA		STDATAX,STDATAY
#define INDATAX		7
#define INDATAY		2
#define	INDATA		INDATAX,INDATAY
#define VLDATAX		5
#define VLDATAY		5
#define	VLDATA		VLDATAX,VLDATAY
#define TMDATAX		6
#define TMDATAY		8
#define	TMDATA		TMDATAX,TMDATAY
#define MFDATAX		8
#define MFDATAY		11
#define	MFDATA		MFDATAX,MFDATAY
#define MSDATAX		21
#define MSDATAY		18
#define	MSDATA		MSDATAX,MSDATAY
#define PROMPTX		11
#define PROMPTY		19

/*
 * Definition of screen origin of viewport -- objects are located
 *  relative when displayed in lists.c
 */
#define	CENTX	19
#define	CENTY	0

/*
 * Dimensions of viewport
 */
#define	XWIND	15
#define	YWIND	7
