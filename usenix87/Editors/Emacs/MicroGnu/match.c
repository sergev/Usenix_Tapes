/*
 * Name:	MicroEMACS
 * 		Limited parenthesis matching routines
 * Version:	Gnu30
 * Last edit:	13-Jul-86
 * Created:	19-May-86 ...!ihnp4!seismo!ut-sally!ut-ngp!mic
 *
 * The hacks in this file implement automatic matching
 * of (), [], {}, and other characters.  It would be
 * better to have a full-blown syntax table, but there's
 * enough overhead in the editor as it is.
 *
 * Since I often edit Scribe code, I've made it possible to
 * blink arbitrary characters -- just bind delimiter characters
 * to "blink-matching-paren-hack"
 */
#include	"def.h"

/* Balance table. When balance() encounters a character
 * that is to be matched, it first searches this table
 * for a balancing left-side character.  If the character
 * is not in the table, the character is balanced by itself.
 * This is to allow delimiters in Scribe documents to be matched.
 */	

static struct balance {
	char left, right;
} bal[] = {
	{ '(', ')' },
	{ '[', ']' },
	{ '{', '}' },
	{ '<', '>' },
	{ '\0','\0'}
};

/*
 * Fake the GNU "blink-matching-paren" variable.
 * If the argument exists, nonzero means show,
 * zero means don't.  If it doesn't exist,
 * pretend it's nonzero.
 */

blinkparen(f, n, k)
{
	register char	*command;
	register SYMBOL	*sp;

	if (f == FALSE)
		n = 1;
	command = (n == 0) ? "self-insert-command" :
			     "blink-matching-paren-hack";
	if ((sp=symlookup(command)) == NULL) {
		ewprintf("blinkparen: no binding for %s",command);
		return (FALSE);
	}
	binding[(KEY) ')'] = sp;		/* rebind paren		*/
	return (TRUE);	
}

/*
 * Self-insert character, then show matching character,
 * if any.  Bound to "blink-matching-paren-command".
 */

showmatch(f, n, k)
{
	register int  i, s;

	if (k == KRANDOM)
		return(FALSE);
	for (i = 0; i < n; i++) {
		if ((s = selfinsert(f, 1, k)) != TRUE)
			return(s);
		if (balance(k) != TRUE)	/* unbalanced -- warn user */
			ttbeep();
	}
	return (TRUE);
}

/*
 * Search for and display a matching character.
 *
 * This routine does the real work of searching backward
 * for a balancing character.  If such a balancing character
 * is found, it uses displaymatch() to display the match.
 */

static balance(k)
int k;
{
	register LINE	*clp;
	register int	cbo;
	int	c;
	int	i;
	int	rbal, lbal;
	int	depth;

	rbal = k & KCHAR;
	if ((k&KCTRL)!=0 && rbal>='@' && rbal<='_') /* ASCII-ify.	*/
		rbal -= '@';

	/* See if there is a matching character -- default to the same */

	lbal = rbal;
	for (i = 0; bal[i].right != '\0'; i++)
		if (bal[i].right == rbal) {
			lbal = bal[i].left;
			break;
		}

	/* Move behind the inserted character.  We are always guaranteed    */
	/* that there is at least one character on the line, since one was  */
	/* just self-inserted by blinkparen.				    */

	clp = curwp->w_dotp;
	cbo = curwp->w_doto - 1;

	depth = 0;			/* init nesting depth		*/

	for (;;) {
		if (cbo == 0) {			/* beginning of line	*/
			clp = lback(clp);
			if (clp == curbp->b_linep)
				return (FALSE);
			cbo = llength(clp)+1;
		}
		if (--cbo == llength(clp))	/* end of line		*/
			c = '\n';
		else
			c = lgetc(clp,cbo);	/* somewhere in middle	*/

		/* Check for a matching character.  If still in a nested */
		/* level, pop out of it and continue search.  This check */
		/* is done before the nesting check so single-character	 */
		/* matches will work too.				 */
		if (c == lbal) {
			if (depth == 0) {
				displaymatch(clp,cbo);
				return (TRUE);
			}
			else
				depth--;
		}
		/* Check for another level of nesting.  */
		if (c == rbal)
			depth++;
	}
}


/*
 * Display matching character.
 * Matching characters that are not in the current window
 * are displayed in the echo line. If in the current
 * window, move dot to the matching character,
 * sit there a while, then move back.
 */

static displaymatch(clp, cbo)
register LINE *clp;
register int  cbo;
{
	register LINE	*tlp;
	register int	tbo;
	register int	cp;
	register int	bufo;
	register int	c;
	int		inwindow;
	char	 	buf[NLINE];

	/* Figure out if matching char is in current window by	*/
	/* searching from the top of the window to dot.		*/

	inwindow = FALSE;
	for (tlp = curwp->w_linep; tlp != lforw(curwp->w_dotp); tlp = lforw(tlp))
		if (tlp == clp)
			inwindow = TRUE;

	if (inwindow == TRUE) {
		tlp = curwp->w_dotp;	/* save current position */
		tbo = curwp->w_doto;

		curwp->w_dotp  = clp;	/* move to new position */
		curwp->w_doto  = cbo;
		curwp->w_flag |= WFMOVE;

		update();		/* show match */
		sleep(1);		/* wait a bit */

		curwp->w_dotp   = tlp;	/* return to old position */
		curwp->w_doto   = tbo;
		curwp->w_flag  |= WFMOVE;
		update();
	}
	else {	/* match not in this window so display line in echo area */
		bufo = 0;
		for (cp = 0; cp < llength(clp); cp++) {	/* expand tabs	*/
			c = lgetc(clp,cp);
			if (c != '\t')
				buf[bufo++] = c;
			else
				do {
					buf[bufo++] = ' ';
				} while (bufo & 7);
		}
		buf[bufo++] = '\0';
		ewprintf("Matches %s",buf);
	}
	return (TRUE);
}
