#ifndef lint
static char *RCSid = "$Header: utmp.c,v 1.1 87/04/15 21:29:39 josh Exp $";
#endif

/*
 *------------------------------------------------------------------
 * Copyright 1987, Josh Siegel
 * All rights reserved.
 *
 * Josh Siegel (siegel@hc.dspo.gov)
 * Dept of Electrical and Computer Engineering,
 * University of New Mexico,
 * Albuquerque , New Mexico
 * (505) 277-2497
 *
 *------------------------------------------------------------------
 *
 * $Source: /usr/pcb/josh/other/network/RCS/utmp.c,v $
 * $Revision: 1.1 $
 * $Date: 87/04/15 21:29:39 $
 * $State: Exp $
 * $Author: josh $
 * $Locker: josh $
 *
 *------------------------------------------------------------------
 *
 * $Log:	utmp.c,v $
 * Revision 1.1  87/04/15  21:29:39  josh
 * Initial revision
 * 
 *
 *------------------------------------------------------------------
 */


#include <stdio.h>
#include <utmp.h>
#include <pwd.h>
#include <sys/file.h>

/*
 * Allow the user to creat a entry in the utmp file. The problem is that this
 * shows the normal hack how to really cause problems via /etc/utmp.  I have
 * a program for the Sun and BSD43 that allows you to edit entries in
 * /etc/utmp but this is another story 
 */

setutmp(host)
	char            host[];
{
	struct passwd  *passwd;
	struct utmp     utmp;
	int             t, f;
	char           *p, *ttyname();

	/* Get the tty slot in the /etc/utmp file */
	t = ttyslot();

	if (t == 0)
		return (-2);	/* Not on a tty? */

	/*
	 * We are going to let the user set the host because it can't hurt
	 * and I use it for my window managers 
	 */

	strncpy(utmp.ut_host, host, 8);

	/* Set the login time to the current time */

	utmp.ut_time = time(0);

	/* Lets get a path to the tty device */

	p = ttyname(0);

	if (p == NULL)
		return (-1);	/* No device and a tty slot? weard! */

	strcpy(utmp.ut_line, p + 5);

	/* Who is the bum we are dealing with */

	passwd = getpwuid(getuid());

	strcpy(utmp.ut_name, passwd->pw_name);

	/* Can I open the utmp file?  On Suns I can */

	f = open("/etc/utmp", O_WRONLY);

	if (f == -1)
		return (-3);	/* Guess your not on a sun or not root */

	lseek(f, (long) (t * sizeof(utmp)), 0);	/* Move out to the place */
	write(f, (char *) &utmp, sizeof(utmp));	/* Write it */
	close(f);		/* Close it */

	/* What a nice guy I am :-) */
}

/*
 * Clear the entry from the utmp file.  This only clears the entry your 0
 * descripter is attached to.  Work it out...  Its a hack!
 */

clrutmp()
{
	int             t, f;
	struct utmp     utmp;

	t = ttyslot();

	if (t == 0)
		return (-1);	/* descripter 0/1/2 not on a tty */
	bzero((char *) &utmp, sizeof(utmp));
	/* I don't trust machines to give me clean stack each time */

	f = open("/etc/utmp", O_WRONLY);
	if (f < 0)
		return (-1);	/* Must not have access to /etc/utmp. */

	lseek(f, (long) (t * sizeof(utmp)), 0);
	write(f, (char *) &utmp, sizeof(utmp));	/* Clear the entry */
	close(f);
	return(0);
}
