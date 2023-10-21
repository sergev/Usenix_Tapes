#include "dca2troff.h"
/* structured field class e8 */
do_sfe8()
{
    switch(sf_type)
	{				/* Margin Text */
	case 0x01:			/* MTTA - Top Margin - All Pages */
	case 0x02:			/* MTTO - Top Margin - Odd Pages */
	case 0x03:			/* MTTE - Top Margin - Even Pages */
	case 0x04:			/* MTBA - Bottom Margin - All Pages */
	case 0x05:			/* MTBO - Bottom Margin - Odd Pages */
	case 0x06:			/* MTBE - Bottom Margin - Even Pages */
	    flush_sf();
	    return;
	case 0x07:			/* BT   - Body Text */
	    do_text();
	    return;
	default:
	    fprintf(stderr, "unknown sf e8 type (x%02x)\n", sf_type);
	    flush_sf();
	    return;
	}
}
