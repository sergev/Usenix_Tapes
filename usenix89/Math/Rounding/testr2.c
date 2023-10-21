#include <stdio.h>
#include <math.h>

extern float round2();

main()
{
    float f[5];
    int i;

    f[0] = round2(M_PI, 5.0);
    f[1] = round2(M_PI, 0.5);
    f[2] = round2(M_PI, 0.1);
    f[3] = round2(M_PI, 0.05);
    f[4] = round2(M_PI, 0.02);

    for (i = 0; i < 5; printf("f[%d] = %f\n", i, f[i++]));
}

