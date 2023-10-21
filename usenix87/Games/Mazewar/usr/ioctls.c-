#ifndef lint
static char rcsid[] = "$Header: ioctls.c,v 1.1 84/08/25 17:11:20 chris Exp $";
#endif

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sgtty.h>
#include "../h/defs.h"
#include "../h/struct.h"
#include "../h/extern.h"

struct sgttyb  _tty_norm;    /* the normal modes */
struct sgttyb  _tty_crnt;    /* the changed modes */

setioctls()
{
	(void) ioctl(0, (int) TIOCGETP, (char *) &_tty_norm);
#ifdef	NOASGNSTRUCT
	bcopy(&_tty_norm, &_tty_crnt, sizeof _tty_norm);
#else
	_tty_crnt = _tty_norm;
#endif
	_tty_crnt.sg_flags |= CBREAK;
	_tty_crnt.sg_flags &= ~(ECHO|RAW);
	(void) ioctl(0, (int) TIOCSETP, (char *) &_tty_crnt);
}

unsetioctls()
{
	(void) ioctl(0, (int) TIOCSETP, (char *) &_tty_norm);
}

/*
** Flush type-ahead
*/
takill()
{
    int takill();
    (void) signal(SIGINT, takill);
    (void) ioctl(fileno(stdin), (int) TIOCFLUSH, (char *) NULL);
    (void) rewind(stdin);
}
