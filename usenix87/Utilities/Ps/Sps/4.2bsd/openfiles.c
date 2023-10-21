# include       <stdio.h>
# include       "sps.h"
# include       "flags.h"

/* Miscellaneous procedures */

/* OPENFILE - Opens the named file */
openfile ( name )

char                            *name ;

{
	register int            fd ;

	if ( (fd = open( name, 0 )) >= 0 )
		return ( fd ) ;
	fprintf( stderr, "sps - Can't open %s", name ) ;
	sysperror() ;
	/* NOTREACHED */
}

/* MEMSEEK - Seek on a special file */
memseek ( fd, pos )

int                             fd ;
long                            pos ;

{
	extern int              errno ;
	extern struct flags     Flg ;
	long                    lseek() ;

	errno = 0 ;
	if ( Flg.flg_k )
		pos &= 0x7fffffff ;
	(void)lseek( fd, pos, 0 ) ;
	if ( errno )
	{
		fprintf( stderr, "sps - Seek failed" ) ;
		sysperror() ;
	}
}

/* SWSEEK - Seek on the swap device */
swseek ( pos )

long                            pos ;

{
	extern int              Flswap ;
	extern int              errno ;
	long                    lseek() ;

	errno = 0 ;
	(void)lseek( Flswap, pos, 0 ) ;
	if ( errno )
	{
		fprintf( stderr, "sps - Seek failed" ) ;
		sysperror() ;
	}
}

# ifdef lint
int                             errno ;
int                             sys_nerr ;
char                            *sys_errlist[] ;
# endif

/* SYSPERROR - Reports a system defined error msg and then exits gracefully */
sysperror ()
{
	extern int              errno ;
	extern int              sys_nerr ;
	extern char             *sys_errlist[] ;

	if ( 0 < errno && errno < sys_nerr )
		fprintf( stderr, " : %s", sys_errlist[errno] ) ;
	(void)fputc( '\n', stderr ) ;
	exit( 1 ) ;
}

/* STRSAVE - Store a string in core for later use. */
char    *strsave ( cp )

register char                   *cp ;

{
	register char           *chp ;
	char                    *getcore(), *strcpy() ;

	chp = getcore( strlen( cp ) + 1 ) ;
	(void)strcpy( chp, cp ) ;
	return ( chp ) ;
}

/* GETCORE - Allocate and return a pointer to the asked for amount of core */
char    *getcore ( size )

register int                    size ;

{
	register char           *chp ;
	char                    *malloc() ;

	if ( chp = malloc( (unsigned)size ) )
		return ( chp ) ;
	fprintf( stderr, "sps - Out of core" ) ;
	sysperror() ;
	/* NOTREACHED */
}

union flaglist  *getflgsp ( argc )

register int                    argc ;

{
	char                    *getcore() ;

	return ( (union flaglist*)getcore( sizeof( union flaglist )*argc ) ) ;
}

/* PREXIT - Print an error message and exit */
/* VARARGS1 */
/* ARGSUSED */
prexit ( fmt, args )

char                            *fmt ;

{
	_doprnt( fmt, &args, stderr ) ;
	exit( 1 ) ;
}
