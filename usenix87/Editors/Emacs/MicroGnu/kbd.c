/*
 *		Terminal independent keyboard handling.
 */
#include	"def.h"

#ifdef	DPROMPT
#define	PROMPTL	80
  char	prompt[PROMPTL], *promptp;
#endif

/*
 * All input from the user (should!) go through
 * getkey. Quotep is true to get raw keys, false to
 * get 11-bit code keys.
 * Getkey is responsible for putting keystrokes away
 * for macros. It also takes keystrokes out of the macro,
 * though other input routines will can also do this.
 * Read in a key, doing the terminal
 * independent prefix handling. The terminal specific
 * "getkbd" routine gets the first swing, and may return
 * one of the special codes used by the special keys
 * on the keyboard. The "getkbd" routine returns the
 * C0 controls as received; this routine moves them to
 * the right spot in 11 bit code.
 * If the KPROMPT bit in the flags is set and DPROMPT is
 * defined, do delayed prompting.  (dprompt routine is 
 * in sys/???/ttyio.c)
 */
KEY
getkey(f) register int f; {
	register KEY	c;
	KEY		keychar();
	int		getkbd(), ttgetc();

	if (kbdmop != NULL) return *kbdmop++;
#ifdef	DPROMPT
	if(!(f&KPROMPT)) prompt[0] = '\0';
#endif
	c = (KEY) mapin(getkbd);
#ifdef	DPROMPT
	if(f&KPROMPT) {
		if(promptp > prompt) *(promptp-1) = ' ';
		if(promptp >= &prompt[PROMPTL-8]) f &= ~KPROMPT;
				/* must have a lot of prefixes.... */
	}
#endif
	if ((f&KQUOTE) == 0) {
#ifdef	DO_METAKEY
		if ((c & ~KCHAR) == KCTRL)	/* Function key		*/
			c &= KCHAR;		/* remove wrapping	*/
		else if ((c >= 0x80) && (c <= 0xFF)) /* real meta key	*/
			c = KMETA | keychar(c & ~0x80, TRUE);
#endif
#ifdef	DPROMPT
		if(f&KPROMPT) {
			keyname(promptp, (c<=0x1F && c>=0x00)?KCTRL|(c+'@'):c);
			strcat(promptp, "-");
		}
#endif
		if (c == METACH)		/* M-			*/
			c = KMETA | keychar(mapin(ttgetc), TRUE);
		else if (c == CTRLCH)		/* C-			*/
			c = KCTRL | keychar(mapin(ttgetc), TRUE);
		else if (c == CTMECH)		/* C-M-			*/
			c = KCTRL | KMETA | keychar(mapin(ttgetc), TRUE);
		else if (c<=0x1F && c>=0x00)	/* Relocate control.	*/
			c = KCTRL | (c+'@');
		if (c == (KCTRL|'X'))		/* C-X			*/
			c = KCTLX | keychar(mapin(ttgetc), TRUE);
	}

	if ((f&KNOMAC) == 0 && kbdmip != NULL) {
		if (kbdmip+1 > &kbdm[NKBDM-3]) {	/* macro overflow */
			(VOID) ctrlg(FALSE, 0, KRANDOM);
			ewprintf("Keyboard macro overflow");
			ttflush();
			return (KCTRL|'G');		/* ^G it for us	*/
		}
		*kbdmip++ = c;
	}
#ifdef	DPROMPT
	if(f&KPROMPT) {
		keyname(promptp, c);
		promptp += strlen(promptp);
		*promptp++ = '-';
		*promptp = '\0';
	}
#endif
	return (c);
}

/*
 * go get a key, and run it through whatever mapping the modes
 * specify.
 */
static mapin(get) int (*get)(); {
	register int	c;

#ifdef	DPROMPT
	if(prompt[0]!='\0' && ttwait()) {
		ewprintf("%s", prompt);		/* avoid problems with % */
		update();			/* put the cursor back	 */
		epresf = KPROMPT;
	}
#endif
	c = (*get)();
	if ((mode&MFLOW) != 0) {
		while (c == CCHR('S') || c == CCHR('Q'))
			c = (*get)();
		if (c == CCHR('^')) c = CCHR('Q');
		else if (c == CCHR('\\')) c = CCHR('S');
	}
	if ((mode&MBSMAP) != 0)
		if (c == CCHR('H')) c = 0x7f;
		else if (c == 0x7f) c = CCHR('H');
	return c;
}
/*
 * Transform a key code into a name,
 * using a table for the special keys and combination
 * of some hard code and some general processing for
 * the rest. None of this code is terminal specific any
 * more. This makes adding keys easier.
 */
VOID
keyname(cp, k) register char *cp; register int k; {
	register char	*np;
	register int	c;
	char		nbuf[3];

	static	char	hex[] = {
		'0',	'1',	'2',	'3',
		'4',	'5',	'6',	'7',
		'8',	'9',	'A',	'B',
		'C',	'D',	'E',	'F'
	};

	if ((k&KCTLX) != 0) {			/* C-X prefix.		*/
		*cp++ = 'C';
		*cp++ = '-';
		*cp++ = 'x';
		*cp++ = ' ';
	}
	if ((k&KMETA) != 0) {			/* Add M- mark.		*/
		*cp++ = 'E';
		*cp++ = 'S';
		*cp++ = 'C';
		*cp++ = ' ';
	}
	if ((k&KCHAR)>=KFIRST && (k&KCHAR)<=KLAST) {
		if ((np=keystrings[(k&KCHAR)-KFIRST]) != NULL) {
			if ((k&KCTRL) != 0) {
				*cp++ = 'C';
				*cp++ = '-';
			}
			(VOID) strcpy(cp, np);
			return;
		}
	}
	c = k & ~(KMETA|KCTLX);
	if (c == (KCTRL|'I'))	/* Some specials.	*/
		np = "TAB";
	else if (c == (KCTRL|'M'))
		np = "RET";
	else if (c == (KCTRL|'J'))
		np = "LFD";
	else if (c == ' ')
		np = "SPC";
	else if (c == 0x7F)
		np = "DEL";
	else if (c == (KCTRL|'['))
		np = "ESC";
	else {
		if ((k&KCTRL) != 0) {		/* Add C- mark.		*/
			*cp++ = 'C';
			*cp++ = '-';
		}
		if ((k&(KCTRL|KMETA|KCTLX)) != 0 && ISUPPER(k&KCHAR) != FALSE)
			k = TOLOWER(k&KCHAR);
		np = &nbuf[0];
		if (((k&KCHAR)>=0x20 && (k&KCHAR)<=0x7E)
		||  ((k&KCHAR)>=0xA0 && (k&KCHAR)<=0xFE)) {
			nbuf[0] = k&KCHAR;	/* Graphic.		*/
			nbuf[1] = 0;
		} else {			/* Non graphic.		*/
			nbuf[0] = hex[(k>>4)&0x0F];
			nbuf[1] = hex[k&0x0F];
			nbuf[2] = 0;
		}
	}
	(VOID) strcpy(cp, np);
}

/*
 * turn a key into an internal char.
 */
KEY
keychar(c, f) register int c, f; {

	if (f == TRUE && ISLOWER(c) != FALSE)
		c = TOUPPER(c);
	else if (c>=0x00 && c<=0x1F)		/* Relocate control.	*/
		c = (KCTRL|(c+'@'));
	return (KEY) c;
}
