/*	EDEF:		Global variable definitions for
			MicroEMACS 3.2

			written by Dave G. Conroy
			modified by Steve Wilhite, George Jones
			greatly modified by Daniel Lawrence
*/

/* some global fuction declarations */

char *malloc();
char *strcpy();
char *strcat();
char *strncpy();
char *itoa();
char *getval();
char *gtenv();
char *gtusr();
char *gtfun();
char *token();
char *ltos();
char *flook();

#ifdef	maindef

/* for MAIN.C */

/* initialized global definitions */

int     fillcol = 72;                   /* Current fill column          */
short   kbdm[NKBDM];			/* Macro                        */
char    pat[NPAT];                      /* Search pattern		*/
char	rpat[NPAT];			/* replacement pattern		*/
char	*execstr = NULL;		/* pointer to string to execute	*/
char	golabel[NPAT] = "";		/* current line to go to	*/
int	execlevel = 0;			/* execution IF level		*/
int	eolexist = TRUE;		/* does clear to EOL exist	*/
int	revexist = FALSE;		/* does reverse video exist?	*/
int	flickcode = FALSE;		/* do flicker supression?	*/
char	*modename[] = {			/* name of modes		*/
	"WRAP", "CMODE", "SPELL", "EXACT", "VIEW", "OVER", "MAGIC", "CRYPT"};
char	modecode[] = "WCSEVOMY";	/* letters to represent modes	*/
int	gmode = 0;			/* global editor mode		*/
int	gfcolor = 7;			/* global forgrnd color (white)	*/
int	gbcolor	= 0;			/* global backgrnd color (black)*/
int     sgarbf  = TRUE;                 /* TRUE if screen is garbage	*/
int     mpresf  = FALSE;                /* TRUE if message in last line */
int	clexec	= FALSE;		/* command line execution flag	*/
int	mstore	= FALSE;		/* storing text to macro flag	*/
struct	BUFFER *bstore = NULL;		/* buffer to store macro text to*/
int     vtrow   = 0;                    /* Row location of SW cursor */
int     vtcol   = 0;                    /* Column location of SW cursor */
int     ttrow   = HUGE;                 /* Row location of HW cursor */
int     ttcol   = HUGE;                 /* Column location of HW cursor */
int	lbound	= 0;			/* leftmost column of current line
					   being displayed */
int	taboff	= 0;			/* tab offset for display	*/
int	metac = CTRL | '[';		/* current meta character */
int	ctlxc = CTRL | 'X';		/* current control X prefix char */
int	reptc = CTRL | 'U';		/* current universal repeat char */
int	abortc = CTRL | 'G';		/* current abort command char	*/

int	quotec = 0x11;			/* quote char during mlreply() */
char	*cname[] = {			/* names of colors		*/
	"BLACK", "RED", "GREEN", "YELLOW", "BLUE",
	"MAGENTA", "CYAN", "WHITE"};
KILL *kbufp  = NULL;		/* current kill buffer chunk pointer	*/
KILL *kbufh  = NULL;		/* kill buffer header pointer		*/
int kused = KBLOCK;		/* # of bytes used in kill buffer	*/
WINDOW *swindow = NULL;		/* saved window pointer			*/
int cryptflag = FALSE;		/* currently encrypting?		*/
short	*kbdptr;		/* current position in keyboard buf */
short	*kbdend = &kbdm[0];	/* ptr to end of the keyboard */
int	kbdmode = STOP;		/* current keyboard macro mode	*/
int	kbdrep = 0;		/* number of repetitions	*/
int	restflag = FALSE;	/* restricted use?		*/
long	envram = 0l;	/* # of bytes current in use by malloc */
int	macbug = FALSE;		/* macro debuging flag		*/
char	errorm[] = "ERROR";	/* error literal		*/
char	truem[] = "TRUE";	/* true literal			*/
char	falsem[] = "FALSE";	/* false litereal		*/
int	cmdstatus = TRUE;	/* last command status		*/

/* uninitialized global definitions */

int     currow;                 /* Cursor row                   */
int     curcol;                 /* Cursor column                */
int     thisflag;               /* Flags, this command          */
int     lastflag;               /* Flags, last command          */
int     curgoal;                /* Goal for C-P, C-N            */
WINDOW  *curwp;                 /* Current window               */
BUFFER  *curbp;                 /* Current buffer               */
WINDOW  *wheadp;                /* Head of list of windows      */
BUFFER  *bheadp;                /* Head of list of buffers      */
BUFFER  *blistp;                /* Buffer for C-X C-B           */

BUFFER  *bfind();               /* Lookup a buffer by name      */
WINDOW  *wpopup();              /* Pop up window creation       */
LINE    *lalloc();              /* Allocate a line              */
char	sres[NBUFN];		/* current screen resolution	*/

#else

/* for all the other .C files */

/* initialized global external declarations */

extern  int     fillcol;                /* Fill column                  */
extern  short   kbdm[];                 /* Holds kayboard macro data    */
extern  char    pat[];                  /* Search pattern               */
extern	char	rpat[];			/* Replacement pattern		*/
extern	char	*execstr;		/* pointer to string to execute	*/
extern	char	golabel[];		/* current line to go to	*/
extern	int	execlevel;		/* execution IF level		*/
extern	int	eolexist;		/* does clear to EOL exist?	*/
extern	int	revexist;		/* does reverse video exist?	*/
extern	int	flickcode;		/* do flicker supression?	*/
extern	char *modename[];		/* text names of modes		*/
extern	char	modecode[];		/* letters to represent modes	*/
extern	KEYTAB keytab[];		/* key bind to functions table	*/
extern	NBIND names[];			/* name to function table	*/
extern	int	gmode;			/* global editor mode		*/
extern	int	gfcolor;		/* global forgrnd color (white)	*/
extern	int	gbcolor;		/* global backgrnd color (black)*/
extern  int     sgarbf;                 /* State of screen unknown      */
extern  int     mpresf;                 /* Stuff in message line        */
extern	int	clexec;			/* command line execution flag	*/
extern	int	mstore;			/* storing text to macro flag	*/
extern	struct	BUFFER *bstore;		/* buffer to store macro text to*/
extern	int     vtrow;                  /* Row location of SW cursor */
extern	int     vtcol;                  /* Column location of SW cursor */
extern	int     ttrow;                  /* Row location of HW cursor */
extern	int     ttcol;                  /* Column location of HW cursor */
extern	int	lbound;			/* leftmost column of current line
					   being displayed */
extern	int	taboff;			/* tab offset for display	*/
extern	int	metac;			/* current meta character */
extern	int	ctlxc;			/* current control X prefix char */
extern	int	reptc;			/* current universal repeat char */
extern	int	abortc;			/* current abort command char	*/

extern	int	quotec;			/* quote char during mlreply() */
extern	char	*cname[];		/* names of colors		*/
extern KILL *kbufp;			/* current kill buffer chunk pointer */
extern KILL *kbufh;			/* kill buffer header pointer	*/
extern int kused;			/* # of bytes used in KB        */
extern WINDOW *swindow;			/* saved window pointer		*/
extern int cryptflag;			/* currently encrypting?	*/
extern	short	*kbdptr;		/* current position in keyboard buf */
extern	short	*kbdend;		/* ptr to end of the keyboard */
extern	int kbdmode;			/* current keyboard macro mode	*/
extern	int kbdrep;			/* number of repetitions	*/
extern	int restflag;			/* restricted use?		*/
extern	long envram;		/* # of bytes current in use by malloc */
extern	int	macbug;			/* macro debuging flag		*/
extern	char	errorm[];		/* error literal		*/
extern	char	truem[];		/* true literal			*/
extern	char	falsem[];		/* false litereal		*/
extern	int	cmdstatus;		/* last command status		*/

/* uninitialized global external declarations */

extern  int     currow;                 /* Cursor row                   */
extern  int     curcol;                 /* Cursor column                */
extern  int     thisflag;               /* Flags, this command          */
extern  int     lastflag;               /* Flags, last command          */
extern  int     curgoal;                /* Goal for C-P, C-N            */
extern  WINDOW  *curwp;                 /* Current window               */
extern  BUFFER  *curbp;                 /* Current buffer               */
extern  WINDOW  *wheadp;                /* Head of list of windows      */
extern  BUFFER  *bheadp;                /* Head of list of buffers      */
extern  BUFFER  *blistp;                /* Buffer for C-X C-B           */

extern  BUFFER  *bfind();               /* Lookup a buffer by name      */
extern  WINDOW  *wpopup();              /* Pop up window creation       */
extern  LINE    *lalloc();              /* Allocate a line              */
extern	char	sres[NBUFN];		/* current screen resolution	*/

#endif

/* terminal table defined only in TERM.C */

#ifndef	termdef
extern  TERM    term;                   /* Terminal information.        */
#endif


