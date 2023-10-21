
#include <stdplt.h>
main()
  {
	static float x[600];
	static float y[600];
	static float z[600];
	int i;
	double sin();

	frame("Frame");
	scale(.75,.75);
	for(i=0;i<600;i++)
	  {
		x[i] = (float)i - 300.;
		x[i] /= 18.;
		if(i == 300)
			x[i] = .000000001;
		y[i] = sin((float)x[i])/(float)x[i];
		z[i] = y[i] + .6;
	  }
	graph("%X  %*l Time %*n 600 %Y %e %*l Frequency %Y %*e second",x,z,"First",y);
  }
