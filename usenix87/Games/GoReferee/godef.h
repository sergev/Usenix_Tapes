/*
**	GODEF -- Definitions for go programs
**	psl 5/84
*/

#define	LEGAL		0
#define	ILLEGAL		1

/* values for whyillegal */
#define	ILL_NOTEMPTY	1
#define	ILL_INTOKO	2
#define	ILL_SUICIDE	4

#define	PASS		(BAREA+1)
#define RESIGN		(BAREA+2)
#define BEBLACK		(BAREA+3)
#define BEWHITE		(BAREA+4)
#define BYOROMI		(BAREA+5)

#define	GRPFLG		1
#define	LIBFLG		2

#define	BSZ	19
#define	BSZ1	(BSZ+1)
#define	BSZ2	(BSZ+2)
#define	BAREA	(BSZ2*BSZ2)

#define	MUP	(-BSZ2)
#define	MDOWN	(BSZ2)
#define	MLEFT	(-1)
#define	MRIGHT	(1)

/* values for whyillegal */
#define	ILL_NOTEMPTY	1
#define	ILL_INTOKO	2
#define	ILL_SUICIDE	4

/* these macros for b_spot pointers */
#define	ABOVE(bsp)	(bsp+UP)
#define	BELOW(bsp)	(bsp+DOWN)
#define	LEFTOF(bsp)	(bsp+LEFT)
#define	RIGHTOF(bsp)	(bsp+RIGHT)

/* values for s_occ */
#define	EMPTY	0
#define	BLACK	1
#define	WHITE	2
#define	BORDER	3

#define	MAXLIBS	24
#define	MAXEYES	2
#define	MAXGRPS	255	/* need to fit this in a char */

struct	spotstr {
	char	s_occ;
	char	s_flg;
};

struct	plyrstr	{
	char    *p_plyr;	/* program name */
	int     p_pid;		/* proc id for plyr */
	short   p_rpipe;	/* pipe from plyr */
	short   p_wpipe;	/* pipe to plyr */
	int	p_captures;	/* how many opponents captured */
	int	p_ko;		/* potential ko from previous move */
	int	p_lycptp;	/* last value of ycptp from utime.c */
	long    p_utime;	/* accumulated user time (seconds) */
};

struct	bdstr	{
	struct	spotstr	b_spot[BAREA];
};

struct	gstr	{	/* groups of stones (a.k.a. strings & blocks) */
	short	g_who;		/* who owns the group (BLACK | WHITE) */
	short	g_size;		/* how many stones are in the group */
	short	g_spot;		/* a representative stone in the group */
	short	g_libs;		/* how many liberties there are */
	short	g_l[MAXLIBS];	/* the first few liberties */
	short	g_eyes;		/* how many eyes there are */
	short	g_h[MAXEYES];	/* the first few eye hole numbers */
};

struct	hstr	{	/* holes between (among?) stones */
	short	h_size;		/* how many spots are in the hole */
	short	h_spot;		/* a representative spot in the hole */
	short	h_grps;		/* how many surrounding groups there are */
	short	h_g[MAXLIBS];	/* the first few groups */
	short	h_who;		/* who "owns" (no one, BLACK, WHITE, both) */
};


extern	char	*passmove;
extern	char	*resignmove;
extern	char	*blackmove;
extern	char	*whitemove;
extern	char	*letters;
extern	char	*color[];
extern	char	fmtbuf[];
extern	char	whyillegal;

extern	int	liblist[MAXLIBS], nlibs;
extern	int     dd[4];
extern	int	movenum;
extern	int	lastflg;
extern	struct  plyrstr	p[3];
extern	struct	bdstr	b;

extern	char    *stoc(), *copy();
