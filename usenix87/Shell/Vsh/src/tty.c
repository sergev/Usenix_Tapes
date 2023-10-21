#include "hd.h"
#ifdef	PWBTTY
#include <sys/sgtty.h>
#endif
#ifdef	V7TTY
#include <sgtty.h>
#endif
#ifdef	USGTTY
#include <termio.h>
#endif

/* This module controls tty options.

   Tty states are treated as a stack.  Use tty_push and tty_pop.

   Push COOKEDMODE and RAWMODE onto the stack.  RAWMODE is now actually
   CBREAK mode.
*/

#ifdef	PWBTTY
struct sgtty newstty, oldstty;	/* sgtty structures */
#endif
#ifdef	V7TTY
struct sgttyb newstty, oldstty;	/* sgtty structures */
#endif
#ifdef	USGTTY
struct termio newstty, oldstty; /* termio structure */
#endif

int ttysp;	/* Sp for tty stack */
extern short ospeed;	/* Speed for Joy's routines */

#define	TTYLIM	16
char tty_stack [TTYLIM];	/* The tty stack */

/* Initialize tty structures */

tty_init () 
{
#ifdef	USGTTY
	ioctl(infile, TCGETA, &oldstty);
#else
    gtty (infile, &oldstty);
#endif
	newstty = oldstty;
#ifdef	PWBTTY
    oldstty.sg_flag &= ~RAW;
    newstty.sg_flag |= CBREAK;
    newstty.sg_flag &= ~(CRMOD|ECHO);
    ospeed = oldstty.sg_ospd;
#endif
#ifdef	V7TTY
    oldstty.sg_flags &= ~(CBREAK|RAW);
    newstty.sg_flags |= CBREAK;
    newstty.sg_flags &= ~(CRMOD|ECHO);
    ospeed = oldstty.sg_ospeed;
#endif
#ifdef	USGTTY
	oldstty.c_lflag |= ICANON;
	newstty.c_lflag &= ~(ICANON|ECHO);
	newstty.c_iflag &= ~ICRNL;
	newstty.c_cc[VMIN] = 1;
	newstty.c_cc[VTIME] = 1;
	ospeed = oldstty.c_cflag&CBAUD;
#endif

    ttysp = 0; tty_set (COOKEDMODE);
}

/* Push a new tty mode.  Use RAWMODE and COOKEDMODE as parameters. */

tty_push (pmode) int pmode; 
{
    ttysp++;
    if (ttysp >= TTYLIM) 
    {
	putmsg ("Tty stack overflow\r\n");
	leave ();
    }
    tty_set (pmode);
}

tty_pop () 
{
    ttysp--;
    if (ttysp < 0) 
    {
	putmsg ("Tty stack underflow\r\n");
	leave ();
    }
    tty_set (tty_stack [ttysp]);
}

/* Tty_set sets the tty mode indicated by its parameter.  Stty is only
   executed if a change will result.
*/

tty_set (pmode) int pmode; 
{

    static int curmode = -1;	/* Current tty mode */

    tty_stack [ttysp] = pmode;
    if (pmode == curmode) return;
    curmode = pmode;
    if (pmode == RAWMODE) tty_raw ();
    else tty_cooked ();
}

tty_raw () 
{
#ifdef	USGTTY
	ioctl(infile, TCSETAF, &newstty);
#else
    stty (infile, &newstty); 
#endif
}

tty_cooked () 
{
#ifdef	USGTTY
	ioctl(infile, TCSETAF, &oldstty);
#else
    stty (infile, &oldstty); 
#endif
}
