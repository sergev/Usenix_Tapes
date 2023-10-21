#include <curses.h>
/*
 *	Rvi - Portable distributed screen editor (DSE).
 *	86/07/16.  Alan Klietz
 *	Copyright (c) 1986, Research Equipment Incorporated
 *                          Minnesota Supercomputer Center
 *
 * Permission is hereby granted to use this software on any computer system
 * and to copy this software, including for purposes of redistribution, subject
 * to the conditions that 
 *
 *  o  The full text of this copyright message is retained and prominently
 *      displayed
 *
 *  o  No misrepresentation is made as to the authorship of this software
 *
 *  o  The software is not used for resale or direct commercial advantage
 *
 *  By copying, installing, or using this software, the user agrees to abide
 *  by the above terms and agrees that the software is accepted on an "as is"
 *  basis, WITHOUT WARRANTY expressed or implied, and relieves Research Equip-
 *  ment Inc., its affiliates, officers, agents, and employees of any and all
 *  liability, direct of consequential, resulting from copying, installing
 *  or using this software.
 */
#ifdef CRAY
#	define un_firstline	un_fline
#	define un_firstcol	un_fcol
#	define sc_firstline	sc_fline
#	define sc_firstcol	sc_fcol
#	define rv_scroll_backward	rv_scr_bk
#endif

/*
 * Make initial program
 * Alan Klietz, Nov 11, 1985.
 */

/*
 * Definitions
 */
#ifndef CTRL
#define CTRL(c)	 ('c'&037)
#endif

#ifdef CRAY
#	define	INT	long  /* for speed */
#	define	boolean	long  /* for speed */
#else
#	define	INT	int
#	define	boolean	char
#endif

#ifdef USG
#define	zero(ptr, len)		memset(ptr, 0, len)
#define copy(to, from, len)	memcpy(to, from, len)
#endif

#define CURLINE			(stdscr->_cury)
#define CURCOLUMN		(stdscr->_curx)

#define COL_FIRST_NONWHITE	(-1023)	/* Column cookie for move_cursor(),
					   move to first non-white char */
#define COL_SAME		(-1024) /* Column cookie for move_cursor(),
					   move to same column as previous */

#define RV_TIMEOUT	20		/* Seconds until ed request times out */


/*
 *  Five major structures,
 * 	 struct line     -  A line of text.
 *       struct yank     -  A yank buffer.
 *	 struct undo     -  The undo-last-change structure. 
 * 	 struct screen   -  The screen proper.
 * 	 struct window   -  Sliding window into file. (Larger than screen)
 *	 struct file	 -  General information about the file.
 */

struct li_line { /* A line of text */
	INT	li_width;	/* Width of this line */
	INT	li_segments;	/* Number of screen segments required */
	char	*li_text;	/* Pointer to actual text. (malloc) */
};

struct ya_yank { /* A yank buffer */
	INT	ya_type;	/* YANK_COLS, YANK_SINGLE, or YANK_LINES */
	char	*ya_text;	/* Pointer to text (malloc) */
	INT	ya_width;	/* # of bytes yanked, if YANK_COLS or
				   YANK_SINGLE */
	INT	ya_numlines;	/* # of lines yanked, if YANK_LINES */
	boolean	ya_madefile;	/* TRUE if tempfile was created */
};
#define YANK_EMPTY  0
#define YANK_COLS   1
#define YANK_SINGLE 2
#define YANK_LINES  3
#define NUM_YANK_BUFS  38   /* 0-9, a-z, '.', and $ */


struct un_undo { /* Structure for recalling last change */
	INT	un_firstline,		/* First line # of change */
		un_firstcol,		/* First column # of change */
		un_lastline,		/* Last line # of change */
		un_lastcol;		/* Last column # of change */
	boolean	un_validcol;		/* TRUE if first/last column defined */
	boolean	un_inserted;		/* TRUE if insertion was done */
	boolean	un_deleted;		/* TRUE if deleting was done */
};


struct sc_screen { /* The screen proper */
	INT	sc_column;		/* Cursor position in current line */
	INT	sc_lineno;		/* Current line number */
	struct  li_line *sc_topline;	/* Ptr to first line on screen */
	struct	li_line	*sc_curline;	/* Ptr to current line on screen */
	struct  li_line *sc_botline;	/* Ptr to last line on screen */
	INT	sc_abovetop;		/* # of segments above the top line
					   after a downwards scroll */
	INT	sc_firstline,		/* First line # of operation */
		sc_firstcol,		/* First column # of operation */
		sc_lastline,		/* Last line # of operation */
		sc_lastcol;		/* Last column # of operation */
	boolean	sc_validcol;		/* TRUE if first/last column defined
					   above is valid */
	struct	li_line sc_origline;	/* Current line before modification, if
					   not modified text is null. */
};

struct wi_window { /* Sliding window into file */
	struct	li_line	*wi_topline;	/* Ptr to first line in window */
	struct	li_line	*wi_botline;	/* Ptr to last line in window */
};

struct fi_file { /* General information about the file */
	INT	fi_numlines;	/* # of lines in file */
	boolean	fi_modified;	/* TRUE if file modified */
	boolean	fi_sysv;	/* TRUE if ed has System V 'H' command */
	boolean	fi_cray;	/* TRUE if ed has Cray style error msgs */
	FILE	*fi_fpin;	/* Input file pointer from ed */
	FILE	*fi_fpout;	/* Output file pointer to ed */
	char	fi_name[128];	/* Name of edited file */
};

/*
 * Global externals
 */
extern struct	fi_file		file;
extern struct	wi_window	window;
extern struct	sc_screen	screen;
extern struct	li_line		*line_array;
extern struct	ya_yank		yank_array[NUM_YANK_BUFS];
extern struct	un_undo		undo;

extern char	**Argv;
extern boolean	use_linemode;	/* TRUE to use line mode in shell escapes */
extern boolean	ed_undo;	/* TRUE if last mod was direct ed cmd */
extern INT	errflag;	/* 1 if error in function call */
extern boolean	input_mode;	/* TRUE if input mode */
extern boolean	change_flag;	/* TRUE if change in progress */
extern boolean	replace_flag;	/* TRUE if R command */
extern char	*nextfile;	/* Next file to edit via 'n' cmd */
extern INT	nwin_lines;	/* Number of window lines */
extern boolean	opened_line;	/* TRUE if openline() called */
extern INT	yank_shift;	/* Shift offset for yank */
extern INT	yank_cmd;	/* "x yank buf command letter x. */

extern boolean	set_autoindent; /* TRUE if margins automatically indented */
extern INT	set_debug;	/* Debugging level */
extern boolean	set_list;	/* TRUE if list mode set (show tabs and EOL) */
extern boolean	set_fortran;	/* TRUE if Fortran-style tabs */
extern INT	set_scroll;	/* # of lines per ^D or ^U */
extern INT	set_tabstops;	/* Tabstop width */
extern boolean	set_timeout;	/* Timeout on function key */
extern INT	set_shiftwidth;	/* # of columns per tab char on input */
extern boolean	set_wrapscan;	/* TRUE if wrap around on string search */

#define NUM_WINDOW_LINES	nwin_lines

void		botprint();	   /* Print msg on bottom line */
void		change();	   /* Change text */
void		delete();	   /* Delete text */
void		dup_insert();	   /* Duplicate inserted text n times */
INT		file_column();	   /* Convert screen column to file column */
void		insert();	   /* Insert text into current line */
void		move_abs_cursor(); /* Move to (lineno, column) from anywhere */
void		move_cursor();	   /* Set cursor to (lineno, column).  Must
				      already be on screen. */
void		openline();	   /* Open new line in specified direction */
void		panic();	   /* Print msg and halt */
void		put();		   /* Put text in specified direction */
void		print_line();	   /* Print text at current cursor */
void		quit();
boolean		read_cmd();	   /* Read a command character */
boolean		recv_sync();	   /* Receive sync */
void		redraw_screen();   /* Update screen to curses */
INT		rv_getchar();
char		*rv_getline();
void		save_Undo();	   /* Save current line to Undo buffer */
INT		screen_column();   /* Convert file column to screen column */
void		sizemsg();	   /* Print file size */
void		toss_undo();	   /* Forget last undo */
char		*xalloc();	   /* Allocate memory */
void		xmit_curline();	   /* Transmit current line if modified */
void		xmit_ed();	   /* Transmit command */
void		xmit_sync();	   /* Transmit sync */
void		yank();		   /* Yank text */
