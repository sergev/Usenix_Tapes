/*
** random.c -	yet another random number generator and random seed maker
**
**	[pm by Peter Costantinidis, Jr. @ University of California at Davis]
*/

#include "pm.h"

/*
** rnd()	- return a number between a and b, inclusive
*/
int	rnd (a, b)
reg	int	a, b;
{
	return((RN % (abs(b - a) + 1)) + a);
}

#ifndef	PATTERNS
/*
** get_seed()	- returns a seed for the random number generator
**		  dependent upon the time and date
*/
int	get_seed ()
{
	reg	int	seed;
	reg 	struct	tm	*timestruct;
	auto	long	clock;
	extern	long	time();
	extern	struct	tm	*localtime();

	clock = time(0);
	timestruct = localtime(&clock);
	seed = timestruct->tm_sec  +
	       timestruct->tm_min  +
	       timestruct->tm_hour +
	       timestruct->tm_mday +
	       timestruct->tm_mon  +
	       timestruct->tm_year +
	       timestruct->tm_yday;
	return((int) ((seed + clock) % 32767));
}
#endif
