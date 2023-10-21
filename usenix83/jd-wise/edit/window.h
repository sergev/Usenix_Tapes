/*
 * data structures for screen managment package
 */
#define editor 1

#define bad	(-1)
#define good	(0)

struct sline{
	struct window	*sl_window;	/* associated window */
	short	sl_first,sl_last;	/* first & last chars */
	short	sl_mfirst,sl_mlast;	/* first & last changes */
	short	sl_pfirst,sl_plast;	/* first printing char */
	short	sl_lno;			/* line number in file */
	short	sl_fileno;		/* file # */
	int*	sl_addr;		/* temporary file index */
	int	sl_staraddr;		/* temporary file address */
	int	sl_tl;			/* temp file block number */
	short	sl_flags;
	char	sl_mark;
	char	sl_at;
	char	sl_data[8];		/* on screen data field */
	char 	sl_text[82];		/* actual contents of line */
	};
#define sl_empty	0
#define sl_written	1
#define sl_contd	2
#define sl_conting	4
#define sl_modified	010		/* line changed since put on screen */
#define sl_insertline	020	/* a, c, or i comd being done on this line */
#define sl_rjust	040		/* right justified */

int	tabstart;			/* first tab column */
/*int	tabspace;			/* tab spacing */
#define intab 2

#define nfields	5
/*
 * as of 3-8-81 all line numbers are screen line numbers
 */
struct window{
	short	wi_size;		/* # of lines in window */
	short	wi_first,wi_last;	/* boundaries of window */
	short	wi_ufirst,wi_ulast;	/* first and last lines used */
	short	wi_defline;		/* default line */
	short	wi_statline;		/* status line */
	short	wi_cmdline;		/* command line */
	char*	wi_prompt;
	short	wi_format;
	short	wi_ldata;		/* length of data field */
	short	wi_ltext;		/* length of text field */
	short	wi_textcol;		/* first text column */
	short	wi_lmarg;		/* left margin */
	short	wi_rmarg;		/* right margin */
	short	wi_field[nfields];	/* stat line fields */
	short	wi_fileno;		/* associated file # */
	short	wi_type;
	short	wi_flags;
	short	wi_curx,wi_cury;	/* current location in window */
	short	wi_index;		/* window # */
	int*	wi_zero;
	int*	wi_dot;
	int*	wi_dol;
	unsigned wi_nlall;
	};
/* window flags */
#define wi_active	1		/* window is currently displayed */
#define wi_hidden	2		/* window is totally covered by another */
#define wi_showdata	4		/* display line #'s and marks */
/* window types */
#define wi_comd		1		/* command line window */
#define wi_edit		2		/* editing window */
#define wi_shell	3		/* shell window */

/* the screen */
struct sline screen[35];

/* the windows */
#define maxwindows 6
struct window window[maxwindows];
int nwindows;	/* the number of edit windows currently on screen */

/* functions */
struct window *addwindow();
