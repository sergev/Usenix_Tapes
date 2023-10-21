/*
 * convert a number of arbitrary size from a string to binary
 * operates until the first non numeric, returns zero if successful, else
 * bad char.
 */

sconv(str, inta, cnt)
char *str;
int *inta, cnt;
{
	register char *p;
	int ten[50];
	register int i, sign;

	for(i = 0; i < cnt; i++)	{
		ten[i] = 0;
		inta[i] = 0;
	}
	ten[0] = 10;
	sign = 0;
	p = str;
	while(*p == ' ')
		p++;
	if (*p == '-')	{
		p++;
		sign = 1;
	}
	while(*p == ' ')
		p++;
	while(*p >= '0' && *p <= '9')	{
		_mul(ten, inta, cnt);
		ten[0] = *p++ - '0';
		_add(ten, inta, cnt);
		ten[0] = 10;
	}
	if (sign)
		_neg(inta, cnt);
	return(*p);
}
