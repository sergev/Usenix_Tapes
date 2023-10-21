/*
 * Q Library -- Multiply routine a * b -> b
 * allows up to 50 integers per number.
 */

int	lcary;

_mul(a, b, c)
int *a, *b, c;
{
	int xreg[100], accu[100], breg[50];
	register int i, base, zero;

	base = 50 - c;
	for(i = 0; i < c; i++)	{
		breg[i] = a[i];
		xreg[base + i] = b[i];
		accu[base + i] = accu[50 + i] = xreg[50 + i] = 0;
	}
	goto start;
loop:
	_asl(&xreg[base], 2*c);
start:
	zero = 0;
	for(i = 0; i < c; i++)
		zero =| breg[i];
	if (!zero)
		goto quit;
	_asr(breg,c);
	if (!lcary)
		goto loop;
	_sub(&xreg[base], &accu[base], 2*c);
mloop:
	_asl(&xreg[base], 2*c);
	zero = 0177777;
	for(i = 0; i < c; i++)
		zero =& breg[i];
	if (zero == 0177777)
		goto quit;
	_asr(breg, c);
	if (lcary)
		goto mloop;
	_add(&xreg[base], &accu[base], 2*c);
	goto loop;
quit:
	for(i = 0; i < c; i++)
		b[i] = accu[base + i];
}
