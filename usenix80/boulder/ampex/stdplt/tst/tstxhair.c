#include "stdplt.h"

main()
   {
	int xp, yp;
	float x, y;
	char c;
	c= _xhair( &xp, &yp);
	x= xp/STDSIZ;
	y= yp/STDSIZ;
	move( x, y);
	for (;;)
	  {
		c= _xhair( &xp, &yp);
		x= xp/STDSIZ;
		y= yp/STDSIZ;
		if( c == 'm' )
			move(x, y);
		else if (c == 'd')
			draw(x, y);
		else
			printf("bad cmd: %c (%3o)\n", c, c);
	  }
   }
