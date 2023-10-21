/* A few string functions missing from Sun unix */

#include <stdio.h>
#include "string.h"

/* Find the first occurrence of c in string */
char	*strchr( s, c )
char	*s, c ;
{
int	length, i ;
	length = strlen(s) ;

	for ( i=0; i<=length; i++ ) if ( s[i] == c ) return( &s[i] ) ;
	return( NULL ) ;
}

/* find the index of the first char in s1 that is not in s2 */
int	strspn( s1, s2 )
char	*s1, *s2 ;
{
int	i ;

	for ( i=0 ; s1[i] != NULL ; i++ ) {
		if ( NULL == strchr(s2,s1[i]) ) break ;
		}
	return(i) ;
}


/* find the index of the first char in s1 that is in s2 */
int	strcspn( s1, s2 )
char	*s1, *s2 ;
{
int	i ;

	for ( i=0 ; s1[i] != NULL ; i++ ) {
		if ( NULL != strchr(s2,s1[i]) ) break ;
		}
	return(i) ;
}
