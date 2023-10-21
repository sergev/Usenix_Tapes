#include "uutty.h"
/* 
** Discard buffered input, and try to get us to a start where
** the next input will be a response to the most recent (or next)
** output.
*/
resync()
{
	if (ibfa < ibfz) {
		dbgtimep = getime();
		if (debug >= 2) Ascdnm(ibfa,ibfz-ibfa,"Drop:");
		if (debug >= 4) Hexdnm(ibfa,ibfz-ibfa,"Drop:");
		restdev();		/* Make sure the device is OK */
		sleep(5);		/* Try not to respond to garbage */
	}
	ibfa = ibfz+1;
}
