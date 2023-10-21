/*
** init.c -    game initializations
**
**     [pm by Peter Costantinidis, Jr. @ University of California at Davis]
*/

#include "pm.h"

#if SYSV|SYSIII

# ifdef SYSIII
# include <sys/ioctl.h>
# endif SYSIII

# ifdef SYSV
WINDOW *cls;
# endif SYSV

struct termio tty;
unss   btmp = 0;               /* temporary variable for symbolic baud */
int    oldfl, baud = 0;
long   _tp;                    /* used for timing                      */
struct tbuffer {
       long utime;
       long stime;
       long cutime;
       long cstime;
} garbage;
#else
struct timeb   _tp;            /* used for timing                      */
#endif
coord  pm_pos;
long   thescore = 0L,          /* player's score                       */
       hi_score = 0L,          /* high score so far                    */
       move_cntr = 0L,         /* # of moves made by player            */
       chcnt = 0L,             /* character count                      */
       demon = 0L;             /* # of loops game made (psuedo time)   */
char   fr_ch,                  /* fruit character                      */
       ch = ' ',               /* current move of pm                   */
       oldch = '\0',           /* old (temporary) move                 */
       newch = '\0',           /* new move (future)                    */

#if !SYSV && !SYSIII
       baud = '\0',            /* output baud rate of terminal         */
#endif

       *argv0,                 /* argv[0]                              */
       *mesg;                  /* pointer to last message              */
int    timeit = FALSE,         /* printing loop/move counter?          */
       quiet = TRUE,           /* bells and whistles                   */
       fast = FALSE,           /* skip senseless looping               */
       timer = 0,              /* duration timer for energizers        */
       level = 0,              /* level (board) number                 */
       seed,                   /* rnd num seed                         */
       fr_val,                 /* fruit value                          */
       d_left = MAX_DOTS,      /* number of dots left on board         */
       e_left = MAX_ENERGY,    /* number of energizers left on board   */
       mons_eaten = -1,        /* number of monsters eaten (<= 4)      */
       pm_eaten = FALSE,       /* got eaten                            */
       pms_left = 3,           /* pm's left (you start with three)     */
       pm_bonus = TRUE,        /* can get a bonus pm                   */
       pm_run = TRUE,          /* TRUE if eatable                      */
       pm_tunn = FALSE,        /*  "   if in tunnel                    */
       pm_extunn,              /* how long left in tunnel              */
       is_wiz = FALSE,         /* TRUE if currently wizard             */
       was_wiz = FALSE,        /* TRUE if ever was wizard              */
       uid;                    /* user's uid                           */
mons   ghosts[MAX_MONS],       /* array of monsters                    */
       *h, *g, *c, *z;         /* pointers into array of monsters      */
char   fruit[] = "%&00++$$~~^^_",
       fruit_eaten[15] = "              ",
       moves[] = "hjkl";
int    fruit_val[] =   /* the values of each succeeding fruit */
{
       100, 300, 500, 500, 700, 700, 1000, 1000, 2000, 2000, 3000, 3000, 5000
};
int    mons_val[] =    /* the values of the monsters when eaten */
{
       200, 400, 800, 1600
};
int    eat_times[] =   /* the duration of the power pill */
{
       100, 95, 90, 75, 90, 85, 85, 85, 75, 70, 65, 55, 45
};

#if !SYSV && !SYSIII
int    bauds[] =
{
   0, 0, 0,  0,  0,  0,  0,  0,  0, 1200, 1800, 2400, 4800, 9600, 19200,   0
};
#endif

/*
** init() -    perform necessary intializations
*/
void   init ()
{
       if ((uid = getuid()) < 0)
       {
               fprintf(stderr, "Who the hell are you???\n");
               exit(1);
       }
       randomize(SEED);
       initslow();       /* set the time for the first call to slow */

       if (pm_user)
		if (chk_pm_user())      /* check user log file */
			fprintf(stderr, "%s: Can not make entry into user log file\n",
				argv0);

#if SYSV|SYSIII
       oldfl = fcntl(0, F_GETFL);
       fcntl(0, F_SETFL, O_NDELAY);
#endif

       hi_score = get_hi_scr();
       if (initscr() == (WINDOW *) ERR)
       {
               fprintf(stderr, "initscr() error\n");
               perror(argv0);

#if SYSV|SYSIII
              fcntl(0, F_SETFL, oldfl);
#endif

               exit(1);
       }
# ifdef SYSV
       cls = newwin(0, 0, 0, 0);
# endif SYSV
       if (!baud)      /* if baud, then we are trying to simulate another */
       {

#if SYSV|SYSIII
              if ((ioctl(fileno(stdout), TCGETA, &tty)) == -1) {
                       perror("pm: ");
                       endwin();
                       fcntl(0, F_SETFL, oldfl);
                       exit(1);
              }
              switch(btmp = (tty.c_cflag & CBAUD)) {
                      case B0:
                      case B50:
                      case B75:
                      case B110:
                      case B134:
                      case B150:
                      case B200:
                      case B300:
                      case B600:
                               fprintf(stderr, "pm: baud rate must be at least %d\n", MIN_BAUD);
                               endwin();
                               fcntl(0, F_SETFL, oldfl);
                               exit(1);
                      case B1200:
                               baud = 1200;
                               break;
                      case B1800:
                               baud = 1800;
                               break;
                      case B2400:
                               baud = 2400;
                               break;
                      case B4800:
                               baud = 4800;
                               break;
                      case B9600:
                               baud = 9600;
                               break;
              }

#else
               if (!bauds[_tty.sg_ospeed])
               {
                       fprintf(stderr, "pm: baud rate must be at least %d(%d)\n", MIN_BAUD, _tty.sg_ospeed);
                       endwin();
                       exit(1);
               }
               baud = _tty.sg_ospeed;
#endif

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
** mons_init() - initialize the monsters
**             - MUST be called before monsters()!!!
**     Note:   I am sure that there was a reason why I did not statically
**             initialize these structures.  When I remember the reason
**             I will mention it here at a later date.
*/
void   mons_init ()
{
       reg     int     i;

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
** m_init()    - initialize a single monster
**             - this function is called from p_monsters() (every time a
**               new screen is entered)
*/
m_init (m)
reg    mons    *m;
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
