int nlines;
#define ed_defline (2*nlines/3)
#define errcol 0
#define statline nlines
#define	errline (nlines+1)
#define cmdline (nlines+2)

#define ed_lnocol 3
#define ed_markcol 4
#define ed_atcol 5
#define ed_textcol 7
#define ed_lline 72

#define cmdcol 2

#define cintr '\177'
#define cquit '\034'
#define eol 0300

extern getfile();


#define notext 1

#define left 0
#define right 1

int	tabstops[10];
/*int	firstused,lastused;*/
int	inline;
/*int	sol;*/
int	cmlptr;
/*int	expsize;	/* maximum number of lines to expand by */

#define cmdmode 0
#define insmode 1
#define modmode 2
int	mode;

char prompt[];

#define escpath 1
#define findpath 2
#define inspath 3
int	modpath;
int	placemark;

char	saveword[80];
int	swptr;

#define next goto loop

struct window *w[6];
#define cmdwindow 5
#define helpwindow 4
#define shellwindow 0

struct window *wc;	/* the current window */
int	fc;		/* the current file # */

int	helping;	/* currently in help function */
int	askhelp;	/* print initial "type help" message */

/* type definitions for subroutines */
int	*makeslot();
int	*finmon();

char	*getenv();
char	*sbrk();
char	*getline();
char	*getblock();
char	*itos();
char	*place();
char	*ttyname();
char	*showctl();
char	*strcopy();
char	*strcpy();

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))
