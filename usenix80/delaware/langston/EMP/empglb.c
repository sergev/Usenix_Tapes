#
/*********************************************************
	      Global definitions for Empire

Compile: cc -O -c -q empglb.c

Copyright (c) 1976 by Peter S. Langston - Camb., Ma. 02140
*********************************************************/

#define D_NATSTR
#define D_SCTSTR
#define D_SHPSTR
#define D_COMSTR
#define D_DCHRSTR
#define D_ICHRSTR
#define D_NWSSTR
#define D_RPTSTR
#define D_MCHRSTR
#define D_TELSTR
#define D_TRTSTR
#define D_TCHRSTR
#define D_LONSTR
#define D_NSCSTR
#define D_TRTYCLAUSE
#define D_NEWSVERBS
#include "empdef.h"

char    *emprog[] {                                 /* empire module names */
	"/sys/games/.../emp/empire",             /* main entry into empire */
	"/sys/games/.../emp/BIN/emp1",
	"/sys/games/.../emp/BIN/emp2",
	"/sys/games/.../emp/BIN/emp3",
	"/sys/games/.../emp/BIN/emp4",
	"/sys/games/.../emp/BIN/emp5",
	"/sys/games/.../emp/BIN/emp6",
	"/sys/games/.../emp/BIN/emp7",
};
char    *empsrc "/sys/games/.../emp";             /* location of "sources" */
char    *empfix "/sys/games/.../emp/BIN/empfix";     /* name of file fixer */
char    *upfil "/sys/games/.../emp/DATA/empup";            /* enables play */
char    *downfil "/sys/games/.../emp/DATA/empdown";      /* prohibits play */

		/* data files (with global file handle variables) */
char    *sectfil "/sys/games/.../emp/DATA/empsect", sectf;
char    *natfil "/sys/games/.../emp/DATA/empnat", natf;
char    *newsfil "/sys/games/.../emp/DATA/empnews", newsf;
char    *shipfil "/sys/games/.../emp/DATA/empship", shipf;
char	*telfil "/sys/games/.../emp/DATA/emptel", telf;
char    *powfil "/sys/games/.../emp/DATA/emppow", powf;
char    *treatfil "/sys/games/.../emp/DATA/emptreat", trtf;
char    *loanfil "/sys/games/.../emp/DATA/emploan", loanf;
char    *infodir "/sys/games/.../emp/INFO", infof;/* where info topics live */
char    *nroffhd "/sys/games/.../emp/INFO/INFO.MAC"; /* std macros for crt */
char    *nroffil "/bin/nroff";             /* where nroff lives (for info) */
char    *shllrg1 "-";                 /* first optional argument for shell */
char    *shllrg2 0;                  /* second optional argument for shell */
char    *shllrg3 0;                   /* third optional argument for shell */

/* parametric goodies */
int     privuid 3;                                     /* uid of priv user */
char    *privname "Peter Langston";                   /* name of priv user */
char    *privlog "psl";                            /* logname of priv user */
int     w_xsize = 64;                     /* world x size, (width), <= 128 */
int     w_ysize = 32;                    /* world y size, (height), <= 128 */
int     l2w_xsize   = 6;                     /* log base 2 of world x size */
#define MAXNOC  8                 /* must not exceed MAX_MAXNOC (empdef.h) */
int     m_m_p_d 60;              /* max mins of play per day (per country) */

int     maxnoc MAXNOC;                             /* max num of countries */
int     maxcno MAXNOC-1;                            /* largest country num */
char	junk[80], combuf[80], *argp[16], *condarg;
int     sx, sy, lx, hx, ly, hy, ix, iy;
int	wxh, wyh, wxl, wyl, capx, capy, nbrx, nbry;
int	capxof[MAXNOC], capyof[MAXNOC];
int	cnum, curup, ntime, nstat, ncomstat, nminused, ntused;
int	broke;
int	owner, proto, redirin;
int	sigaddr[4], ttymod[3];
int	dn[][2] { 0,1,0,-1,1,0,-1,0 };

double  up_offset;                    /* offset for update calcs (1/2 hrs) */
double	lasttime, dolcost;

extern	int fmtlen;
extern	struct	comstr	coms[];

struct  boundstr    nrealm[4];
struct  sctstr      sect;
struct  natstr      nat;
struct  nwsstr      nws;
struct  shpstr      ship, as, vs;
struct  telstr      tgm;
struct  trtstr      trty;
struct  lonstr      loan;

char	nulls[64];

/*	efficiency adverbs -- vaguely describe efficiency */
char	*effadv[] { "minimally", "partially", "moderately", "completely" };

char    *numnames[NUMNUMNAMES] {
	"zero", "one", "two", "three", "four", "five", "six",
	"seven", "eight", "nine", "ten", "eleven", "twelve",
	"thirteen", "fourteen", "fifteen", "sixteen",
	"seventeen", "eighteen", "nineteen", "twenty",
};

/*    designation characteristics -- one entry per designation type
*/
#define GUN     &0->sct_guns               /* pointers to sctstr & ichrstr */
#define	SHL	&0->sct_shell
#define	ORE	&0->sct_ore
#define	GOR	&0->sct_gold
#define	PLN	&0->sct_plane
#define TEC     25                  /* must agree with ichr initialization */
#define RES     26                                                /* ditto */
struct	dchrstr dchr[] {
/*      mnem ptyp mcst  pkg ostr dstr value name    */
	'.',   0,   0,   0,   0,   0,   0, "sea",
	'^',   0, 255,   0,   2,  16,  10, "mountain",
	's',   0,   2,   0,   0,  99, 127, "sanctuary",
	'-',   0,   3,   0,   2,   2,   5, "wilderness",
	'c',   0,   2,   0,   2,   4, 127, "capital",
	'u',   0,   2,   2,   2,   2,  10, "urban area",
	'd', GUN,   2,   0,   2,   2,  10, "defense plant",
	'i', SHL,   2,   0,   2,   2,  10, "shell industry",
	'm', ORE,   2,   0,   2,   2,  20, "mine",
	'g', GOR,   2,   0,   2,   2,  20, "gold mine",
	'h',   0,   2,   0,   2,   2,  20, "harbor",
	'w',   0,   2,   1,   2,   2,  10, "warehouse",
	'*', PLN,   2,   0,   2,   2,  10, "airfield",
	'a',   0,   2,   0,   2,   2,  10, "agribusiness",
	't', TEC,   2,   0,   2,   2,   0, "technical center",
	'f',   0,   2,   0,   4,   8,  20, "fortress",
	'r', RES,   2,   0,   2,   2,  10, "research lab",
	'+',   0,   1,   0,   2,   2,  10, "highway",
	')',   0,   2,   0,   2,   2,  20, "radar installation",
	'!',   0,   2,   0,   2,   2,  10, "weather station",
	'#',   0,   2,   0,   2,   2,  15, "bridge head",
	'=',   0,   1,   0,   1,   1,  15, "bridge span",
	'b',   0,   2,   3,   2,   2,  10, "bank",
	'x',   0,   2,   0,   2,   2,  10, "exchange",
	0,     0,   0,   0,   0,   0,   0, 0,
};

/*	item characteristics -- one entry per sector item
*/
struct	ichrstr	ichr[] {
/*mnem prdct      del        shp         mch   bid value lbs pkg{rg,wh,ur,bnk  name
*/  0,   0,         0,     &0->shp_own,    0,    0,   0,  0, { 1, 1, 1, 1},       "owner",
    0,   0,         0,     &0->shp_type,   0,    0,   0,  0, { 1, 1, 1, 1},       "designation",
    0,   0,         0,     &0->shp_effc,   0,    0,   0,  0, { 1, 1, 1, 1},       "efficiency",
    0,   0,         0,         0,          0,    0,   1,  0, { 1, 1, 1, 1},       "minerals",
    0,   0,         0,         0,          0,    0,   1,  0, { 1, 1, 1, 1},       "gold mineral",
    0,   0,         0,     &0->shp_mbl,    0,    0,   1,  0, { 1, 1, 1, 1},       "mobility",
    0,   0,         0,         0,          0,    0,   1,  0, { 1, 1, 1, 1},       "production",
    0,   0,         0,         0,          0,    0,   0,  0, { 1, 1, 1, 1},       "checkpoint",
    0,   0,         0,         0,          0,    0,   0,  0, { 1, 1, 1, 1},       "defended",
    0,   0,         0,         0,          0,    0,   0,  0, { 1, 1, 1, 1},       "contracted",
  'c',   0, &0->sct_c_use, &0->shp_crew,&0->m_civil, 0, 1, 1,{ 1, 1,10, 1},       "civilians",
  'm',   0, &0->sct_m_use, &0->shp_crew,&0->m_milit, 0, 1, 1,{ 1, 1, 1, 1},       "military",
  's',   2, &0->sct_s_use,&0->shp_shels,&0->m_shels, 3, 2, 2,{ 1,10, 1, 1},       "shells",
  'g',  10, &0->sct_g_use,  &0->shp_gun,  &0->m_gun, 3,10,10,{ 1,10, 1, 1},       "guns",
  'p',  25, &0->sct_p_use, &0->shp_plns, &0->m_plns, 3,25,20,{ 1, 1, 1, 1},       "planes",
  'o',   0, &0->sct_o_use,   &0->shp_or,   &0->m_or, 2, 1, 1,{ 1,10, 1, 1},       "ore",
  'b',   5, &0->sct_b_use,  &0->shp_gld, &0->m_gld,33,100,50,{ 1, 1, 1, 4},       "bars of gold",
    0,   0,         0,         0,          0,    0,   0,  0, { 0, 0, 0, 0},       "<filler>",
    0,   0,         0,         0,          0,    0,   0,  0, { 1, 1, 1, 1},       "c_delivery",
    0,   0,         0,         0,          0,    0,   0,  0, { 1, 1, 1, 1},       "m_delivery",
    0,   0,         0,         0,          0,    0,   0,  0, { 1, 1, 1, 1},       "s_delivery",
    0,   0,         0,         0,          0,    0,   0,  0, { 1, 1, 1, 1},       "g_delivery",
    0,   0,         0,         0,          0,    0,   0,  0, { 1, 1, 1, 1},       "p_delivery",
    0,   0,         0,         0,          0,    0,   0,  0, { 1, 1, 1, 1},       "o_delivery",
    0,   0,         0,         0,          0,    0,   0,  0, { 1, 1, 1, 1},       "b_delivery",
    0,  25,         0,         0,          0,    3,  25,  0, { 0, 0, 0, 0},       "technological breakthroughs",
    0,  25,         0,         0,          0,    3,  25,  0, { 0, 0, 0, 0},       "medical discoveries",
    0,   0,         0,         0,          0,    0,   0,  0, { 0, 0, 0, 0},       0,
};

/*	marine characteristics -- one entry per ship type */
struct	mchrstr	mchr[] {
/* prdct spd vis vrng frng  civ  mil  sh  gun plns  ore gold  name
*/  30,  50, 10,   4,  1,   0,  10,  10,   1,   0,   0,   0,  "pt boat",
    50,  20, 20,   3,  1,   0,  25,  10,   1,   0,   0,   0,  "minesweep",
    60,  35, 15,   4,  3,   0,  80,  40,   2,   0,   0,   0,  "destroyer",
    70,  25,  2,   3,  2,   0,  25,  25,   2,   0,   0,   0,  "submarine",
    80,  20, 20,   3,  0, 127,   0, 127, 127,   0, 127, 127,  "freighter",
   100,  30, 20,   3,  1,   0, 100, 127,  30,   0,   0,   0,  "tender",
   127,  25, 25,   6,  8,   0, 127, 127,   4,   0,   0,   0,  "battleship",
   127,  25, 25,   4,  2,   0,  60,  40,   2, 127,   0,   0,  "carrier",
     0,   0,  0,   0,  0,   0,   0,   0,   0,   0,   0,   0,  0,
};

/*   treaty clause characteristics -- one entry per possible treaty clause */
struct	tchrstr	tchr[] {
	SEAATT,	"no attacks on ships",
	SEAFIR,	"no shelling ships",
	LANATT,	"no sector attacks",
	LANFIR,	"no shelling land",
	TRTSPY,	"no peeking",
	TRTENL,	"no enlistments",
	TRTRAD,	"no radar",
	TRTBUI,	"no building"
};

/*	report -- one entry per type of news item */
struct	rptstr rpt[] {
/*  nice    page    text
*/  0,      3,      "did nothing in particular to %s",
    -3,     1,      "won a sector from %s",
    -3,     1,      "was repulsed by %s",
    -1,     2,      "had a spy shot by %s",
    1,      2,      "sent a telegram to %s",
    3,      3,      "signed a treaty with %s",
    2,      3,      "made a loan to %s",
    1,      3,      "repaid a loan from %s",
    0,      3,      "made a sale to %s",
    2,      2,      "granted land to %s",
    -2,     1,      "Shelled land owned by %s",
    -2,     1,      "Shelled a ship owned by %s",
    0,      2,      "took over unoccupied land",
    0,      1,      "had a ship torpedoed",
    0,      1,      "fired on %s in self-defense",
    0,      2,      "broke sanctuary",
    -2,     1,      "bombed one of %s's sectors",
    -2,     1,      "bombed a ship flying the flag of %s",
    -2,     1,      "boarded a(n) %s ship",
    -3,     1,      "was repelled by %s while attempting to board a ship",
    -1,     1,      "fired on %s aircraft",
    -2,     3,      "seized a sector from %s in collecting on a loan",
    -1,     2,      "considered an action which would have violated a treaty with %s",
    -4,     1,      "violated a treaty with %s",
    0,      1,      "dissolved its government",
    0,      1,      "ship hit a mine",
    5,      1,      "announced an alliance with %s",
    0,      1,      "declared their neutrality toward %s",
    -5,     1,      "declared WAR on %s",
    -5,     1,      "disavowed former alliance with %s",
    5,      1,      "disavowed former war with %s",
    0,      2,      "sector damaged by hurricane",
    0,      2,      "navy sufferred hurricane damages",
    0,      1,      "reports outbreak of PLAGUE",
    0,      1,      "citizens killed by PLAGUE",
    0,      2,      "went through a name change",
    0,      0,      0
};

int	n_max_verb N_MAX_VERB;

struct	nscstr	ns_cond[8];

