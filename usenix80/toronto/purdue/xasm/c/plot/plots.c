#include "/v/wa1yyn/c/plot/plot.h"
plots(pstat,string)
	int pstat[];
	char *string;
/*
 *	Prints out the string on the device.  X and Y are
 *	updated to refect current position.
 */
{
	register char *k;

	k = string;
	while(*k)plotc(pstat,*k++);
}
