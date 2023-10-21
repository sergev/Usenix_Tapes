/*
 *	util.c
 *
 *	general purpose utilities
 */

# include	<math.h>

double
dist (x0, y0, x1, y1)
double	x0, y0, x1, y1;
{
	register double	tx, ty;
	
	tx = x0 - x1;
	ty = y0 - y1;
	return sqrt (tx*tx + ty*ty);
}

printinbase (base, value)
double	base, value;
{
	register int	ibase;
	register int	ivalue;
	char	buf[256];
	register char	*c;
	int		sign;
	register int	digit;
	
	ivalue = value;
	if ((ibase = base) <= 0) {
		printf ("Illegal base: %d\n", ibase);
	}
	c = buf + sizeof (buf);
	*--c = '\0';
	sign = 1;
	if (ivalue < 0) {
		sign = -1;
		ivalue = -ivalue;
	}
	while (ivalue) {
		digit = ivalue % ibase;
		if (digit >= 10)
			*--c = digit + 'a';
		else
			*--c = digit + '0';
		ivalue /= ibase;
	}
	if (sign == -1)
		*--c = '-';
	puts (c);
}
