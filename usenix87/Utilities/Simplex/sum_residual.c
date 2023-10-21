#include "simplex.h"

#define sqr(x) ((x) * (x))

sum_residual(x)
vector    x;
{
     register int   i;

     x[N-1] = 0.0;

     for (i=0 ; i<np ; i++)
          x[N-1] += sqr(f(x, data[i]) - data[i][1]);
}
