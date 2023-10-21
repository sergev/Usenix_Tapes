#
/*
**      FFDEF -- Global definitions for Fast Food
** (c) P. Langston
*/

	/* uncommonly modified stuff; requires complete re-compile */
#define LIM_X_SIZE      26      /* horiz board size */
#define LIM_Y_SIZE      10      /* vert board size */
#define LIM_PLAYERS     16      /* maximum number of tycoons in game */
#define LIM_CHAINS      26      /* maximum concurrent chains */
#define LIM_PLOTS       10      /* max plots each player holds */
#define LIM_UPDATES     24      /* max # of update time slots */
#define CNAME_LENGTH    24      /* how long chain names are */
#define PNAME_LENGTH    24      /* how long player names are */

	/* never modified stuff */
#define	SINGLE_STORE	-1	/* "chain number" for independents */
#define	NO_PLOT_LEFT	-1	/* the null plot */

#define	BUY	0		/* codes for the trade command */
#define	SELL	1

#define FIRST_LOCK      0       /* codes for the lock routine */
#define UPDATE_LOCK     0
#define	PLAYER_LOCK	1	/* (must agree with order in *lockfile[] */
#define	BOARD_LOCK	2
#define	CHAIN_LOCK	3
#define MISC_LOCK       4
#define LAST_LOCK       MISC_LOCK

#define	SAFE	0		/* := get requires a lock */
#define	SORRY	1		/* := get doesn't require lock */

#define LOCK_FAIL   0           /* returned by failure in lock */
#define LOCK_OK     1           /* returned by successful lock call */
#define NOT_LOCKED  LOCK_FAIL   /* not locked by us */

#define END_OF_GAME 1       /* nextup value at end of game */

struct	misc {
	long    m_nextup;                   /* time of next update */
	struct  times {
	    char    t_hr;                   /* 0-23 */
	    char    t_min;                  /* 0-59 */
	} m_time[LIM_UPDATES];              /* hr min of updates */
	char    m_satsun;                   /* 1 => updates allowed on sat & sun */
	char    m_lstchn;                   /* last chain created */
	int     m_lstplt;                   /* last plot given out */
};

struct	player {
	char    p_name[PNAME_LENGTH];
	struct  plot {
	    char    p_x;
	    char    p_y;
	} p_plot[LIM_PLOTS];
	long	p_time;
	long	p_money;
	long	p_div;
	int	p_uid;
	char    p_shares[LIM_CHAINS + 1];
	int	p_trans;
};

struct        {
      int     p_xy;           /* used to set/transfer entire plot */
};

struct	chain {
	char    c_name[CNAME_LENGTH];
	int     c_size;            /* also used to hold last chain created */
	int	c_index;
	int	c_shares;
	int	c_volume;
	int	c_change;
};

struct  histstr {
	long    h_date;             /* time of this snapshot */
	struct  htycstr {
	    long    ht_money;       /* cash $ */
	    long    ht_stock;       /* stock $ */
	} h_tyc[LIM_PLAYERS];
	struct  hchnstr {
	    int     hc_size;        /* size of chain */
	    int     hc_shares;      /* # of shares available */
	    int     hc_price;       /* price per share */
	} h_chn[LIM_CHAINS + 1];
};

struct  holder  {
	char    h_pnum;         /* player number */
	char    h_shares;       /* number of shares */
	char    h_maj;          /* share of major bonus */
	char    h_min;          /* share of minor bonus */
};

extern  char    *plyrfil, *boardfil, *miscfil, *chainfil;
extern  char    *newsfil, *onwsfil, *histfil, *upfil;
extern  char    *locknode, *lockfile[], locks[];
extern	char	*upd_prog;
extern	char	*infodir, *nroffhd, *nroffil;
extern  char    board[LIM_X_SIZE][LIM_Y_SIZE];
extern  int     pnum, mfh, bfh, pfh, cfh;
extern  int     x_size, y_size, num_players, num_chains, num_plots;
extern  int     trans_limit, safe_size, plot_cost, new_bonus;
extern  int     merge1_bonus, merge2_bonus, initial_cash;
extern	long	period, fperiod, price();
extern	struct	player	plyr;
extern	struct	chain	chain;
extern	struct	misc	misc;
extern  struct  holder  hldr[];

extern  char    *getchrs();
