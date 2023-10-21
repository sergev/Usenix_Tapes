/*
 * modified version of the c library locv, takes an address of the 
 * first word, followed by a word count. returns a pointer to 
 * a static buff.
 */

locv(addr, cnt)
int cnt, *addr;
{
	register int i, c, p;
	static char buf[31];
	int ten[50], a[50];
	extern int divr[];

	clear(buf, sizeof buf);
	for(c = 0; c < cnt; c++)	{
		ten[c] = 0;
		a[c] = addr[c];
	}
	if (a[cnt - 1] < 0)
		_neg(a, cnt);
	ten[0] = 10;
	c = 29;
	do	{
		_div(a, ten, cnt);
		buf[c--] = divr[0] + '0';
		p = 0;
		for(i = 0; i < cnt; i++)	{
			p =| a[i];
			a[i] = ten[i];
			ten[i] = 0;
		}
		ten[0] = 10;
		} while(p);
	if (addr[cnt - 1] < 0)
		buf[(c--)+1] = '-';
	return(&buf[c+2]);
}
