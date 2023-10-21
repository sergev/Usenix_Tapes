/*
 * rtblap - copies text files from specified RT-11 device blocks
 *	(CRs [unless 6th argument supplied] and NULs are removed)
 *
 *	last edit: 13-Jun-1980  D A Gwyn
 */

main( argc, argv )
int	argc;
char	*argv[];
{
	int	devfd, fd, i;
	char	buf[512];

	if ( argc < 5  ||  argc > 7 )  {
		printf( "usage: " );
		printf( "rtblap device filename startblock length" );
		printf( " {cr {bin}}\n" );
		exit();
	} 
	if ( (fd = creat( argv[2], 0640 ))  <  0 )  {
		printf( "can't create %s\n", argv[2] );
		exit();
	} 
	if ( (devfd = open( argv[1], 0 ))  <  0 )  {
		printf( "can't open %s\n", argv[1] );
		exit();
	}
	seek( devfd, atoi( argv[3] ), 3 );
	for ( i = atoi( argv[4] );  i > 0;  i-- )
		if ( read( devfd, buf, 512 ) == 512 )
			write( fd, buf, shrnk( buf, argc ) );
		else  {
			printf( "read error rel blk %d\n",
				atoi( argv[4] ) - i );
			break;
		}
	close( devfd );
	close( fd );
}


shrnk( buf, argc )
char	*buf;
int	argc;
{
	register char	*s, *t;

	for ( s = t = buf;  t < &buf[512];  t++ )
		if ( argc == 7  ||
		     *t != 0  &&  (*t != '\r' || argc == 6) )
			*s++ = *t;
	return ( s - buf );
}
