#include <stdio.h>

char *_argv0;			/* argv[0], set by C startup code */

/*
 * error - University of Maryland specific (sigh)
 *
 * Useful for printing error messages.  Will print the program name
 * and (optionally) the system error associated with the values in
 * <errno.h>.
 *
 * Note that the type (and even the existence!) of ``arg'' is undefined.
 */
error(quit, e, fmt, arg)
	int quit;
	register int e;
	char *fmt;
{
	extern char *sys_errlist[];
	extern int sys_nerr;
	register char *p = _argv0;

	if (p != NULL) {
#ifdef optional
		char *s, *rindex();

		if ((s = rindex(p, '/')) != NULL)
			p = s + 1;
#endif
		(void) fprintf(stderr, "%s: ", p);
	}
	_doprnt(fmt, &arg, stderr);	/* magic */
	if (e > 0) {
		if (e < sys_nerr)
			(void) fprintf(stderr, ": %s", sys_errlist[e]);
		else
			(void) fprintf(stderr, ": unknown error number %d", e);
	}
	(void) putc('\n', stderr);
	(void) fflush(stderr);
	if (quit)
		exit(quit);
}
