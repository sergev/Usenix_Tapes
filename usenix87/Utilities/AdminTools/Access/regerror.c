#include <stdio.h>

extern char *progname;

void
regerror(s)
char *s;
{
	fprintf(stderr, "%s: regexp(3): %s", progname, s);
	exit(1);
}
