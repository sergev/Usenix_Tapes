#include <stdio.h>
main()
  {
	float a[2][2], ainv[2][2], aiinv[2][2], id[2][2];

	for (;;)
	  {
		scanf("%f%f%f%f", &a[0][0], &a[0][1], &a[1][0], &a[1][1]);
		printf("a = \n%f %f\n%f %f\n", a[0][0], a[0][1], a[1][0], a[1][1]);
		if (_inv2(a, ainv))
			printf("singular\n");
		printf("ainv = \n%f %f\n%f %f\n", ainv[0][0], ainv[0][1], ainv[1][0], ainv[1][1]);
		if (_inv2(ainv, aiinv))
			printf("a singular\n");
		printf("aiinv = \n%f %f\n%f %f\n", aiinv[0][0], aiinv[0][1], aiinv[1][0], aiinv[1][1]);
		id[0][0] = ainv[0][0]*a[0][0] + ainv[0][1]*a[1][0];
		id[0][1] = ainv[0][0]*a[0][1] + ainv[0][1]*a[1][1];
		id[1][0] = ainv[1][0]*a[0][0] + ainv[1][1]*a[1][0];
		id[1][1] = ainv[1][0]*a[0][1] + ainv[1][1]*a[1][1];
		printf("ainv*a = \n%e %e\n%e %e\n", id[0][0], id[0][1], id[1][0], id[1][1]);
	  }
  }
