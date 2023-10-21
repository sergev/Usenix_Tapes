// This may look like C code, but it is really -*- C++ -*-
/* 
Copyright (C) 1988 Free Software Foundation
    written by Doug Lea (dl@rocky.oswego.edu)

This file is part of GNU CC.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.  No author or distributor
accepts responsibility to anyone for the consequences of using it
or for whether it serves any particular purpose or works at all,
unless he says so in writing.  Refer to the GNU CC General Public
License for full details.

Everyone is granted permission to copy, modify and redistribute
GNU CC, but only under the conditions described in the
GNU CC General Public License.   A copy of this license is
supposed to have been given to you along with GNU CC so you
can know your rights and responsibilities.  It should be in a
file named COPYING.  Among other things, the copyright notice
and this notice must be preserved on all copies.  
*/

/*
 * the value HUGE should be checked against the one in /usr/include/math.h
 */

#ifndef MATHH
#define MATHH

extern double acos(double);
extern double acosh(double);
extern double asin(double);
extern double asinh(double);
extern double atan(double);
extern double atan2(double, double);
extern double atanh(double);
extern double cbrt(double);
extern double ceil(double);
extern double copysign(double,double);
extern double cos(double);
extern double cosh(double);
extern double drem(double,double);
extern double erf(double);
extern double erfc(double);
extern double exp(double);
extern double expm1(double);
extern double fabs(double);
extern double finite(double);
extern double floor(double);
extern double frexp(double, int*);
extern double gamma(double);
extern double hypot(double,double);
extern double infnan(int);
extern int    isinf(double);
extern int    isnan(double);
extern double j0(double);
extern double j1(double);
extern double jn(int, double);
extern double ldexp(double, int);
extern double lgamma(double);
extern double log(double);
extern double log10(double);
extern double log1p(double);
extern double logb(double);
extern double modf(double, int*);
extern double pow(double, double);
extern double rint(double);
extern double scalb(double, int);
extern double sin(double);
extern double sinh(double);
extern double sqrt(double);
extern double tan(double);
extern double tanh(double);
extern double y0(double);
extern double y1(double);
extern double yn(int, double);

#if defined(sun)
struct exception
{
  int type;
  char* name;
  double arg1, arg2, retval;
};
int matherr(exception*);
#include <values.h>             // sun already provides some of defs below

#endif                          // sun


#ifndef HUGE
#define HUGE    1.701411733192644270e38 // correct for vax
#endif

/* These seem to be sun & sysV names of these constants */


#ifndef M_E
#define M_E         2.7182818284590452354
#endif
#ifndef M_LOG2E
#define M_LOG2E     1.4426950408889634074
#endif
#ifndef M_LOG10E
#define M_LOG10E    0.43429448190325182765
#endif
#ifndef M_LN2
#define M_LN2       0.69314718055994530942
#endif
#ifndef M_LN10
#define M_LN10      2.30258509299404568402
#endif
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2      1.57079632679489661923
#endif
#ifndef M_1_PI
#define M_1_PI      0.31830988618379067154
#endif
#ifndef M_PI_4
#define M_PI_4      0.78539816339744830962
#endif
#ifndef M_2_PI
#define M_2_PI      0.63661977236758134308
#endif
#ifndef M_2_SQRTPI
#define M_2_SQRTPI  1.12837916709551257390
#endif
#ifndef M_SQRT2
#define M_SQRT2     1.41421356237309504880
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2   0.70710678118654752440
#endif

#ifndef PI                      // as in stroustrup
#define PI  M_PI
#endif
#ifndef PI2
#define PI2  M_PI_2
#endif

#endif                          // MATHH
