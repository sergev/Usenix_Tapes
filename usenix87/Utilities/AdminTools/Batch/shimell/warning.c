#ifndef lint
#ifndef NSCCS
static char sccsid[] = "%W%";
#endif
#endif

#include <stdio.h>

extern char *program;

unsigned wcount;

/* VARARGS1 */
warning(s, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
char *s;
{
	wcount++;
	fflush(stdout);
	fprintf(stderr, "%s: warning, ", program);
	fprintf(stderr, s, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	fprintf(stderr, "\n");
}
