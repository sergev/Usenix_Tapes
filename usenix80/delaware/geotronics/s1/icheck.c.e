316a
	for ( ;  i >= low + n - 1;  i =- n )
		for ( j = 0;  j < n;  j++ )
			freebl( i - adr[j] );
	for ( ;  i >= low;  i-- )
		freebl( i );
.
312,315c

	high = sblock.s_fsize - 1;
	low = sblock.s_isize + 2;
	free( 0 );
	for ( i = high;  lrem( 0, i+1, n );  i-- )  {
		if ( i < low )
			break;
		freebl( i );
.
306a
makefree( file )
char	*file;
{
	register char *i, *j;
	char	*n, *m;
	char	*high, *low;
	static char	adr[100], flag[100];

	for ( j = file;  j[0];  j++ )
		if ( j[0] == 'r' )
			switch ( j[1] )  {
			case 'k':
				n = 24;
				m = 3;
				break;
			case 'p':
				n = 10;
				m = 4;
				break;
			default:
				;
			}
	if ( n > 100 )
		n = 100;
	for ( i = 0;  i < n;  i++ )
		flag[i] = 0;
	j = 0;
	for ( i = 0;  i < n;  i++ )  {
		while ( flag[j] )
			j = (j+1) % n;
		adr[i] = j;
		flag[j]++;
		j = (j+m) % n;
	}
.
305c
	if ( (bmap[(i>>4)&07777] & (1<<(i&017))) == 0 )
		free( i );
}
.
303c
freebl( i )
int	i;
.
110c
		makefree( file );
.
5,6c
	"/dev/rrpa",
	"/dev/rrpb",
.
1a
/*
 * icheck.c - file system storage consistency check
 *
 *	modified 28-May-1980 by D A Gwyn:
 *	1) changed default devices;
 *	2) fixed bug involving -s with device having >32768 blocks;
 *	3) allocate blocks like mkfs to improve latency.
 */
.
w
q
