#include "dca2troff.h"
/* structured field class e5 */
do_sfe5()
{
    switch(sf_type)
	{
	case 0x01:			/* MPB  - Bottom Margin Text Param */
	case 0x04:			/* MPT  - Top Margin Text Parameters */
	case 0x07:			/* PIP  - Page Image Parametrs */
	case 0x08:			/* PIN  - Page Image Numbering */
	    flush_sf();
	    return;
	default:
	    fprintf(stderr, "unknown sf e5 type (x%02x)\n", sf_type);
	    flush_sf();
	    return;
	}
}
