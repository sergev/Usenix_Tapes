
#include "dca2troff.h"

/* multi byte class d8 */
do_mbd8()
{
    switch (mb_type)
	{
	case 0x82:			/* SSCA  - Set Spelling Check Attrib. */
	case 0x94:			/* CWB   - Conditional Word Break */
	case 0x95:			/* NPD   - Note Partition Delimiter */
	case 0x96:			/* LOP   - Locate Process Output */
	    do_flush(mb_count);
	    return;
	case 0x62:			/* BCL   - Begin Column Layout */
	    outstr("\\\" Begin Column Layout request\n");
	    do_flush(mb_count);
	    return;
	case 0x66:			/* ECL   - End Column Layout */
	    outstr("\\\" End Column Layout request\n");
	    do_flush(mb_count);
	    return;
	case 0x6a:			/* BFT   - Begin Formatted Text */
	    outstr("\n.nf\n");
	    do_flush(mb_count);
	    return;
	case 0x6e:			/* EFT   - End Formatted Text */
	    outstr("\n.fi\n");
	    do_flush(mb_count);
	    return;
	default:
	    fprintf(stderr, "unknown mb d8 type (%02x)\n", mb_type);
	    do_flush(mb_count);
	    return;
	}
}
