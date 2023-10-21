#include "dca2troff.h"
#include "ebtab.h"

/* text block */
do_text()
{
    int c;
    for ( ;; )
	{
	if ( sf_incnt >= sf_length )
		return;
	c = get1ch();
	c &= 0377;
	switch (ebaray[c].type)
	    {
	    case 0:		/* forget it */
		break;
	    case 1:		/* simple character string */
		outstr(ebaray[c].arg);
		break;
	    case 2:		/* single character control */
		do_single(c);
		break;
	    case 3:		/* multibyte control */
		do_multi();
		break;
	    case 4:		/* accented character */
		do_accent(ebaray[c].arg);
		break;
	    case 5:		/* troff special character */
		outstr(ebaray[c].arg);
		break;
	    case 6:		/* non-troff special character */
		do_spchar(ebaray[c].arg);
		break;
	
	    }
	}
}
