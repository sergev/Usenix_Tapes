#include <math.h>
#include "rtd.h"
#include "extern.h"
#include "macros.h"


double  findo (m, s) /* finds where a ray inside the ball exits. */
struct mat *m;
struct sphere  *s;
{
/* foops id the rotated position vector. */
    struct vector   foops;
    double  t;
    MTV (foops, (*m), s -> cent);
/* see if it hits the ball (it better)*/
    t = s -> rad * s -> rad - foops.y * foops.y - foops.z * foops.z;
    if (t > 0)
	t = foops.x + sqrt (t);
    else
	t = 0;
/* return how far along the ray you were when you hit */
    return (t);
}

double  find (m, s)/* finds whether a ray hits a ball*/
struct mat *m;
struct sphere  *s;
{
    struct vector   foops;
    double  t;
    MTV (foops, (*m), s -> cent);
    t = s -> rad * s -> rad - foops.y * foops.y - foops.z * foops.z;
    if (t > 0)
	t = foops.x - sqrt (t);
    else
	t = 0;
    return (t);
}

double  finds (m, s)/* finds if a ball is between a point and a 
			lightsource. Returns how obscuring the ball is */
struct mat *m;
struct sphere  *s;
{
    struct vector   foops;
    double  t;
    MTV (foops, (*m), s -> cent);
    t = s -> rad - sqrt (foops.y * foops.y + foops.z * foops.z);
    if (t > 0)
	t = t / foops.x;
    else
	t = 0;
    return (t);
}




double  shadow (p)/* finds if a point is in a shadow, or if it is on edge */
struct vector  *p;
{
    struct mat  trans;
    struct sphere   ss;
    struct vector   d;
    int     c,
            i;
    double  l,
            k,
            x,
            y,
            z,
            finds ();
    l = 0.0;
    c = -1;
    SV (d, ls.cent, (*p));
    d.l = LEN (d);
    d.xzl = XZL (d);
    mt (&(d), &trans);

    for (i = 0; i < nob; i++) {
	ss.rad = bl[i] -> s.rad;
	SV (ss.cent, bl[i] -> s.cent, (*p));
	if ((k = finds (&trans, &ss)) > l) {
	    c = i;
	    l = k;
	}
    }
    if (c == -1)
	k = 200.0;
    else {
	k = 1.0 - l / ((ls.rad) / (d.l));
	if (k < 0.0)
	    k = 0.0;
	k *= 200.0;
    }
    return (k);
}
