/*
 * Copyright 1986 by Larry Campbell, 73 Concord Street, Maynard MA 01754 USA
 * (maynard!campbell).  You may freely copy, use, and distribute this software
 * subject to the following restrictions:
 *
 *  1)	You may not charge money for it.
 *  2)	You may not remove or alter this copyright notice.
 *  3)	You may not claim you wrote it.
 *  4)	If you make improvements (or other changes), you are requested
 *	to send them to me, so there's a focal point for distributing
 *	improved versions.
 *
 * John Chmielewski (tesla!jlc until 9/1/86, then rogue!jlc) assisted
 * by doing the System V port and adding some nice features.  Thanks!
 */

#include <stdio.h>
#include "x10.h"

char *E_2MANY = EM_2MANY;
char *E_INVCN = EM_INVCN;
char *E_WNA   = EM_WNA;
char *E_NMA   = EM_NMA;
char *E_NOCMD = EM_NOCMD;

void exit();

usage(s)
char *s;
{

/*
 * Don't combine the two calls to fputs or my compiler will
 * gag with "token too long"
 */

(void) fprintf(stderr, "Command error: %s\n", s);
(void) fputs("Usage:\n\
 x10 data ann on|off id      store id with housecode a, unit nn and state\n\
 x10 date                    returns date in date(1) input format\n\
 x10 delete events|data nums deletes specified events or data slots\n\
 x10 diagnostic              run interface self-test diagnostics\n\
 x10 dump   events|data|all  dumps events or data or both\n\
 x10 fdump  events|data      dumps binary events or data to stdout\n", stderr);
(void) fputs("\
 x10 finfo  events|data      displays contents of event or data file on stdin\n\
 x10 fload  events|data      loads interface from event file on stdin\n\
 x10 info                    gets current settings\n\
 x10 monitor                 monitor X10 activity (end with <BREAK>)\n\
 x10 reset  [housecode]      reset interface to 'A' or specified housecode\n\
 x10 schedule ann[,nn...] mode [day] hh:mm on|off|dim [vv]     schedule event\n\
 x10 setclock                sets X10 clock to system clock\n\
 x10 unit ann[,nn...] on|off|dim [vv]    turns unit nn, housecode a, to state\n\
", stderr);
exit(1);
}

error(s)
char *s;
{
(void) fprintf(stderr, "x10: %s\n", s);
quit();
}
