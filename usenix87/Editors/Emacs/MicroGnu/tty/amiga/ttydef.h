/*
 * Name:	MicroEMACS
 *		Amiga console device virtual terminal header file
 * Version:	Gnu v30
 * Last edit:	26-Aug-86
 * Created:	20-Apr-86 ...!ihnp4!seismo!ut-sally!ut-ngp!mic
 */
#define	GOSLING				/* Compile in fancy display.	*/

#define	NROW	48			/* Max rows (only in interlace)	*/
#define	NCOL	80			/* Columns (borderless window)	*/

/*
 * Special keys for the default Amiga console device keymap.
 * Function key codes are in the form <CSI>v~
 * where v is a 1 or 2-digit code between 0 and 19,
 * so they comprise the first 20 entries in the key
 * table.  The next 12 entries are for the help and
 * arrow keys.
 */
#define	KF1	K01
#define	KF2	K02
#define	KF3	K03
#define	KF4	K04
#define	KF5	K05
#define	KF6	K06
#define	KF7	K07
#define	KF8	K08
#define	KF9	K09
#define	KF10	K0A
#define	KSF1	K0B
#define	KSF2	K0C
#define	KSF3	K0D
#define	KSF4	K0E
#define	KSF5	K0F
#define	KSF6	K10
#define	KSF7	K11
#define	KSF8	K12
#define	KSF9	K13
#define	KSF10	K14
#define	KUP	K15
#define	KSUP	K16
#define	KDOWN	K17
#define	KSDOWN	K18
#define	KLEFT	K19
#define	KSLEFT	K1A
#define	KRIGHT	K1B
#define	KSRIGHT	K1C
#define	KHELP	K1D

/* The 'menu' key doesn't really appear on the
 * Amiga keyboard.  When ttgetc() sees a menu
 * event, it saves the menu number and item,
 * then stuffs the sequence for KMENU into
 * the input buffer.
 */
#define	KMENU	K1E

/*
 * Intuition menu interface.  Each set of menu items
 * kept in a table of MenuBinding structures, which
 * is in turn kept in a table of MenuInfo structures.
 *
 * These tables are indexed via the menu and item
 * numbers to find the internal extended name of
 * the function associated with a certain item.
 */
struct MenuBinding {
	char *Command;
	char *Binding;
};

struct MenuInfo {
	char *Name;			/* name of menu			*/
	short NumItems;			/* # of items			*/
	struct MenuBinding *Items;	/* item name, internal binding	*/
};

#define NITEMS(arr) (sizeof(arr) / (sizeof(arr[0])))

/*
 * If either MENU, or BROWSER is defined, we need to define
 * DO_MENU to get the code for dealing with menu selections
 * compiled in.
 */

#ifdef	MENU
#define	DO_MENU
#else
#ifdef	BROWSER
#define	DO_MENU
#endif	BROWSER
#endif	MENU

/*
 * MODE_RENDITION and TEXT_RENDITION
 * determine the way the mode line and
 * text area are rendered (using the SGR
 * sequence).  TEXT_* and MODE_* set the
 * foreground (FG) and background (BG)
 * color to the specified number.  If you
 * #define CHANGE_COLOR, you can redefine
 * these dynamically.
 */

#ifndef MODE_RENDITION
#define	MODE_RENDITION 7
#endif

#ifndef TEXT_RENDITION
#define	TEXT_RENDITION 0
#endif

#ifndef	TEXT_FG
#define TEXT_FG 1
#endif

#ifndef TEXT_BG
#define TEXT_BG 0
#endif

#ifndef	MODE_FG
#define MODE_FG 1
#endif

#ifndef	MODE_BG
#define MODE_BG 0
#endif

/*
 * Return the width and height of
 * the default font for a window.
 */

#define	FontWidth(w) (w)->RPort->TxWidth
#define	FontHeight(w) (w)->RPort->TxHeight
