#include	<stdio.h>

char	*
strchr(s, c)
char	*s, c;
{
	register char	*p;

	for( p = s; *p != '\0'; p++ ) {
		if( *p == c ) return(p);
	}
	return((char *)NULL);
}
