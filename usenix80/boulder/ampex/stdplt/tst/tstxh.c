#include "stdplt.h"

main()
   {
	float x, y;
	char c;

	frame("a");
	scale(.7,.7);
	vector(0,0,1,1);
	c= xhair( &x, &y, ".");
	move( x, y);
	for (;;)
	  {
		c= xhair( &x, &y, ".");
		if( c == 'm' )
			move(x, y);
		else if (c == 'd')
			draw(x, y);
		else
			printf("bad cmd: %c (%3o)\n", c, c);
	  }
   }
