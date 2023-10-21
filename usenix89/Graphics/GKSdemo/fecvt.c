#include	<stdio.h>

#define	fabs(x)	((x < 0) ? -x: x)


	/* convert a floating point to frac * 10 ^ ex format */

f_to_e (f, frac, ex)
double f, *frac;
int *ex;
{

	int sign, i;

	if (f == 0) {
		*frac = 0;
		*ex = 0;
		return;
	}

	/* determine the sign and save it for later */
	if (f < 0)
		sign = -1;
	else
		sign = 1;

	/* convert to exponential notation */
	f = fabs(f);
	if (f < 1)
		for (i = 0; f < 1; i--)
			f *= 10;
	else
		for (i = 0; f > 10; i++)
			f /= 10;

	*frac = sign * f;
	*ex = i;
}
	

	/* convert base 10 exponential notation to floating point */

double
e_to_f (x, ex)
double x;
int ex;
{

	if (x == 0)
		return (0);
	if (ex > 0)
		for (; ex > 0; ex--)
			x *= 10;
	else
		for (; ex < 0; ex++)
			x /= 10;
	return (x);
}
