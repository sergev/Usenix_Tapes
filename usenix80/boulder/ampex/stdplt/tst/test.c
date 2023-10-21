#include "stdplt.h"

main()
  {
	float x, y;
	int i;

	frame("a");
	origin(.2,.2);
	scale(.5,.5);
	rotate(1.);
	move(.2,.2);
	draw(.8, .2);
	draw(.8, .8);
	draw(.2, .8);
	draw(.2, .2);
	cwidth(.02, "/");
	textf("Hello there this is a new line");
	draw(.5,.5);
	cwidth(.1, "/");
	textf("0123456789");
  }
