/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

/*
 * This file contains all termcap-using functions.  The game uses
 * these function in order to move the cursor, enter standout mode,
 * etc.
 */

FILE * tty;

struct terminal ttycs[2];
struct terminal *ttyc;

char    capbuf[1024];
char   *xxxx = capbuf;
char  **cbuf = &xxxx;

/*
 * This function tries to fill the buffer with the termcap
 * entry of the player's terminal.
 */

fillterm (s, t)
char   *s;
struct terminal *t;
{
    char   *SO,
           *SE,
           *CM,
           *CE,
           *CL,
           *FL,
           *FB,
           *IS,
           *KS,
           *TE,
           *KE,
           *TI;
    int     SG;
    char   *buf;
    char   *malloc ();
    char   *tgetstr ();

    buf = malloc (1024);
    if (buf == 0) {
        writes (2, "OUT OF MEMORY IN FILLTERM.\r\n");
        exit (126);
    }
    t -> t_name = s;
    if (tgetent (buf, s) != 1) {
        fprintf (stderr, "Sorry: Cannot get termcap entry for %s\n",
                s);
        exit (-1);
    }

    SO = tgetstr ("so", cbuf);
    SE = tgetstr ("se", cbuf);
    CM = tgetstr ("cm", cbuf);
    CE = tgetstr ("ce", cbuf);
    CL = tgetstr ("cl", cbuf);
    FL = tgetstr ("fl", cbuf);
    FB = tgetstr ("fb", cbuf);
    IS = tgetstr ("is", cbuf);
    KS = tgetstr ("ks", cbuf);
    TE = tgetstr ("te", cbuf);
    KE = tgetstr ("ke", cbuf);
    TI = tgetstr ("ti", cbuf);

    SG = tgetnum ("sg");

    if (SG == -1)
        SG = 0;

    if (!CM || !SO || !SE || !CL) {
        fprintf (stderr, "The %s terminal lacks basic capabilities. Sorry.\n", s) ;
        exit(1) ;
    }

    t -> t_so = SO;
    t -> t_se = SE;
    t -> t_cm = CM;
    t -> t_ce = CE;
    t -> t_cl = CL;
    t -> t_fl = FL;
    t -> t_fb = FB;
    t -> t_sg = SG;
    t -> t_is = IS;
    t -> t_ks = KS;
    t -> t_te = TE;
    t -> t_ke = KE;
    t -> t_ti = TI;

    free(buf);
}

/* put a single char to a given terminal */
int     xputchar (c)
int     c;
{
    putc (c, tty);
}

/* The initialization for the terminal */
cap_set (t)
struct terminal *t;
{
    tputs (t -> t_is, 1, xputchar);
    tputs (t -> t_ks, 1, xputchar);
    tputs (t -> t_ke, 1, xputchar);
}

/* put the cursor in a x,y position on the screen */
pos (x, y)
int     x,
        y;
{
/*
 * We use only CM.   No CM means no galaxy....
 */
    char   *s;
    char   *tgoto ();

    s = tgoto (ttyc -> t_cm, y, x);
    tputs (s, 1, xputchar);
}

/* output a string, encountering it with end of standout */
se (x, y, s)
char   *s;
{
    pos (x, y);
    tputs (ttyc -> t_se, 1, xputchar);
    print (s);
    tputs (ttyc -> t_se, 1, xputchar);
}

/* output a string, in the standout mode */
so (x, y, s)
char   *s;
{
    pos (x, y);
    tputs (ttyc -> t_so, 1, xputchar);
    print (s);
    tputs (ttyc -> t_se, 1, xputchar);
}

/* clear the whole screen */
clear () {
    pos (1, 1);
    tputs (ttyc -> t_cl, 1, xputchar);
}

/*
 * If the termcap entry was suitably modified, and the terminal
 * has more than one display page, this game benefits greatly
 * from such cases.
 * In short, `fl' entry contains the string that will cause the
 * terminal to "flip" forward , and `fb' entry holds the string
 * that will flip it backward.
 * Whenever any of this are missing, the "flip" is simulated
 * simply by clearing the screen, and writing the relevant
 * information on it.
 */

flip () {
    if (ttyc -> t_fl != 0) {
        if (ttyc -> t_curpage == 0) {
            tputs (ttyc -> t_fl, 1, xputchar);
            ttyc -> t_curpage = 1;
        }
        else {
            tputs (ttyc -> t_fb, 1, xputchar);
            ttyc -> t_curpage = 0;
        }
        return;
    }
    else {
        if (ttyc -> t_curpage == 0)
            ttyc -> t_curpage = 1;
        else {
            init_dis ();
            ttyc -> t_curpage = 0;
        }
    }
}

/* clear to end of line from current cursor position */
ceol () {
    tputs (ttyc -> t_ce, 1, xputchar);
}

/* clear to end of line from given cursor position */
cleol (x, y) {
    pos (x, y);
    tputs (ttyc -> t_ce, 1, xputchar);
}

writes (fd, s)
char   *s;
{
    (void) write (fd, s, strlen (s));
}


/*
 * this function brings the cursor to the current planet.
 * It will restor the video attribute to NORMAL .
 * Care must be taken to consider the effect of video attribute
 * that takes a place on the screen (tvi 925...) and also
 * that if not ended properly, it might extend to the end-of-line
 * or to the end-of-page.
 */

curse_map (pp)
planet * pp;
{
    int     x = pp -> coord[0];
    int     y = pp -> coord[1] + 1;
    draw_map (pp);
    pos (x, y);
}

draw_map (pp)
planet * pp;
{
    char    s[10];
    int     x = pp -> coord[0];
    int     y = pp -> coord[1] + 1;
    s[0] = pp -> d_symbol[player];
    s[1] = 0;

    se (x, y - ttyc -> t_sg, s);
}

/*
 * In this case we LEAVE the map, and go with the cursor to the
 * Command line. Thus we leave a mark at the current planet
 * so reference to it will be easy.
 */

curse_com (pp)
planet * pp;
{
    char    s[10];
    int     x = pp -> coord[0];
    int     y = pp -> coord[1] + 1;

    s[0] = pp -> d_symbol[player];
    s[1] = 0;
    so (x, y - ttyc -> t_sg, s);
}
