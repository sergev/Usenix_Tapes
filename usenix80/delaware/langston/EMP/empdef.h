#
/*
**              Basic Empire definitions
**Copyright (c) 1976 by Peter S. Langston - Camb., Ma. 02140
*/

	/* miscellany */

#define MAX_MAXNOC      32             /* max value for MAXNOC (in empglb) */
#define MAX_W_XSIZE     128     /* w_xsize can't exceed this (in empglb.c) */
#define MAXTELSIZE      512                      /* max telegram text size */
#define LAND            0      /* returned by landorsea() to indicate land */
#define SEA             1       /* returned by landorsea() to indicate sea */
#define NUMNUMNAMES     21     /* number of names in numnames[] (empglb.c) */

#define min(x,y)    ((x) < (y)? x : y)
#define max(x,y)    ((x) > (y)? x : y)


#ifdef D_CHARCODES
	/* special character codes (Empire runs in "raw" mode) [?] */
#define EOT         004
#define BS          010
#define HT          011
#define LF          012
#define CR          015
#define RETYPE      022
#define LINE_DEL    025
#define ESC         033
#define RUB_OUT     0177
#define EOF         -1
#endif D_CHARCODES

	/* return codes from command routines */
#define NORM_RETURN 0   /* command completed sucessfully */
#define FAIL_RETURN 1   /* command completed unsucessfully [?] */
#define SYN_RETURN  2   /* syntax error in command */
#define SYS_RETURN  3   /* system error (missing file, etc) */


#ifdef D_UPDATE
	/* selective update flags */
#define UP_NONE         0       /* no update */
#define UP_OWN          1       /* only when sect.owned == cnum */
#define UP_ALL          2       /* ignore sect.owned */
#define UP_TIME         4       /* even if little work done */
#define UP_GOD          8       /* even if cnum == 0 */
#define	UP_VERBOSE	16	/* mention delivery backlogs, etc */
#define	UP_QUIET	32	/* don't mention anything */
#endif D_UPDATE


#ifdef D_NATSTAT
	/* nation status types */
#define	STAT_DEAD	0
#define	STAT_VISITOR	1
#define	STAT_NEW	2
#define	STAT_NOCAP	3
#define	STAT_NORMAL	4
#define	STAT_GOD	7
#endif D_NATSTAT

	/* nation relation codes */
#define	NEUTRAL	0
#define	ALLIED	1
#define	HOSTILE	2
#define	AT_WAR	3


#ifdef D_SECTDES
	/* sector types (must agree with order in dchr, empglb.c) */
#define	S_WATER	0	/* basics */
#define	S_MOUNT	1
#define	S_SANCT	2
#define	S_RURAL	3
#define	S_CAPIT	4
#define	S_URBAN	5
#define S_ARMSF 6       /* industries */
#define S_AMMOF 7
#define S_MINE  8
#define S_GMINE 9
#define S_HARBR 10
#define S_WAREH 11
#define S_AIRPT 12
#define S_AGRI  13
#define S_TECH  14      /* military/scientific */
#define S_FORTR 15
#define S_RSRCH 16
#define S_HIWAY 17      /* communications */
#define S_RADAR 18
#define S_WETHR 19
#define S_BHEAD 20
#define S_BSPAN 21
#define S_BANK  22      /* financial */
#define S_XCHNG 23

#define S_MAXNO 23
#endif D_SECTDES

#define	SELL_NOT_DELIVER	2	/* in "_use" field => contract */

#ifdef D_SHIPTYP
	/* ship types */
#define	S_PT	0
#define	S_MIN	1
#define	S_DES	2
#define	S_SUB	3
#define	S_FRE	4
#define	S_TEN	5
#define	S_BAT	6
#define	S_CAR	7
#define	TMAXNO	7
#endif D_SHIPTYP

#ifdef D_NEWSVERBS
	/* news verbs */
#define	N_WON_SECT	1
#define	N_SCT_LOSE	2
#define	N_SPY_SHOT	3
#define	N_SENT_TEL	4
#define	N_SIGN_TRE	5
#define	N_MAKE_LOAN	6
#define	N_REPAY_LOAN	7
#define	N_MAKE_SALE	8
#define	N_GRANT_SECT	9
#define N_SCT_SHELL     10
#define N_SHP_SHELL     11
#define N_TOOK_UNOCC    12
#define N_TORP_SHIP     13
#define N_FIRE_BACK     14
#define N_BROKE_SANCT   15
#define N_SCT_BOMB      16
#define N_SHP_BOMB      17
#define N_BOARD_SHIP    18
#define N_SHP_LOSE      19
#define N_FLAK          20
#define N_SEIZE_SECT    21
#define N_HONOR_TRE     22
#define N_VIOL_TRE      23
#define N_DISS_GOV      24
#define N_HIT_MINE      25
#define N_DECL_ALLY     26
#define N_DECL_NEUT     27
#define N_DECL_WAR      28
#define N_DIS_ALLY      29
#define N_DIS_WAR       30
#define N_SCT_STORM     31
#define N_SHP_STORM     32
#define N_OUT_PLAGUE    33
#define N_DIE_PLAGUE    34
#define N_NAME_CHNG     35
#define N_MAX_VERB      35
#define	N_MAX_PAGE	3		/* depends on values in rpt (empglb) */

#define NEWS_PERIOD     302400.         /* max duration of news (seconds) */
#endif D_NEWSVERBS

#ifdef D_TRTYCLAUSE
	/* treaty clauses */
#define	SEAATT	01
#define	SEAFIR	02
#define	LANATT	04
#define	LANFIR	010
#define	TRTSPY	020
#define	TRTENL	040
#define	TRTRAD	0100
#define	TRTBUI	0200
#endif D_TRTYCLAUSE

#ifdef D_PLGSTAGES
#define	PLG_HEALTHY	0
#define	PLG_DYING	1
#define	PLG_INFECT	2
#define	PLG_INCUBATE	3
#define	PLG_EXPOSED	4
#endif D_PLGSTAGES


	/*	STRUCT DEFINITIONS FOR EMPIRE	*/

struct	{ char O; };

#ifdef D_NATSTR
struct	boundstr {
	char	b_xl, b_xh;		/* horizontal bounds */
	char	b_yl, b_yh;		/* vertical bounds */
};
extern	struct	boundstr	nrealm[];
struct	natstr {
	char    nat_cnam[20];               /* country name */
	char    nat_pnam[20];               /* representative */
	int     nat_btu;                    /* bureaucratic time units */
	int     nat_nuid;                   /* nation user-id */
	char    nat_playing;                /* number of current tty */
	char    nat_tgms;                   /* # of telegrams to be announced */
	char    nat_xcap, nat_ycap;         /* location in abs coords */
	char    nat_stat;                   /* visitor, nocap, etc */
	char    nat_dayno;                  /* day of the year mod 128 */
	int     nat_minused;                /* number of minutes used today */
	struct  boundstr nat_b[4];          /* realm bounds */
	long    nat_date;                   /* last logoff */
	long    nat_money;                  /* moola */
	int     nat_relate[MAX_MAXNOC/8];   /* two bits for each other country
		coded:	00 : NEUTRAL	01 : ALLIED
			10 : HOSTILE	11 : AT_WAR */
	float   nat_t_level;                /* level of technology */
	float   nat_r_level;                /* level of research */
	float   nat_up_off;                 /* update offset (in #0 only) */
};
extern	struct	natstr		nat;
#endif D_NATSTR

#ifdef D_SCTSTR
struct	sctstr {
	char    sct_owned;          /* owner's country num */
	char    sct_desig;          /* sector type */
	char    sct_effic;          /* 0% to 100% */
	char    sct_miner;          /* ease of mining ore */
	char    sct_gmin;           /* amount of gold ore */
	char    sct_mobil;          /* mobility units */
	char    sct_prdct;          /* production units */
	char    sct_chkpt;          /* non-zero if checkpointed */
	char    sct_dfend;          /* relative dx,dy of protector */
	char    sct_contr;          /* % of standard contract price */
	char    sct_civil;          /* num of civilians */
	char    sct_milit;          /* num of military */
	char    sct_shell;          /* num of shells */
	char    sct_guns;           /* num of guns */
	char    sct_plane;          /* num of planes */
	char    sct_ore;            /* amt of ore */
	char    sct_gold;           /* num of gold bars */
	char    sct_f1;
	char    sct_c_use;          /* transport for civil */
	char    sct_m_use;          /* transport for milit */
	char    sct_s_use;          /* price or deliv for shells */
	char    sct_g_use;          /* ditto guns */
	char    sct_p_use;          /* ditto planes */
	char    sct_o_use;          /* ditto ore */
	char    sct_b_use;          /* ditto bars */
	char    sct_p_stage;        /* stage of plague */
	char    sct_p_time;         /* # half_hours till next stage */
	char    sct_f4,sct_f5,sct_f6;
	int     sct_lstup;          /* last update time */
};
extern	struct	sctstr		sect;
#endif D_SCTSTR

#ifdef D_SHPSTR
struct	shpstr {
	char    shp_own;            /* country # of owner */
	char    shp_type;           /* ship type */
	char    shp_effc;           /* 0 - 100 */
	char    shp_xp, shp_yp;     /* location in abs coords */
	char    shp_fleet;          /* group membership */
	char    shp_crew;           /* military | civvies on board */
	char    shp_shels;          /* shells on board */
	char    shp_gun;            /* etc */
	char    shp_plns;
	char    shp_or;
	char    shp_gld;
	char    shp_spric;          /* ship price, if for sale */
	char    shp_mbl;            /* mobility */
	int     shp_lstp;           /* time of last update */
};
extern  struct  shpstr  ship;
#endif D_SHPSTR

#ifdef D_COMSTR
struct	comstr {
	char	*c_form;	/* prototype of command */
	char	c_prog;		/* # of module that contains it */
	char	c_cost;		/* btu cost of command */
	int	c_addr;		/* core addr of appropriate routine */
	int	c_permit;	/* who is allowed to "do" this command */
};
extern  struct  comstr  coms[];
#endif D_COMSTR

#ifdef D_POWSTR
struct powstr {
	float	p_sects;
	float	p_effic;
	float	p_civil;
	float	p_milit;
	float	p_shell;
	float	p_guns;
	float	p_plane;
	float	p_ore;
	float	p_gold;
	float	p_ships;
	float	p_money;
	float	p_power;
};
#endif D_POWSTR

#ifdef D_DCHRSTR
struct	dchrstr	{
	char	d_mnem;		/* map symbol */
	char	d_ptyp;		/* index into ichr for product (if any) */
	char	d_mcst;		/* movement cost */
	char	d_pkg;		/* type of packaging in these sects */
	char	d_ostr;		/* offensive strength */
	char	d_dstr;		/* defensive strength */
	char	d_value;	/* resale ("collect") value */
	char	*d_name;	/* full name of sector type */
};
extern	struct	dchrstr dchr[];
#endif D_DCHRSTR

#ifdef D_ICHRSTR
struct	ichrstr {
	char	i_mnem;		/* usually the initial letter */
	char	i_prdct;	/* # units of prdct to make one */
	char	i_del;		/* index of delivery slot in sect */
	char	i_shp;		/* index into ship struct */
	char	i_mch;		/* index into marine characteristics */
	char	i_bid;		/* average amount paid on contract */
	char	i_value;	/* mortgage value */
	char	i_lbs;		/* how hard to move */
	char	i_pkg[4];	/* units for reg, ware, urb, spare */
	char	*i_name;	/* full name of item */
};
extern	struct	ichrstr ichr[];
#endif D_ICHRSTR

#ifdef D_NWSSTR
struct	nwsstr {
	char    nws_ano;            /* "actor" country # */
	char    nws_vrb;            /* action (verb) */
	char    nws_vno;            /* "victim" country # */
	char    nws_ntm;            /* btu of "actor" at time */
	long    nws_when;           /* time of action */
};
extern	struct	nwsstr nws;
#endif D_NWSSTR

#ifdef D_RPTSTR
struct	rptstr {
	char    r_good_will;      /* how "nice" the action is */
	char    r_newspage;       /* which page this item belongs on */
	char    *r_newstory;      /* text for fmt( */
};
extern	struct	rptstr rpt[];
#endif D_RPTSTR

#ifdef D_MCHRSTR
struct	mchrstr {
	char	m_prdct;	/* units of prdct to build */
	char	m_speed;	/* how fast it can go */
	char	m_visib;	/* how well it can be seen */
	char	m_vrnge;	/* how well it can see */
	char	m_frnge;	/* how far it can fire */
	char	m_civil;	/* how many it can hold */
	char	m_milit;	/* " */
	char	m_shels;
	char	m_gun;
	char	m_plns;
	char	m_or;
	char	m_gld;
	char	*m_name;	/* full name of type of ship */
};
extern	struct	mchrstr	mchr[];
#endif D_MCHRSTR

#ifdef D_TELSTR
struct	telstr {
	char	tel_from;	/* sender */
	char	tel_spare;
	long    tel_date;       /* when sent */
	int	tel_length;	/* how long */
};
extern	struct	telstr tgm;
#endif D_TELSTR

#ifdef D_TRTSTR
struct	trtstr {
	char    trt_cna;            /* proposed by */
	char    trt_cnb;            /* accepted by (if >0, else pending) */
	char    trt_acond;          /* conditions for proposer */
	char    trt_bcond;          /* conditions for accepter */
	long    trt_exp;            /* expiration date */
};
extern  struct  trtstr trty;
#endif D_TRTSTR

#ifdef D_TCHRSTR
struct	tchrstr {
	char	t_cond;		/* bit to indicate this clause */
	char	*t_name;	/* description of clause */
};
extern	struct	tchrstr	tchr[];
#endif D_TCHRSTR

#ifdef D_LONSTR
struct	lonstr {
	char    l_loner;          /* loan shark */
	char    l_lonee;          /* sucker */
	char    l_irate;          /* interest rate */
	char    l_ldur;           /* intended duration */
	int     l_amtpaid;        /* amount paid so far */
	int     l_amtdue;         /* amount still owed */
	long    l_lastpay;        /* date of most recent payment */
	long    l_duedate;        /* date after which interest doubles, etc */
};
extern	struct	lonstr loan;
#endif D_LONSTR

#ifdef D_NSCSTR
struct	nscstr {
	char    n_fld1;         /* first commodity or number */
	char    n_oper;         /* required relationship operator */
	char    n_fld2;         /* second commodity or number */
};
struct  nstr    {               /* for sectors */
	char    n_x, n_y;       /* current sector x & y */
	char    n_lx, n_hx;     /* x bounds */
	char    n_ly, n_hy;     /* y bounds */
	char    n_ix, n_iy;     /* x & y increments */
	char    n_ncond;        /* number of conditions */
	struct  nscstr  n_cond[8];  /* the conditions */
};
#define NBLISTMAX   32
struct  nbstr    {              /* for ships (boats) */
	int     nb_sno;         /* current ship number */
	char    nb_cno;         /* country number, (0 => all countries) */
	char    nb_mode;        /* 0 => all ships, -1 => fleet, */
				/* -2 => area, >0 => list length */
	int     nb_nums[NBLISTMAX]; /* list of ships */
	char    nb_scnt;        /* current index into ship list (nb_nums) */
	char    nb_fleet;       /* fleet letter */
	char    nb_lx, nb_hx;   /* x bounds */
	char    nb_ly, nb_hy;   /* y bounds */
	char    nb_ix, nb_iy;   /* x & y increments */
	char    nb_ncond;       /* number of conditions */
	struct  nscstr  nb_cond[8]; /* the conditions */
};
#endif D_NSCSTR

/*      External definitions
*/
extern  char    *emprog[], *empsrc, *empfix;

#ifdef D_FILES
extern	char	*upfil, *downfil;
extern	char	*sectfil, sectf;
extern	char	*natfil, natf;
extern	char	*newsfil, newsf;
extern	char	*loanfil, loanf;
extern	char	*infodir, infof;
extern	char	*nroffil, *nroffhd;
extern	char	*shipfil, shipf;
extern	char	*telfil, telf;
extern	char	*powfil, powf;
extern	char	*treatfil, trtf;
#endif D_FILES

/* parametric goodies */
extern  int     privuid, w_xsize, w_ysize;
extern  int     maxnoc, maxcno, m_m_p_d, n_max_verb;
extern	char	*privname, *privlog;

extern  char    *Version;                                 /* version ident */
extern	char	junk[], combuf[], *argp[], *condarg;
extern  char    nulls[], *effadv[], *numnames[];
extern  char    *shllrg1, *shllrg2, *shllrg3;
extern  int     sx, sy, lx, hx, ly, hy, ix, iy;
extern	int	wxh, wyh, wxl, wyl, capx, capy, nbrx, nbry;
extern	int	capxof[], capyof[];
extern	int	cnum, curup, ntime, nstat, ncomstat, nminused, ntused;
extern	int	broke;
extern	int	owner, proto, redirin;
extern	int	sigaddr[4], ttymod[3], weirdmode;
extern	int	dn[][2];
extern	int	fmtlen;
extern	double	up_offset;
extern  double  lasttime, dolcost;
