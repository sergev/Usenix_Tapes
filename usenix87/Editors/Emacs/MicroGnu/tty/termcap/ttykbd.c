/*
 * Name:	MicroEmacs
 * Version:	30
 *		Termcap keyboard driver
 * Created:	21-Aug-1986
 *		Mic Kaczmarczik ...!ihnp4!seismo!ut-sally!ut-ngp!mic
 * Last edit:	03-Sep-86
 *
 * [ Several of the nasty comments about the XKEYS code are
 *   by me.  [Bob Larson (usc-oberon!blarson)]  It is my opinion
 *   that function keys cannot be made to work with standard
 *   emacs keybindings except on a very limited set of terminals.
 *   I just work with to many that do not fit the assumptions Mic's
 *   XKEYS code makes to consider it useful to me, and think that
 *   others considering using this code should look and see what
 *   it realy does first.
 * ]
 *
 * If XKEYS is defined this routine looks for the following 
 * termcap sequences, which are obtained by "tty.c":
 *
 *	ks	-- start using the function keypad
 *	ke	-- finish using the function keypad
 *	kh	-- home key
 *	ku	-- up arrow
 *	kd	-- down arrow
 *	kl	-- left arrow
 *	kr	-- right arrow
 *	k0-k9	-- standard termcap function keys
 *	l0-l9	-- labels for termcap function keys
 *	(nonstandard)
 *	K0-K9	-- extra keys that we look for -- the get mapped
 *		   internally to F10-F19
 *	L0-L9	-- labels for same.
 *
 * Bugs/features/problems:
 *
 *	XKEYS and DPROMPT do not work together well.
 *
 *	If the META introducer is used as the initial character of
 *	a function key sequence, what should the key parser do when the
 *	user wants to type a META-ed key, or just the META introducer
 *	alone?	This is of practical importance on DEC terminals, where
 *	the META introducer is the Escape key.  Even worse things happen
 *	on terminals that have something (or more than one thing) other
 *	than the META introducer as the inital character of a function
 *	sequence.
 *
 *	The approach I took was that if the META introducer is the first
 *	character in a function sequence, and the second character c
 *	isn't part of a function key sequence, the parser returns
 *	(KMETA | c).  If it sees two META introducers in a row, it
 *	returns one instance of METACH.   This approach is subject to
 *	discussion and debate, but it works.  [In at lease some cases.]
 *
 *	If the META introducer is NOT the first character in a function
 *	sequence (including arrow keys) this code has a very nasty
 *	side effect of eating that key.  For example, on an Adds viewpoint
 *	60, six normal control characters are eaten if you have defined
 *	XKEYS and put the keys in the termcap.  More than a little 
 *	creativity is needed because ^U is one of the arrow keys, and
 *	prefixes aren't bindable.
 *
 *	[ From a quick look at the code, it seems that a single character
 *	  funciton key won't work, but it is still put in the table.
 *	]
 */
#include	"def.h"

/*
 * Default key name table.  Can be overridden by
 * definitions of l0-l9 in the termcap entry.  You
 * can't redefine the names for the arrow keys
 * and the home key.
 */

#ifdef	XKEYS
/* key sequences (from tty.c) */
extern	char	*K[], *L[], *KS, *KE, *KH, *KU, *KD, *KL, *KR;
extern	int	putpad();	/* also from tty.c */
char	*keystrings[] = {
	NULL,		"Home",		"Down-Arrow",	"Up-Arrow",
	"Left-Arrow",	"Right-Arrow",	"F0",		"F1",
	"F2",		"F3",		"F4",		"F5",
	"F6",		"F7",		"F8",		"F9",
	"F10",		"F11",		"F12",		"F13",
	"F14",		"F15",		"F16",		"F17",
	"F18",		"F19",		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL
};
#else
char	*keystrings[] = {
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL
};
#endif

#ifdef	XKEYS
/*
 * Type declarations for data structure we
 * use to parse for function key sequences
 */
#define	NODE		0	/* internal node		*/
#define	VALUE		1	/* internal key code value	*/
#define SENTINEL	2	/* sentinel value		*/

typedef struct trienode {
	int type;	/* one of NODE, LEAF */
	struct trienode *sibling, *child;
	KEY value;
} TRIENODE, *TRIE;

TRIE keywords, sentinel, talloc(), tinsert();
#endif

/*
 * Get keyboard character, and interpret
 * any special keys on the keyboard.  If XKEYS is
 * #defined, use a dictionary organized as a
 * trie to keep the parsing overhead down.
 *
 * To keep the function call overhead down, do the
 * first level of parse() inside getkbd().
 *
 * Also, since ESC (the usual value of METACH) is
 * the first character in many function key sequences,
 * we  return (KMETA | ch) if METACH-<ch> is not
 * the start of an escape sequence.  Blecch.  Furthermore,
 * if we see METACH-METACH, we return the value METACH.
 * Phhhht.
 */
getkbd() {
#ifndef	XKEYS
	return (ttgetc());
#else
	register TRIE	t;
	register int	c;
	KEY		code;

	c = ttgetc();
	for (t = keywords; t->type == NODE; t = t->sibling)
		if (t->value == c) {	/* possible function key sequence  */
			if (c != METACH)
				return (parse(t->child));
			else {		/* maybe sequence, maybe META char */
				c = ttgetc();
				for (t = t->child; t->type == NODE; t = t->sibling)
					if (t->value == c)
						return (parse(t->child));
				/* METACH-METACH -> METACH */
				if (c == METACH)
					return (METACH);
				/* Else make c into a META character */
				if (ISLOWER(c) != FALSE)
					c = TOUPPER(c);
				if (c>=0x00 && c<=0x1F)
					c = KCTRL | (c+'@');
				return (KMETA | c);
			}
		}
	return (c);
#endif
}

#ifdef	XKEYS
static parse(first)
TRIE first;
{
	register TRIE	t;
	register int	c;

	if (first->type == VALUE)		/* found a match!	*/
		return (first->value);

	c = ttgetc();
	for (t = first; t->type == NODE; t = t->sibling)/* look thru list   */
		if (t->value == c)
			return (parse(t->child));	/* try next level   */
	return (c);	/* nothing matched */
}
#endif

/*
 * If XKEYS is defined, get key definitions from the termcap 
 * entry and put them in the parse table.
 */
ttykeymapinit()
{
#ifdef	XKEYS
	register int	i;
	register int	s;
	register char	*cp;
	register SYMBOL	*sp;

	if (KS && *KS)			/* turn on keypad	*/
		putpad(KS);		

	tinit();			/* set up initial trie */

	for (i = 0; i < NFKEYS; i++) {
		if (K[i] && *K[i])
			adddict(K[i], (KEY) (KF0 + i));
		if (L[i] && *L[i])	/* record new name */
			keystrings[(KF0-KFIRST)+i] = L[i];
	}

	/*
	 * Add the home and arrow keys
	 */
	if (KH && *KH)
		adddict(KH, (KEY) KHOME);
	if (KU && *KU)
		adddict(KU, (KEY) KUP);
	if (KD && *KD)
		adddict(KD, (KEY) KDOWN);
	if (KL && *KL)
		adddict(KL, (KEY) KLEFT);
	if (KR && *KR)
		adddict(KR, (KEY) KRIGHT);

	/*
	 * Bind things to the movement keys
	 */
	keydup(KHOME,	"beginning-of-buffer");	/* for now */
	keydup(KUP,	"previous-line");
	keydup(KDOWN,	"next-line");
	keydup(KLEFT,	"backward-char");
	keydup(KRIGHT,	"forward-char");

	/*
	 * These bindings sort of go with the termcap I use for my vt220
	 * clone, which is why they're #ifdef'd.  I don't really use
	 * them, but it gives you an example of what to do...
	 */
#ifdef	MPK
	keydup((KEY)KF0, "describe-key-briefly");	/* Help		*/
	keydup((KEY)KF1, "execute-extended-command");	/* Do		*/
	keydup((KEY)KF2, "search-forward");		/* Find		*/
	keydup((KEY)KF3, "yank");			/* Insert here	*/
	keydup((KEY)KF4, "kill-region");		/* Remove	*/
	keydup((KEY)KF5, "set-mark-command");		/* Select	*/
	keydup((KEY)KF6, "scroll-down");		/* Prev Screen	*/
	keydup((KEY)KF7, "scroll-up");			/* Next Screen	*/

	/* Don't expect these to make much sense, I'm just filling in	*/
	/* the keymap B-]						*/
	keydup((KEY)KF10, "suspend-emacs");		/* PF1		*/
	keydup((KEY)KF11, "query-replace");		/* PF2		*/
	keydup((KEY)KF12, "call-last-kbd-macro");	/* PF3		*/
	keydup((KEY)KF13, "save-buffers-kill-emacs");	/* PF4		*/
#endif	MPK
#endif	XKEYS
}

#ifdef	XKEYS
/*
 * Clean up the keyboard -- called by tttidy()
 */
ttykeymaptidy()
{
	tdelete(keywords);	/* get rid of parse tree	*/
	free(sentinel);		/* remove sentinel value	*/
	if (KE && *KE)
		putpad(KE);	/* turn off keypad		*/
}

/*
 * * * * * * * * Dictionary management * * * * * * * * *
 */

/*
 * Add a key string to the dictionary.
 */

static adddict(kstr, kcode)
char *kstr;
KEY kcode;
{
	keywords = tinsert(kstr, kcode, keywords);
}

/*
 * Initialize the parse tree by creating the sentinel value
 */

static tinit()
{
	keywords = sentinel = talloc();
	sentinel->type = SENTINEL;
	sentinel->value = (KEY) -1;
	sentinel->sibling = sentinel->child = sentinel;	/* set up a loop */
}

/*
 * Deallocate all the space used by the trie --
 * Tell all the siblings to deallocate space, then
 * all the children.
 */

static tdelete(t)
register TRIE t;
{
	if (t->type != SENTINEL) {
		tdelete(t->sibling);
		tdelete(t->child);
		free(t);
	}
}

/*
 * Insert a dictionary key string and a value into the dictionary,
 * returning as the value the first sibling in the current sublevel,
 * which may have been changed by an insertion into the list of siblings.
 */

static TRIE tinsert(kstring, kcode, first)
register char *kstring;
register KEY kcode;
TRIE first;
{
	register TRIE	match;
	register TRIE	p;
	
	if (!*kstring) {	/* base case -- return a value node */
		p = talloc();
		p->type = VALUE;
		p->value = kcode;
		p->sibling = p->child = sentinel;
		return (p);
	}
	/* recursive case -- insert rest of string in trie */

	/* search for sibling that matches the current character */
	match = NULL;
	for (p = first; p->type == NODE; p = p->sibling)
		if (p->value == *kstring) {
			match = p;
			break;
		}

	if (match == NULL) {	/* if not, add it to beginning of the list */
		match = talloc();
		match->type = NODE;
		match->value = *kstring;
		match->sibling = first;
		match->child = sentinel;
		first = match;
	}
	/* add rest of string to this child's subtrie */
	match->child = tinsert(kstring+1, kcode, match->child);
	return (first);
}

/*
 * Allocate a trie node
 */
static TRIE talloc()
{
	char *malloc();
	TRIE t;

	if ((t = (TRIE) malloc(sizeof(TRIENODE))) == NULL)
		panic("talloc: can't allocate trie node!");
	return (t);
}
#endif
