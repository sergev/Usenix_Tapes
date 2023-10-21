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
#ifdef SYSV
#include <string.h>
#else
#include <strings.h>
#endif
#include "x10.h"

struct id id[DTOTAL];

readid()
{
register n, cnt = 0, c;
unsigned num;
char xfile[100];
int noskip;
FILE *idfile;

(void) strcat(strcpy(xfile, XDIR), IDFILE);

if ((idfile = fopen(xfile, "r")) == NULL)
    {
    (void) fprintf(stderr,
	"Warning: cannot open description file %s\n", xfile);
    return;
    }

while(++cnt)
    {
    if (fscanf(idfile, "%d%*[ \t]", &num) == EOF)
	break;
    if (num < 1 || num > DTOTAL -1) (void) fprintf(stderr,
	"Warning: id number out of range, ignored line %d in %s\n",
	cnt, xfile), noskip = 0;
    else noskip = 1;
    for (n = 0; (c = fgetc(idfile)) != EOF; n++)
	{
	if (c == '\n') break;
	if (n < DLENGTH - 1 && noskip)
	    id[num].describe[n] = (char) c;
	}
    if (n >= DLENGTH - 1) (void) fprintf(stderr,
	"Warning: description truncated, line %d in %s\n",
	cnt, xfile);
    }
    (void) fclose(idfile);
}
