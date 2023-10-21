#include "dca2troff.h"

/* structured field class e1 */
do_sfe1()
{
    switch(sf_type)
	{
	case 0x03:			/* FUP  - Format Unit Prefix */
	case 0x04:			/* TUP  - Text Unit Prefix */
	case 0x06:			/* EUP  - End Unit Prefix */
	    flush_sf();
	    return;
	default:
	    fprintf(stderr, "unknown sf e1 type (x%02x)\n", sf_type);
	    flush_sf();
	    return;
	}
}
