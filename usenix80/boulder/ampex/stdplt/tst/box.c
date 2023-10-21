#include "stdplt.h"
#define NBOX 25
#define PI 3.14159265

char name[] = "/a";
double sin(), cos();

main(argc, argv)
  int argc;
  char *argv[];
  {
	int i, j;
	int nbox;
	float scl;

	erase();
	nbox = NBOX;
	if (argc == 2)
		nbox = atoi(argv[1]);
	scl = 1./(sin(PI/nbox) + cos(PI/nbox));
	for (i = -1; i < 3; i += 2)
	  {	frame(name);
		name[1]++;
		origin(.5, .5);
		for (j = 0; j < nbox; j++)
		  {	if (j)
			  {	rotate(i*PI/nbox);
				scale(scl, scl);
			  }
			move(-.25, -.25);
			draw( .25, -.25);
			draw( .25,  .25);
			draw(-.25,  .25);
			draw(-.25, -.25);
		  }
	  }
  }
