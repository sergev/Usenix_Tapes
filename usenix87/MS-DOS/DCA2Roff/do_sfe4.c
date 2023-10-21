
#include "dca2troff.h"

/* structured field class e4 */
do_sfe4()
{
    switch(sf_type)
	{
	case 0x02:			/* PM   - Print Medium */
	case 0x03:			/* OM   - Operator Message */
	    flush_sf();
	    return;
	default:
	    fprintf(stderr, "unknown sf e4 type (x%02x)\n", sf_type);
	    flush_sf();
	    return;
	}
}
