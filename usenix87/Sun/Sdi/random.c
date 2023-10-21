/*************************  random.c  ***************************/
/*
 * NOT Copyright 1987 by Mark Weiser.
 * This routine placed in the public domain. (:-)
 */

/*
 *  returns a normally distributed random number
 */

#define MAXRANDOM (~(1<<31))
#define COUNT (8)
#define SD (1000)
normal(mean,sd)
{
	long a, b, c, d, e, f;
	register long sum = 0;
	long random();
	register int count=COUNT;

	while (count--) {
		sum  += random() / (COUNT*SD);
	}

	return ((sum - (MAXRANDOM/(2*SD)))*sd)/(MAXRANDOM/(COUNT*SD)) + mean;
}

