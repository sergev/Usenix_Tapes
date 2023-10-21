#ifndef lint
static char *RCSid = "$Header: logfile.c,v 1.7 86/12/02 20:27:42 mcooper Locked $";
#endif

/*
 *------------------------------------------------------------------
 *
 * $Source: /big/src/usc/bin/gr/RCS/logfile.c,v $
 * $Revision: 1.7 $
 * $Date: 86/12/02 20:27:42 $
 * $State: Exp $
 * $Author: mcooper $
 * $Locker: mcooper $
 *
 *------------------------------------------------------------------
 *
 * Michael Cooper (mcooper@oberon.USC.EDU)
 * University Computing Services,
 * University of Southern California,
 * Los Angeles, California,   90089-0251
 * (213) 743-3469
 *
 *------------------------------------------------------------------
 * $Log:	logfile.c,v $
 * Revision 1.7  86/12/02  20:27:42  mcooper
 * "#ifdef DEBUG"'s all dprintf's.
 * 
 * Revision 1.6  86/07/15  14:54:18  mcooper
 * Determine user name in setup() instead of
 * everytime logfile() is called.
 * 
 * Revision 1.5  86/07/14  15:54:42  mcooper
 * Moved most of the compile time configuration
 * options to the control file.
 * 
 * Revision 1.4  86/05/14  16:29:12  mcooper
 * Somewhat de-linted.
 * 
 * Revision 1.3  86/03/25  15:44:35  mcooper
 * more of same.
 * 
 * Revision 1.2  86/03/25  15:35:43  mcooper
 * New headers...
 * 
 *------------------------------------------------------------------
 */


#include <stdio.h>
#include "gr.h"

extern int debug;
extern char *user;
extern char *mytty;

logfile(msg, file)
char *msg;
char *file;
{
	char *ctime();
	double now;
	FILE *fd, *fopen();

	(void) time(&now);

#ifdef DEBUG
	dprintf("logfile(): user = '%s' tty = '%s'\n", user, mytty);
#endif DEBUG

	if((fd = fopen(file, "a")) != NULL){
		(void) fprintf(fd, "%-9s tty%-3s %10.24s  [ %s ]\n", 
				user, mytty, ctime(&now), msg);
		(void) fclose(fd);
	}
}
