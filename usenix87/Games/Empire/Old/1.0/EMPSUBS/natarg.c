#define D_FILES
#include	"empdef.h"

natarg(cp, np)
char	*cp, *np;
{
	register	n;
	char	*getstar();

	cp = getstar(cp, np);
	n = (*cp >= '0' && *cp <= '9') ? atoi(cp) : cnumb(cp);
	if( n < 0 || n > maxcno ) {
		printf("No such country exists.\n");
		n = -1;
	}
	return(n);
}
