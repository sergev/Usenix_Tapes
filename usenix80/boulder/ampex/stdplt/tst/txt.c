#include "stdplt.h"

main()
  {
	float x, y;
	int i;

	frame("a");
	scale(1,1);
	rotate(.3);
	move(0,0);
	draw(0,1);
	draw(1,1);
	draw(1,0);
	draw(0,0);
	cwidth(.1, ".");
	textf("0123456789");
	draw(.5,.5);
	draw(.5,.5);
  }
