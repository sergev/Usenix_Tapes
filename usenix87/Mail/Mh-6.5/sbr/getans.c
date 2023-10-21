/* getans.c - get an answer from the user and return a string array */

#include "../h/mh.h"
#ifdef	BSD42
#include <setjmp.h>
#endif	BSD42
#include <signal.h>
#include <stdio.h>


static	char ansbuf[BUFSIZ];
#ifndef	BSD42
static	int interrupted;
#else	BSD42
static	jmp_buf sigenv;
#endif	BSD42
int	intrser ();

char  **getans (prompt, ansp)
register char  *prompt;
register struct swit   *ansp;
{
    register int    i;
    int    (*istat) ();
    register char  *cp,
                  **cpp;

#ifndef	BSD42
    interrupted = 0;
    istat = signal (SIGINT, intrser);
#else	BSD42
    switch (setjmp (sigenv)) {
	case OK: 
	    istat = signal (SIGINT, intrser);
	    break;

	default: 
	    (void) signal (SIGINT, istat);
	    return NULL;
    }
#endif	BSD42
    for (;;) {
	printf ("%s", prompt);
	(void) fflush (stdout);
	cp = ansbuf;
	while ((i = getchar ()) != '\n') {
#ifndef	BSD42
	    if (i == EOF || interrupted) {
		interrupted = 0;
		(void) signal (SIGINT, istat);
		return NULL;
	    }
#else	BSD42
	    if (i == EOF)
		longjmp (sigenv, DONE);
#endif	BSD42
	    if (cp < &ansbuf[sizeof ansbuf - 1])
		*cp++ = i;
	}
	*cp = 0;
	if (ansbuf[0] == '?' || cp == ansbuf) {
	    printf ("Options are:\n");
	    printsw (ALL, ansp, "");
	    continue;
	}
	cpp = brkstring (ansbuf, " ", NULLCP);
	switch (smatch (*cpp, ansp)) {
	    case AMBIGSW: 
		ambigsw (*cpp, ansp);
		continue;
	    case UNKWNSW: 
		printf (" -%s unknown. Hit <CR> for help.\n", *cpp);
		continue;
	    default: 
		(void) signal (SIGINT, istat);
		return cpp;
	}
    }
}


static	int intrser () {
#ifndef	BSD42
	(void) signal(SIGINT, intrser);
	interrupted = 1;
#else	BSD42
	longjmp (sigenv, NOTOK);
#endif	BSD42
}
