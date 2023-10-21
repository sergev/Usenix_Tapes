/*
 * symbols.c - display "symbol" character set
 */

char	code[33];

main()
	{
	register int	i , j ;

	for ( j = 0 ; j <= 96 ; j += 32 )
		{
		for ( i = 0 ; i <= 31 ; ++ i )
			code[i] = j + i + 128 ;
		code[32] = 0 ;
		symbol( code , 100 , 3000 - 20 * j , 180 , 0.0 );
		}
	}
