/*	copy - copy data structures
	84/12/18.  A. E. Klietz.
*/

#include "rv.h"

#ifdef copy
#undef copy
#endif

#ifndef USG
void
copy(to, from, len)
char *to, *from;
int len;
{
	for (; len > 0; --len)
		*(to++) = *(from++);
}
#endif
