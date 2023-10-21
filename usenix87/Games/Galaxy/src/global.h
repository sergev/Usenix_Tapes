/*
 * %W% (mrdch&amnnon) %G%
 */

/*
 * This file contains the definitions of all the
 * global variables and return value types of functions.
 */

extern  jmp_buf jparse;
extern  planet * capitals[2];
extern int  skip;               /* number of chars to skip.      */
extern  info * info_head[2];    /*  linked info head */
extern  planet * spntr;         /* pointer to current 'star'     */
extern  planet * apntr;         /* pointer to current 'ampers'   */
extern  planet * curpln;
extern  FILE * tty;             /* We write to this tty          */
extern  FILE * ttys[];          /* these are the players terminals */
extern int  player;             /* the current player            */
extern int  teller[2];          /* the money available to each   */
extern int  food[2];            /* the money invested in food    */
extern int  trade[2];           /* the money invested in trade  */
extern  planet pl[];            /* the planets themselves */
extern char *ocup_name[];       /* the names of the occupations */
extern int  chand[MAXCHAN];
extern int  year;               /* The current year              */
extern int  iswiz[2];           /* Am i a wizard?                */
extern int  wants_quit[2];
extern int  wants_save[2];
extern int  wants_restor[2];
extern int  wants_newy[2];
extern int  wants_pause[2];
extern int  Pause;              /* Are we having a break or not */
extern char ply0[100],
           *ply1;               /* the names of the players */
extern int  espcost[ESPTYP];
extern char *inftypes[ESPTYP];
extern struct terminal  ttycs[2];
extern struct terminal *ttyc;

planet * getid ();
planet * dogetpl ();            /* getpl is a macro              */
time_t time ();
long    lseek ();
char   *strcpy ();
char   *sprintf ();
char   *strcat ();
