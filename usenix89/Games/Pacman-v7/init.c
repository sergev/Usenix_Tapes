/*
** init.c -	game initializations
**
**	[pm by Peter Costantinidis, Jr. @ University of California at Davis]
*/
#include "pm.h"

coord	pm_pos;
struct	timeb	_tp;		/* used for timing			*/
long	thescore = 0L,		/* player's score			*/
	hi_score = 0L, 		/* high score so far			*/
	move_cntr = 0L,		/* # of moves made by player		*/
	chcnt = 0L, 		/* character count			*/
	demon = 0L;		/* # of loops game made (psuedo time)	*/
char	fr_ch,			/* fruit character			*/
	ch = ' ',		/* current move of pm			*/
	oldch = '\0',		/* old (temporary) move			*/
	newch = '\0',		/* new move (future)			*/
	baud = '\0',		/* output baud rate of terminal		*/
	*argv0,			/* argv[0]				*/
	*mesg;			/* pointer to last message		*/
int	timeit = FALSE,		/* printing loop/move counter?		*/
	quiet = TRUE,		/* bells and whistles			*/
	fast = FALSE,		/* skip senseless looping		*/
	timer = 0,		/* duration timer for energizers	*/
	level = 0,		/* level (board) number			*/
	seed,			/* rnd num seed				*/
	fr_val,			/* fruit value 				*/
	d_left = MAX_DOTS,	/* number of dots left on board		*/
	e_left = MAX_ENERGY,	/* number of energizers left on board	*/
	mons_eaten = -1,	/* number of monsters eaten (<= 4)	*/
	pm_eaten = FALSE,	/* got eaten				*/
	pms_left = 3,		/* pm's left (you start with three)	*/
	pm_bonus = TRUE,	/* can get a bonus pm			*/
	pm_run = TRUE,		/* TRUE if eatable			*/
	pm_tunn = FALSE,	/*  "   if in tunnel			*/
	pm_extunn,		/* how long left in tunnel		*/
	is_wiz = FALSE,		/* TRUE if currently wizard		*/
	was_wiz = FALSE,	/* TRUE if ever was wizard		*/
	uid;			/* user's uid				*/
mons	ghosts[MAX_MONS],	/* array of monsters			*/
	*h, *g, *c, *z;		/* pointers into array of monsters	*/
char	fruit[] = "%&00++$$~~^^_",
	fruit_eaten[15] = "              ",
	moves[] = "hjkl";
int	fruit_val[] =	/* the values of each succeeding fruit */
{
	100, 300, 500, 500, 700, 700, 1000, 1000, 2000, 2000, 3000, 3000, 5000
};
int	mons_val[] =	/* the values of the monsters when eaten */
{
	200, 400, 800, 1600
};
int	eat_times[] =	/* the duration of the power pill */
{
	130, 130, 120, 115, 110, 100, 95, 85, 75, 70, 65, 55, 45
};
int	bauds[] =
{
/* 0  1  2   3   4   5   6   7   8     9    10    11    12    13     14   15*/
/* 0 50 75 110 134 150 200 300 600  1200  1800  2400  4800  9600   EXTA EXTB*/
   0, 0, 0,  0,  0,  0,  0,  0,  0, 1200, 1800, 2400, 4800, 9600, 19200,   0
};

/*
** init() -	perform necessary intializations
*/
void	init ()
{
	if ((uid = getuid()) < 0)
	{
		fprintf(stderr, "Who the hell are you???\n");
		exit(1);
	}
	randomize(SEED);
#ifdef	PM_USER
	if (chk_pm_user())	/* check user log file */
		fprintf(stderr, "%s: Can not make entry into user log file\n",
			argv0);
#endif
	hi_score = get_hi_scr();
	if (initscr() == ERR)
	{
		fprintf(stderr, "initscr() error\n");
		perror(argv0);
		exit(1);
	}
	if (!baud)	/* if baud, then we are trying to simulate another */
	{
		if (!bauds[_tty.sg_ospeed])
		{
			fprintf(stderr, "pm: baud rate must be at least %d(%d)\n", MIN_BAUD, _tty.sg_ospeed);
			endwin();
			exit(1);
		}
		baud = _tty.sg_ospeed;
	}
	trap(0);
	h = &ghosts[0];
	g = &ghosts[1];
	c = &ghosts[2];
	z = &ghosts[3];
	mons_init();
	crmode();
	noecho();
	mesg = NULL;
	draw_screen();
}

/*
** mons_init()	- initialize the monsters
**		- MUST be called before monsters()!!!
**	Note:	I am sure that there was a reason why I did not statically
**		initialize these structures.  When I remember the reason
**		I will mention it here at a later date.
*/
void	mons_init ()
{
	reg	int	i;

	h->mo_attrib = SMART | SLOW;
	g->mo_attrib = SMART | FAST;
	c->mo_attrib = NORMAL | FAST;
	z->mo_attrib = DUMB | MED;
	h->mo_name = HARPO;
	g->mo_name = GROUCHO;
	c->mo_name = CHICO;
	z->mo_name = ZEPPO;
	for (i = 0; i < 4; i++)
		m_init(&ghosts[i]);
	g->mo_inside = FALSE;
}

/*
** m_init()	- initialize a single monster
**		- this function is called from p_monsters() (every time a
**		  new screen is entered)
*/
m_init (m)
reg	mons	*m;
{
	m->mo_inch = EMPTY;
	m->mo_run = FALSE;
	m->mo_tunn = FALSE;
	m->mo_eaten = FALSE;
	m->mo_inside = TRUE;
	m->mo_ch = ' ';
	m->mo_cnt = 0;
	m->mo_extunn = 0;
}
