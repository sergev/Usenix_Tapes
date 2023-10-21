/* Copyright (c) 1985 by Thomas S. Fisher - Westminster, CO 80030 */

#include	"emp.h"

secno(x, y)
int	x, y;
{
	return((x + XYMAX) % XYMAX + ((y + XYMAX) % XYMAX) * XYMAX);
}
