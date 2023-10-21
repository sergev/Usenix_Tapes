#
#include        "./gparam"

/*
**      This function will close the device and disable
**      the LDC command (in case a user  tries to still
**      use the area)
*/

extern int gopflg;
extern int gopfd;
int gclose(area)
struct abuf *area;
{
	register struct abuf *rarea;

	rarea = area;

	if(gopflg--) {
		if(!gopflg)
			if(close(gopfd) == -1) return(-1);
	}
	else {
		gopflg = 0;
		return(-1);
	}

	rarea->fdesc = -1;

	rarea->ldc = LDC;       /* LDC | NONE-LOADED */

	return(0);
}
