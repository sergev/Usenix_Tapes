/*
 * baddies.c - find out who has tried to log in with bad name/password
 *
 *	last edit:  30-Aug-1980  D A Gwyn
 */

struct	{
	char		name[8];
	char		tty;
	char		tfill;
	int		time[2];
	int		fill;
	char		passwd[8];
	}	xtmp;

main( argc , argv )
	int	argc;
	char	*argv[];
	{
	int	f;

	if ( (f = open( "/etc/xtmp" , 0 )) < 0 )
		{
		write( 2 , "\07No history file.\07\n" , 19 );
		exit( 1 );
		}
	while ( read( f , &xtmp , 24 ) == 24 )
		printf( "%-8.8s %-8.8s tty%c %s" ,
			xtmp.name , xtmp.passwd , xtmp.tty ,
			ctime( xtmp.time ) );
	exit( 0 );
	}
