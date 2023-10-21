#include <math.h>

#ifdef R2DEBUG
#include <stdio.h>

float round2(a, b)
float a, b;
{
    float c;

    c = (floor((1.0 / b) * (a + (b / 2.0)))) / (1.0 / b);

    printf("rounding %f to nearest %f; result: %f\n",a, b, c);

    return c;
}

#else

float round2(a, b)
float a, b;
{
    return (floor((1.0 / b) * (a + (b / 2.0)))) / (1.0 / b);
}

#endif

