
#include "dca2troff.h"

/* multi byte class d2 */
do_mbd2()
{
    switch (mb_type)
	{
	case 0x01:			/* STAB  - Set Tabs */
	case 0x0b:			/* RLM   - Release Left Margin */
	case 0x35:			/* SVA   - Set Visual Attributes */
	    do_flush(mb_count);
	    return;
	default:
	    fprintf(stderr, "unknown mb d2 type (%02x)\n", mb_type);
	    do_flush(mb_count);
	    return;
	}
}
