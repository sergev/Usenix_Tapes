/*
 * this subroutine does all the gritty work- it calculates 
 * what shade each pixel should be. I like recursion.
 */
#include <math.h>
#include "rtd.h"
#include "macros.h"
#include "extern.h"

int     shade (r)
struct ray *r;
{
    int     i,
            c,
            refract ();
    struct ray  refr;
    double  lght,
            x,
            y,
            z,
            l,
            k,
            dot (), find (), shadow ();
    int     sx,
            sy;
    double  stupid;
    struct vector   new,
                    norm;
    struct mat  trans;
    struct sphere   ss;
    if (++level <= LEVEL) {
	c = -1;
	l = HUGE;
/* get vector length and xz component for mt() */
	r -> dir.l = LEN (r -> dir);
	r -> dir.xzl = XZL (r -> dir);
/* make a transform matrix that rotates something in space so
   that the ray will be aligned with the x axis */
	mt (&(r -> dir), &trans);

/* for starters we find out whether we hit anything. */
	for (i = 0; i < nob; i++) {
	    ss.rad = bl[i] -> s.rad;
	    SV (ss.cent, bl[i] -> s.cent, r -> org);
	    if ((k = find (&trans, &ss)) > 0.0 && k < l) {
		c = i;
		l = k;
	    }
	}
	if (c >= 0 && (l * trans.x.y + r -> org.y) > 0.0) {
				/* WE HIT SOMETHING */
	    MV (l * trans.x.x, l * trans.x.y, l * trans.x.z, new);
/* move the new orgin of the ray to the intersection */
	    AV (refr.org, new, r -> org);
	    AV (r -> org, new, r -> org);
	    MV (r -> dir.x, r -> dir.y, r -> dir.z, refr.dir);
/* get a normal vector for the intersection point */
	    SV (norm, r -> org, bl[c] -> s.cent);
	    norm.l = LEN (norm);

/* ambient lighting */
	    lght = 200.0 * bl[c] -> amb;

/* shaded lighting (diffuse). subroutine shadow is in find.c */
	    if (bl[c] -> dif != 0.0) {
		SV (new, ls.cent, r -> org);
		new.l = LEN (new);
		if ((k = DOT (new, norm)) > 0.0)
		    lght += bl[c] -> dif * shadow (&(r -> org)) * k / (new.l) / (norm.l);
	    }

/*reflection... easy */
	    if (bl[c] -> rfl != 0.0) {
/* make the normal unit length */
		norm.l = LEN (norm);
		SCMLT ((1.0 / norm.l), norm);
/* get the length of the ray's component in the normal direction */
		stupid = 2.0 * DOT (norm, r -> dir);
		SCMLT (stupid, norm);
/* subtract double the normal component- !reflection! */
		SV (r -> dir, r -> dir, norm);
		lght += bl[c] -> rfl * (double) shade (r);
	    }

/* refraction. this is ugly as sin, which is why I choose to deal with
   it in it's own subroutine which is in it's own file now*/
	    if (bl[c] -> rfr != 0.0) {
		lght += bl[c] -> rfr * (double) refract (&refr, bl[c]);
	    }



	}
	else {			/* hit no objects... */
	    if ((r -> dir.y) < 0.0) {/* crosses floor */
		z = -(r -> org.y) / (r -> dir.y);
		(r -> org.x) += z * (r -> dir.x);
		(r -> org.z) += z * (r -> dir.z);
		(r -> org.y) = 0.0;
/*find out the mod of the value to see where it hits susie. (ouch)*/
/* mess *here* if you only want the pattern once */
		SV (new, ls.cent, r -> org);
		new.l = LEN (new);
		sx = (int) (r -> org.x / 1.5) % xsue;
		if (sx < 0)
		    sx += xsue;
		sy = -(int) (r -> org.z / 1.5) % ysue;
		if (sy < 0)
		    sy += ysue;
		lght = (sam * suzie[sx][sy] + 1.0 - sam) * (0.8 *
			shadow (&(r -> org)) * (new.y) / (new.l) + 40.0);


	    }
	    else {		/* check to see if it hit lightsource */
		SV (ss.cent, ls.cent, r -> org);
		ss.rad = ls.rad;
		if (find (&trans, &(ss.cent)) > 0.0)
		    lght = 255;
		else
		    lght = 0;
	    }
	}
    }
/* to many levels return 0 cause it shouldn't matter */
    else
	lght = 0;
    level--;
    if (lght < 0.0)
	lght = 0.0;
    if (lght > 255.0)
	lght = 255.0;
    return ((int) lght);
}
