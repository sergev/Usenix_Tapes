#include "/v/wa1yyn/c/plot/plot.h"
plotty(pstat,n)
	int pstat[];
	int n;
/*	Prints out a 10-bit Tektronics number supplied in n.
 */
{
	register char c;
	register i,j;

	i = n/10;
	if((n % 10) > 4)i++;
	j = i >> 5;
	plotp(pstat,(j & 037) + 040);
	plotp(pstat,(i & 037) + '`');
}
