#ifndef lint
#ifndef NSCCS
static char sccsid[] = "%W%";
#endif
#endif

#include <stdio.h>

extern char *program;

unsigned ecount;

/* VARARGS1 */
error(s, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
char *s;
{
	ecount++;
	fflush(stdout);
	fprintf(stderr, "%s: error, ", program);
	fprintf(stderr, s, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	fprintf(stderr, "\n");
}
