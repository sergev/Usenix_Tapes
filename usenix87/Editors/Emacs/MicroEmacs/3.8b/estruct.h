/*	ESTRUCT:	Structure and preprocesser defined for
			MicroEMACS 3.8

			written by Dave G. Conroy
			modified by Steve Wilhite, George Jones
			greatly modified by Daniel Lawrence
*/

#ifdef	LATTICE
#undef	LATTICE		/* don't use their definitions...use ours	*/
#endif
#ifdef	MSDOS
#undef	MSDOS
#endif
#ifdef	CPM
#undef	CPM
#endif
#ifdef	AMIGA
#undef	AMIGA
#endif

/*	Version definition			*/

#define	VERSION	"3.8b"

/*	Machine/OS definitions			*/

#define AMIGA   0                       /* AmigaDOS			*/
#define ST520   0                       /* ST520, TOS                   */
#define MSDOS   0                       /* MS-DOS                       */
#define V7      0                       /* V7 UN*X or Coherent or BSD4.2*/
#define	BSD	0			/* UNIX BSD 4.2	and ULTRIX	*/
#define	USG	1			/* UNIX system V		*/
#define VMS     0                       /* VAX/VMS                      */
#define CPM     0                       /* CP/M-86                      */
#define	FINDER	0			/* Macintosh OS			*/

/*	Compiler definitions			*/
#define MWC86   0	/* marc williams compiler */
#define	LATTICE	0	/* Lattice 2.14 thruough 3.0 compilers */
#define	AZTEC	0	/* Aztec C 3.20e */
#define	MSC	0	/* MicroSoft C compile version 3 */
#define MEGAMAX 0	/* Megamax C compiler */

/*	Profiling options	*/
#define	APROF	0	/* turn Aztec C profiling on? */
#define	NBUCK	100	/* number of buckets to profile */
#define	RAMSIZE	0	/* dynamic RAM memory usage tracking */
#define	RAMSHOW	0	/* auto dynamic RAM reporting */

/*   Special keyboard definitions            */

#define WANGPC	0		/* WangPC - mostly escape sequences     */
 
/*	Terminal Output definitions		*/

#define ANSI    0			/* ansi escape sequences	*/
#define	HP150	0			/* HP150 screen driver		*/
#define	HP110	0			/* HP110 screen driver		*/
#define	VMSVT	0			/* various VMS terminal entries	*/
#define VT52    0                       /* VT52 terminal (Zenith).      */
#define VT100   0                       /* Handle VT100 style keypad.   */
#define LK201   0                       /* Handle LK201 style keypad.   */
#define RAINBOW 0                       /* Use Rainbow fast video.      */
#define TERMCAP 1                       /* Use TERMCAP                  */
#define	IBMPC	0			/* IBM-PC CGA/MONO driver	*/
#define	EGA	0			/* EGA IBM-PC specific driver	*/
#define	DG10	0			/* Data General system/10	*/
#define	TIPC	0			/* TI Profesional PC driver	*/
#define	MAC	0			/* Macintosh			*/
#define	ATARI	0			/* Atari 520/1040ST screen	*/

/*	Configuration options	*/

#define CVMVAS  1	/* arguments to page forward/back in pages	*/
#define	NFWORD	1	/* forward word jumps to beginning of word	*/
#define	CLRMSG	0	/* space clears the message line with no insert	*/
#define	ACMODE	1	/* auto CMODE on .C and .H files		*/
#define	CFENCE	1	/* fench matching in CMODE			*/
#define	TYPEAH	1	/* type ahead causes update to be skipped	*/
#define DEBUGM	1	/* Global SPELL mode triggers macro debugging	*/
#define	VISMAC	0	/* update display during keyboard macros	*/
#define	CTRLZ	0	/* add a ^Z at end of files under MSDOS only	*/

#define	REVSTA	1	/* Status line appears in reverse video		*/
#define	COLOR	0	/* color commands and windows			*/

#define	FILOCK	0	/* file locking under unix BSD 4.2		*/
#define	ISRCH	1	/* Incremental searches like ITS EMACS		*/
#define	WORDPRO	1	/* Advanced word processing features		*/
#define	FLABEL	0	/* function key label code			*/
#define	APROP	1	/* Add code for Apropos command			*/
#define	CRYPT	1	/* file encryption enabled?			*/
#define MAGIC	1	/* include regular expression matching?		*/

#define ASCII	1	/* always using ASCII char sequences for now	*/
#define EBCDIC	0	/* later IBM mainfraim versions will use EBCDIC	*/

/*	System dependant library redefinitions, structures and includes	*/

/*  Megamax C defines short to be 1 byte and int to be 2 bytes */
#if	MEGAMAX & ST520
#define SHORT int
#else
#define SHORT short
#endif

#if	AZTEC
#undef	fputc
#undef	fgetc
#define	fputc	aputc
#define	fgetc	agetc
#define	int86	sysint
#define	intdos(a, b)	sysint(33, a, b)
#define	inp	inportb

struct XREG {
	int ax,bx,cx,dx,si,di;
};

struct HREG {
	char al,ah,bl,bh,cl,ch,dl,dh;
};

union REGS {
	struct XREG x;
	struct HREG h;
};
#endif

#if	MSDOS & MWC86
#include	<dos.h>
#define	int86(a, b, c)	intcall(b, c, a)
#define	inp	in

struct XREG {
	int ax,bx,cx,dx,si,di,ds,es,flags;
};

struct HREG {
	char al,ah,bl,bh,cl,ch,dl,dh;
	int ds,es,flags;
};

union REGS {
	struct XREG x;
	struct HREG h;
};
#endif

#if	MSDOS & MSC
#include	<dos.h>
#define	movmem(a, b, c)		memcpy(b, a, c)
#endif

#if	MSDOS & LATTICE
#undef	CPM
#undef	LATTICE
#include	<dos.h>
#undef	CPM
#endif

#if	VMS
#define	unlink(a)	delete(a)
#endif

/*	define memory mapped flag	*/

#define	MEMMAP	0

#if	IBMPC
#undef	MEMMAP
#define	MEMMAP	1
#endif

#if	IBMMONO
#undef	MEMMAP
#define	MEMMAP	1
#endif

#if	EGA
#undef	MEMMAP
#define	MEMMAP	1
#endif

/*	internal constants	*/

#define	NBINDS	256			/* max # of bound keys		*/
#define NFILEN  80                      /* # of bytes, file name        */
#define NBUFN   16                      /* # of bytes, buffer name      */
#define NLINE   256                     /* # of bytes, line             */
#define	NSTRING	128			/* # of bytes, string buffers	*/
#define NKBDM   256                     /* # of strokes, keyboard macro */
#define NPAT    128                     /* # of bytes, pattern          */
#define HUGE    1000                    /* Huge number                  */
#define	NLOCKS	100			/* max # of file locks active	*/
#define	NCOLORS	8			/* number of supported colors	*/
#define	KBLOCK	250			/* sizeof kill buffer chunks	*/
#define	NBLOCK	16			/* line block chunk size	*/
#define	NVSIZE	10			/* max #chars in a var name	*/

#if	0
#define AGRAVE  0x60                    /* M- prefix,   Grave (LK201)   */
#define METACH  0x1B                    /* M- prefix,   Control-[, ESC  */
#define CTMECH  0x1C                    /* C-M- prefix, Control-\       */
#define EXITCH  0x1D                    /* Exit level,  Control-]       */
#define CTRLCH  0x1E                    /* C- prefix,   Control-^       */
#define HELPCH  0x1F                    /* Help key,    Control-_       */
#endif

#define CTRL    0x0100                  /* Control flag, or'ed in       */
#define META    0x0200                  /* Meta flag, or'ed in          */
#define CTLX    0x0400                  /* ^X flag, or'ed in            */
#define	SPEC	0x0800			/* special key (function keys)	*/

#define FALSE   0                       /* False, no, bad, etc.         */
#define TRUE    1                       /* True, yes, good, etc.        */
#define ABORT   2                       /* Death, ^G, abort, etc.       */
#define	FAILED	3			/* not-quite fatal false return	*/
#define	RET	4			/* a return from buffer		*/
#define	GOLINE	5			/* exit flagging a GOTO		*/

#define	STOP	0			/* keyboard macro not in use	*/
#define	PLAY	1			/*		  playing	*/
#define	RECORD	2			/*		  recording	*/

#define	FORWARD	0			/* forward direction		*/
#define REVERSE	1			/* backwards direction		*/

#define FIOSUC  0                       /* File I/O, success.           */
#define FIOFNF  1                       /* File I/O, file not found.    */
#define FIOEOF  2                       /* File I/O, end of file.       */
#define FIOERR  3                       /* File I/O, error.             */
#define	FIOLNG	4			/* line longer than allowed len	*/
#define	FIOFUN	5			/* File I/O, eod of file/bad line*/

#define CFCPCN  0x0001                  /* Last command was C-P, C-N    */
#define CFKILL  0x0002                  /* Last command was a kill      */

#define	BELL	0x07			/* a bell character		*/
#define	TAB	0x09			/* a tab character		*/

#if	V7 | USG | BSD
#define	PATHCHR	':'
#else
#define	PATHCHR	';'
#endif

#define	INTWIDTH	sizeof(int)/3 + 2

/*	Macro argument token types					*/

#define	TKNUL	0			/* end-of-string		*/
#define	TKARG	1			/* interactive argument		*/
#define	TKBUF	2			/* buffer argument		*/
#define	TKVAR	3			/* user variables		*/
#define	TKENV	4			/* environment variables	*/
#define	TKFUN	5			/* function....			*/
#define	TKDIR	6			/* directive			*/
#define	TKLBL	7			/* line label			*/
#define	TKLIT	8			/* numeric literal		*/
#define	TKSTR	9			/* quoted string literal	*/
#define	TKCMD	10			/* command name			*/

/*	Internal defined functions					*/

/* DIFCASE represents the integer difference between upper
   and lower case letters.  It is an xor-able value, which is
   fortunate, since the relative positions of upper to lower
   case letters is the opposite of ascii in ebcdic.
*/

#ifdef	islower
#undef	islower
#endif

#if	ASCII

#define	DIFCASE		0x20
#define isletter(c)	(('a' <= c && 'z' >= c) || ('A' <= c && 'Z' >= c))
#define islower(c)	(('a' <= c && 'z' >= c))
#endif

#if	EBCDIC

#define	DIFCASE		0x40
#define isletter(c)	(('a' <= c && 'i' >= c) || ('j' <= c && 'r' >= c) || ('s' <= c && 'z' >= c) || ('A' <= c && 'I' >= c) || ('J' <= c && 'R' >= c) || ('S' <= c && 'Z' >= c))
#define islower(c)	(('a' <= c && 'i' >= c) || ('j' <= c && 'r' >= c) || ('s' <= c && 'z' >= c))
#endif

/*	Dynamic RAM tracking and reporting redefinitions	*/

#if	RAMSIZE
#define	malloc	allocate
#define	free	release
#endif

/*
 * There is a window structure allocated for every active display window. The
 * windows are kept in a big list, in top to bottom screen order, with the
 * listhead at "wheadp". Each window contains its own values of dot and mark.
 * The flag field contains some bits that are set by commands to guide
 * redisplay; although this is a bit of a compromise in terms of decoupling,
 * the full blown redisplay is just too expensive to run for every input
 * character.
 */
typedef struct  WINDOW {
        struct  WINDOW *w_wndp;         /* Next window                  */
        struct  BUFFER *w_bufp;         /* Buffer displayed in window   */
        struct  LINE *w_linep;          /* Top line in the window       */
        struct  LINE *w_dotp;           /* Line containing "."          */
        short   w_doto;                 /* Byte offset for "."          */
        struct  LINE *w_markp;          /* Line containing "mark"       */
        short   w_marko;                /* Byte offset for "mark"       */
        char    w_toprow;               /* Origin 0 top row of window   */
        char    w_ntrows;               /* # of rows of text in window  */
        char    w_force;                /* If NZ, forcing row.          */
        char    w_flag;                 /* Flags.                       */
#if	COLOR
	char	w_fcolor;		/* current forground color	*/
	char	w_bcolor;		/* current background color	*/
#endif
}       WINDOW;

#define WFFORCE 0x01                    /* Window needs forced reframe  */
#define WFMOVE  0x02                    /* Movement from line to line   */
#define WFEDIT  0x04                    /* Editing within a line        */
#define WFHARD  0x08                    /* Better to a full display     */
#define WFMODE  0x10                    /* Update mode line.            */
#define	WFCOLR	0x20			/* Needs a color change		*/

/*
 * Text is kept in buffers. A buffer header, described below, exists for every
 * buffer in the system. The buffers are kept in a big list, so that commands
 * that search for a buffer by name can find the buffer header. There is a
 * safe store for the dot and mark in the header, but this is only valid if
 * the buffer is not being displayed (that is, if "b_nwnd" is 0). The text for
 * the buffer is kept in a circularly linked list of lines, with a pointer to
 * the header line in "b_linep".
 * 	Buffers may be "Inactive" which means the files accosiated with them
 * have not been read in yet. These get read in at "use buffer" time.
 */
typedef struct  BUFFER {
        struct  BUFFER *b_bufp;         /* Link to next BUFFER          */
        struct  LINE *b_dotp;           /* Link to "." LINE structure   */
        short   b_doto;                 /* Offset of "." in above LINE  */
        struct  LINE *b_markp;          /* The same as the above two,   */
        short   b_marko;                /* but for the "mark"           */
        struct  LINE *b_linep;          /* Link to the header LINE      */
	char	b_active;		/* window activated flag	*/
        char    b_nwnd;                 /* Count of windows on buffer   */
        char    b_flag;                 /* Flags                        */
	char	b_mode;			/* editor mode of this buffer	*/
        char    b_fname[NFILEN];        /* File name                    */
        char    b_bname[NBUFN];         /* Buffer name                  */
#if	CRYPT
	char	b_key[NPAT];		/* current encrypted key	*/
#endif
}       BUFFER;

#define BFINVS  0x01                    /* Internal invisable buffer    */
#define BFCHG   0x02                    /* Changed since last write     */

/*	mode flags	*/
#define	NUMMODES	8		/* # of defined modes		*/

#define	MDWRAP	0x0001			/* word wrap			*/
#define	MDCMOD	0x0002			/* C indentation and fence match*/
#define	MDSPELL	0x0004			/* spell error parcing		*/
#define	MDEXACT	0x0008			/* Exact matching for searches	*/
#define	MDVIEW	0x0010			/* read-only buffer		*/
#define MDOVER	0x0020			/* overwrite mode		*/
#define MDMAGIC	0x0040			/* regular expresions in search */
#define	MDCRYPT	0x0080			/* encrytion mode active	*/

/*
 * The starting position of a region, and the size of the region in
 * characters, is kept in a region structure.  Used by the region commands.
 */
typedef struct  {
        struct  LINE *r_linep;          /* Origin LINE address.         */
        short   r_offset;               /* Origin LINE offset.          */
        long	r_size;                 /* Length in characters.        */
}       REGION;

/*
 * All text is kept in circularly linked lists of "LINE" structures. These
 * begin at the header line (which is the blank line beyond the end of the
 * buffer). This line is pointed to by the "BUFFER". Each line contains a the
 * number of bytes in the line (the "used" size), the size of the text array,
 * and the text. The end of line is not stored as a byte; it's implied. Future
 * additions will include update hints, and a list of marks into the line.
 */
typedef struct  LINE {
        struct  LINE *l_fp;             /* Link to the next line        */
        struct  LINE *l_bp;             /* Link to the previous line    */
        short   l_size;                 /* Allocated size               */
        short   l_used;                 /* Used size                    */
        char    l_text[1];              /* A bunch of characters.       */
}       LINE;

#define lforw(lp)       ((lp)->l_fp)
#define lback(lp)       ((lp)->l_bp)
#define lgetc(lp, n)    ((lp)->l_text[(n)]&0xFF)
#define lputc(lp, n, c) ((lp)->l_text[(n)]=(c))
#define llength(lp)     ((lp)->l_used)

/*
 * The editor communicates with the display using a high level interface. A
 * "TERM" structure holds useful variables, and indirect pointers to routines
 * that do useful operations. The low level get and put routines are here too.
 * This lets a terminal, in addition to having non standard commands, have
 * funny get and put character code too. The calls might get changed to
 * "termp->t_field" style in the future, to make it possible to run more than
 * one terminal type.
 */
typedef struct  {
	short	t_mrow;			/* max number of rows allowable */
        short   t_nrow;                 /* current number of rows used  */
        short   t_mcol;                 /* max Number of columns.       */
        short   t_ncol;                 /* current Number of columns.   */
	short	t_margin;		/* min margin for extended lines*/
	short	t_scrsiz;		/* size of scroll region "	*/
	int	t_pause;		/* # times thru update to pause */
        int     (*t_open)();            /* Open terminal at the start.  */
        int     (*t_close)();           /* Close terminal at end.       */
	int	(*t_kopen)();		/* Open keyboard		*/
	int	(*t_kclose)();		/* close keyboard		*/
        int     (*t_getchar)();         /* Get character from keyboard. */
        int     (*t_putchar)();         /* Put character to display.    */
        int     (*t_flush)();           /* Flush output buffers.        */
        int     (*t_move)();            /* Move the cursor, origin 0.   */
        int     (*t_eeol)();            /* Erase to end of line.        */
        int     (*t_eeop)();            /* Erase to end of page.        */
        int     (*t_beep)();            /* Beep.                        */
	int	(*t_rev)();		/* set reverse video state	*/
	int	(*t_rez)();		/* change screen resolution	*/
#if	COLOR
	int	(*t_setfor)();		/* set forground color		*/
	int	(*t_setback)();		/* set background color		*/
#endif
}       TERM;

/*	TEMPORARY macros for terminal I/O  (to be placed in a machine
					    dependant place later)	*/

#define	TTopen		(*term.t_open)
#define	TTclose		(*term.t_close)
#define	TTkopen		(*term.t_kopen)
#define	TTkclose	(*term.t_kclose)
#define	TTgetc		(*term.t_getchar)
#define	TTputc		(*term.t_putchar)
#define	TTflush		(*term.t_flush)
#define	TTmove		(*term.t_move)
#define	TTeeol		(*term.t_eeol)
#define	TTeeop		(*term.t_eeop)
#define	TTbeep		(*term.t_beep)
#define	TTrev		(*term.t_rev)
#define	TTrez		(*term.t_rez)
#if	COLOR
#define	TTforg		(*term.t_setfor)
#define	TTbacg		(*term.t_setback)
#endif

/*	structure for the table of initial key bindings		*/

typedef struct  {
        short   k_code;                 /* Key code                     */
        int     (*k_fp)();              /* Routine to handle it         */
}       KEYTAB;

/*	structure for the name binding table		*/

typedef struct {
	char *n_name;		/* name of function key */
	int (*n_func)();	/* function name is bound to */
}	NBIND;

/*	The editor holds deleted text chunks in the KILL buffer. The
	kill buffer is logically a stream of ascii characters, however
	due to its unpredicatable size, it gets implemented as a linked
	list of chunks. (The d_ prefix is for "deleted" text, as k_
	was taken up by the keycode structure)			*/

typedef	struct KILL {
	struct KILL *d_next;	/* link to next chunk, NULL if last */
	char d_chunk[KBLOCK];	/* deleted text */
} KILL;

