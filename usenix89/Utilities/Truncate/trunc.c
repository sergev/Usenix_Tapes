
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>

main( argc, argv )
int	argc;
char	**argv;
{

	extern	int errno;
	if( argc < 3 ) {
		fprintf( stderr, "Usage:%s file size [file size [file size...]]\n", argv[0] );
		exit( 1 );
	}
	for( argc--, argv++; argc > 1; argc-=2, argv+=2 ) {
		errno = 0;
		if( truncate( argv[0], atoi( argv[1] )) == -1 ) {
			fprintf( stderr, "file %s %d truncate failed\n", argv[0], atoi( argv[1] ) );
			if( errno ) {
				perror("" );
			}
		}
	}
	exit( 0 );
}

#define	TMPNAME	"tmpXXXXXX"

truncate( name, size )
char	*name;
int	size;
{

	char*	mktemp();
	char*	rindex();
	char*	malloc();
	char	*Myname;					  /* Temp file name */
	char	*slash;					  /* Where the last '/' is in Myname */
	FILE*	fopen();
	FILE*	fdopen();
	FILE	*InFd;
	FILE	*OutFd;
	int	RawFd;
	int	c;
	char	IoBuf[BUFSIZ];
	struct	stat	StatBuf;

	if( stat( name, &StatBuf ) == -1 ) {
#ifdef	TRUNC_DEBUG				  /* TRUNC_DEBUG */
		fprintf( stderr, "stat of %s failed\n", name );
#endif								  /* TRUNC_DEBUG */
		return -1;
	}
	if( StatBuf.st_size <= size ) {
		return 1;
	}
	if( StatBuf.st_uid != geteuid() || StatBuf.st_gid != getegid() ) {
#ifdef	TRUNC_DEBUG				  /* TRUNC_DEBUG */
		if( StatBuf.st_uid != geteuid() ) {
			fprintf( stderr, "file uid %d, euid %d\n", StatBuf.st_uid, geteuid() );
		}
		if( StatBuf.st_gid != getegid() ) {
			fprintf( stderr, "file gid %d, egid %d\n", StatBuf.st_gid, getegid() );
		}
#endif								  /* TRUNC_DEBUG */
		return -1;
	}
	if( !(Myname = malloc( strlen(name) + 1 + strlen( TMPNAME ) )) ) {
#ifdef	TRUNC_DEBUG				  /* TRUNC_DEBUG */
		fprintf( stderr, "malloc failed" );
#endif								  /* TRUNC_DEBUG */
		return -1;
	}
	(void) strcpy( Myname, name );
	slash = rindex( Myname, '/' );
	if( *slash && *(slash+1) ) {
		(void) strcpy( (slash+1), mktemp( TMPNAME ));
	} else {
		(void) strcpy( Myname, mktemp( TMPNAME )); /* Not a path,  just a file name */
	}
	if( (RawFd = creat( Myname, (StatBuf.st_mode & (~S_IFMT)))) == -1 ) {
		(void) free( Myname );
#ifdef	TRUNC_DEBUG				  /* TRUNC_DEBUG */
		fprintf( stderr, "creat on %s failed", Myname );
#endif								  /* TRUNC_DEBUG */
		return -1;
	}
	if( !(OutFd = fdopen( RawFd, "w" )) ) {
		(void) free( Myname );
#ifdef	TRUNC_DEBUG				  /* TRUNC_DEBUG */
		fprintf( stderr, "fdopen failed" );
#endif								  /* TRUNC_DEBUG */
		return -1;
	}
	if( !(InFd = fopen( name, "r" )) ) {
		(void) free( Myname );
		(void) fclose( OutFd );
#ifdef	TRUNC_DEBUG				  /* TRUNC_DEBUG */
		fprintf( stderr, "fopen of %s failed\n", name );
#endif								  /* TRUNC_DEBUG */
		return -1;
	}
	for( ; size> 0 && (c = fgetc(InFd)) != EOF && ! ferror( InFd ) && ! ferror( OutFd ); size-- ) {
		(void) fputc( c, OutFd );
	}
	(void) fclose( InFd );
	(void) fclose( OutFd );
	if( size != 0 && !feof( InFd ) ) {
		(void) unlink( Myname );
		(void) free( Myname );
#ifdef	TRUNC_DEBUG				  /* TRUNC_DEBUG */
		fprintf( stderr, "size is %d\n", size );
#endif								  /* TRUNC_DEBUG */
		return -1;
	} else {
		if( unlink( name ) == -1 ) {
			(void) unlink( Myname );
			(void) free( Myname );
#ifdef	TRUNC_DEBUG				  /* TRUNC_DEBUG */
			fprintf( stderr, "unlink of %s failed\n", name );
#endif								  /* TRUNC_DEBUG */
			return -1;
		}
		if( rename( Myname, name ) == -1 ) {
			(void) free( Myname );
#ifdef	TRUNC_DEBUG
			fprintf( stderr, "rename of %s to %s failed\n", Myname, name );
#endif								  /* TRUNC_DEBUG */
			return -1;
		}
		return 0;
	}
}
