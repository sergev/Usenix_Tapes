#include "dca2troff.h"
/* structured field class e6 */
do_sfe6()
{
    switch(sf_type)
	{
	case 0x01:			/* LP  - Line Parameters */
	case 0x02:			/* TP  - Tab Parameters */
	case 0x03:			/* LN  - Line Numbering */
	    flush_sf();
	    return;
	default:
	    fprintf(stderr, "unknown sf e6 type (x%02x)\n", sf_type);
	    flush_sf();
	    return;
	}
}
