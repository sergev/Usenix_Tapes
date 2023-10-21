/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"
# include <sgtty.h>
# include <signal.h>

/* LINTLIBRARY */

/**********************************************************************\
|   Ctrl library.   ctrlinit should be called before any of the        |
| other functions is called. it returnes 0 for error, 1 when           |
| everything is ok.  FUNCTIONS :                                       |
|                       ctrlinit        initialize                     |
|                       ctrlreset       reset tty state                |
|                       echo            select echo mode               |
|                       noecho          select noecho mode             |
|                       crmode          select cbreak mode             |
|                       nocrmode        select no cbreak mode          |
|                       raw             select raw mode                |
|                       noraw           select no raw mode             |
|                       dogetstr        get a string in raw | cbreak   |
|                                                                      |
|       All function calls work on the current player's terminal       |
|                                                                      |
\**********************************************************************/


struct sgttyb   oldsg[2];
struct sgttyb   newsg[2];
struct ltchars  oldch[2];
struct ltchars  newltchars = {
         -1, -1, -1, -1, -1, -1
 };
struct tchars   oldtch[2] ;
struct tchars   newtchars = {
        -1, -1, -1, -1, -1, -1
};

ctrlinit () {
    if (gtty (fileno (tty), &oldsg[player]) == -1)
        return (0);
    newsg[player] = oldsg[player];
    noecho ();
    setchrs ();
    return (1);
}

ctrlreset () {
    newsg[player] = oldsg[player];
    (void) stty (fileno (tty), &newsg[player]);
    ioctl (fileno (tty), TIOCSLTC, &oldch[player]);
    ioctl (fileno (tty), TIOCSETC, &oldtch[player]);
}

echo () {
    newsg[player].sg_flags |= ECHO;
    (void) stty (fileno (tty), &newsg[player]);
}

noecho () {
    newsg[player].sg_flags &= ~ECHO;
    (void) stty (fileno (tty), &newsg[player]);
}

crmode () {
    newsg[player].sg_flags |= CBREAK;
    (void) stty (fileno (tty), &newsg[player]);
}

nocrmode () {
    newsg[player].sg_flags &= ~CBREAK;
    (void) stty (fileno (tty), &newsg[player]);
}

raw () {
    newsg[player].sg_flags |= RAW;
    (void) stty (fileno (tty), &newsg[player]);
}

noraw () {
    newsg[player].sg_flags &= ~RAW;
    (void) stty (fileno (tty), &newsg[player]);
}

setchrs () {
    ioctl (fileno (tty), TIOCGLTC, &oldch[player]);
    ioctl (fileno (tty), TIOCGETC, &oldtch[player]);

    newtchars.t_startc = oldtch[player].t_startc;
    newtchars.t_stopc = oldtch[player].t_stopc;

    ioctl (fileno (tty), TIOCSLTC, &newltchars);
    ioctl (fileno (tty), TIOCSETC, &newtchars);
}

/*
 * this function is used in order to read strings from a terminal.
 * If it is back_space, it goes backwards in the buffer.
 * If it is cntrl/p it outputes the name of the current planet
 * by calling itself recursively.
 * Any other cntr char is ignored, and the terminal's bell is ringed.
 * Other char is echoed to the terminal, & inserted in the buffer.
 */

# define buf  bufs[player]
# define bufp bufsp[player]
extern char bufs[2][100];
extern int  bufsp[2];

dogetstr (c)
char    c;
{
    char        *topid ;

    if (c == '\b') {
        if (bufp == 0)
            ding ();
        else {
            fprintf (tty, "\b");
            ceol ();
            bufp--;
        }
        return ;
    }
      /* If a kill character is given, then remove entire input line */
      /* kill characters are ^X and ^] */
      if (c == '\030' || c == '\035') {
        if (bufp == 0)
            ding ();
        else {
            cleol (22, 11);
            bufp = 0;
        }
        return ;
      }
      else {
        if (c == '\020' ) {      /* cntr/p */
            topid = curpln -> pid ;
            dogetstr(*topid++) ;
            dogetstr(*topid++) ;
            dogetstr(*topid) ;
            dogetstr(' ');
            return ;
        }
        if (iscntrl (c)) {
            ding ();
            return;
        }
        else
            fprintf (tty, "%c", c);
        buf[bufp++] = c;
    }
}

ding () {
    fprintf (tty, "\07");
}
