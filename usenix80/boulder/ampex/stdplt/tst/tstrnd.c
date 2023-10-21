#include <stdplt.h>
char buf[200];
main()
  {
	double x;
	int base, digits;
	double _rnd();

	for(;;)
	  {	printf("base, digits = ");
		scanf("%d%d", &base, &digits);
		printf("x = ");
		scanf("%lf", &x);
		printf("rnd = ");
		gcvt(_rnd(x, base, digits), 20, buf);
		printf("%s\n", buf);
	  }
  }
