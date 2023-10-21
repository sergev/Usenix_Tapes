#ifndef lint
static char rcsid[] = "$Header: ioctls.c,v 1.1 84/08/25 17:11:20 chris Exp $";
#endif

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#ifdef PLUS5
#include <termio.h>
#else
#include <sgtty.h>
#endif
#include "../h/defs.h"
#include "../h/struct.h"
#include "../h/extern.h"

#ifdef PLUS5
struct termio  _tty_norm;    /* the normal modes */
struct termio  _tty_crnt;    /* the changed modes */
#else
struct sgttyb  _tty_norm;    /* the normal modes */
struct sgttyb  _tty_crnt;    /* the changed modes */
#endif

#ifdef PLUS5
setioctls()
{
	ioctl(0, TCGETA, &_tty_norm);
	_tty_crnt = _tty_norm;
}
#else
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
#endif

#ifdef PLUS5
unsetioctls()
{
	ioctl(0, TCSETA, &_tty_norm);
}
#else
unsetioctls()
{
	(void) ioctl(0, (int) TIOCSETP, (char *) &_tty_norm);
}
#endif

/*
** Flush type-ahead
*/
#ifdef PLUS5
takill()
{
    int takill();
    (void) signal(SIGINT, takill);
    (void) ioctl(fileno(stdin), (int) TCFLSH, (char *) NULL);
    (void) rewind(stdin);
}
#else
takill()
{
    int takill();
    (void) signal(SIGINT, takill);
    (void) ioctl(fileno(stdin), (int) TIOCFLUSH, (char *) NULL);
    (void) rewind(stdin);
}
#endif
