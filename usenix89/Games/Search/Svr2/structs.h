/*
 *
 * search
 *
 * multi-player and multi-system search and destroy.
 *
 * Original by Greg Ordy	1979
 * Rewrite by Sam Leffler	1981
 * Socket code by Dave Pare	1983
 * Ported & improved
 *      by Matt Crawford	1985
 *
 * file contains data structure declarations used in search.
 * originally these were spread across several .h files.
 *
 * Copyright (c) 1979
 *
 * $Header: structs.h,v 2.2 85/08/06 22:29:49 matt Exp $
 * $Log:	structs.h,v $
 * Revision 2.2  85/08/06  22:29:49  matt
 * Change handling of "r", "b", "g", "j", "q" commands to
 * provide better feedback, using per-player message buffer.
 * 
 * Revision 2.1  85/04/10  17:32:03  matt
 * Major de-linting and minor restructuring.
 * 
 * Revision 1.4  85/02/11  12:44:06  matt
 * added GUTS mode
 * 
 * Revision 1.3  84/07/08  13:40:29  matt
 * Added two bytes of padding to the t_file structure.  Vax compiler
 * added them implicitly but Sun compiler did not, giving "bad new
 * player read".
 * 
 * Revision 1.2  84/07/07  18:11:31  matt
 * Rearranged structure t_file because it gets sent across the
 * net and may be padded differently by different compilers.
 * 
 */

struct plist {
	char	zx;
	char	zy;
	char	zchar;
	char	zflg;
	struct plist *zpnt;
};

/*
 * Shrapnel description data base
 */
typedef struct {
	char    cbx;		/* x coord of center */
	char    cby;		/* y coord of center */
	char    cbactive;	/* any pieces still visible */
	char    cbcnt;		/* time left to stay active */
	char    shrap[9][2];	/* offsets from center of pieces */
	char    shrapd[9];	/* offset from center of each piece */
} t_burst;

/*
 * Player structure -- contains everything you'd ever
 *  want to know about a player in the game
 */
typedef struct {
	char	plname[20];	/* player's name */
	int	uid;		/* Uniq identifier of player */
	short	energy;		/* current energy level */
	short	maxmaxe;	/* max max energy */
	short	maxe;		/* max energy to accumulate */
	char	curx;		/* current x coord */
	char	cury;		/* current y coord */
	char	gmess[40];	/* group message buffer */
/* status stuff */
	struct {
		unsigned orb : 1;	/* in orbit? */
		unsigned bur : 1;	/* pending shrapnel damage */
		unsigned ap : 1;	/* in the middle of an autopilot */
		unsigned crash : 1;	/* crash into quartone? */
		unsigned alive : 1;	/* player alive? */
		unsigned killed : 1;	/* was the player killed? */
		unsigned invis : 1;	/* are we invisible */
		unsigned begin : 1;	/* new player? */
		unsigned guts : 1;	/* wants to play "guts" */
	} status;
/* points */
	int	points;		/* current point total for this round */
	long	pltime;		/* playing time accumulated */
	int	pkills;		/* kills of other players accumulated */
	int	phits;		/* hits of other players accumulated */
	int	ahits;		/* alien hits accumulated */
/* i/o */
	char	cmdpend;	/* pending command */
	char	cmd;		/* current command */
	char	inputq[QSIZE];	/* for buffering input from terminal */
	char	*pinputq;	/* next char to read */
	char	ninput;		/* # of chars in input q -- must fit QSIZE */
	char	outputq[2048];	/* characters to output - null terminated */
	char	*eoq;		/* pointer to the current end-of-queue */
/* autopilot */
	union	thing_u	*home[3];	/* autopilot channels */
	char	*apx;		/* autopilot pointer - x coord */
	char	*apy;		/* autopilot pointer - y coord */
	union	thing_u	*whocent;	/* center over who? */
/* screen stuff */
	struct	plist *plstp;	/* head of screen list */
	int	preve;		/* last energy value displayed */
	int	prevp;		/* last point total displayed */
	int	mflg;		/* magnification factor */
	char	offx;		/* x offset from center */
	char	offy;		/* y offset from center */
/* quartone stuff */
	int	ocnt;		/* orbit count */
	int	qkill;		/* quartonites killed during current xploit */
	int	nkcnt;		/* nuke count */
	int	xcount;		/* xploitation count */
	int	scabcount;	/* scab threshold */
/* Termcap additions */
	int	ttyspeed;	/* baud rate for padding */
	char	PC;		/* pad char */
	char	*BC;		/* backspace one char */
	char	*UP;		/* move up one line */
	char	*CM;		/* cursor motion string */
	char	*CL;		/* clear screen */
	char	*CE;		/* clear to end of line */
/* radio and broadcast message stuff */
	char	mesgbuf[40];	/* message buffer */
	int	mesglen;	/* characters in buffer */
	int	mesgdst;	/* radio message destination */
} t_player;

/*
 * Definition of an alien being
 */
typedef struct {
	char	type;		/* shanker, X-alien, etc. */
	char	onscr;		/* on screen? */
	char	cx;		/* current x coord */
	char	cy;		/* current y coord */
	char	cix;		/* current increment in x direction */
	char	ciy; 		/* current increment in y direction */
	char	fx;		/* x firing direction */
	char	fy;		/* y firing direction */
	char	stats;		/* state bits */
	char	hit;		/* # of hits taken */
	char	count;		/* # in existence */
	char	aname;		/* alien moniker */
	union	thing_u	*whotoget;	/* player to attack -- shanker */
} t_alien;

/*
 * Planet definitions
 */
typedef struct {
	char	*planetname;	/* planet's name */
	char	places[17][2];	/* offsets from center of pieces */
} t_planet;

/*
 * Location of starbases
 */
typedef struct {
	char	xpos;		/* x coord of center */
	char	ypos;		/* y coord of center */
} t_sbase;

/*
 * Info regarding torpedoes
 */
typedef struct {
	char	torx;		/* current x coord */
	char	tory;		/* current y coord */
	union	thing_u	*owner;		/* who shot it? */
	union	thing_u	*target;	/* destination */
	char	xinc;		/* velocity increment in x direction */
	char	yinc;		/* velocity increment in y direction */
	char	tcount;		/* time to live counter */
} t_torps;

typedef union thing_u {		/* need a tag for recursive structs */
	t_player	u_player;
	t_alien		u_alien;
	t_sbase		u_sbase;
	t_torps		u_torps;
} thing;

/*
 * User point totals data base
 */
typedef struct {
	char	ptname[20];	/* player name */
	int	ptpoints;	/* total points */
	int	pthits;		/* total player hits */
	int	ptahits;	/* total alien hits */
	int	ptkills;	/* total player kills */
	int	ptkilled;	/* total times killed */
	int	ptlast;		/* last game's score */
	int	ptbest;		/* best score */
	int	ptgames;	/* # games played */
} t_totals ;

/*
 * One file structure is read in for each player
 *   that gets into a game
 */

typedef struct {
	int	uid;		/* Uniq id for the player */
	char	p_name[20];	/* player's chosen moniker */
	char	p_BC[20];	/* backspace one character */
	char	p_UP[20];	/* move up one line */
	char	p_CM[40];	/* cursor motion string */
	char	p_CL[20];	/* clear screen */
	char	p_CE[20];	/* clear to end of line */
	int	p_speed;	/* terminal's ioctl speed */
	char	p_PC;		/* pad character */
	char	p_flags;	/* status bits of player */
	char	p_xxx[2];	/* padding for portability */
} t_file;

struct message {
    long mtype;
    int ident;
    char text[10000];
};
