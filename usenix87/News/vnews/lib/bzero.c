/*
 * Set nc bytes, starting at cp, to zero.
 */

#include "config.h"

#if BSDREL < 42

bzero(cp, nc)
register char	*cp;
register int	nc;
{
	while (--nc >= 0)
		*cp++ = 0;
}

#endif
