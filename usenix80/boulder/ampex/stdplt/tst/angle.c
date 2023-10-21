#include "stdplt.h"
#define TWOPI (2.*3.14159265)
main()
  {
	int i,j;

	erase();
	frame("a");
	origin(.5,.5);

	for (i=0; i<256; i++)
	  {	if (i)
			rotate(TWOPI/256.);
		move(0,0);
		textf("                 angle = %4f", 360.*i/256.);
		rdraw(0,0);
		rdraw(0,0);
	  }
  }
