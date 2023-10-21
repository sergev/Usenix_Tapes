/*
 * Symbol tables, and keymap setup.
 * The terminal specific parts of building the
 * keymap has been moved to a better place.
 */
#include	"def.h"

#ifdef	HASH
Since you're seeing this, you must have defined HASH to try and get the
hashing code back. You're getting an error because I (mwm@ucbvax) want you
to read this.

With the change in function completion, there is at least one linear search
through the function list for every hash lookup (ignoring the startup code).
Given that there are probably actually MANY more linear searches for
completion than fullname lookups, some structure other than a hash table is
better suited to this purpose. I suggest trying sorting the lists for more
speed, then going to a binary search tree, and finally going to a trie.
#endif	HASH

/*
 * Defined here so to collect the #ifdef MEYN config stuff in one file
 * If you set either MINDENT or MFILL, then you need to change the bindings
 * in this file to match: KCTRL|'M' -> newline-and-indent and KCTRL|'J' ->
 * insert-newline for MINDENT, and ' ' -> insert-with-wrap for MFILL.
 * MEYN is used for compile-time customization of the system for micros.
 */
#ifndef	MEYN
int	mode = 0;			/* All modes off		*/
#else
int	mode = MBSMAP|MINDENT;
#endif

/*
 * Defined by "main.c".
 */
extern	int	ctrlg();		/* Abort out of things		*/
extern	int	quit();			/* Quit				*/
extern	int	ctlxlp();		/* Begin macro			*/
extern	int	ctlxrp();		/* End macro			*/
extern	int	ctlxe();		/* Execute macro		*/
extern  int	showversion();		/* Show version numbers, etc.	*/

/*
 * Defined by "search.c".
 */
extern	int	forwsearch();		/* Search forward		*/
extern	int	backsearch();		/* Search backwards		*/
extern  int	searchagain();		/* Repeat last search command	*/
extern  int	forwisearch();		/* Incremental search forward	*/
extern  int	backisearch();		/* Incremental search backwards	*/
extern  int	queryrepl();		/* Query replace		*/

/*
 * Defined by "basic.c".
 */
extern	int	gotobol();		/* Move to start of line	*/
extern	int	backchar();		/* Move backward by characters	*/
extern	int	gotoeol();		/* Move to end of line		*/
extern	int	forwchar();		/* Move forward by characters	*/
extern	int	gotobob();		/* Move to start of buffer	*/
extern	int	gotoeob();		/* Move to end of buffer	*/
extern	int	forwline();		/* Move forward by lines	*/
extern	int	backline();		/* Move backward by lines	*/
extern	int	forwpage();		/* Move forward by pages	*/
extern	int	backpage();		/* Move backward by pages	*/
extern	int	pagenext();		/* Page forward next window	*/
extern	int	setmark();		/* Set mark			*/
extern	int	swapmark();		/* Swap "." and mark		*/
extern	int	gotoline();		/* Go to a specified line.	*/

/*
 * Defined by "buffer.c".
 */
extern	int	listbuffers();		/* Display list of buffers	*/
extern	int	usebuffer();		/* Switch a window to a buffer	*/
extern	int	poptobuffer();		/* Other window to a buffer	*/
extern	int	killbuffer();		/* Make a buffer go away.	*/
extern	int	savebuffers();		/* Save unmodified buffers	*/
extern	int	bufferinsert();		/* Insert buffer into another	*/
extern	int	notmodified();		/* Reset modification flag	*/

#ifdef	DIRLIST
/*
 * Defined by "dirlist.c".
 */
extern	int	dirlist();		/* Directory list.		*/
#endif

/*
 * Defined by "file.c".
 */
extern	int	filevisit();		/* Get a file, read write	*/
extern	int	poptofile();		/* Get a file, other window	*/
extern	int	filewrite();		/* Write a file			*/
extern	int	filesave();		/* Save current file		*/
extern	int	fileinsert();		/* Insert file into buffer	*/

/*
 * Defined by "match.c"
 */
extern	int	blinkparen();		/* Fake blink-matching-paren var */
extern	int	showmatch();		/* Hack to show matching paren	 */

/*
 * Defined by "random.c".
 */
extern	int	selfinsert();		/* Insert character		*/
extern	int	showcpos();		/* Show the cursor position	*/
extern	int	twiddle();		/* Twiddle characters		*/
extern	int	quote();		/* Insert literal		*/
extern	int	openline();		/* Open up a blank line		*/
extern	int	newline();		/* Insert CR-LF			*/
extern	int	deblank();		/* Delete blank lines		*/
extern	int	delwhite();		/* Delete extra whitespace	*/
extern	int	indent();		/* Insert CR-LF, then indent	*/
extern	int	forwdel();		/* Forward delete		*/
extern	int	backdel();		/* Backward delete in		*/
extern	int	killline();		/* Kill forward			*/
extern	int	yank();			/* Yank back from killbuffer.	*/
extern	int	bsmapmode();		/* set bsmap mode		*/
extern	int	flowmode();		/* set flow mode		*/
extern	int	indentmode();		/* set auto-indent mode		*/
extern	int	fillmode();		/* set word-wrap mode		*/

/*
 * Defined by "region.c".
 */
extern	int	killregion();		/* Kill region.			*/
extern	int	copyregion();		/* Copy region to kill buffer.	*/
extern	int	lowerregion();		/* Lower case region.		*/
extern	int	upperregion();		/* Upper case region.		*/
#ifdef	PREFIXREGION
extern	int	prefixregion();		/* Prefix all lines in region	*/
extern	int	setprefix();		/* Set line prefix string	*/
#endif

/*
 * Defined by "spawn.c".
 */
extern	int	spawncli();		/* Run CLI in a subjob.		*/
#ifdef	VMS
extern	int	attachtoparent();	/* Attach to parent process	*/
#endif

/*
 * Defined by "window.c".
 */
extern	int	reposition();		/* Reposition window		*/
extern	int	refresh();		/* Refresh the screen		*/
extern	int	nextwind();		/* Move to the next window	*/
extern  int	prevwind();		/* Move to the previous window	*/
extern	int	onlywind();		/* Make current window only one	*/
extern	int	splitwind();		/* Split current window		*/
extern	int	delwind();		/* Delete current window	*/
extern	int	enlargewind();		/* Enlarge display window.	*/
extern	int	shrinkwind();		/* Shrink window.		*/

/*
 * Defined by "word.c".
 */
extern	int	backword();		/* Backup by words		*/
extern	int	forwword();		/* Advance by words		*/
extern	int	upperword();		/* Upper case word.		*/
extern	int	lowerword();		/* Lower case word.		*/
extern	int	capword();		/* Initial capitalize word.	*/
extern	int	delfword();		/* Delete forward word.		*/
extern	int	delbword();		/* Delete backward word.	*/

/*
 * Defined by "extend.c".
 */
extern	int	extend();		/* Extended commands.		*/
extern	int	desckey();		/* Help key.			*/
extern	int	bindtokey();		/* Modify key bindings.		*/
extern	int	unsetkey();		/* Unbind a key.		*/
extern	int	wallchart();		/* Make wall chart.		*/
#ifdef	STARTUP
extern	int	evalexpr();		/* Extended commands (again)	*/
extern	int	evalbuffer();		/* Evaluate current buffer	*/
extern	int	evalfile();		/* Evaluate a file		*/
#endif

/*
 * defined by "paragraph.c" - the paragraph justification code.
 */
extern	int	gotobop();		/* Move to start of paragraph.	*/
extern	int	gotoeop();		/* Move to end of paragraph.	*/
extern	int	fillpara();		/* Justify a paragraph.		*/
extern	int	killpara();		/* Delete a paragraph.		*/
extern	int	setfillcol();		/* Set fill column for justify.	*/
extern	int	fillword();		/* Insert char with word wrap.	*/

/*
 * defined by prefix.c
 */
extern	int	help();			/* Parse help key.		*/
extern	int	ctlx4hack();		/* Parse a pop-to key.		*/

typedef	struct	{
	KEY	k_key;			/* Key to bind.			*/
	int	(*k_funcp)();		/* Function.			*/
	char	*k_name;		/* Function name string.	*/
}	KEYTAB;

/*
 * Default key binding table. This contains
 * the function names, the symbol table name, and (possibly)
 * a key binding for the builtin functions. There are no
 * bindings for C-U or C-X. These are done with special
 * code, but should be done normally.
 */
KEYTAB	key[] = {
#ifdef	MEYN		/* Add meyer's peculiar bindings */
	KCTRL|'J',	newline,	"insert-newline",
	KCTRL|'M',	indent,		"newline-and-indent",
	KCTLX|'N',	nextwind, 	"next-window",
	KCTLX|'P',	prevwind,	"previous-window",
	KMETA|KCTRL|'C',quit,		"save-buffers-kill-emacs",
	KMETA|KCTRL|'L',refresh,	"redraw-display",
	KMETA|'G',	gotoline,	"goto-line",
	KMETA|'J',	fillpara,	"fill-paragraph",
	KMETA|'Q',	queryrepl,	"query-replace",
#endif
	KCTRL|'@',	setmark,	"set-mark-command",
	KCTRL|'A',	gotobol,	"beginning-of-line",
	KCTRL|'B',	backchar,	"backward-char",
	KCTRL|'D',	forwdel,	"delete-char",
	KCTRL|'E',	gotoeol,	"end-of-line",
	KCTRL|'F',	forwchar,	"forward-char",
	KCTRL|'G',	ctrlg,		"keyboard-quit",
	KCTRL|'H',	help,		"help",
	KCTRL|'I',	selfinsert,	"self-insert-command",
#ifndef	MEYN
	KCTRL|'J',	indent,		"newline-and-indent",
#endif
	KCTRL|'L',	reposition,	"recenter",
	KCTRL|'K',	killline,	"kill-line",
#ifndef	MEYN
	KCTRL|'M',	newline,	"insert-newline",
#endif
	KCTRL|'N',	forwline,	"next-line",
	KCTRL|'O',	openline,	"open-line",
	KCTRL|'P',	backline,	"previous-line",
	KCTRL|'Q',	quote,		"quoted-insert",
	KCTRL|'R',	backisearch,	"isearch-backward",
	KCTRL|'S',	forwisearch,	"isearch-forward",
	KCTRL|'T',	twiddle,	"transpose-chars",
	KCTRL|'V',	forwpage,	"scroll-up",
	KCTRL|'W',	killregion,	"kill-region",
	KCTRL|'Y',	yank,		"yank",
#ifdef	VMS
	KCTRL|'Z',	attachtoparent,	"suspend-emacs",
#else
	KCTRL|'Z',	spawncli,	"suspend-emacs",
#endif
	KCTLX|KCTRL|'B',listbuffers,	"list-buffers",
#ifndef	MEYN
	KCTLX|KCTRL|'C',quit,		"save-buffers-kill-emacs",
#endif
#ifdef	DIRLIST
	KCTLX|KCTRL|'D',dirlist,	"display-directory",
#endif
	KCTLX|KCTRL|'F',filevisit,	"find-file",
	KCTLX|KCTRL|'L',lowerregion,	"downcase-region",
	KCTLX|KCTRL|'O',deblank,	"delete-blank-lines",
	KCTLX|KCTRL|'S',filesave,	"save-buffer",
	KCTLX|KCTRL|'U',upperregion,	"upcase-region",
	KCTLX|KCTRL|'W',filewrite,	"write-file",
	KCTLX|KCTRL|'X',swapmark,	"exchange-point-and-mark",
	KCTLX|'=',	showcpos,	"what-cursor-position",
	KCTLX|'(',	ctlxlp,		"start-kbd-macro",
	KCTLX|')',	ctlxrp,		"end-kbd-macro",
	KCTLX|'^',	enlargewind,	"enlarge-window",
	KCTLX|'0',	delwind,	"delete-window",
	KCTLX|'1',	onlywind,	"delete-other-windows",
	KCTLX|'2',	splitwind,	"split-window-vertically",
	KCTLX|'4',	ctlx4hack,	"ctrlx-four-hack",
	KCTLX|'B',	usebuffer,	"switch-to-buffer",
	KCTLX|'E',	ctlxe,		"call-last-kbd-macro",
	KCTLX|'F',	setfillcol,	"set-fill-column",
	KCTLX|'I',	fileinsert,	"insert-file",
	KCTLX|'K',	killbuffer,	"kill-buffer",
	KCTLX|'S',	savebuffers,	"save-some-buffers",
#ifndef	MEYN
	KCTLX|'O',	nextwind,	"next-window",
	KMETA|'%',	queryrepl,	"query-replace",
#endif
	KMETA|KCTRL|'V',pagenext,	"scroll-other-window",
	KMETA|'>',	gotoeob,	"end-of-buffer",
	KMETA|'<',	gotobob,	"beginning-of-buffer",
	KMETA|'[',	gotobop,	"backward-paragraph",
	KMETA|']',	gotoeop,	"forward-paragraph",
	KMETA|' ',	delwhite,	"just-one-space",
	KMETA|'B',	backword,	"backward-word",
	KMETA|'C',	capword,	"capitalize-word",
	KMETA|'D',	delfword,	"kill-word",
	KMETA|'F',	forwword,	"forward-word",
	KMETA|'L',	lowerword,	"downcase-word",
#ifndef	MEYN
	KMETA|'Q',	fillpara,	"fill-paragraph",
#endif
	KMETA|'R',	backsearch,	"search-backward",
	KMETA|'S',	forwsearch,	"search-forward",
	KMETA|'U',	upperword,	"upcase-word",
	KMETA|'V',	backpage,	"scroll-down",
	KMETA|'W',	copyregion,	"copy-region-as-kill",
	KMETA|'X',	extend,		"execute-extended-command",
	KMETA|'~',	notmodified,	"not-modified",
#ifndef	MEYN
	-1,		prevwind,	"previous-window",
	-1,		refresh,	"redraw-display",
	-1,		gotoline,	"goto-line",
#endif
#ifdef	STARTUP
	-1,		evalexpr,	"eval-expression",
	-1,		evalbuffer,	"eval-current-buffer",
	-1,		evalfile,	"load",
#endif
	-1,		bsmapmode,	"bsmap-mode",
	-1,		flowmode,	"flow-mode",
	-1,		indentmode,	"auto-indent-mode",
	-1,		fillmode,	"auto-fill-mode",
	-1,		fillword,	"insert-with-wrap",
	-1,		shrinkwind,	"shrink-window",
	-1,		searchagain,	"search-again",
	-1,		unsetkey,	"global-unset-key",
	-1,		bindtokey,	"global-set-key",
	-1,		killpara,	"kill-paragraph",
	-1,		showversion,	"emacs-version",
	-1,		blinkparen,	"blink-matching-paren",
	-1,		showmatch,	"blink-matching-paren-hack",
	-1,		bufferinsert,	"insert-buffer",
#ifdef	VMS
	-1,		spawncli,	"push-to-dcl",
#endif
#ifdef	PREFIXREGION
	-1,		prefixregion,	"prefix-region",
	-1,		setprefix,	"set-prefix-string",
#endif
	/* You can actually get to these with keystrokes. See prefix.c */
	-1,		poptobuffer,	"switch-to-buffer-other-window",
	-1,		poptofile,	"find-file-other-window",
	-1,		desckey,	"describe-key-briefly",
	-1,		wallchart,	"describe-bindings",
};

#define	NKEY	(sizeof(key) / sizeof(key[0]))

/*
 * Symbol table lookup.
 * Return a pointer to the SYMBOL node, or NULL if
 * the symbol is not found.
 */
SYMBOL	*
symlookup(cp) register char *cp; {
	register SYMBOL	*sp;

#ifdef	HASH
	sp = symbol[symhash(cp)];
#else
	sp = symbol[0];
#endif
	while (sp != NULL) {
		if (strcmp(cp, sp->s_name) == 0)
			return (sp);
#ifdef	HASH
		if ((sp->s_flags&SFEND) != 0) break;
#endif
		sp = sp->s_symp;
	}
	return (NULL);
}

#ifdef	HASH
/*
 * Take a string, and compute the symbol table
 * bucket number. This is done by adding all of the characters
 * together, and taking the sum mod NSHASH. The string probably
 * should not contain any GR characters; if it does the "*cp"
 * may get a nagative number on some machines, and the "%"
 * will return a negative number!
 */
symhash(cp) register char *cp; {
	register int	c;
	register int	n;

	n = 0;
	while ((c = *cp++) != 0)
		n += c;
	return (n % NSHASH);
}
#endif

/*
 * Build initial keymap. The funny keys
 * (commands, odd control characters) are mapped using
 * a big table and calls to "keyadd". The printing characters
 * are done with some do-it-yourself handwaving. The terminal
 * specific keymap initialization code is called at the
 * very end to finish up. All errors are fatal.
 */
keymapinit() {
	register SYMBOL	*sp;
	register KEYTAB	*kp;
	register int	i;

	for (i=0; i<NKEYS; ++i)
		binding[i] = NULL;
	for (kp = &key[0]; kp < &key[NKEY]; ++kp)
		keyadd(kp->k_key, kp->k_funcp, kp->k_name);
	keydup((KEY) (KCTLX|KCTRL|'G'),	"keyboard-quit");
	keydup((KEY) (KMETA|KCTRL|'G'),	"keyboard-quit");
	keyadd((KEY) (KMETA|0x7F), delbword,
				"backward-kill-word");
	keyadd((KEY) 0x7F, backdel,	"backward-delete-char");
	/*
	 * add duplicates (GNU-stuff)
	 */
	keydup((KEY) (KCTLX|KCTRL|'Z'), "suspend-emacs");
	/*
	 * Should be bound by "tab" already.
	 */
	if ((sp=symlookup("self-insert-command")) == NULL)
		panic("no self-insert-command in keymapinit");
	for (i=0x20; i<0x7F; ++i) {
		if (binding[i] != NULL)
			panic("nonull binding in keymapinit");
		binding[i] = sp;
	}
	ttykeymapinit();
#ifdef	HASH
	/* Link up the symbol table entries	*/
	for (sp = symbol[i = 0]; i < NSHASH-1; sp = sp->s_symp)
		if (sp->s_symp == NULL) sp->s_symp = symbol[++i];
#endif			
}

/*
 * Create a new builtin function "name"
 * with function "funcp". If the "new" is a real
 * key, bind it as a side effect. All errors
 * are fatal.
 */
keyadd(new, funcp, name) register KEY new; int (*funcp)(); char *name; {
	register SYMBOL	*sp;
#ifdef	HASH
	register int	hash;
#endif

	if ((sp=(SYMBOL *)malloc(sizeof(SYMBOL))) == NULL)
		panic("No memory");
#ifdef	HASH
	hash = symhash(name);
	if (symbol[hash] == NULL) sp->s_flags |= SFEND;
	sp->s_symp = symbol[hash];
	symbol[hash] = sp;
#else
	sp->s_symp = symbol[0];
        symbol[0] = sp;
#endif
	sp->s_name = name;
	sp->s_funcp = funcp;
	if (new >= 0) {				/* Bind this key.	*/
		if (binding[new] != NULL)
			panic("rebinding old symbol");
		binding[new] = sp;
	}
}

/*
 * Bind key "new" to the existing
 * routine "name". If the name cannot be found,
 * or the key is already bound, abort.
 */
keydup(new, name) register KEY new; char *name; {
	register SYMBOL	*sp;

	if (binding[new]!=NULL || (sp=symlookup(name))==NULL) {
#ifdef	KEYDUP_ERROR
		fprintf (stderr, "keydup: binding[%d] = %x",
				new, binding[new]);
		fprintf (stderr, " and symlookup(%s) == %x\n", name, sp);
#endif
		panic("keydup");
	}
	binding[new] = sp;
}
