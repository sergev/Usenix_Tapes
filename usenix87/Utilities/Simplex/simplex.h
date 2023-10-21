#include <math.h>
#include <stdio.h>

#define M      2
#define NVPP   2
#define N      M+1
#define MNP    200
#define ALPHA  1.0
#define BETA   0.5
#define GAMMA  2.0
#define LW     5
#define ROOT2  1.414214

typedef double vector[N];
typedef double datarow[NVPP];

extern int       h[N], l[N];
extern int       np, maxiter, niter;
extern vector    next, mean, error, maxerr, step, simp[N];
extern datarow   data[MNP];
extern FILE      *fpdata;

extern double    f();
