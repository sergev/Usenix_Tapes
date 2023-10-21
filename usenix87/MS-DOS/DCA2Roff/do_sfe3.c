#include "dca2troff.h"
/* structured field class e3 */
do_sfe3()
{
    switch(sf_type)
	{
	case 0x01:			/* EPM  - Estab Primary Master Fmt */
	case 0x02:			/* EAM  - Estab Alternate Master Fmt */
	case 0x03:			/* RTMF - Return to Master Format */
	    flush_sf();
	    return;
	default:
	    fprintf(stderr, "unknown sf e3 type (x%02x)\n", sf_type);
	    flush_sf();
	    return;
	}
}
