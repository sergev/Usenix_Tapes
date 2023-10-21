#
/*
 *
 * search: multiplayer chase and destroy game
 * Greg Ordy, Case Western Reserve University, 1978
 * Copyright (c)
 *
 * this file contains defines and data structure declarations.
*/

#define		NPLAYER 10
#define		MAXMAG	'3'
#define		NBURST  2
#define		SHRAPCOST  20
#define		ALIVE	0
#define		DEAD	7
#define		NORM	11
#define		SHANK	13
#define		SCABLETTER  25
#define		WANDER	17
#define		NEW	20
#define		MIDDLE	38
#define		NALIEN  16
#define		ON	23
#define		OFF	17
#define		NBASE	3
#define		NSHANK  6
#define		NWAND	4
#define		NAMEAL	'#'
#define		NAMESH	'@'
#define		NAMEWD	'X'
#define		NAMEP	'%'
#define		NTORPS	26
#define		TSIZE	37
#define		UP	'8'
#define		NE	'9'
#define		EA	'6'
#define		SE	'3'
#define		SO	'2'
#define		SW	'1'
#define		WE	'4'
#define		NW	'7'
#define		FIRE	'0'
#define		STOP	'5'
#define		XWIND	15
#define		YWIND	7
#define		PLANET	"Quartone         "
#define		POFFSET 50
#define		AOFFSET 30
#define		SOFFSET 90
#define		TOFFSET 100
#define		KOFFSET 115
#define		INVIS	11
#define		CENTX	26
#define		CENTY	0
#define		TPCOST  10
#define		INCOST  1
#define		POSTITLE  62,1
#define		POS1DATA   62,2
#define		POS2DATA   68,2
#define		EGYTITLE  63,4
#define		EGYDATA   64,5
#define		HRTITLE   60,7
#define		H1TITLE   60,8
#define		H2TITLE   60,9
#define		H3TITLE   60,10
#define		H1DATA    63,8
#define		H2DATA    63,9
#define		H3DATA    63,10
#define		PTTITLE	  63,12
#define		PTDATA    64,13
#define		STTITLE   26,16
#define		STDATA    34,16
#define		INTITLE   10,1
#define		INDATA    14,2
#define		VLTITLE	  12,4
#define		VLDATA    12,5
#define		TMTITLE   14,7
#define		TMDATA    14,8
#define		MFTITLE   12,10
#define		MFDATA    15,11
#define		MSTITLE	  26,18
#define		MSDATA    35,18

#define		aliens	ia = 0; ia < NALIEN; ia++
#define		sbases  is = 0; is < NBASE; is++
#define		players	ip = 0; ip < NPLAYER; ip++
#define		tpdoes  pp = 0; pp < NTORPS; pp++

#define		onscreen(ox,oy,x,y)     (ox >= (x - XWIND*mf) && \
ox <= (x + XWIND * mf) && \
oy >= (y - YWIND * mf) && \
oy <= (y + YWIND * mf))

struct fileform
{
    char    plname;
    char    pltype;
    int     plpid;
    int     pluid;
}
                fileform;

struct bursts
{
    char    cbx;
    char    cby;
    char    cbactive;
    char    cbcnt;
    char    shrap[9][2];
    char    shrapd[9];
}
                bursts[NBURST];

struct player
{
    char    name;
    char    curx;
    char    cury;
    char    orb;
    char    bur;
    int     points;
    int     fildes;
    char    home[3];
    char    ocnt;
    char    offx;
    char    offy;
    char   *apx;
    char   *apy;
    char    aflg;
    char    whocent;
    char    cmdpend;
    char    cmd;
    int     ttype;
    int     ppid;
    int     puid;
    char    status;
    char    mflg;
    char    xcount;
    int     maxe;
    struct plist   *plstp;
    int     energy;
            long pltime;
    int     pkill;
    int     phits;
    int     ahits;
    char    crash;
    char    scabcount;
    char    gmess[40];
    int     qkill;
    char    nkcnt;
}

                player[NPLAYER];

struct alien
{
    char    type;
    char    onscr;
    char    cx;
    char    cy;
    char    cix;
    char    ciy;
    char    fx;
    char    fy;
    char    stats;
    char    hit;
    char    count;
    char    whotoget;
    char    aname;
}
                alien[NALIEN];

struct planet
{
    char   *planetname;
    char    places[17][2];
}
                planet;

struct sbase
{
    char    xpos;
    char    ypos;
}
                sbase[NBASE];

struct torps
{
    char    torx;
    char    tory;
    char    owner;
    char    target;
    char    xinc;
    char    yinc;
    char    tcount;
}
                torps[NTORPS];


int     pntfd;
char    itoabuf[12];
int     nplayer;
int     ia,
        ip,
        is,
        pp;
char    queue[1800];
int     qlen;
int     whoscab;
char    xxi,
        yyi;
char    bbuffer[40];
char   *bbpnt;
char    b_owner;
char    rmessb[40];
char   *rmp;
char    ppx;
char    ppy;
char    dx;
char    dy;
char    r_owner;
char    r_dst;
long ptime;
int     lfd;
int     SCAB;
int     scabfd;
char    scabchar;
int     sfcount;
int     sfflag;
struct player  *pply;
struct alien   *palpt;
struct torps   *ptppt;
struct player  *splpnt;
struct sbase   *pbase;

char	visual[NPLAYER][NPLAYER];
