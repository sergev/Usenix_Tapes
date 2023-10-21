#include "types.h"
#include "ext.h"
#ifdef	BSD42
#include <sys/time.h>
#endif

/*
 * seed the random number generator
 *
 */
seedrand()
{
#ifdef BSD42
	struct timeval tp;
	struct timezone tpz;
#endif

#ifdef BSD42
	gettimeofday(&tp,&tpz);
	srandom((int)tp.tv_sec);
#else
	randomize();
#endif
}

/*
 * get_rand - return a random number 1-6
 *
 */
get_rand()
{
#ifdef BSD42
	long random();
#else
	double ranm();
#endif

#ifdef BSD42
	return( (int) random() % 6 + 1);
#else
	return( (int) (ranm() * 6.0) + 1.0);
#endif
}
