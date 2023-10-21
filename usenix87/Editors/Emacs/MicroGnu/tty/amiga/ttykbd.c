/*
 * Name:	MicroEMACS
 * 		Amiga virtual terminal keyboard, default console keymap.
 * Version:	Gnu v30
 * Last edit:	25-Oct-86
 * Created:	19-Apr-86 ...!ihnp4!seismo!ut-sally!ut-ngp!mic
 *		This goes with the Intuition terminal driver,
 *		and implements Mike Meyer's hot mouse.
 */
#include	<exec/types.h>
#include	<intuition/intuition.h>
#undef	TRUE
#undef	FALSE
#include	"def.h"

#define	ESC	0x1B			/* Escape, arrows et al.	*/
#define	CSI	0x9B			/* Amiga CSI			*/

#ifdef	MOUSE
/* Stuff for the hot mouse.  Since right now the mouse
 * keys get bound into a wierd place, a lot of the
 * jiggery-pokery up here is to make it possible
 * to shift their location in the keymap without too
 * much fuss.  Sorry for the horrid macro names...
 */

/* key code w/o mini qualifiers */
#define	WMOUSE	((KEY)(KCTRL | KMETA | 'a'))
#define SMOUSE	((KEY)(KCTRL | KMETA | 'i'))
#define	EMOUSE	((KEY)(KCTRL | KMETA | 'q'))

/* mini qualifiers */
#define	SHFT	1
#define ALT	2
#define CTRL	4

/* macros to create qualified key codes */

#define	S(k)	(k + SHFT)
#define	A(k)	(k + ALT)
#define	C(k)	(k + CTRL)
#define	AS(k)	(k + SHFT + ALT)
#define	CS(k)	(k + CTRL + SHFT)
#define	CA(k)	(k + CTRL + ALT)
#define	CAS(k)	(k + SHFT + ALT + CTRL)
#endif /* ifdef MOUSE */

#ifdef	XKEYS
/*
 * The function keys on the Amiga send back
 * escape sequences of the form <ESC>[code~, where code
 * is a one or two-character code for the key.  To make
 * it easier to decode, we place the internal key values
 * for these codes in this table.
 */

short	consolemap[] = {
	KF1,		KF2,		KF3,		KF4,
	KF5,		KF6,		KF7,		KF8,
	KF9,		KF10,		KSF1,		KSF2,
	KSF3,		KSF4,		KSF5,		KSF6,
	KSF7,		KSF8,		KSF9,		KSF10
};
#define	NFUNCKEYS ((sizeof consolemap)/(sizeof consolemap[0]))
#endif

/*
 * Names for the keys with basic keycode
 * between KFIRST and KLAST (inclusive). This is used by
 * the key name routine in "kbd.c".  KFIRST is KRANDOM,
 * which we don't bind anything useful to.  "The Menu" is
 * special; we implement menus as another function key,
 * but there isn't really any "MENU" key on the keyboard.
 * There is no shifted value for the help key.  Mouse clicks
 * have been moved to a hitherto unused part of the keymap.
 */
#ifdef	DO_MENU
#define MENUNAME "The Menu"
#else
#define MENUNAME NULL
#endif

char	*keystrings[] = {
#ifdef	XKEYS
	NULL,		"F1",		"F2",		"F3",
	"F4",		"F5",		"F6",		"F7",
	"F8",		"F9",		"F10",		"Shift-F1",
	"Shift-F2",	"Shift-F3",	"Shift-F4",	"Shift-F5",
	"Shift-F6",	"Shift-F7",	"Shift-F8",	"Shift-F9",
	"Shift-F10",	"Up",		"Shift-Up",	"Down",
	"Shift-Down",	"Left",		"Shift-Left",	"Right",
	"Shift-Right",	"Help",		MENUNAME,	NULL
#else
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		MENUNAME,	NULL
#endif
};

/*
 * Read in a key, doing the low level mapping
 * of ASCII code to 11 bit code. This level deals with
 * mapping the special keys into their spots in the C1
 * control area. The C0 controls go right through, and
 * get remapped by "getkey".  Returning function keys
 * with KMETA set distinguishes these keys from codes
 * generated by ALT-ing a control key (which I want
 * to have mapped in the usual way).
 */

getkbd()
{
	register int	c;
#ifdef	XKEYS
	register int	n;
#endif
loop:
	c = ttgetc();
	if (c == CSI) {
		c = ttgetc();
#ifdef	MOUSE
		if (c == 'P') {			/* mouse sequence	*/
			ttgetc();		/* discard '~'		*/
			return (KCTRL | getmouse());	/* create key code	*/
		}
#endif

#ifdef	DO_MENU
		if (c == 'M') {			/* (fake) menu key	*/
			ttgetc();		/* discard '~'		*/
			return (KCTRL | KMENU);
		}
#endif

#ifdef	XKEYS
		if (c == '?') {			/* HELP key		*/
			ttgetc();		/* discard '~'		*/
			return (KCTRL | KHELP);
		}
		/* Arrow keys */
		if (c == 'A')
			return (KCTRL | KUP);
		if (c == 'B')
			return (KCTRL | KDOWN);
		if (c == 'C')
			return (KCTRL | KRIGHT);
		if (c == 'D')
			return (KCTRL | KLEFT);
		if (c == 'T')
			return (KCTRL | KSUP);
		if (c == 'S')
			return (KCTRL | KSDOWN);
		/* Shifted left, right arrow */
		if (c == ' ') {
			c = ttgetc();
			if (c == 'A' || c == '@')
				return (KCTRL | ((c == 'A') ?
					(KSLEFT) : (KSRIGHT)));
			goto loop;		/* try again, sucker */
		}

		/* Function keys	*/
		if (c >= '0' && c <= '9') {
			n = 0;
			do {
				n = 10*n + c - '0';
				c = ttgetc();
			} while (c>='0' && c<='9');
			if (c == '~' && n < NFUNCKEYS) {
				c = consolemap[n];
				if (c != KRANDOM)
					return (KCTRL | c);
				goto loop;
			}
			else 
				goto loop;	/* Try again */
		}
#endif
		goto loop;		/* Try again */
	}
	return (c);
}

#ifdef	MOUSE
/*
 * A hack for the hot mouse -- peek ahead at the
 * next mouse event and construct an internal key
 * code.  Might as well use those extra binding slots...
 */

getmouse()
{
	USHORT		row, col, qual;
	int ttmouse();
	register int	code = 0;
	register struct WINDOW *wp;

	ttmouse(FALSE, &row, &col, &qual);	/* look at mouse */

	/* was the click in a window ???	*/
	for (wp = wheadp; wp != NULL; wp = wp->w_wndp)
		if ((row >= wp->w_toprow) &&
		    (row <= (wp->w_toprow + wp->w_ntrows)))
			break ;

	/* figure out what area the click was in			*/
	if (wp == NULL)			/* not found; assume echo line	*/
		code = EMOUSE;
	else if (row == (wp->w_toprow + wp->w_ntrows))	/* status line	*/
		code = SMOUSE;
	else
		code = WMOUSE;	/* click in a window		*/

	/* figure out 'mini' qualifiers -- ADD them, because 'a' isn't	*/
	/* divisible by 2...  This took me 2 hours to realize...       	*/
	if (qual & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
		code += SHFT;
	if (qual & (IEQUALIFIER_LALT | IEQUALIFIER_RALT))
		code += ALT;
	if (qual & IEQUALIFIER_CONTROL)
		code += CTRL;

	return (code);
}
#endif

/*
 * Terminal specific keymap initialization.
 *
 * Bind all of the assigned graphics in the
 * Amiga alternate character set to self-insert.
 *
 * #ifdef XKEYS, attach the special keys to the appropriate
 * built-in functions.
 *
 * #ifdef DO_MENU, bind the fake KMENU code to amigamenu()
 * to do menu selection as transparently as possible.
 *
 * #ifdef MOUSE, bind the hot mouse keys to the
 * functions. In the case of window and mode line
 * selections, call special functions
 * special functions that select the appropriate
 * object (i.e. character, window) before performing
 * the task.
 *
 * As is the case of all the keymap routines, errors
 * result in panic.
 */

extern	int	togglewindow();		/* Defined by "ttyio.c"		*/

#ifdef	DO_MENU
extern	int	amigamenu();		/* Defined by "ttymenu.c" 	*/
#endif

#ifdef	CHANGE_FONT
extern	int	setfont();		/* Defined by "ttyio.c"		*/
#endif

#ifdef	MOUSE
extern	int	amigamouse();		/* Defined by "ttymouse.c"	*/
extern	int	mreposition();		/* Functions which select the	*/ 
extern	int	mdelfword();		/* window the click was in,	*/
extern	int	mkillline();		/* then call another one	*/
extern	int	mforwdel();		/* Defined in "ttymouse.c"	*/
extern	int	mdelwhite();
extern	int	mkillregion();
extern	int	myank();
extern	int	mforwpage();
extern	int	mbackpage();
extern	int	msplitwind();
extern	int	mdelwind();
extern	int	mgotobob();
extern	int	mgotoeob();
extern	int	menlargewind();
extern	int	mshrinkwind();
#endif

#ifdef	CHANGE_COLOR
	/* functions to mess with the mode line rendition, window colors*/
extern	int	ttmode();		/* Defined by "tty.c"		*/
extern	int	tttext();		/*  ""				*/
extern	int	textforeground();	/*  ""				*/
extern	int	textbackground();	/*  ""				*/
extern	int	modeforeground();	/*  ""				*/
extern	int	modebackground();	/*  ""				*/
#endif

ttykeymapinit()
{
	register SYMBOL	*sp;
	register int	i;

	/* Intuition window manipulation 	*/

#ifndef	MEYN
	keydup((KEY)KMETA|KCTRL|'L',	"redraw-display");
#endif
 	keyadd((KEY)-1,	togglewindow,	"toggle-window-hack");

	/*
	 * Bind all positions that correspond
	 * to characters in the Amiga alternate
	 * character set to "ins-self". These characters may
	 * be used just like any other character.  Of course,
	 * if DO_METAKEY is defined in kbd.c, the alternate character
	 * set will get mapped to META-ed keys...
	 */

	if ((sp=symlookup("self-insert-command")) == NULL)
		panic("ttykeymapinit: can't find self-insert-command");
	for (i=0xA0; i<0xFF; ++i) {
		if (binding[i] != NULL)
			panic("ttykeymapinit: key already bound");
		binding[i] = sp;
	}

#ifdef	DO_MENU
	/* "Menu" key, if compiled in	*/
	keyadd((KEY)KMENU,	amigamenu,	"amiga-menu");
#endif

#ifdef	CHANGE_FONT
	keyadd((KEY)-1,	setfont,	"set-font");
#endif

#ifdef	CHANGE_COLOR
	/* Functions to allow you to change colors	*/
	keyadd((KEY)-1, ttmode,		"set-mode-rendition");
	keyadd((KEY)-1,	tttext,		"set-text-rendition");
	keyadd((KEY)-1,	textforeground,	"set-text-foreground");
	keyadd((KEY)-1,	textbackground,	"set-text-background");
	keyadd((KEY)-1, modeforeground, "set-mode-foreground");
	keyadd((KEY)-1,	modebackground,	"set-mode-background");
#endif

#ifdef	XKEYS
	/* Arrow keys	*/
	keydup((KEY)KUP,	"previous-line");
	keydup((KEY)KDOWN,	"next-line");

	keydup((KEY)KSUP,	"backward-paragraph");
	keydup((KEY)KSDOWN,	"forward-paragraph");	

	keydup((KEY)KRIGHT,	"forward-char");
	keydup((KEY)KLEFT,	"backward-char");

	keydup((KEY)KSRIGHT,	"forward-word");
	keydup((KEY)KSLEFT,	"backward-word");

	/* Function keys 	*/
	keydup((KEY)KHELP,	"describe-key-briefly");

	keydup((KEY)KF1,	"find-file");
	keydup((KEY)KSF1,	"find-file-other-window");

	keydup((KEY)KF2,	"save-buffer");
	keydup((KEY)KSF2,	"write-file");

	keydup((KEY)KF3,	"scroll-up");
	keydup((KEY)KSF3,	"scroll-down");

	keydup((KEY)KF4,	"next-window");
	keydup((KEY)KSF4,	"previous-window");

	keydup((KEY)KF5,	"enlarge-window");
	keydup((KEY)KSF5,	"shrink-window");

	keydup((KEY)KF6,	"fill-paragraph");
	keydup((KEY)KSF6,	"query-replace");

	keydup((KEY)KF7,	"split-window-vertically");
	keydup((KEY)KSF7,	"delete-other-windows");

	keydup((KEY)KF8,	"global-set-key");
	keydup((KEY)KSF8,	"global-unset-key");

	keydup((KEY)KF9,	"start-kbd-macro");
	keydup((KEY)KSF9,	"end-kbd-macro");

	keydup((KEY)KF10,	"call-last-kbd-macro");
	keydup((KEY)KSF10,	"save-buffers-kill-emacs");
#endif

#ifdef	MOUSE
	/* Mouse clicks in a window do editing functions on the	*/
	/* window.						*/

	keyadd(WMOUSE,		amigamouse,	"amiga-mouse");
	keyadd(S(WMOUSE),	mreposition,	"mouse-recenter");
	keyadd(A(WMOUSE),	mdelfword,	"mouse-kill-word");
	keyadd(AS(WMOUSE),	mkillline,	"mouse-kill-line");
	keyadd(C(WMOUSE),	mforwdel,	"mouse-delete-char");
	keyadd(CS(WMOUSE),	mdelwhite,	"mouse-just-one-space");
	keyadd(CA(WMOUSE),	mkillregion,	"mouse-kill-region");
	keyadd(CAS(WMOUSE),	myank,		"mouse-yank");

	/* Mouse clicks in the status line select that window and	*/
	/* then perform a command on that window or buffer.		*/
	/* Use keyadd() because they haven't been bound before		*/

	keyadd(SMOUSE,	   mforwpage,	"mouse-scroll-up");
	keyadd(S(SMOUSE),  mbackpage,	"mouse-scroll-down");
	keyadd(A(SMOUSE),  msplitwind,	"mouse-split-window-vertically");
	keyadd(AS(SMOUSE), mdelwind,	"mouse-delete-window");
	keyadd(C(SMOUSE),  mgotobob,	"mouse-beginning-of-buffer");
	keyadd(CS(SMOUSE), mgotoeob,	"mouse-end-of-buffer");
	keyadd(CA(SMOUSE), menlargewind,"mouse-enlarge-window");
	keyadd(CAS(SMOUSE),mshrinkwind,	"mouse-shrink-window");

	/* mouse clicks in echo line do global things		*/

#ifdef	MEYN
	keydup(EMOUSE,		"save-buffer");
#else
	keydup(EMOUSE,		"switch-to-buffer");
#endif
	keydup(S(EMOUSE),	"kill-buffer");
	keydup(A(EMOUSE),	"describe-key-briefly");
	keydup(AS(EMOUSE),	"describe-bindings");
	keydup(C(EMOUSE),	"suspend-emacs");
	keydup(CS(EMOUSE),	"save-buffers-kill-emacs");
	keydup(CA(EMOUSE),	"list-buffers");
	keydup(CAS(EMOUSE),	"toggle-window-hack");
#endif
}
