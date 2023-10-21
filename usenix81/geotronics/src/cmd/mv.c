/*
	mv -- move file[s] (compatible with 7th Edition UNIX)

	last edit: 25-Jan-1981	D A Gwyn
*/

#define	OLDUNIX	pre-version 6.5 clobbers args on exec

#include	<errno.h>
#include	<signal.h>
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>

extern char		*rindex() , *strncat() , *strncpy();
extern int		access() , fork() , link() ,
			stat() , unlink() , wait();
extern int		errno ;

#define	WACCESS	2			/* "access" write mode */

static int		oldint , oldquit , oldterm ;	/* signals */

static struct stat	inst , outst ;

static FILE		*userin = NULL ;	/* for user approval */
#define	MAXLINE	100
static char		resp[MAXLINE];

#define	MAXNAME	200
static char		catbuf[MAXNAME+1];

static char		*dest ;		/* destination (last arg) */
static int		todir ;		/* 1 => destination is dir. */
static char		*newname ;	/* individual destination */
static int		errs = 0 ;	/* tallys "mv" failures */

static int		copy() , mv();


main( argc , argv )
	register int	argc ;
	register char	*argv[];
	{
	if ( --argc < 2 )
		usage();

	todir = stat( dest = argv[argc] , &outst ) == 0 &&
		(outst.st_mode & S_IFMT) == S_IFDIR ;

	if ( ! todir && argc > 2 )
		usage();

	signal( SIGHUP , SIG_IGN );

	while ( --argc > 0 )
		errs += mv( *++argv );

	exit( errs );
	}


static int
mv( source )				/* move single file */
	register char	*source ;
	{
	register int	exists ;	/* 1 => dest already exists */

	if ( stat( source , &inst ) != 0 )
		{
		perror( source );
		return ( 1 );
		}

	/* The following differs from "mv" in 6th Edition UNIX: */
	if ( (inst.st_mode & S_IFMT) == S_IFDIR )
		{
		fputs( source , stderr );	/* avoid "fprintf" */
		fputs( ": Directory not movable\n" , stderr );
		return ( 1 );
		}

	if ( todir )
		{
		register char	*cp = rindex( source , '/' );

		if ( cp == NULL )
			cp = source ;	/* no `/' in source name */
		else
			++ cp ;		/* past last `/' in name */
		newname = strncat( strncat( strncpy( catbuf ,
						     dest , MAXNAME ) ,
					    "/" , MAXNAME ) ,
				   cp , MAXNAME );
		}
	else
		newname = dest ;

	if ( exists = stat( newname , &outst ) == 0 )
		{			/* destination exists */
		if ( outst.st_dev == inst.st_dev &&
		     outst.st_ino == inst.st_ino )
			{
			fputs( newname , stderr );
			fputs( ": Moving to itself\n" , stderr );
			return ( 0 );	/* "successful" */
			}

		/* We can't rely on "unlink" to fail for super-user */
		if ( (outst.st_mode & S_IFMT) == S_IFDIR )
			{
			fputs( newname , stderr );
			fputs( ": Would lose directory\n" , stderr );
			return ( 1 );
			}

		if ( access( newname , WACCESS ) != 0 )
			{		/* requires user approval */
			fputs( newname , stderr );
			fputs( ": mode " , stderr );
			outmode( outst.st_mode );
			if ( userin == NULL &&
			(userin = fopen( "/dev/tty" , "r" )) == NULL )
				return ( 1 );	/* no user on hand */
			fputs( "; go ahead? " , stderr );
			if ( fgets( resp , MAXLINE , userin ) == NULL )
				exit( ++errs );			
			if ( resp[0] != 'y' && resp[0] != 'Y' )
				return ( 1 );
			}

		disable();		/* disable interrupts */

		while ( unlink( newname ) != 0 )
			if ( errno != EINTR )
				{
				perror( newname );
				enable();
				return ( 1 );
				}
		}
	else
		disable();		/* disable interrupts */

	while ( link( source , newname ) != 0 )
		if ( errno == EINTR )
			continue ;	/* interrupted, try again */
		else if ( errno == EXDEV )	/* copy between devs */
			if ( copy( source , newname ) != 0 )
				{
				enable();
				return ( 1 );
				}
			else
				break ;
		else	{
			perror( source );
			if ( exists )
				{
				fputs( newname , stderr );
				fputs( ": Removed\n" , stderr );
				}
			enable();
			return ( 1 );
			}

	while ( unlink( source ) != 0 )
		if ( errno != EINTR )
			{
			perror( source );
			enable();
			return ( 1 );
			}

	enable();
	return ( 0 );
	}


static
outmode( mode )				/* output protection mode */
	register short	mode ;
	{
	register int	bit ;

	for ( bit = 9 ; bit >= 0 ; bit -= 3 )
		fputc( '0' + (mode >> bit & 07) , stderr );
	}


static int
copy( from , to )			/* copy file */
	register char	*from , *to ;
	{
	register int	pid ;		/* process ID from "fork" */
	int		status ;	/* for "wait" system call */

	while ( (pid = fork()) == -1 )
		;			/* wait for process space */

	if ( pid == 0 )
		{			/* child process */
#ifdef	OLDUNIX
		char	ftemp[MAXNAME+1], ttemp[MAXNAME+1];

		ftemp[MAXNAME] = ttemp[MAXNAME] = '\0' ;
		strncpy( ftemp , from , MAXNAME );
		strncpy( ttemp , to , MAXNAME );
		execl( "/bin/cp" , "cp" , ftemp , ttemp , NULL );
#else
		execl( "/bin/cp" , "cp" , from , to , NULL );
#endif
		fputs( "Somebody stole /bin/cp\n" , stderr );
		exit( 1 );
		}

	while ( wait( &status ) != pid )
		;			/* must have been signal */

	if ( status != 0 )
		{
		fputs( to , stderr );
		fputs( ": Copy failed\n" , stderr );
		}

	return ( status );
	}


static
disable()				/* disable interrupts */
	{
	oldint = signal( SIGINT , SIG_IGN );
	oldquit = signal( SIGQUIT , SIG_IGN );
	oldterm = signal( SIGTERM , SIG_IGN );
	}


static
enable()				/* re-enable interrupts */
	{
	signal( SIGINT , (int (*)())oldint );
	signal( SIGQUIT , (int (*)())oldquit );
	signal( SIGTERM , (int (*)())oldterm );
	}


static
usage()					/* print usage */
	{
	error( "Usage: mv source_file[ ...] destination" );
	}


static
error( msg )				/* print fatal error message */
	char	*msg ;
	{
	fputs( "\07* mv * " , stderr );	/* avoid "fprintf" */
	fputs( msg , stderr );
	putc( '\n' , stderr );
	exit( 1 );
	}
