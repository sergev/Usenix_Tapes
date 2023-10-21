/*
	mtzero -- write blank tape for mailing safely
*/

main()  {
	extern int      open(), write();
	register int    fd = open( "/dev/rmt0" , 1 );
	register int    i = 10000 ;
	register char   space = ` ' ;
	static char     buf[10000];

	do
		buf[i-1] = space ;
		while ( --i != 0 );

	while ( write( fd , buf , 10000 ) == 10000 )
		;
	}
