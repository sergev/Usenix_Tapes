#
/*  This is the main loop of vsh.  Each interation processes a command.
    Commands are one character long.  They are acquired in raw mode
    and processed without the need to press return.  The goal of vsh
    is to minimize keypresses as much as possible.
*/

#include "hd.h"
#include "mydir.h"

process () 
{
    register cmd, next;	/* single character command */

    next = REPLOT;
    for (;;) 
    {			/* loop forever */
	if (next & REPLOT)
		dispdir (1);
	atxy (1, window-1);
	printf (" %s", BC);
	if (next & REPLOT)
		chkmail();
	cmd = getch ();
#ifdef	PWBTTY
	/* dyt flush input */
	tty_push(COOKEDMODE);
	tty_pop();
#endif
	printf ("%s", BC);

	next = command (cmd, DIRCMD);
    }
}

chkmail()
{
	static time_t mailtime = 0;
	long clock;

	if (*MAIL == 0)
		return;
	time(&clock);

	if (clock - mailtime < MAILCLK)
		return;

	if (stat(MAIL, &scr_stb) == 0
#ifdef	V6
		&& scr_stb.st_size1
#else
		&& scr_stb.st_size
#endif
		&& scr_stb.st_atime <= scr_stb.st_mtime
		&& (scr_stb.st_atime > mailtime || scr_stb.st_mtime > mailtime)
		&& mailtime)
		putmsg("You have new mail");

	mailtime = clock;
}
