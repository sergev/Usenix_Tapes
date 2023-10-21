struct lang {
	int splangn;
	char *splang;
} spellchk[] = {
	0,	"No spellcheck",
	1,	"American English",
	2,	"UK English",
	3,	"German",
	4,	"Dutch",
	5,	"National French",
	6,	"Canadian French",
	7,	"Italian",
	8,	"Spanish",
	9,	"Swedish",
	10,	"Finnish",
	11,	"Danish",
	12,	"Norwegian",
	0xffff,	"Use current language"
};

#include "dca2troff.h"
/* structured field class e2 */
do_sfe2()
{
    switch(sf_type)
	{
	case 0x01:			/* PMF  - Primary Master Format */
	case 0x02:			/* AMF  - Alternate Master Format */
	case 0x04:			/* TUFC - Text Unit Format Change */
	    flush_sf();
	    return;
	case 0x05:			/* DP   - Document Parameters */
	    flush_sf();
	    return;
	default:
	    fprintf(stderr, "unknown sf e2 type (x%02x)\n", sf_type);
	    flush_sf();
	    return;
	}
}
