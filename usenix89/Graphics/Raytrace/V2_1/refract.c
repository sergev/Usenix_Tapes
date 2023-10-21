#include <math.h>
#include "rtd.h"
#include "macros.h"
#include "extern.h"

int	rlev;
int     refract (r, bll)
struct ray *r;
struct ball *bll;
{
    struct vector   new,
                    norm;
    struct mat  trans;
    struct ray  ir;
    double  l,
            refk (), getcapt (), capt, inside ();
    double  stupid;
    struct sphere   ss;

    SV (norm, r -> org, bll -> s.cent);
    norm.l = bll-> s.rad;

    capt = getcapt (&norm, &(r -> dir), bll -> ior);


/* get the addition factor for the normal for refraction*/
    stupid = refk (&(norm), &(r -> dir), bll -> ior);
    SCMLT (stupid, norm);

    AV (ir.dir, r -> dir, norm);
    MV (r -> org.x, r -> org.y, r -> org.z, ir.org);

/* now get it for reflection */
    SV (norm, r -> org, bll -> s.cent);
    norm.l = bll -> s.rad;
    SCMLT (1.0 / norm.l, norm);
    stupid = 2.0 * DOT (norm, r -> dir);
    SCMLT (stupid, norm);
    SV (r -> dir, r -> dir, norm);

    return ((int) ((1.0 - capt) * (double) shade (r) + ((capt) * inside (&ir, bll))));
}

double  inside (r, bll)
struct ray *r;
struct ball *bll;
{
    struct vector   new,
                    norm;
    struct mat  trans;
    struct ray  er;
    double  findo (), lght, l, refk (), getcapt (), capt;
    double  stupid;
    struct sphere   ss;


    if (++rlev < RLEV) {
	r -> dir.l = LEN (r -> dir);
	r -> dir.xzl = XZL (r -> dir);
	mt (&(r -> dir), &trans);
	ss.rad = bll -> s.rad;
	SV (ss.cent, bll -> s.cent, r -> org);

	l = findo (&trans, &ss);
	MV (l * trans.x.x, l * trans.x.y, l * trans.x.z, new);
	AV (er.org, r -> org, new);
	AV (r -> org, r -> org, new);
	SV (norm, er.org, bll -> s.cent);

	norm.l = bll -> s.rad;
	capt = getcapt (&norm, &(r -> dir), 1.0 / bll -> ior);

	stupid = refk (&norm, &(r -> dir), 1.0 / bll -> ior);
	SCMLT (stupid, norm);
	AV (er.dir, norm, r -> dir);

	SCMLT (1.0 / norm.l, norm);
	stupid = 2.0 * DOT (norm, r -> dir);
	SCMLT (stupid, norm);
	SV (r -> dir, r -> dir, norm);
	lght = (1.0 - capt) * inside (r, bll) + (capt * (double) shade (&er));
    }
    else
	lght = 0.0;
    rlev--;
     if (lght<0.0) lght=0.0;
     if (lght>255.0) lght=255.0;
    return (lght);
}



double  refk (nrm, in, ior)
struct vector  *nrm,
               *in;
double  ior;
{
    double  dt,
            ln,
            li,
            ret;

    ior = ior * ior;
    dt = DOT ((*nrm), (*in));
    ln = LN2 ((*nrm));
    li = LN2 ((*in));
    if (dt < 0)
	ret = (-dt - sqrt (dt * dt - ln * li * (1 - ior))) / ln;
    else
	ret = (-dt + sqrt (dt * dt - ln * li * (1 - ior))) / ln;
    return (ret);
}

double  getcapt (nrm, dr, ior)
struct vector  *nrm,
               *dr;
double  ior;
{
    double  dt,
            cs1,
            cs2,
            p,
            s;
    dt = DOT ((*nrm), (*dr));
    dt = dt * dt / LN2 ((*nrm)) / LN2 ((*dr));
    cs1 = sqrt (dt);
    cs2 = sqrt (1.0 - (1.0 - dt) / ior);
    p = cs1 / (cs1 + ior * cs2);
    s = cs1 / (ior * cs1 + cs2);
    return (2.0 * (p * p + s * s));
}
