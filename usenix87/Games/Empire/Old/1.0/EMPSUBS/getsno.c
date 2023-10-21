#define D_FILES
#define D_UPDATE
#include	"empdef.h"

getsno(cp, np)
char	*cp, *np;
{
	char	*getstri();

	if( cp == 0 || *cp == '\0' ) {
		cp = getstri(np);
	}
	np = cp;
	sx = atoip(&cp);
	if( *cp++ != ',' ) {
		printf("Format is x,y\n");
		return(-1);
	}
	sy = atoip(&cp);
	return(getsect(sx, sy, UP_OWN));
}
