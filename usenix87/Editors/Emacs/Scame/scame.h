/*	SCAME scame.h				*/

/*	Revision 1.0.1  1985-02-23		*/

/*	Copyright (C) 1985 by Leif Samuelsson	*/


# include "config.h"
# include <stdio.h>
# include <signal.h>
# include <sys/types.h>
# include <sys/stat.h>

typedef int (*funcp)();
typedef char Bool;
/* typedef enum {FALSE, TRUE} Bool; */

/* Some useful macros */
# define min(I,J) ((I) < (J) ? (I) : (J))
# define max(I,J) ((I) > (J) ? (I) : (J))
# define abs(X) ((X) < 0 ? -(X) : (X))
# define sign(X) ((X) >= 0 ? 1 : -1)
# define pswap(I,J) { char *h; h=I; I=J; J=h; }
#ifdef COMMENT
# define upcase(c,nat) ((((c) & 0177) >= 'a' && \
	(((c) & 0177) <= ((nat) ? '}' : 'z'))) ? (c & 0337) : (c))
# define lowcase(c,nat) ((nat) ? nlocasarr[c] : locasearr[c])
#endif
# define upcase(c,nat)  ((nat) ? nupcasarr[c] : upcasearr[c])
# define lowcase(c,nat) ((((c) & 0177) >= 'A' && \
	(((c) & 0177) <= ((nat) ? ']' : 'Z'))) ? (c | 0040) : (c))
# define NIL (char *) 0
# define FALSE '\0'
# define TRUE '\001'
# define GET 0
# define PUT 1
# define BELL '\007'
# define ESC '\033'
# define DEL '\177'
# define FILENAMESIZE 80
# define MAXBUFS 20
# define BUFNAMESIZE 10
# define GAPSIZE 1000L
# define GAPDIST SCRDIMY*SCRDIMX
# define LASTINPUTSIZE 60	/* Number of input characters remembered */
				/* for the HELP L function */
# define TABWID 8

/* Error codes */
enum errcode_t {A2W, BF, BTF, CCF, FTB, FTF, ILA, ILQ, MTF, NIB, NKM, NSB,
		NSF, NYI, O1W, RKM, SSF, UBK};

/* Major Modes */
enum majormode_t {ADA, C, FUNDAMENTAL, LISP, PASCAL, SWEDISH};

/* Minor Modes */
struct minmodstruct {
	long	autofill,
		overwrite
};

struct gvarstruct {		/* Global Variables */
	long	auto_push_point_option,
		bottom_display_margin,
		cbreak_mode,
		commcol,
		hackmatic,	/* Set by ME in termcap if term has Meta-key */
		mail_check_interval,
		quote_char,
		search_backward,
		search_exit_char,
		search_forward,
		system_output_holding,	/* XX in termcap */
		tab_self_insert,
		top_display_margin;
};

struct tabstruct {
	char *name;
	char *pointer;
};

struct comstruct { char *name; int (*funcp)(); };

extern struct comstruct x_comtab[];

extern funcp disptab[512], c_x_disptab[128], m_o_disptab[128];


/* various quantities */
enum quantity_t {WORD, LINE, SENTENCE, REGION, PARAGRAPH, BUFFER};

struct lvarstruct {
	long	disp_matching_paren,
		fillcolumn;
};

struct bufstruct {
	char name[BUFNAMESIZE];
	char filename[FILENAMESIZE];
	int dot;
	int home;
	enum majormode_t majormode;
	Bool modified;
	time_t mtime;
	struct minmodstruct minormodes;
	struct lvarstruct lvars;
};

extern char 	*sbrk(), *getenv(), *getenv(), *tgetstr();
extern FILE	*popen();

/* variables defined in main.c */
extern char	currentdir[], commandprompt[], *dot, *buf, *z,
		*home, *oldhome, screen[SCRDIMY][SCRDIMX+1],
		oldscreen[SCRDIMY][SCRDIMX+1], *modes[], tempfile[], lastinput[],
		more[], *mark[], fillprefix[], tmpmode_string[], scamelib[],
		*otherdot, *otherhome, *gaps, *gape, kbdmacfile[],
		killbuffile[], *away, *username, mailfile[];
extern Bool	killing, typing, xarg, xxarg, defining_kbd_mac, quiet,
		updateflag, echobusy, control_prefix, meta_prefix, savearg,
		pop_pending, savehpos;
extern int	cmdchar, screenlen, screenwidth, cury, curx, linpos,
		noofbufs, bufno, oldbuf, pipeup[], pipedown[], ppid, uid,
		lstindex, wintop, winbot, windows, otherbuf,
		upcasearr[], nupcasarr[],
		kbdmfd, execfd, oldhpos, recursive_level;
extern long	arg, bufsize, chkmailcnt;
extern struct bufstruct buftab[], cb;
extern struct gvarstruct gvars;



/* procedures defined in scame0.c */
extern		echo(), outchar(), cur(), update(), tmodlin(), stty(),
		findxy(), delchar(), insertstr(), refresh(), pushpopmark(),
		prevscreen(), closegap(), whatcursorposition(),
		comment(), extended(), markquant(), rot13_region(), stop(),
		findmode(),insertfile(), revertfile();
extern int	inchar(), instring();
extern long	backchar();
extern Bool	insertc(), getfilename(), incbuf(), gapto();

/* variables in scame1.c */
extern char	oldsstr[];


/* procedures defined in scame1.c */
extern		clearecho(), selectbuffer(),
		exec(), tabify(), savescame(), help(),
		listbuffers(), fixbuftab(), findfile(), kbuffer(),
		hangup(), vfile(), apropos();
extern Bool	restart(), wtmp(), rtmp(), typeout(), getbuffer();


/* procedures defined in search.c */
extern void	isearch(), queryreplace(), replace();
extern Bool	search();


/* variables defined in io.c */
extern int 	hpos, vpos;
extern char	*AL, *CE, *CS, *DL;

/* procedures defined in io.c */
extern		setupterm(), inittypeaheadcheck(), cls(), cleol(), cur(),
		pchar(), pstring(), outchar(), strnout(), strnprep(),
		invmod(), vt100a(), ttypad(), errmes(), echo(), push(),
 		unget(), gettermtype(), ttycbreak(), blockmove(), vtscroll();
extern Bool	typeahead();
extern int 	instring(), inchar(), getline();
extern short	yesorno();
extern char	*func_name(), *key_name();
extern funcp	getfuncname();
/* variables in scame2.c */
extern char	oldshellcommand[], oldmailname[];

/* procedures in scame2.c  */
extern 	changedir(), viewworkingdir(), delblanklines(),
	shellcommand(), flush_matching_lines(),
	onewindow(), twowindows(), otherwindow(), scrollotherwindow(),
	growwindow(), sortbuf(), saveforundo(), undo(), viewfile(),
	writeregion(), fillregion(), justifyregion(), setfcol(),
	breakline();

/* macro.c */

extern	definekbdmac(), savekbdmac(), loadkbdmac(), execfile();

/* x_funcs.c */
extern	x_adamode(), x_autofillmode(), x_ovwrtmode(), x_cbreakmode(),
	x_checkmail(), x_cmode(), x_dired(),
	x_flush_matching_lines(), x_fundamentalmode(),
	x_inschr(), x_insertfile(), x_lispmode(), x_loadkbdmac(),
	x_pascalmode(), x_replace(), x_revert_file(),
	x_savekbdmac(), x_set_key(),
	x_setvariable(), x_sortbuf(),
	x_swedishmode(), x_tabify(), x_viewfile(),
	x_wallchart(), x_rot13_region();

/* vars.c */
extern	setvariable(), viewvariable();

/* files.c */
extern		buildfilename(), cleanfilename();
extern Bool	fileexists(), filnprep(), filncpy(), filncat();
extern time_t	filemodified();
extern off_t	filesize();
extern char	*getwd();

/* Procedures in strings.c */

extern		basename(), initupcase();
extern int	strsize(), strsub();
extern Bool	strneq(), isblank(), endofquantp();
extern char	*index(), *rindex();
