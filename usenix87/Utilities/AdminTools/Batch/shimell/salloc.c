#include <stdio.h>

#ifndef lint
#ifndef NSCCS
static char sccsid[] = "%W%";
#endif
#endif

extern char *malloc();
extern char *strcpy();

/*
 * Allocate and copy a string
 * into dynamic memory.
 */

char *
salloc(nam)
char *nam;
{
	register char *p;

	if ((p = malloc((unsigned)strlen(nam) + 1)) == NULL)
		quit("dynamic memory exhausted");
	return(strcpy(p, nam));
}
