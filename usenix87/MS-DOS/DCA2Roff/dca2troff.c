#include "dca2troff.h"


/* dca is a series of structured fields */

main()
{
    for (;;)
	{				/* do a structured field */
	sf_incnt = 0;			/* init the character count */
	sf_length = get1num(); 		/* positive - includes length bytes */
	sf_class = get1ch();		/* get the paramaters for the field */
	sf_type = get1ch();
	sf_format = get1ch();

	switch (sf_class)		/* what class of sf? */
	    {
	    case 0xe1:
		do_sfe1();
		break;
	    case 0xe2:
		do_sfe2();
		break;
	    case 0xe3:
		do_sfe3();
		break;
	    case 0xe4:
		do_sfe4();
		break;
	    case 0xe5:
		do_sfe5();
		break;
	    case 0xe6:
		do_sfe6();
		break;
	    case 0xe8:
		do_sfe8();
		break;
	    case 0xe9:
		do_sfe9();
		break;
	    default:
		fprintf(stderr, "Unknown SF class (%02x)\n", sf_class); 
	    }
	}
}
