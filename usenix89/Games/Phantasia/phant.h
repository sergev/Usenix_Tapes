/*
 * phant.h  Include file for Phantasia
 */

#include <setjmp.h>
#include <curses.h>
#include <time.h>
#include <pwd.h>
#include <signal.h>
#include <math.h>

/* ring constants */
#define NONE	0
#define NAZBAD	1
#define NAZREG	2
#define DLREG	3
#define DLBAD	4
#define SPOILED 5

/* some functions and pseudo-functions */
#define toupper(CH) ((CH) > 96 ? (CH) ^ 32 : (CH))  /* may be upper or lower */
#define tolower(CH) ((CH) | 32) 	/* must be upper */
#define rnd()	    (((double) rand()) / ((double) (RAND)))
#define roll(BASE,INTERVAL) floor((BASE) + (INTERVAL) * rnd())
#define sgn(x)	((x) < 0 ? -1 : ((x) > 0 ? 1 :0))
#define abs(x)	((x) < 0 ? -(x) : (x))
#define circ(x,y)   floor(dist(x, 0.0, y, 0.0) /125 + 1)
#define max(A,B)    ((A) > (B) ? (A) : (B))
#define min(A,B)    ((A) < (B) ? (A) : (B))
#define valarstuff(ARG) decree(ARG)
#define illcmd()    mvaddstr(5,0,"Illegal command.\n")
#define maxmove     (charac.lvl * 1.5 + 1)
#define illmove()   mvaddstr(5,0,"Too far.\n")
#define rndattack() if (rnd() < 0.2 && charac.status == PLAYING && !throne) \
		fight(&charac,-1)
#define strcalc(STR,SICK)   max(0,min(0.9 * STR, SICK * STR/20))
#define spdcalc(LVL,GLD,GEM)	max(0,((GLD + GEM/2) - 1000)/200.0 - LVL)
#define illspell()  mvaddstr(5,0,"Illegal spell.\n")
#define nomana()    mvaddstr(5,0,"Not enough mana for that spell.\n")
#define somebetter()	addstr("But you already have something better.\n")

/* status constants */
#define OFF 0
#define PLAYING 1
#define CLOAKED 2
#define INBATTLE    3
#define DIE 4
#define QUIT	5

/* tampered constants */
#define NRGVOID 1
#define GRAIL	2
#define TRANSPORT   3
#define GOLD	4
#define CURSED	5
#define MONSTER 6
#define BLESS	7
#define MOVED	8
#define HEAL	9
#define VAPORIZED   10
#define STOLEN	11

/* structure definitions */
struct	stats	    /* player stats */
    {
    char    name[21];	/* name */
    char    pswd[9];	/* password */
    char    login[10];	/* login */
    double  x;	    /* x coord */
    double  y;	    /* y coord */
    double  exp;	/* experience */
    int lvl;	    /* level */
    short   quk;	/* quick */
    double  str;	/* strength */
    double  sin;	/* sin */
    double  man;	/* mana */
    double  gld;	/* gold */
    double  nrg;	/* energy */
    double  mxn;	/* max. energy */
    double  mag;	/* magic level */
    double  brn;	/* brains */
    short   crn;	/* crowns */
    struct
	{
	short	type;
	short	duration;
	}   rng;    /* ring stuff */
    bool    pal;	/* palantir */
    double  psn;	/* poison */
    short   hw;     /* holy water */
    short   amu;	/* amulets */
    bool    bls;	/* blessing */
    short   chm;	/* charms */
    double  gem;	/* gems */
    short   quks;	/* quicksilver */
    double  swd;	/* sword */
    double  shd;	/* shield */
    short   typ;	/* character type */
    bool    vrg;	/* virgin */
    short   lastused;	/* day of year last used */
    short   status;	/* playing, cloaked, etc. */
    short   tampered;	/* decree'd, etc. flag */
    double  scratch1, scratch2; /* var's for above */
    bool    blind;	/* blindness */
    int wormhole;   /* # of wormhole, 0 = none */
    long    age;	/* age in seconds */
    short   degen;	/* age/2500 last degenerated */
    short   istat;	/* used for inter-terminal battle */
    };

struct	mstats	    /* monster stats */
    {
    char    name[26];	/* name */
    double  str;	/* strength */
    double  brn;	/* brains */
    double  spd;	/* speed */
    double  hit;	/* hits (energy) */
    double  exp;	/* experience */
    int trs;	    /* treasure type */
    int typ;	    /* special type */
    int flk;	    /* % flock */
    };

struct	nrgvoid     /* energy void */
    {
    bool    active;	/* active or not */
    double  x,y;	/* coordinates */
    };

#ifndef SMALL
struct	worm_hole   /* worm hole */
    {
    char    f, b, l, r; /* forward, back, left, right */
    };
#endif

struct	sb_ent			/* scoreboard entry */
    {
    char    name[21];
    char    login[10];
    int     level;
    short   type;
    };

/* files */
extern char monsterfile[],
	    peoplefile[],
	    gameprog[],
	    messfile[],
	    lastdead[],
	    helpfile[],
	    motd[],
	    goldfile[],
	    voidfile[],
	    enemyfile[],
	    sbfile[];

/* library functions and system calls */
unsigned sleep();
long	time(), ftell();
char	*getlogin(), *getpass(), *ctime(), *strchr();
struct	passwd	*getpwuid();
char	*strcpy(), *strncpy();

/* function and global variable declarations */
void	adjuststats(), callmonster(), checkinterm(), checkmov(), checktampered(),
    cstat(), death(), decree(), error(), exit1(), fight(), genchar(), getstring(),
    init1(), initchar(), interm(), kingstuff(), leave(), mesg(), movelvl(),
    neatstuff(), paws(), printmonster(), printplayers(), printstats(), purge(),
    scoreboard(), scramble(), showall(), show_sb(), showusers(), statread(),
    tampered(), titlestuff(), trade(), treasure(), trunc(), update(), voidupdate();
int allocvoid(), findchar(), findname(),  findspace(), getans(), gch(), interrupt(),
    ill_sig(), rngcalc(), level();
double	dist(), inflt();
char	*printloc();
#ifdef	OK_TO_PLAY
bool	ok_to_play();
#endif

extern	jmp_buf fightenv, mainenv;
extern	double	strength, speed;
extern	bool	beyond, marsh, throne, valhala, changed, fghting, su, wmhl, timeout;
#ifndef SMALL
extern	struct worm_hole    w_h[];
#endif
extern	long	secs;
extern	int fileloc, users;
