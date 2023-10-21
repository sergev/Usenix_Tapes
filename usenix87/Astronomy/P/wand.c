#include	"p.h"

/*
 *  (Sect. 31; Pg. 52)
 */
REAL
angsep(eqp1, eqp2)
register struct	equat	*eqp1, *eqp2;
	{
	REAL	alph1, alph2, del1, del2;
	REAL	d;

	/*
	 *  Hours to degrees to rads.
	 */
	alph1 = eqp1->ra * 15.0 / RAD;
	alph2 = eqp2->ra * 15.0 / RAD;

	del1 = eqp1->dec / RAD;
	del2 = eqp2->dec / RAD;

	d = (REAL) sin(del1) * (REAL) sin(del2)
	     + (REAL) cos(del1) * (REAL) cos(del2) * (REAL) cos(alph1 - alph2);

	d = (REAL) acos(d) * RAD;

	return(d);
}

/*
 *  (Sect. 32; Pg. 54)
 */
struct	rs *
riseset(eqp)
register struct	equat	*eqp;
	{
	REAL	Ar, As, alpha, delta, H;
	static	struct	rs	rs;
	struct	rs	*drsp;

	delta = eqp->dec;
	alpha = eqp->ra;		/* alpha is in hours */

	Ar = (REAL) sin(delta/RAD) / (REAL) cos(HERELAT/RAD);

	rs.riseaz = rs.setaz = -1.0;

	if (Ar <= -1.0) {
norise:
		rs.risetime = rs.settime = NORISE;
		return(&rs);
	}
	if (Ar >= 1.0) {
noset:
		rs.risetime = rs.settime = NOSET;
		return(&rs);
	}

	Ar = (REAL) acos(Ar) * RAD;
	As = 360.0 - Ar;
	rs.riseaz = Ar;
	rs.setaz = As;

	H = -(REAL) tan(HERELAT/RAD) * (REAL) tan(delta/RAD);
	if (H >= 1.0)
		goto norise;
	if (H <= -1.0)
		goto noset;

	H = (REAL) acos(H) * RAD;
	H /= 15.0;			/* degrees to hour angle */

	/*
	 *  Now correct for atmospheric refraction.
	 */
	drsp = atmref(eqp);
	rs.riseaz -= drsp->riseaz;
	rs.setaz += drsp->setaz;

	rs.risetime = tcanon(24.0 + alpha - H - drsp->risetime);

	rs.settime = tcanon(alpha + H + drsp->settime);
	/*
	 *  The rising and setting times are LST.
	 */
	return(&rs);
}

/*
 *  Correct equatorial coordinates for precession.
 *  (Sect. 33, Pg. 58)
 */
precess(eqp)
register struct	equat	*eqp;
	{
	REAL	da, dd;
	REAL	N;
	REAL	d, a;

	a = eqp->ra / RAD;		/* hours to rads */
	d = eqp->dec / RAD;		/* degrees to rads */
	/*
	 *  Use epoch 1950 precession values.
	 */
	N = (REAL) now.lt.tm_year - 1950.0;	/* forget fractions */

#define	ATERM	(3.07327 / 3600.0)	/* seconds to hours */
#define	BTERM	(1.33617 / 3600.0)	/* seconds to hours */
#define	CTERM	(20.0426 / 3600.0)	/* arcseconds to degrees */

	/*
	 *  Compute the deltas.
	 */
	da = (ATERM + BTERM * (REAL) sin(a) * (REAL) tan(d)) * N;
	dd = CTERM * (REAL) cos(a) * N;

	eqp->ra += da;			/* in hours */
	eqp->dec += dd;			/* in degrees */
}

/*
 *  (Sect. 34; Pg. 60)
 */
struct	rs *
atmref(eqp)
register struct	equat	*eqp;
	{
	static	struct	rs	drs;
	REAL	x, y, psi, dA, dt;
	
	x = 0.566667;
	psi = (REAL) sin(HERELAT/RAD) / (REAL) cos(eqp->dec/RAD);
	psi = (REAL) acos(psi) * RAD;
	dA = (REAL) tan(x/RAD) / (REAL) tan(psi/RAD);
	dA = (REAL) asin(dA) * RAD;

	y = (REAL) sin(x/RAD) / (REAL) sin(psi/RAD);
	y = (REAL) asin(y) * RAD;
	dt = (240.0 * y) / (REAL) cos(eqp->dec/RAD);
	dt /= 3600.0;

	drs.riseaz = drs.setaz = dA;
	drs.risetime = drs.settime = dt;
	return(&drs);
}

/*
 *  Correct the equatorial coordinates of the Moon for geocentric
 *  parallax.
 *  (Sect. 35, 36; Pg. 63, 66)
 */
geopalx(eqp)
register struct	equat	*eqp;
	{
	REAL	u, hp, rhosinphi, rhocosphi;
	REAL	dalpha, deltap, r;
	REAL	n, d, H, Hp;

	/*
	 *  First find rhosinphi and rhocosphi.
	 */
	u = (REAL) tan(HERELAT/RAD);
	u = (REAL) atan(0.996647 * u);

	hp = HERELEV / 6378140.0;

	rhosinphi = 0.996647 * (REAL) sin(u) + hp * (REAL) sin(HERELAT/RAD);

	rhocosphi = (REAL) cos(u) + hp * (REAL) cos(HERELAT/RAD);
	/*
	 *  Then use them in the parallax calculations.
	 */
	H = ratoha(eqp->ra) * 15.0;
	r = moon.dist / 6378.16;
	n = rhocosphi * (REAL) sin(H/RAD);
	d = r * (REAL) cos(eqp->dec/RAD) * rhocosphi
		* (REAL) cos(H/RAD);
	/*
	 *  Delta alpha to degrees, then hours; then apply it.
	 */
	dalpha = (REAL) atan(n/d) * RAD;
	eqp->ra -= dalpha / 15.0;

	Hp = H + dalpha;

	n = r * (REAL) sin(eqp->dec/RAD) - rhosinphi;
	d = r * (REAL) cos(eqp->dec/RAD) * (REAL) cos(H/RAD) - rhocosphi;
	deltap = (REAL) atan((REAL) cos(Hp/RAD) * n / d);
	/*
	 *  Replace the old declination with the new.
	 */
	eqp->dec = deltap;
}

/*
 *  Compute the planetary position data for Earth.
 */
earthorbit()
	{
	earth = *orbit(&epod);
}

/*
 *  Compute the planetary data for the given planet number.
 *  (Sect. 50, 53, 54, 56; Pp. 98, 113, 115, 118)
 */
struct	planet *
plandata(plano)
	{
	REAL	a, x, y, psi, lp, rp, A, rho;
	static	struct	planet	planet;
	register struct	planpos	*ppos;
	struct	eclipt	ecl;
	struct	equat	*eqp;

	/*
	 *  Compute the heliocentric coordinates for the requested
	 *  planet.  Earth has already been done.
	 */
	ppos = orbit(&pod[plano]);

	a = (ppos->L - pod[plano].omega) / RAD;
	
	psi = (REAL) sin(a) * (REAL) sin(pod[plano].i/RAD);
	psi = (REAL) asin(psi) * RAD;

	y = (REAL) sin(a) * (REAL) cos(pod[plano].i/RAD);

	x = (REAL) cos(a);

	a = (REAL) atan2a(y, x) * RAD;

	lp = a + pod[plano].omega;

	rp = ppos->R * (REAL) cos(psi/RAD);
	/*
	 *  Different geometry for inner and outer planets.
	 */
	if (plano == MERCURY || plano == VENUS) {
		a = (earth.L - lp) / RAD;
		A = (rp * (REAL) sin(a)) / (earth.R - rp * (REAL) cos(a));
		A = (REAL) atan(A) * RAD;

		ecl.lon = dcanon(180.0 + earth.L + A);

		ecl.lat = (rp * (REAL) tan(psi/RAD)
		    * (REAL) sin((ecl.lon - lp)/RAD))
			/ (earth.R * (REAL) sin((lp - earth.L)/RAD));
		ecl.lat *= RAD;
	} else {
		a = (lp - earth.L) / RAD;

		x = (earth.R * (REAL) sin(a)) / (rp - earth.R * (REAL) cos(a));
		x = (REAL) atan(x) * RAD + lp;
		ecl.lon = dcanon(x);

		x = (rp * (REAL) tan(psi/RAD) * (REAL) sin((ecl.lon - lp)/RAD))
			/ (earth.R * (REAL) sin(a));
		ecl.lat = (REAL) atan(x) * RAD;
	}
	/*
	 *  Convert heliocentric (ecliptic) coordinates to
	 *  equatorial ones.
	 */
	eqp = ectoeq(&ecl);
	/*
	 *  Apply precession correction.
	 */
	precess(eqp);
	/*
	 *  Stash the equatorial coords.
	 */
	planet.equat.ra = eqp->ra;
	planet.equat.dec = eqp->dec;
	/*
	 *  Compute the horizon coordinates.
	 */
	planet.hzn = *eqtohzn(eqp);
	/*
	 *  Compute the rise and set times and azimuths.
	 *  (The times are LST.)
	 */
	planet.rs = *riseset(eqp);
	/*
	 *  Determine the distance.
	 */
	rho = earth.R * earth.R + ppos->R * ppos->R
	      - 2.0 * earth.R * ppos->R * (REAL) cos(ppos->L/RAD - earth.L/RAD);
	rho = (REAL) sqrt(rho);
	planet.rho = rho;
	/*
	 *  Do light travel time.
	 */
	degtodms(&planet.ltt, rho * 0.1386);
	/*
	 *  Do angular diameter.
	 */
	planet.th = pod[plano].th0 / rho;
	/*
	 *  Determine the phase.
	 */
	planet.F = 0.5 * (1.0 + (REAL) cos((ecl.lon - ppos->L) / RAD));
	/*
	 *  And last, but not least, determine the apparent
	 *  brightness.
	 */
	x = (ppos->R * rho) / (pod[plano].A * (REAL) sqrt(planet.F));
	planet.m = 5.0 * (REAL) log10(x) - 26.7;

	return(&planet);
}

struct	planpos *
orbit(plp)
register struct	pod	*plp;
	{
	static	struct	planpos	scr;
	REAL	e80, N, eps, w, e, a;

	e80 = epoch80(&now.lt);
	eps = plp->epsilon;
	e = plp->e;
	w = plp->w;
	a = plp->a;

	N = (360.0 / 365.2422) * (e80 / plp->Tp);
	N = dcanon(N);

	scr.M = N + eps - w;
	scr.L = N + (360.0/PI) * e * (REAL) sin(scr.M/RAD) + eps;
	scr.L = dcanon(scr.L);

	scr.v = scr.L - w;

	scr.R = a * (1.0 - e * e) / (1.0 + e * (REAL) cos(scr.v/RAD));
	return(&scr);
}

#ifdef	KEEPOUT
/*
 * This function is under construction.  Invoke at your own risk.
 */
orbit(plp)
register struct	pod	*plp;
	{
	REAL	E, N, D, M, v;

	D = epoch80(&now.lt);
	N = (360.0 / 365.2422) * (D / plp->Tp);
	N = dcanon(N);

	M = N + plp->epsilon - plp->w;
	M = dcanon(M);
	E = kepler(M/RAD, plp->e);
	v = (REAL) sqrt((1.0 + plp->e) / (1.0 - plp->e)) * (REAL) tan(E/2);
	v = (REAL) atan(v);
	v *= 2 * RAD;
	eclipt.lon = dcanon(v + plp->epsilon);
	eclipt.lat = 0.0;

	return(&eclipt);
}
#endif
