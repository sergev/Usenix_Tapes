#include	"empdef.h"

landorsea(cp)
char	*cp;
{
	register char	c;

	while( (c = *cp++) != '\0' ) {
		if( c == ',' || c == '#' ) return(LAND);
	}
	return(SEA);
}
