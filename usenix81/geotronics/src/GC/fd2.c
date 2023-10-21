/*
	fd2 -- invoke command with stderr redirected

	last edit: 18-Feb-1981	D A Gwyn

Usage:
	% fd2[ -a] err_log command[ arguments]
	!	-a means "append to err_log instead of overwriting"
*/

#include	<stdio.h>

extern		execv(), exit(), strncpy();
extern char	*malloc();
extern int	fork(), strcmp(), system(), wait();

static char	usrbin[] = "/usr/bin/XXXXXXXXXXXXXX" ;

main( argc , argv )
	int		argc ;
	register char	*argv[];
	{
	register char	*ap , *sp ;
	char		*cmdlin ;	// command line buffer
	int		pid , status , wid ;

	if ( --argc < 2 )
		{
		fprintf( stderr ,
		"\07Usage: fd2[ -a] err_log command[ arguments]\n" );
		exit( 1 );
		}

	if ( strcmp( *++argv , "-a" ) == 0 )
		{
		-- argc ;
		if ( freopen ( *++argv , "a" , stderr ) == NULL )
			{
			freopen( "/dev/tty" , "w" , stderr );
			fprintf( stderr , "\07* fd2 * Can't append %s\n" ,
			*argv );
			exit( 1 );
			}
		}
	else
		if ( freopen( *argv , "w" , stderr ) == NULL )
			{
			freopen( "/dev/tty" , "w" , stderr );
			fprintf( stderr , "\07* fd2 * Can't write %s\n" ,
			*argv );
			exit( 1 );
			}
	++ argv ;

	while ( (pid = fork()) == -1 )
		;			// wait for resources
	if ( pid == 0 )
		{			// child process
		execv( *argv , argv );	// try directly
		strncpy( &usrbin[9] , *argv , 14 );
		execv( &usrbin[4] , argv );	// try /bin/command
		execv( usrbin , argv );	// try /usr/bin/command

		// pass to shell:
		cmdlin = sp = malloc( (unsigned)5120 );
		while ( -- argc > 0 )
			{
			ap = *argv++ ;	// -> argument string
			while ( *sp++ = *ap++ )
				;	// copy string
			sp[-1] = ` ' ;	// replaces null byte
			}
		sp[-1] = `\0' ;		// terminates command
		exit ( system ( cmdlin ) );
		}
	// parent process
	while ( (wid = wait( &status )) != pid && wid != -1 )
		;			// wait for non-signal

	if ( wid == -1 )
		status = 1 ;
	else
		status = status >> 8 & 0377 ;
	exit( status );
	}
