/*
	cp -- copy file[s] (compatible with 7th Edition UNIX)

	last edit: 25-Jan-1981	D A Gwyn
*/

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>

extern char		*rindex() , *strncat() , *strncpy();
extern int		creat() , fstat() , stat();

static struct stat	inst , outst ;

#define	MAXNAME	200
static char		catbuf[MAXNAME+1];

static char		*dest ;		/* destination (last arg) */
static int		todir ;		/* 1 => destination is dir. */
static char		*newname ;	/* individual destination */

static int		cp();


main( argc , argv )
	register int	argc ;
	register char	*argv[];
	{
	register int	errs = 0 ;	/* tallys "cp" failures */

	if ( --argc < 2 )
		usage();

	todir = stat( dest = argv[argc] , &outst ) == 0 &&
		(outst.st_mode & S_IFMT) == S_IFDIR ;

	if ( ! todir && argc > 2 )
		usage();

	while ( --argc > 0 )
		errs += cp( *++argv );

	exit( errs );
	}


static int
cp( source )				/* copy single file */
	char		*source ;
	{
	register int	c ;
	int		exists ;	/* 1 => dest already exists */

	if ( freopen( source , "r" , stdin ) == NULL ||
	     fstat( fileno( stdin ) , &inst ) != 0 )
		{
		perror( source );
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
			fputs( ": Copying to itself\n" , stderr );
			return ( 0 );	/* "successful" */
			}
		}
	else
		close( creat( newname , 0600 ) );	/* security */

	if ( freopen( newname , "w" , stdout ) == NULL )
		{
		perror( newname );
		return ( 1 );
		}

	while ( (c = getchar()) != EOF )
		{
		putchar( c );
		if ( ferror( stdout ) )
			{
			perror( newname );
			return ( 1 );
			}
		}

	if ( ! exists )
		{
		fclose( stdout );
		chmod( newname , inst.st_mode );
		}

	if ( (inst.st_mode & S_IFMT) == S_IFDIR )
		{			/* copied, but warn user */
		fputs( source , stderr );	/* avoid "fprintf" */
		fputs( ": Directory\n" , stderr );
		}

	return ( 0 );
	}


static
usage()					/* print usage */
	{
	error( "Usage: cp source_file[ ...] destination" );
	}


static
error( msg )				/* print fatal error message */
	char	*msg ;
	{
	fputs( "\07* cp * " , stderr );	/* avoid "fprintf" */
	fputs( msg , stderr );
	putc( '\n' , stderr );
	exit( 1 );
	}
