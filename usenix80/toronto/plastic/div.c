/*
 * Q Library -- Routine to divide a / b -> b
 * remainder is avail in extern int _divr[]
 */

int	lcary;
int	divr[50];

_div(a, b, c)
int *a, *b, c;
{
	int	breg[50], xreg[100];
	register int count, i, base;

	base = 50 - c;
	for(i = 0; i < c; i++)	{
		breg[i] = b[i];
		xreg[base + i] = a[i];
		xreg[50 + i] = 0;
	}
	if (b[c - 1] < 0)
		_neg(breg, c);
	if (a[c - 1] < 0)
		_neg(&xreg[base], c);
	count = 0;
	for(i = 0; i < c; i++)
		count =| breg[i];
	if (!count)	{
		lcary = 1;
		return;
	}
	count = c * 16 + 1;
	while(count--)	{
		_sub(breg, &xreg[50], c);
		i = !lcary;
		if (xreg[49 + c] < 0)
			_add(breg, &xreg[50], c);
		_asl(&xreg[base], 2 * c);
		xreg[base] =& 0177776;
		xreg[base] =| (i & 01);
	}
	_asr(&xreg[50], c);
	if ((a[c-1] < 0 || b[c-1] < 0) && !(a[c-1] < 0 && b[c-1] < 0))
		_neg(&xreg[base], c);
	for(i = 0; i < c; i++)	{
		b[i] = xreg[base + i];
		divr[i] = xreg[50 + i];
	}
	lcary = 0;
}
