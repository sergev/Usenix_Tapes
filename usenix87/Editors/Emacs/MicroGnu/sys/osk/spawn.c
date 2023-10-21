/*
 * Name:	MicroEMACS
 *		OS9/68k Spawn Shell
 * Version:	29ral
 * Last edit:	04/20/86
 * By:		Blarson@Usc-Ecl.Arpa
 *
 */
#include	"def.h"

#include	<sgstat.h>

extern	struct	sgbuf	oldtty;		/* There really should be a	*/
extern	struct	sgbuf	newtty;		/* nicer way of doing this, so	*/

spawncli(f, n, k)
					/* what are f, n, and k?  They 	*/
					/* arn't used by ultrix, so I	*/
					/* ignore them too.		*/
{
	register int	pid;
	register int	wpid;
	int		status;

	ttcolor(CTEXT);
	ttnowindow();
	ttmove(nrow-1, 0);
	if (epresf != FALSE) {
		tteeol();
		epresf = FALSE;
	}
	ttflush();
	if(_ss_opt(0, &oldtty) == -1) {
		ewprintf("_ss_opt #1 to terminal failed");
		return (FALSE);
	}
	if((pid=os9fork("shell", 0, "", 0, 0, 0, 0)) == -1) {
		ewprintf("Failed to create process");
		return (FALSE);
	}
	while ((wpid=wait(&status))>=0 && wpid!=pid)
		;
	sgarbf = TRUE;				/* Force repaint.	*/
	if(_ss_opt(0, &newtty) == -1) {
		ewprintf("_ss_opt #2 to terminal failed");
		return (FALSE);
	}
	return (TRUE);
}
