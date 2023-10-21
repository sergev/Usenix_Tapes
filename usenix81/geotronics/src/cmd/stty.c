/*
	stty -- set terminal options (compatible with 7th Edition UNIX)

	last edit:	22-Jun-1981	D A Gwyn
*/

#include	<sgtty.h>
#include	<stdio.h>

static struct	{
		char	*string ;
		int	speed ;
		}	speeds[] =
		{
		"0" ,		0 ,
		"50" ,		1 ,
		"75" ,		2 ,
		"110" ,		3 ,
		"134" ,		4 ,
		"150" ,		5 ,
		"200" ,		6 ,
		"300" ,		7 ,
		"600" ,		8 ,
		"1200" ,	9 ,
		"1800" ,	10 ,
		"2400" ,	11 ,
		"4800" ,	12 ,
		"9600" ,	13 ,
		"exta" ,	14 ,
		"extb" ,	15 ,
		NULL
		};

static struct	{
		char	*string ;
		int	set ;
		int	reset ;
		}	modes[] =
		{
		"even" ,
		EVENP , 0 ,

		"-even" ,
		0 , EVENP ,

		"odd" ,
		ODDP , 0 ,

		"-odd" ,
		0 , ODDP ,

		"raw" ,
		RAW , 0 ,

		"-raw" ,
		0 , RAW ,

		"cooked" ,
		0 , RAW ,

		"-nl" ,
		CRMOD , 0 ,

		"nl" ,
		0 , CRMOD ,

		"echo" ,
		ECHO , 0 ,

		"-echo" ,
		0 , ECHO ,

		"LCASE" ,
		LCASE , 0 ,

		"lcase" ,
		LCASE , 0 ,

		"-LCASE" ,
		0 , LCASE ,

		"-lcase" ,
		0 , LCASE ,

		"-tabs" ,
		XTABS , 0 ,

		"tabs" ,
		0 , XTABS ,

		"cr0" ,
		CR0 , CRDELAY ,

		"cr1" ,
		CR1 , CRDELAY ,

		"cr2" ,
		CR2 , CRDELAY ,

		"cr3" ,
		CR3 , CRDELAY ,

		"tab0" ,
		TAB0 , TBDELAY ,

		"tab1" ,
		TAB1 , TBDELAY ,

		"tab2" ,
		TAB2 , TBDELAY ,

		"tab3" ,
		TAB3 , TBDELAY ,

		"nl0" ,
		NL0 , NLDELAY ,

		"nl1" ,
		NL1 , NLDELAY ,

		"nl2" ,
		NL2 , NLDELAY ,

		"nl3" ,
		NL3 , NLDELAY ,

		"ff0" ,
		FF0 , VTDELAY ,

		"ff1" ,
		FF1 , VTDELAY ,

		"bs0" ,
		BS0 , BSDELAY ,

		"bs1" ,
		BS1 , BSDELAY ,

		"tty33" ,
		CR1 , ALLDELAY ,

		"tty37" ,
		FF1 + CR2 + TAB1 + NL1 , ALLDELAY ,

		"vt05" ,
		NL2 , ALLDELAY ,

		"tn300" ,
		CR1 , ALLDELAY ,

		"ti700" ,
		CR2 , ALLDELAY ,

		"tek" ,
		FF1 , ALLDELAY ,

		"hup" ,
		HUPCL , 0 ,

		"-hup" ,
		0 , HUPCL ,

		NULL
		};

static char		*arg ;
static struct sgttyb	mode ;

main( argc , argv )
	int	argc ;
	char	*argv[] ;
	{
	register int	i ;

	gtty( fileno( stderr ) , &mode );

	if ( argc == 1 )
		{
		prmodes();
		exit( 0 );
		}

	while ( --argc > 0 )
		{
		arg = *++argv ;

		if ( eq( "ek" ) )
			{
			mode.sg_erase = '#' ;
			mode.sg_kill = '@' ;
			}

		if ( eq( "erase" ) )
			{
			if ( (*++argv)[0] == '^' )
				mode.sg_erase = (*argv)[1] & 037 ;
			else
				mode.sg_erase = (*argv)[0] ;
			-- argc ;
			}

		if ( eq( "kill" ) )
			{
			if ( (*++argv)[0] == '^' )
				mode.sg_kill = (*argv)[1] & 037 ;
			else
				mode.sg_kill = (*argv)[0] ;
			-- argc ;
			}

		for ( i = 0 ; speeds[i].string ; ++ i )
			if ( eq( speeds[i].string ) )
				mode.sg_ispeed = mode.sg_ospeed =
						speeds[i].speed ;

		for ( i = 0 ; modes[i].string ; ++ i )
			if ( eq( modes[i].string ) )
				{
				mode.sg_flags &= ~ modes[i].reset ;
				mode.sg_flags |= modes[i].set ;
				}

		if ( arg )
			fprintf( stderr , "\07* stty * %s?\n" , arg );
		}

	stty( fileno( stderr ) , &mode );
	exit( 0 );
	}

static int
eq( strp )
	register char	*strp ;
	{
	register char	*argp = arg ;

	if ( argp == NULL )
		return ( 0 ) ;

	do
		if ( *argp != *strp++ )
			return ( 0 ) ;
		while ( *argp++ ) ;

	arg = NULL ;				/* processed */
	return ( 1 ) ;
	}

static
prmodes()
	{
	register int	m ;

	if ( mode.sg_ispeed != mode.sg_ospeed )
		{
		prspeed( "input speed " , mode.sg_ispeed );
		prspeed( "output speed" , mode.sg_ospeed );
		}
	else
		prspeed( "speed" , mode.sg_ispeed );

	if ( mode.sg_erase < 040 )
		printf( "erase = ^%c; " , mode.sg_erase + 0100 );
	else if ( mode.sg_erase == 0177 )
		printf( "erase = DEL; " );
	else
		printf( "erase = `%c'; " , mode.sg_erase );

	if ( mode.sg_kill < 040 )
		printf( "kill = ^%c\n" , mode.sg_kill + 0100 );
	else if ( mode.sg_kill == 0177 )
		printf( "kill = DEL\n" );
	else
		printf( "kill = `%c'\n" , mode.sg_kill );

	m = mode.sg_flags ;
	if ( m & EVENP )
		printf( "even " );
	if ( m & ODDP )
		printf( "odd " );
	if ( m & RAW )
		printf( "raw " );
	if ( m & CRMOD )
		printf( "-nl " );
	if ( m & ECHO )
		printf( "echo " );
	if ( m & LCASE )
		printf( "lcase " );
	if ( (m & XTABS) == XTABS )
		printf( "-tabs " );
	if ( m & HUPCL )
		printf( "hup " );

	delay( m >> 8 , "nl" );
	delay( m >> 10 , "tab" );
	delay( m >> 12 , "cr" );
	delay( (m >> 14) & 01 , "ff" );
	delay( (m >> 15) & 01 , "bs" );

	putchar( '\n' );
	}

static
delay( m , s )
	int	m ;
	char	*s ;
	{
	if ( m &= 03 )
		printf( "%s%d " , s , m );
	}

static
prspeed( c , s )
	char	*c ;
	int	s ;
	{
	static int	speed[14] =
		{
		0 , 50 , 75 , 110 , 134 , 150 , 200 , 300 ,
		600 , 1200 , 1800 , 2400 , 4800 , 9600
		};

	if ( s >= 14 )
		printf( "%s ext%c\n" , c , 'a' + (s - 14) );
	else
		printf( "%s %d baud\n" , c , speed[s] );
	}
