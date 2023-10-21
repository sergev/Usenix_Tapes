#include    "../ffdef.h"
putchain(cnum)
{
	seek(cfh, cnum * sizeof chain, 0);
	if (write(cfh, &chain, sizeof chain) != sizeof chain)
	    printf("Oops!  Error writing chain %d\n", cnum);
	unlock(CHAIN_LOCK);
}
