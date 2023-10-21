#include    "../ffdef.h"
long
price(cp)
struct  chain   *cp;
{
	long p;

	p = (cp->c_size + 2) * (long) cp->c_index / 4;
	return(p);
}
