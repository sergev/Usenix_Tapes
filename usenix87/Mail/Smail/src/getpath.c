#ifndef lint
static char 	*sccsid="@(#)getpath.c	2.2 (smail) 1/26/87";
#endif

# include	<stdio.h>
# include	<sys/types.h>
# include	<ctype.h>
# include	"defs.h"
#ifdef BSD
#include <strings.h>
#else
#include <string.h>
#endif

extern enum edebug debug;	/* how verbose we are 		*/ 
extern char *pathfile;		/* location of path database	*/

/*
**
** getpath(): look up key in ascii sorted path database.
**
*/

getpath( key, path , cost)
char *key;		/* what we are looking for */
char *path;		/* where the path results go */
int *cost;		/* where the cost results go */
{
	long pos, middle, hi, lo;
	static long pathlength = 0;
	register char *s;
	int c;
	static FILE *file;
	int flag;

DEBUG("getpath: looking for '%s'\n", key);

	if( !pathlength )	/* open file on first use */
	{
		if( ( file=fopen( pathfile, "r" ) ) == NULL )
		{
			(void) printf( "can't access %s.\n", pathfile );
			pathlength = -1;
		} else {
			(void) fseek( file, 0L, 2 );		/* find length */
			pathlength = ftell( file );
		}
	}
	if( pathlength == -1 )
		return( EX_OSFILE );

	lo = 0;
	hi = pathlength;
	(void) strcpy( path, key );
	(void) strcat( path, "\t" );
/*
** "Binary search routines are never written right the first time around."
** - Robert G. Sheldon.
*/
	for( ;; ) {
		pos = middle = ( hi+lo+1 )/2;
		(void) fseek( file, pos, 0 );	/* find midpoint */
		if (pos != 0)		/* to beginning of next line */
			while( ( c=getc( file ) ) != EOF && c != '\n' );
		for( flag = 0, s = path; flag == 0; s++ ) { /* match??? */
			if ( *s == '\0' ) {
				goto solved;
			}
			c = getc( file );
			flag = lower( c ) - lower( *s );
		} 
		if ( lo>=middle )		/* failure? */
			return( EX_NOHOST );
		if ( c != EOF && flag < 0 )	/* close window */
			lo = middle;
		else 
			hi = middle - 1;
	}
/* 
** Now just copy the result.
*/
solved:
	while(((c  = getc(file)) != EOF) && (c != '\t') && (c != '\n')) {
		*path++ = c;
	}
	*path = '\0';
/*
** See if the next field on the line is numeric.
** If so, use it as the cost for the route.
*/
	if(c == '\t') {
		int tcost = -1;
		while(((c = getc(file)) != EOF) && isdigit(c)) {
			if(tcost < 0) tcost = 0;
			tcost *= 10;
			tcost += c - '0';
		}
		if(tcost >= 0) *cost = tcost;
	}
	return ( EX_OK );
}
