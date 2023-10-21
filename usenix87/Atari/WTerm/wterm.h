#define N_COL		80 /* num of char to be displayed  in window */
#define N_ROW		24 /* how many numbers in stack to display in window */

#define	CURSCNT		2 /* how long to wait between displays of cursor */
			   /* has to be done because of slowness of some */
			   /* of the VDI/AES calls... damn. */
/*
 * The following several defines are used by Tek 4010 mode.
 */
#define GRAPH   1
#define ALPHA   0
#define GS      0x1d
#define US      0x1f

#define ESC	0x1b	/* Terminal Command initiator */
#define	CE	'K'	/* Clear to end of line */
#define	CL	'E'	/* Clear whole screen */
#define CM	'Y'	/* Postion cursor -- followed by two chars */
#define	DO	'B'	/* cursor down */
#define	HO	'H'	/* Home cursor */
#define	LE	'D'	/* cursor left */
#define	ND	'C'	/* cursor right */
#define	UP	'A'	/* cursor up */
#define SC	'j'	/* Save Cursor location */
#define	RC	'k'	/* Restore Cursor */
#define SO	'p'	/* StandOut mode */
#define SE	'q'	/* StandEnd mode */
#define	TS	0x0e	/* Control-N --> to status line */
#define	FS	0x0f	/* Control-O --> from Status line */
#define FF	0x0c	/* Form Feed for TEK 4010 mode */

#define	UNDO	0x61
#define	HELP	0x62
#define	HOME	0x47
#define	UARROW	0x48
#define	LARROW	0x4b
#define	DARROW	0x50
#define	RARROW	0x4d

#define	B300	9
#define	B1200	7
#define	B2400	4
#define	B4800	2
#define	B9600	1

#ifndef EXTERN
extern	int	gl_apid;
extern int	Handle;
extern int	Ix, Iy, Iw, Ih; /* inner size of window */

extern int	Xdesk, Ydesk, Wdesk, Hdesk;
extern int	CharW, CharH;	/* character cell width and height */
extern int	Baseline;	/* Bottom line relative to baseline */

extern char	Screen[N_ROW+1][1+N_COL];		/* image of whole screen */
extern int	Scr_col, Scr_row;
extern int	Sysl_row, Sysl_col, Sysline;

extern int	command, comd_pos;
extern int	Half;	/* for half/full duplex */
extern int	T_mode, T_count, T_draw;
extern int	T_x, T_y;

extern int	contrl[12];
extern int	intin[128];
extern int	ptsin[128];
extern int	intout[128];
extern int	ptsout[128];
#endif
