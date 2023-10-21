# include <stdio.h>
# include <stdplt.h>
main()
  {
	float y[1000];
	float q[1000];
	float x[1000];
	double sin();
	double j;
	int i;
	extern double min[2],max[2];

	for(i = 1;i < 1000;i++)
	  {
		j = i;
		x[i-1] = j * j * j;
		y[i-1] = j;
		q[i-1] = j * j;
	  }
	erase();
	graph("%X %*n 999  %Y   ",x,y);
	dispts("%X %*n 999  %Y  %*m 100 %*M 500  ",x,y);
	/*grid("%X %*n 999 %Y",x,y);*/
	erase();
	graph("%X %Y %o %*G1 %*n 999",x,y);
	erase();
	graph("%X %o %*G2 %Y %o %*n 999",x,y);
  }
