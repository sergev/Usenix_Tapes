/* Header file for all procedures in Vsh */
#include "stdio.h"

/* #define	PWBTTY		1		/* Some local stuff */
/* #define	USGTTY		1		/* 3.0-5.0 drivers */
#define	V7TTY		1		/* V7 - ?.BSD drivers */
/* #define	V6		1		/* Version 6 stuff */
#define	NDIR		1		/* New 4.2BSD directories */
#define	SYMLINK		1		/* 4.2BSD symbolic links */
#define	VFORK		1		/* Has vfork() */
/* #define	MACMAJOR	1		/* types.h missing major() */

/*
 * Buffer size for backing up more(). This buffer is malloc()'d in page()
 * unless MBUFSTACK is defined. PDP-11's and the like should probably
 * define MBUFSTACK, otherwise less data space will be available for
 * directories. page() will try to malloc() less, if reasonable.
 */
/*#define	MBUFSTACK	1		/* Put more buffer on stack */
#define	MBUFSIZE	7000		/* Used only with MBUFSTACK */
#define	MMAXSIZE	"10000"			/* To init param moresize */

#define VERSION	"4.2"

#define max(arg1,arg2)	((arg1 > arg2) ? arg1 : arg2)
#define min(arg1,arg2)	((arg1 < arg2) ? arg1 : arg2)
#define compe(arg1,arg2)  (strcmp (arg1, arg2) == 0)

/* Standard file numbers */
#define infile		0
#define outfile		1
#define errorfile	2

/* The values of special keys */
#define EOT		4
#define	RUBOUT		0177
#define	CR		015
#define	LF		012

/* Directory commands */
#define	CTLF	006
#define	CTLL	014
#define	CTLU	025
#define	TABCMD	011			/* Multi-column shift */

/* Emacs select positioning */
#define	ESUP	020			/* ^P */
#define	ESDN	016			/* ^N */
#define	ESLF	002			/* ^B */
#define	ESRT	006			/* ^F */
#define	ESBS	001			/* ^A back series */
#define	ESFS	005			/* ^E forward series */
#define	ESNP	026			/* ^V next page */

/* Display offsets */
#define	OFFFILE	5
#define	OFFARROW 3

/* Standard file names */
#define LOGFILE		"/usr/adm/vsh.log"
#define DEBUGGER	"/bin/adb"

/* Other parameters */
#define	STRMAX	120		/* Length of string buffers */
#define CNULL	((char *) 0)	/* Null char pointer */
#define ARGVMAX		20	/* Size of a readarg argv */
#define	PAGEMIN	5		/* Minimum page window */
#define	MAILCLK	(1*60)		/* Check for mail interval */

#define	VSHTOP	2		/* Lines needed at top of display */
#define	VSHBOT	2		/* Lines needed at bottom of display */
#define	VSHLEFT	4		/* Spaces needed at left of display */

/* Parms loadable through .vshrc are accessed through command.h.
   Alter their default values in cmdini.c	*/

/* Tty_set parameters */
#define RAWMODE		0
#define	COOKEDMODE	1

/* Functions called by command return the next command to execute.  */
/* In addition, the following bits are returned.  */

#define CMDMASK		0x00ff	/* Bits of next command */
#define	REPLOT		0x0100	/* Must replot directory */
#define NOOP		0x0200	/* Return for command not found */
#define ENTERDIR	0x0400	/* New directory entered */

/* If no special return is necessary, use return REPLOT or NOREPLOT. */
#define NOREPLOT	0x0000

/* GOOD or BAD returns */
#define	GOOD	1
#define	BAD	0

/* When calling command, indicate type of command:  */
#define DIRCMD	1
#define SHOWCMD	2
extern char dircmds[];

/* Show operates in two modes */
#define GREPMODE	0
#define MAKEMODE	1

/* Select or enter mode */
#define	ENTERMODE	0
#define	SELECTMODE	's'
extern char selectname[STRMAX];
extern int selectcmd;
extern int selectpage;
extern int selecttype;

extern char username[];
extern char *arrow;
extern char *helptext;

extern int window, owindow, ewindow;
extern int xoff;			/* Column offset for vary CRT widths */

/*
 * Custom enter text info
 */
#define	ENTEREDIT	"$EDITOR"	/* Enter $EDITOR for text */
#define	ENTERDISP	"display"	/* Use internal display() for text */

/* Parameters from .vshrc file. P_name is the parameter name, p_val is
   the parameter's value.
*/

struct parmstruct {
	char *p_name,
	     *p_val;
};

extern struct parmstruct  parmtab[];

/* References to the various parameters */

#define	EDITOR		parmtab[0].p_val
#define	MAKE		parmtab[1].p_val
#define	GREP		parmtab[2].p_val
#define	RMHELP		parmtab[3].p_val
#define	SHOWHELP	parmtab[4].p_val
#define	MAKERROR	parmtab[5].p_val
#define	GREPOUT		parmtab[6].p_val
#define	VSHMODE		(*parmtab[7].p_val)
#define	QUITCHAR	(*parmtab[8].p_val)
#define	PAGECHAR	(*parmtab[9].p_val)
#define	PATH		parmtab[10].p_val
#define	TERM		parmtab[11].p_val
#define	HOME		parmtab[12].p_val
#define	SHELL		parmtab[13].p_val
#define	ENTERTEXT	parmtab[14].p_val
#define	WINDOW		parmtab[15].p_val
#define	HELPFILE	parmtab[16].p_val
#define	COLUMN		parmtab[17].p_val
#define	NOARG		(*parmtab[18].p_val)
#define	VIMOTION	(*parmtab[19].p_val)
#define	MAIL		parmtab[20].p_val
#define	MORESIZE	parmtab[21].p_val
#define	ENTERPATH	parmtab[22].p_val

/* Termcap */
extern char *BC, *UP, *CM, *CL, *tgoto();
extern char *CE, *SO, *SE;
extern short PC;
extern int CO, LI;
extern char *CS;
/* A new one, clear to beginning of display */
extern char *CB;
extern char *BO, *BE;
extern char *SR, *CD;
extern int AM, XN;
extern char *TI, *TE;
