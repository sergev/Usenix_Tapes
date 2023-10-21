#include	"p.h"

/*
 *  (Sect. 24; Pg. 39)
 */
REAL
ratoha(alpha)
REAL	alpha;
	{
	REAL	lsthour;

	lsthour = hmstohr(&now.lst);
	lsthour -= alpha;
	lsthour = tcanon(lsthour);
	return(lsthour);
}

/*
 *  (Sect. 24; Pg. 39)
 */
REAL
hatora(H)
REAL	H;
	{
	REAL	lsthour;

	lsthour = hmstohr(&now.lst);
	lsthour -= H;
	lsthour = tcanon(lsthour);
	return(lsthour);
}

/*
 *  (Sect. 25; Pg. 40)
 */
struct	ae *
eqtohzn(eqp)
register struct	equat	*eqp;
	{
	static	struct	ae	ae;
	REAL	a, A, delta, H;

	H = ratoha(eqp->ra);		/* its hour angle */
	H *= 15.0;			/* hour angle to degrees */
	delta = eqp->dec;

	a = (REAL) sin(delta/RAD) * (REAL) sin(HERELAT/RAD)
		+ (REAL)cos(delta/RAD) * (REAL)cos(HERELAT/RAD)
			* (REAL)cos(H/RAD);
	a = (REAL) asin(a) * RAD;

	A = ((REAL) sin(delta/RAD) - (REAL) sin(HERELAT/RAD)
		* (REAL) sin(a/RAD))
			/ ((REAL) cos(HERELAT/RAD) * (REAL) cos(a/RAD));
	A = (REAL) acos(A) * RAD;

	if (sin(H/RAD) > 0.0)
		A = 360.0 - A;

	ae.az = A;
	ae.el = a;
	return(&ae);
}

/*
 *  (Sect. 26; Pg. 42)
 */
struct	equat *
hzntoeq(aep)
register struct	ae	*aep;
	{
	static	struct	equat	equat;
	REAL	a, A, delta, H;

	A = aep->az;
	a = aep->el;

	delta = (REAL) sin(a/RAD) * (REAL) sin(HERELAT/RAD)
		+ (REAL) cos(a/RAD) * (REAL) cos(HERELAT/RAD) * (REAL) cos(A/RAD);
	delta = (REAL) asin(delta) * RAD;

	H = ((REAL) sin(a/RAD) - (REAL) sin(HERELAT/RAD) * (REAL) sin(delta/RAD))
		/ ((REAL) cos(HERELAT/RAD) * (REAL) cos(delta/RAD));
	H = (REAL) acos(H) * RAD;

	if (sin(A/RAD) > 0.0)
		H = 360.0 - H;

	equat.ra = H / 15.0;		/* convert deg to hour angle */
	/*
	 *  Then convert hour angle to RA.
	 */
	equat.ra = hatora(equat.ra);
	equat.dec = delta;
	return(&equat);
}

/*
 *  (Sect. 27; Pg. 44)
 */
struct	equat *
ectoeq(ecp)
register struct	eclipt	*ecp;
	{
	REAL	alpha, beta, delta, lambda, epsilon, x, y;
	static	struct	equat	equat;

	beta = ecp->lat / RAD;
	lambda = ecp->lon / RAD;

	epsilon = obliq(now.jd) / RAD;

	delta = (REAL) sin(beta) * (REAL) cos(epsilon)
		+ (REAL) cos(beta) * (REAL) sin(epsilon) * (REAL) sin(lambda);

	delta = asin(delta) * RAD;

	y = (REAL) sin(lambda) * (REAL) cos(epsilon)
		- (REAL) tan(beta) * (REAL) sin(epsilon);
	x = (REAL) cos(lambda);

	alpha = atan2a(y, x) * RAD;
	alpha /= 15.0;			/* degrees to RA */
	equat.ra = alpha;
	equat.dec = delta;

	return(&equat);
}

/*
 *  (Sect. 28; Pg. 46)
 */
struct	eclipt *
eqtoec(eqp)
register struct	equat	*eqp;
	{
	REAL	alpha, beta, delta, lambda, epsilon, x, y;
	static	struct	eclipt	eclipt;

	delta = eqp->dec / RAD;
	alpha = (eqp->ra * 15.0) / RAD;

	epsilon = obliq(now.jd) / RAD;

	beta = (REAL) sin(delta) * (REAL) cos(epsilon)
		- (REAL) cos(delta) * (REAL) sin(epsilon) * (REAL) sin(alpha);

	beta = asin(beta) * RAD;

	y = (REAL) sin(alpha) * (REAL) cos(epsilon)
		+ (REAL) tan(delta) * (REAL) sin(epsilon);
	x = (REAL) cos(alpha);

	lambda = atan2a(y, x) * RAD;

	eclipt.lat = beta;
	eclipt.lon = lambda;

	return(&eclipt);
}

REAL
obliq(jd)
REAL	jd;
	{
	REAL	T, de, epsilon;

	jd -= 2415020.0;		/* JD for 1900 */
	T = jd / 36525.0;

	de = 46.845 * T + 0.0059 * T * T - 0.00181 * T * T * T;
	de /= 3600.0;

	epsilon = 23.452294 - de;
	return(epsilon);
}
