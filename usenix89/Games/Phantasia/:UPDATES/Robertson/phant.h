/*
 * phant.h  Include file for Phantasia
 */

#include <setjmp.h>
#include <curses.h>
#include <time.h>
#include <pwd.h>
#include <signal.h>
#include <math.h>

#ifdef NOVOID
#define	void	int
#endif

/* ring constants */

#define NONE	0
#define NAZBAD	1
#define NAZREG	2
#define DLREG	3
#define DLBAD	4
#define SPOILED 5

/* some functions and pseudo-functions */

#define toupper(CH) 			((CH) > 96 ? (CH) ^ 32 : (CH)) /* upper/lower */
#define tolower(CH)				((CH) | 32)						/* upper only */
#ifndef SMALL
#define rnd()			    	((rand () % 1000)/1000.0)
#endif
#define roll(BASE,INTERVAL)		floor((BASE) + (INTERVAL) * rnd())
#define sgn(x)					((x) < 0 ? -1 : ((x) > 0 ? 1 :0))
#define abs(x)					((x) < 0 ? -(x) : (x))
#define circ(x,y)				floor(dist(x, 0.0, y, 0.0) /125 + 1)
#define max(A,B)				((A) > (B) ? (A) : (B))
#define min(A,B)				((A) < (B) ? (A) : (B))
#define valarstuff(ARG)			decree(ARG)
#define maxmove					(charac.lvl * 1.5 + 1)
#ifndef SMALL
#define illcmd()				mvaddstr(5,0,"Illegal command.\n")
#define illmove()				mvaddstr(5,0,"Too far.\n")
#define nomana()			mvaddstr(5,0,"Not enough mana for that spell.\n")
#define somebetter()		addstr("But you already have something better.\n")
#define illspell()				mvaddstr(5,0,"Illegal spell.\n")
#endif
#define randattack()			if ((float) rnd() < 0.2 && charac.status \
								== PLAYING && !throne)  fight(&charac,-1)
#define strcalc(STR,SICK)		max(0,min(0.9 * STR, SICK * STR/20))
#define spdcalc(LVL,GLD,GEM)	max(0,((GLD + GEM/2) - 1000)/200.0 - LVL)

/* status constants */

#define OFF 		0
#define PLAYING 	1
#define CLOAKED 	2
#define INBATTLE    3
#define DIE 		4
#define QUIT		5

/* tampered constants */

#define NRGVOID 	1
#define GRAIL		2
#define TRANSPORT   3
#define GOLD		4
#define CURSED		5
#define MONSTER 	6
#define BLESS		7
#define MOVED		8
#define HEAL		9
#define VAPORIZED   10
#define STOLEN		11

/* structure definitions */

struct	stats	    	/* player stats */
    {
    char    name[21];	/* name */
    char    pswd[9];	/* password */
    char    login[10];	/* login */
    double  x;	    	/* x coord */
    double  y;	    	/* y coord */
    double  exp;		/* experience */
    int 	lvl;    	/* level */
    short   quk;		/* quick */
    double  str;		/* strength */
    double  sin;		/* sin */
    double  man;		/* mana */
    double  gld;		/* gold */
    double  energy;		/* energy */
    double  mxn;		/* max. energy */
    double  mag;		/* magic level */
    double  brn;		/* brains */
    short   crn;		/* crowns */
    struct
	{
		short	type;
		short	duration;
	}   rng;    		/* ring stuff */
    bool    pal;		/* palantir */
    double  psn;		/* poison */
    short   hw;    	 	/* holy water */
    short   amu;		/* amulets */
    bool    bls;		/* blessing */
    short   chm;		/* charms */
    double  gem;		/* gems */
    short   quks;		/* quicksilver */
    double  swd;		/* sword */
    double  shd;		/* shield */
    short   typ;		/* character type */
    bool    vrg;		/* virgin */
    short   lastused;	/* day of year last used */
    short   status;		/* playing, cloaked, etc. */
    short   tampered;	/* decree'd, etc. flag */
    double  scratch1, scratch2; /* var's for above */
    bool    blind;		/* blindness */
    int		wormhole;  	/* # of wormhole, 0 = none */
    long    age;		/* age in seconds */
    short   degen;		/* age/2500 last degenerated */
    short   istat;		/* used for inter-terminal battle */
	short	lives;		/* # of times resurrected -- max 3 lives unless Valar */
};

struct	mstats	    	/* monster stats */
{
    char    mname[20];	/* name */
    double  mstr;		/* strength */
    double  mbrn;		/* brains */
    double  mspd;		/* speed */
    double  mhit;		/* hits (energy) */
    double  mexp;		/* experience */
    int		mtrs;    	/* treasure type */
    int		mtyp;    	/* special type */
    int		mflk;    	/* % flock */
};

struct	energyvoid     	/* energy void */
{
    bool    active;		/* active or not */
    double  v_x,v_y;	/* coordinates */
};

#ifdef WORM
struct	worm_hole   	/* worm hole */
{
    char    f, b, l, r; /* forward, back, left, right */
};
#endif

struct	sb_ent			/* scoreboard entry */
{
    char    s_name[21];
    char    s_login[10];
    int     s_level;
    short   s_type;
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

unsigned	sleep();
long		time(), ftell();
char		*getlogin(), *getpass(), *ctime(), *strchr();
struct		passwd	*getpwuid();
char		*strcpy(), *strncpy();

/* function and global variable declarations */

void	adjuststats(), callmonster(), checkinterm(), checkmov(), 
		checktampered(), cstat(), death(), decree(), error(), exit1(), fight(),
		genchar(), getstring(), init1(), initchar(), interm(), kingstuff(),
		leave(), mesg(), movelvl(), neatstuff(), more(), printmonster(),
		printplayers(), printstats(), purge(), shellcmd (), scoreboard(),
		scramble(), showall(), show_sb(), showusers(), statread(), tampered(),
		titlestuff(), trade(), treasure(), trunc(), update(), voidupdate();

int 	allocvoid(), findchar(), findname(),  findspace(), getans(), gch(),
		interrupt(), ill_sig(), rngcalc(), level();

double	dist(), inflt();
char	*printloc(), *ptype();

#ifdef	OK_TO_PLAY
bool	ok_to_play();
#endif

extern	jmp_buf fightenv, mainenv;
extern	double	strength, speed;
extern	bool	beyond, marsh, throne, valhala, changed, fghting, su, wmhl,
		inshell, timeout;

#ifdef WORM
extern	struct worm_hole    w_h[];
#endif

extern	long	secs;
extern	int 	fileloc, users;
extern	FILE	*read_pfile, *access_pfile;		/* pointers to peoplefile */
extern	FILE	*mfile;							/* pointer to monsterfile */

#ifdef SMALL
double 	rnd();
void 	illcmd(), illmove(), nomana(), somebetter(), illspell();
#endif
