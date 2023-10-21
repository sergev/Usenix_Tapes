#include "simplex.h"

double f(x, d)
vector    x;
datarow   d;
{
     return (x[0]*d[0]/(x[1]+d[0]));
}
