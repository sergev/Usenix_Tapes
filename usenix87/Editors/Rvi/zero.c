/*	zero - zero data structures
	84/12/18.  A. E. Klietz.
*/

#include "rv.h"

#ifdef zero
#undef zero
#endif

#ifndef USG
void
zero(ptr, len)
char *ptr;
int len;
{
	for (; len > 0; --len)
		*(ptr++) = 0;
}
#endif
