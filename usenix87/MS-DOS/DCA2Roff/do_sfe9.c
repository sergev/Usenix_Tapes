#include "dca2troff.h"
/* structured field class e9 */
do_sfe9()
{
    switch(sf_type)
	{
	case 0x01:			/* PFA  - Punct. Format. Arithmetic */
	case 0x02:			/* PFC  - Punct. Format. Character */
	case 0x03:			/* NFP  - Note Format Paramerers */
	case 0x04:			/* AOP  - Auto-Outline Parameters */
	case 0x05:			/* PFP  - Page Formating Paramerers */
	    flush_sf();
	    return;
	default:
	    fprintf(stderr, "unknown sf e9 type (x%02x)\n", sf_type);
	    flush_sf();
	    return;
	}
}
