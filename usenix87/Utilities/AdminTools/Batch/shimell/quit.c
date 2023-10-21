#ifndef lint
#ifndef NSCCS
static char sccsid[] = "%W%";
#endif
#endif

int fail_status = 1;	/* Exit status on error */

#include <stdio.h>

extern char *program;

/* VARARGS1 */
quit(s, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
char *s;
{
	fflush(stdout);
	fprintf(stderr, "%s: fatal, ", program);
	fprintf(stderr, s, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	fprintf(stderr, "\n");
	exit(fail_status);
}
