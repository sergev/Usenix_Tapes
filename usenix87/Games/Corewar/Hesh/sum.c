/*
** sum.c	- add two numbers, wrapping if sum is out-of-bounds
**
**	[cw by Peter Costantinidis, Jr. @ University of California at Davis]
**
static	char	rcsid[] = "$Header: sum.c,v 7.0 85/12/28 14:36:36 ccohesh Dist $";
**
*/

#include "cw.h"


/*
** sum()	- sum two integers w/ memory wrapping
*/
int	sum (x, y)
reg	int	x, y;
{
	reg	int	tot = x + y;

	if (tot < 0)
		return(sum(MAX_MEM + tot, 0));
	if (tot >= MAX_MEM)
		return(tot % MAX_MEM);
	return(tot);
}
