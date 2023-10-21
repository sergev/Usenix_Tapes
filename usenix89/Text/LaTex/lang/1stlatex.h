#include <osbind.h>


/* define this if need be */
#define	DEBUG

#ifdef EXT
#define EXT
#else
#define EXT extern
#endif

#include <stdio.h>

#define EXSPACE '\034'	/* expandable -- stretch space */
#define INSPACE '\035'	/* indent space */
#define VSPACE '\036'	/* variable space */
#define output( c )	putc( c, Outfile );
#define outstr( s )	fputs( s, Outfile );

typedef	int	FILEDESC;
typedef int	boolean;

EXT FILE	*Outfile;	/* use gemlib for going out */
EXT FILEDESC	Infile;		/* use bios for reading in */

EXT boolean	underline;	/* unresolved underline: do no make
				   new paragraph, kludge ALERT */
