
#include "dca2troff.h"

/* multi byte class d4 */
do_mbd4()
{
    switch (mb_type)
	{
	case 0x02:			/* BLFC  - Begin Line Fmt. Change */
	case 0x04:			/* RMLF  - Return to Mstr. Line Fmt. */
	case 0x05:			/* SLP   - Set Line Parameters */
	case 0x06:			/* ELFC  - End Line Fmt. Change */
	case 0x13:			/* ATL   - Return To Master Font */
	case 0x47:			/* PPIN  - Print Page Image Number */
	case 0x63:			/* PTUN  - Print Text Unit Name */
	case 0x72:			/* BOS   - Begin Overstrike */
	case 0x76:			/* EOS   - End Overstrike */
	case 0x7a:			/* BLM   - Begin Linguistic Mark */
	case 0x7e:			/* ELM   - End Linguistic Mark */
	    do_flush(mb_count);
	    return;
	case 0x0a:			/* BUS   - Begin Underscore */
	    outstr("\\fI");
	    do_flush(mb_count);
	    return;
	case 0x0b:			/* ATF   - Align Text Field */
	    itemp = get1ch();			/* get alignment type */
	    --mb_count;
	    jtemp = 0;
	    if(mb_count >= 2)
		{
		jtemp = get1num();		/* get alignment pos */
		--mb_count;
		--mb_count;
		}
	    if(itemp == 3)			/* center request */
		outstr("\n.ce\n");
	    do_flush(mb_count);
	    return;
	case 0x0e:			/* EUS   - End Underscore */
	    outstr("\\fR");
	    do_flush(mb_count);
	    return;
	case 0x0f:			/* ATL   - Align Text Line */
	    itemp = get1ch();			/* get alignment type */
	    --mb_count;
	    if(itemp == 1)			/* center */
		outstr("\n.ce\n");
	    if(itemp == 2)			/* right */
		;
	    do_flush(mb_count);
	    return;
	case 0x62:			/* BK    - Begin Keep */
	    outstr("\\\" Begin Keep\n");
	    do_flush(mb_count);
	    return;
	case 0x66:			/* EK    - End Keep */
	    outstr("\\\" End Keep\n");
	    do_flush(mb_count);
	    return;
	case 0x90:			/* DPS   - Display Prompt and Stop */
	    ctemp = get1ch();			/* get prompt */
	    sprintf(tline, "\\\" DPS found - keyboard data request, prompt=x%02x\n", ctemp);
	    outstr(tline);
	    do_flush(mb_count);
	    return;
	default:
	    fprintf(stderr, "unknown mb d4 type (%02x)\n", mb_type);
	    do_flush(mb_count);
	    return;
	}
}
