
#include "dca2troff.h"

/* multi byte class d9 */
do_mbd9()
{
    switch (mb_type)
	{
	case 0x6a:			/* AO   - Auto-Outline */
	case 0x82:			/* INS  - Insert */
	case 0x85:			/* NR   - Note Reference */
	case 0x86:			/* NTR  - Note Text Reference */
	    do_flush(mb_count);
	    return;
	case 0x81:			/* IU   - Include Unit */
	    fprintf(stderr, " This document has a request to include another document\n");
	    outstr("\\\" Include request");
	    do_flush(mb_count);
	    return;
	default:
	    fprintf(stderr, "unknown mb d9 type (%02x)\n", mb_type);
	    do_flush(mb_count);
	    return;
	}
}

