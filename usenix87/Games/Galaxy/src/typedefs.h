/*
 * %W% (mrdch&amnnon) %G%
 */

/*
 * this include file describes the differnet types used
 * by the game.
 */
typedef struct _movable movable;
typedef struct _info    info;
typedef struct _planet  planet;
typedef struct _chan    chan;

struct _info {
    int     owner;              /* who owns that massege */
    int     nmsg;               /* the current massege no. */
    char    msg[MSGSIZ];        /* max chars in one massesge */
            info * next;        /* and here comes another one.. */
};

struct _movable {               /* what can be taken planet */
    int     popul[CLASES];      /* population            */
    int     metals;             /* how much metal was digged */
    int     know;               /* knowledge level on    */
};


struct _planet {                /* describe a planet */
    char    symbol;             /* the planet original symbol */
    char    d_symbol[2];        /* the planet displayed symbol */
    char    pid[4];             /* planet id */
    int     coord[2];           /* planet's cooridinates */
            planet * gate[10];  /* where can we go from it? */
    int     whos;               /* who owns it */
            movable inventar;   /* what is actually there */
            movable to_take;    /* what ordered to be moved */
    int     secur;              /* security - Black-Out */
    int     alms;               /* how many alm's were left */
    int     paint;              /* how much money for paint */
    int     detect;             /* how much money to detect */
    int     to_build[3];        /* money level & no. to build */
            info * reports;     /* reportes gathered */
    int     ships[MAXSHIPS];    /* no. of ships on planet */
    int     missile[MAXSHIPS];  /* how many missiles are there */
    int     espion[2][ESPTYP][ESPSIZ];/* espionage */
};

/*
 * the following structure is used to communicate between terminals.
 * Since galaxy can't read both terminals, it reads a pipe. The pipe
 * is written on by two local processes, one for each terminal.
 * Each write on the pipe, writes this structure.
 */
struct _chan {
    int     ichan;              /* terminal number */
    char    c;                  /* character       */
};

struct terminal {               /* terminal information */
    char   *t_name;             /* terminal name         */
    char   *t_ke;               /* end keypad transmit   */
    char   *t_so;               /* begin standout mode   */
    char   *t_se;               /* end standout mode     */
    char   *t_cm;               /* cursor motion         */
    char   *t_ce;               /* clear to end of line */
    char   *t_ks;               /* keypad transmit mode */
    char   *t_cl;               /* clear screen          */
    char   *t_fl;               /* move to page 0        */
    char   *t_fb;               /* move to page 1        */
    char   *t_is;               /* initialisation string */
    char   *t_te;               /* program begin use cm  */
    char   *t_ti;               /* program end using cm */
    int     t_sg;               /* # spaces left by so   */
    int     t_curpage;          /* current page          */
};
