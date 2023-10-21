#include "st_def.h"

/*
**      Global Storage Allocations for Star-Drek
**      ======-=======-===========-===-====-====
**              (c) Peter S. Langston 1976
**
** Compile: cc -O -c -q st_glb.c
*/

#ifdef NOEMPTY
char    *glbwhat "@(#)st_glb.c	1.12  WITHOUT empty(), last mod 3/24/80 -- (c) psl 1976";
#else
char    *glbwhat "@(#)st_glb.c	1.12  with empty(), last mod 3/24/80 -- (c) psl 1976";
#endif
char    *glbh_sccs H_SCCS;

char    *st_mother  "psl";          /* logname of person responsible */
int     privuid     PRIVUID;        /* uid of priveledged user */
int     maxusers    MAXUSERS;       /* maximum # of starfleet members */

double  repair_time = 300.;         /* seconds per % of damage */

int     uid;                        /* uid of current user */
double	pos[NUM_OBJECTS][4], vel[NUM_OBJECTS][4], *p0, *v0, *dp;
int     active[NUM_OBJECTS], recnum, ttymod[4], oldmode, belsup;
double  energy, sensors, shields, range, etime, time0, ccomp;
double  uobl;
int     xrots, yrots, zrots;
double	damage[12], perdam[12], comarg;
char	*system[] {
	"damage control",	"di-lithium crystal",	"warp drive",
	"impulse drive",	"sensors",		"life support",
	"auto-pilot",		"bridge monitor",	"main phaser bank",
	"antimatter generator",	"tractor pressor",	"shield generators"
};
	int	damcntl 0,		reactor 1,		warpd 2;
	int	impulsd 3,		sensrs 4,		lifsup 5;
	int	autop 6,		vismon 7,		phaser 8;
	int	photorp 9,		tpgen 10,		shieldg 11;
int     enemy, kills, hcap, rebirth, lsup;
int     nearshp, neardst 32767, targ, badnubie, autopi, junk[40];
int     helm, recf, urecf, cumf, msgf;
struct	shapestr shape[] {
	'?', '?',           /* Human */
	'+', '?',           /* Nubian */
	'Y', '?',           /* Klingon */
	'#', '?',           /* Romulan */
	'X', '?',           /* Vallician */
	'@', '?',           /* Rigellian */
	'*', '*',           /* torpedo */
	'O', 'O',           /* Starbase */
	'.', '.',           /* star */
};
char    bodyfill    0177;               /* <DEL>, some my prefer ':' */

	       /* Note: Much of the display-dependant data is in st_viz.c */

int     o[MAX_O];                       /* used in dials & moninit */
char	*sorm, *title;
char    dispsbf[128];
int     old[25][11], new[25][11], dislen;

    /* Terminal specific stuff (this set up is for Interactive OWL) */
char    crtblon[]       { 026, '$', 0140, 0 };              /* start blink */
char    crtbloff[]      { 026, '$', 0040, 0 };               /* stop blink */
char    crtclr[]        { 014, 0 };                        /* clear screen */
char    crtrpt[]        { 013, 0 };         /* repeated a single character */
#ifndef NOCRTLDEL
char    crtldel[]       { 021, 0 };              /* close line & scroll up */
#endif
char    crtinit[]   {                    /* draw box around bridge monitor */
026,042,040+M_TOP-1,040+M_LEFT-1,040+M_HEIGHT+2,040+M_WIDTH+2,/* def window */
026,044,033,                                            /* top left corner */
013,040+M_WIDTH,026,044,003,                                   /* top line */
026,044,037,                                           /* top right corner */
026,042,040+M_TOP,040+M_LEFT-1,040+M_HEIGHT,040,     /* skinny left window */
013,040+M_HEIGHT,026,044,007,                                 /* left line */
026,041,                                           /* out of skinny window */
026,042,040+M_TOP,040+M_LEFT+M_WIDTH,040+M_HEIGHT,040, /* skinny right one */
013,040+M_HEIGHT,026,044,007,                                /* right line */
026,041,                                           /* out of skinny window */
017,040+M_TOP+M_HEIGHT,040,                      /* goto lower left corner */
026,044,027,                                          /* lower left corner */
013,040+M_WIDTH,026,044,003,                                /* bottom line */
026,044,043,                                         /* lower right corner */
026,042,040,040,040+23,040+79,             /* re-establish ordinary window */
017,040+M_TOP-1,040+M_MIDW,026,044,023,                   /* top tick mark */
017,040+M_MIDH,040+M_LEFT-1,026,044,047,                 /* left tick mark */
017,040+M_TOP+M_HEIGHT,040+M_MIDW,026,044,017,         /* bottom tick mark */
017,040+M_MIDH,040+M_LEFT+M_WIDTH,026,044,053,          /* right tick mark */
0       };

	/* mapping single characters into commands */
char	com_map[] {
     0,     0,     0,     0,     0,     0,     0,     0,        /*  0 - 7  */
cIGNOR,     0,cMXROT,     0,     0,     0,     0,     0,        /*  8 - 15 */
     0,     0,     0,     0,     0,     0,     0,     0,        /* 16 - 23 */
     0,     0,     0,     0,cPXROT,     0,cMYROT,cPYROT,        /* 24 - 31 */
     0,cSHELL,     0,     0,cSCORE,     0,     0,     0,        /* sp - '  */
     0,     0, cTORP,     0,     0,     0,     0,     0,        /*  ( - /  */
     0,     0,     0,     0,     0,     0,     0,     0,        /*  0 - 7  */
     0,     0,     0,     0,     0,     0,     0,     0,        /*  8 - ?  */
     0,     0,     0,     0,     0,cRFRSH,     0,     0,        /*  @ - G  */
     0, cTORP, cPHAZ,     0,     0, cTRAC,     0,     0,        /*  H - O  */
     0,     0,     0,     0,     0,     0,     0,     0,        /*  P - W  */
     0,     0,     0,     0,     0,     0,cRFRSH,     0,        /*  X - _  */
     0, cAUTO,     0,  cCCC, cDOCK,     0,cTHRST,     0,        /*  ` - g  */
     0,cISTOP,     0,     0,     0,     0,     0,     0,        /*  h - o  */
 cPHAZ, cSALV,  cDAM, cSHLD, cTRAC,     0,     0, cWARP,        /*  p - w  */
 cXROT, cYROT, cZROT,     0,     0,     0, cTEXT,  cSOS,        /*  x - del */

};

#ifdef  NOEMPTY
char    *hlm "st_tmp";              /* temp file used by helm proc */
char    *HELM_PROG "/sys/games/.../st/helm";           /*PATH*/
int     helmp;                      /* pid of helm proc */
#endif

#ifndef NOEMPTY
int     spcmod[3];
#endif

char    *comcp, *comce, *tty "/dev/tty?";
char    *cum "/sys/games/.../st/st_cum";               /*PATH*/
char    *records "/sys/games/.../st/st_rec";           /*PATH*/
char    *thank_you_letter "/sys/games/.../st/st_10q";  /*PATH*/

char    *MAIN_PROG "/sys/games/.../st/main";           /*PATH*/
char    *LOG_PROG  "/sys/games/.../st/log";            /*PATH*/

char    *msgfil    "/sys/games/.../st/st_message";     /*PATH*/
char    *infofil   "/sys/games/.../st/st_txt";         /*PATH*/

char    *shellprog "/bin/sh";                           /*PATH*/
char    *toprog    "/bin/to";                           /*PATH*/

    /* names of races */
char    *race[] {
	"Human",
	"Nubian", "Klingon", "Romulan",
	"Vallician", "Rigellian",
	"torpedo", "Starbase", "star",
};
    /* mapping between ship numbers and race */
char    rac[] {     /* note: things like S_BASE_NUM in st_def.h must agree */
	R_HUMAN,
	R_NUBIAN, R_NUBIAN, R_NUBIAN, R_NUBIAN, R_NUBIAN,
	R_KLINGON, R_KLINGON, R_KLINGON, R_KLINGON, R_KLINGON,
	R_ROMULAN, R_ROMULAN, R_ROMULAN, R_ROMULAN, R_ROMULAN,
	R_VALLICIAN, R_VALLICIAN, R_VALLICIAN, R_VALLICIAN, R_VALLICIAN,
	R_RIGELLIAN,
	R_TORPEDO, R_TORPEDO, R_TORPEDO, R_TORPEDO, R_TORPEDO,
	R_TORPEDO, R_TORPEDO, R_TORPEDO, R_TORPEDO, R_TORPEDO,
	R_TORPEDO, R_TORPEDO, R_TORPEDO, R_TORPEDO, R_TORPEDO,
	R_TORPEDO, R_TORPEDO, R_TORPEDO, R_TORPEDO, R_TORPEDO,
	R_STARBASE,
	R_STAR, R_STAR, R_STAR, R_STAR, R_STAR,
	R_STAR, R_STAR, R_STAR, R_STAR, R_STAR,
	R_STAR, R_STAR, R_STAR, R_STAR, R_STAR,
	R_STAR, R_STAR, R_STAR, R_STAR,
};

    /* ship characteristics for various races */
struct  scstr   sc[] {
/*   phas  torp  trac shld pvul tvul acc wrp  vis  attd   mass radius */
    100,  100,  100,  100, 100, 100, 50,100,  140, 999,    100, 30,  /* human */
      0,    0,    0,   50, 100, 100, 10,  0,  200,   0,    200, 50,  /* nube */
    100,   10,  100,  100,  75, 150, 20,  0,  140,  70,    100, 30,  /* kling */
     10,  160,    0,   40, 150,  75, 10,100,  100,  55,     50, 15,  /* rom */
     80,   40,   60,  200,  80, 120, 15,  0,  160,  80,     75, 20,  /* vall */
     80,   33,   33,  500,  10, 300, 20,  0,  160,  50,    300, 50,  /* rigel */
      0,    0,    0,    0, 100, 100,  0,  0,   50,   0,     10,  1,  /* torp */
    100,    0,    0, 1000,  80,  80,  0,  0,  600,   0,  10000,200,  /* s.b. */
      0,    0,    0,    0,   0,   0,  0,  0,  999,   0,  32767,100,  /* stars */
};

	/* NOTE: the following must agree in order with defines in st_def.h */
struct	dialstr	dialinfo[] {
/*      labx,y  label               valx,y  valfmt  */
	0,0,    "",                 0,0,    "",
	3,0,    "Mission Vector",   3,1,    "%4d,%4d,",
	0,0,    "",                 12,1,   ",%4d ",
	50,0,   "Energy",           50,1,   "%4d ",
	65,0,   "E. Time",          65,1,   "%5d ",
	3,3,    "Starbase Vector",  3,4,    "%4d,%4d,",
	0,3,    "",                 12,4,   ",%4d ",
	50,3,   "Vis Range",        50,4,   "%4d ",
	65,3,   "Sensors",          65,4,   "%5d ",
	3,6,    "Velocity Vector",  2,7,    "%4d,%4d,",
	0,6,    "",                 11,7,   ",%5.1f ",
	50,6,   "Speed",            50,7,   "%5.1f ",
	65,6,   "Condition",        65,7,   "%s%s%s ",
	3,9,    "Rotation Vector",  3,10,   "%4d,%4d,%4d ",
	/* the following are not permanently labelled */
	/* the label info is used for erasing the dial */
	52,9,   "                ", 52,9,   "%2d A.T.U. remain ",
	52,10,  "            ",     52,10,  "C.C.C. @%3.0f%% ",
	52,11,  "             ",    52,11,  "Shields @%3.0f%% ",
};

struct	record	rec;
struct	score	this;

	/* SYSTEM DEPENDENT ROUTINES */

int myruid()        /* return "real" user id */
{
	return(getuid() & 0377);
}

int myeuid()        /* return "effective" user id */
{
	return(getuid() >> 8 & 0377);
}
