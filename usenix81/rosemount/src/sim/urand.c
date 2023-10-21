#include <sys/types.h>
#include <sys/timeb.h>

/*
 * random number generator data
 */

int rtab[128],r1,r2;





/*
 * randomize - start random number generator from 'random' seed.
 */

randomize()
{
	struct timeb tp ;
	struct {
		long utime ;
		long stime ;
		long cutime ;
		long cstime ;
		} buf ;
	int pid ;
	int seed1, seed2 ;

	ftime( &tp ) ;
	pid = getpid() ;
	times( &buf ) ;

	seed1 = pid + tp.millitm ;
	seed2 = tp.time + buf.utime - buf.stime ;

	rseed( seed1, seed2 ) ;

	pid = (seed1 - seed2 ) & 077 ;
	while(pid--)
		urand() ;

	} /* randomize */





/*
 * rseed - start random number generator
 */

rseed(a,b)
int a,b ;
{
	int i ;

	r1 = a ;
	r2 = b ;

	for (i=0; i<128; i++)
		rtab[i] = randy() ;

	} /* rseed */





/*
 * urand - uniform random number generator.
 */

urand()
{
	int register i,j;

	i = (randx() >> 7) & 127 ;
	j = rtab[i];
	rtab[i] = randy();
	return(j);
}



randx()
{
	r2 = ((5462 * r2 + 1337) >> 1) % 16383 ;
	return(r2);
}





randy()
{
	r1 = ((4682 * r1 + 1131) >> 1) % 32767 ;

	return (r1 & 32767) ;

	} /* randy */
