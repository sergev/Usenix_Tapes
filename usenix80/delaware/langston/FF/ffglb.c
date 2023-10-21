#include	"ffdef.h"
/*
**      FFGLB -- Global storage allocations for Fast Food
** Compile: cc -O -c -q ffglb.c
** (c) P. Langston
*/

char    *upd_prog       = "/sys/games/.../FF/update";
char    *infodir        = "/sys/games/.../FF/INFO";
char    *nroffhd        = "/sys/games/.../FF/INFO/info.mac";
char    *nroffil        = "/bin/nroff";
char    *plyrfil        = "/sys/games/.../FF/plyr.ff";
char    *boardfil       = "/sys/games/.../FF/board.ff";
char    *chainfil       = "/sys/games/.../FF/chain.ff";
char    *miscfil        = "/sys/games/.../FF/misc.ff";
char    *newsfil        = "/sys/games/.../FF/news.ff";
char    *onwsfil        = "/sys/games/.../FF/onews.ff";
char    *histfil        = "/sys/games/.../FF/hist.ff";
char    *upfil          = "/sys/games/.../FF/upfile";
char    *locknode       = "/sys/games/.../FF/locknode";
char	*lockfile[] {
	"/sys/games/.../FF/lock.update",
	"/sys/games/.../FF/lock.player",
	"/sys/games/.../FF/lock.board",
	"/sys/games/.../FF/lock.chain",
	"/sys/games/.../FF/lock.misc",
};

char    locks[] = {
	NOT_LOCKED, NOT_LOCKED, NOT_LOCKED, NOT_LOCKED, NOT_LOCKED,
};

	/* parametric miscellany */
int     x_size      = 16;       /* horiz board size <= LIM_X_SIZE */
int     y_size      = 10;       /* vert board size <= LIM_Y_SIZE */
int     num_players = 8;        /* number of tycoons <= LIM_PLAYERS */
int     num_chains  = 8;        /* maximum concurrent chains <= LIM_CHAINS */
int     num_plots   = 10;       /* how many each player holds <= LIM_PLOTS */
int     trans_limit = 16;       /* max # transactions per period */
int     safe_size   = 24;       /* unmergable size for chains */
int     plot_cost   = 100;      /* cost to develop a plot */
int     new_bonus   = 5;        /* # of free shares for creating a chain */
int     merge1_bonus= 2000;     /* bonus to maj holder of merged out chain */
int     merge2_bonus= 1000;     /* bonus to min holder of merged out chain */
int     initial_cash= 10000;    /* starting moola */

char    board[LIM_X_SIZE][LIM_Y_SIZE];

int     pnum, mfh, bfh, pfh, cfh;

struct	player	plyr;
struct	chain	chain;
struct	misc	misc;
struct  holder  hldr[LIM_PLAYERS];
