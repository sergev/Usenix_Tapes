#include	"p.h"

/*
 *  (Sect. 42; Pg. 80)
 */
sunorbit()
	{
	REAL	E, N, D, M, v;

	D = epoch80(&now.lt);
	N = (360.0 / 365.2422) * D;
	N = dcanon(N);
	M = N + S_EPSG - S_OMEGG;
	M = dcanon(M);
	E = kepler(M/RAD, S_E);
	v = (REAL) sqrt((1.0 + S_E) / (1.0 - S_E)) * (REAL) tan(E/2);
	v = (REAL) atan(v);
	v *= 2 * RAD;
	sun.eclipt.lon = dcanon(v + S_OMEGG);
	sun.eclipt.lat = 0.0;
	sun.M = M;		/* needed for calculating Luna's orbit */
	/*
	 *  Do the incidentals.
	 */
	sun.dist = (1.0 - S_E * S_E) / (1.0 + S_E * (REAL) cos(v / RAD));
	sun.th = S_TH0 * (1.0 + S_E * (REAL) cos(v/RAD))
		/ (1.0 - S_E * S_E);
}

/*
 *  (Sect. 45; Pg. 88)
 */
struct	rs *
sunriseset()
	{
	static	struct	rs	rs;
	struct	rs	*rsp, rs1, rs2;
	struct	equat	*eqp, eq1, eq2;
	struct	eclipt	eclipt;
	REAL	dayfrac;

	/*
	 *  Make a copy of the sun's location.
	 */
	eclipt = sun.eclipt;
	/*
	 *  Compute the fraction of the current day so the Sun
	 *  can be "moved back" to it's position on the previous
	 *  midnight.
	 */
	dayfrac = hmstohr(&now.lt) / 24.0;
	/*
	 *  Then move it back.
	 */
	eclipt.lon -= dayfrac * 0.985647;

	eqp = ectoeq(&eclipt);
	/*
	 *  Copy over first set of equatorial coords.
	 */
	eq1 = *eqp;
	/*
	 *  Then move the Sun to its position on midnight, tomorrow.
	 */
	eclipt.lon += 0.985647;

	eqp = ectoeq(&eclipt);
	/*
	 *  Copy over second set of equatorial coords.
	 */
	eq2 = *eqp;

	rsp = riseset(&eq1);
	rs1 = *rsp;
	rsp = riseset(&eq2);
	rs2 = *rsp;

	rs.risetime = (24.07 * rs1.risetime)
		/ (24.07 + rs1.risetime - rs2.risetime);
	rs.settime = (24.07 * rs1.settime)
		/ (24.07 + rs1.settime - rs2.settime);

	rs.riseaz = (rs1.riseaz + rs2.riseaz) / 2.0;
	rs.setaz = (rs1.setaz + rs2.setaz) / 2.0;
	/*
	 *  Make a rough correction to allow for the Sun's
	 *  finite diameter and refraction.  (+- 5 minutes.)
	 */
	rs.risetime -= 5.0/60.0;
	rs.settime += 5.0/60.0;
	/*
	 *  The rising and setting times are LST.
	 */
	return(&rs);
}

/*
 *  (Sect. 61; Pg. 139)
 */
struct	equat *
moonorbit(tmp, nowflag)
register struct	tm	*tmp;
	{
	static	struct	eclipt	eclipt;
	REAL	D, l, Mm, N, Ev, Ae, A3, Mpm, Ec;
	REAL	A4, lp, V, lpp, Np, x, y;
	REAL	t;

	D = epoch80(tmp);

	l = dcanon(13.176396 * D + L_L0);
	Mm = dcanon(l - 0.1114041 * D - L_P0);
	N = dcanon(L_N0 - 0.0529539 * D);

	Ev = 1.2739 * (REAL) sin(2.0 * (l/RAD - sun.eclipt.lon/RAD) - Mm/RAD);

	Ae = 0.1858 * (REAL) sin(sun.M / RAD);
	A3 = 0.37 * (REAL) sin(sun.M / RAD);

	Mpm = Mm + Ev - Ae - A3;

	Ec = 6.2886 * (REAL) sin(Mpm / RAD);

	A4 = 0.214 * (REAL) sin(2.0 * Mpm / RAD);

	lp = l + Ev + Ec - Ae + A4;

	V = 0.6583 * (REAL) sin(2.0 * (lp/RAD - sun.eclipt.lat/RAD));

	lpp = lp + V;

	Np = N - 0.16 * (REAL) sin(sun.M / RAD);

	t = lpp / RAD - Np / RAD;
	y = (REAL) sin(t) * (REAL) cos(L_I/RAD);
	x = (REAL) cos(t);

	eclipt.lon = (REAL) atan2a(y, x) * RAD + Np;

	x = (REAL) sin(t) * (REAL) sin(L_I/RAD);
	eclipt.lat = (REAL) asin(x) * RAD;
	/*
	 *  Stash away the other quantities that will be needed
	 *  elsewhere.  Don't stash the quantities computed as a
	 *  result of the moon rise and set calls.
	 */
	if (nowflag == NOW) {
		moon.lpp = lpp;
		moon.Mpm = Mpm;
		moon.Np = Np;
		moon.Ec = Ec;
		moon.heliolon = eclipt.lon;
	}
	/*
	 *  Return equatorial coords.
	 */
	return(ectoeq(&eclipt));
}

/*
 *  (Sect. 63; Pg. 145)
 */
moondata()
	{
	/*
	 *  Compute the Moon's orbit.
	 */
	moon.equat = *moonorbit(&now.lt, NOW);
	/*
	 *  Apply precession correction.
	 */
	precess(&moon.equat);
	/*
	 *  And then convert to horizon coords.
	 */
	moon.hzn = *eqtohzn(&moon.equat);
	/*
	 *  Do age and Fase.
	 */
	moon.age = dcanon(moon.lpp - sun.eclipt.lon) / 13.0;
	moon.F = 0.5 * (1.0 - (REAL) cos(moon.age * 13.0 / RAD));
	/*
	 *  Find the distance in km.
	 */
	moon.dist = L_A * (1.0 - L_E * L_E)
		/ (1.0 + L_E * (REAL) cos((moon.Mpm + moon.Ec)/RAD));
	/*
	 *  Now the moon's apparent diameter, in degrees.
	 */
	moon.th = L_TH0 * L_A / moon.dist;
	/*
	 *  Last, but not least, compute moonrise and moonset.
	 */
	moon.rs = *moonriseset();
}

#define	NOECL		0
#define	LUNAR		1
#define	SOLAR		2
#define	MAYBE		3
#define	MUST		4
/*
 *  Look at the Solar and Lunar coordinates and determine if a
 *  Lunar or Solar eclipse is possible.  If so, display an
 *  appropriate message.  If a message has been displayed, and
 *  the conditions for the eclipse no longer hold, take down
 *  the message.
 */
eclipse()
	{
	static	int	eclmsg;
	static	REAL	svdelta;
	REAL	londiff, nodediff;
	register int	ecltype, echance;

	/*
	 *  Make the angles canonical, and make the node angle
	 *  within 0 <= nodediff < 180.
	 */
	londiff = dcanon(moon.heliolon - sun.eclipt.lon);
	nodediff = dcanon(moon.lpp - moon.Np);
	if (nodediff >= 180.0)
		nodediff -= 180.0;
	/*
	 *  If an eclipse is possible, which one will it be?
	 */
	if (londiff >= 353.0 || londiff <= 7.0)
		ecltype = SOLAR;
	else if (londiff >= 173.0 && londiff <= 187.0)
		ecltype = LUNAR;
	else
		ecltype = NOECL;
	/*
	 *  Slow down the passage of time in order to take a
	 *  closer look.  Or speed up because the configuration
	 *  is past.
	 *  The nodediff test is needed so we don't slow down
	 *  every opposition or conjunction, in spite of the
	 *  location of the nodes.
	 */
	if (ecltype != NOECL && nodediff <= 20.0) {
		if (svdelta == 0.0) {
			svdelta = deltat;
			/*
			 *  Back up before slowing down.
			 */
			deltat = -(deltat * 0.75);
			ticktock();
			gestalt();
			deltat = svdelta;
			/*
			 *  Then decide how much to slow down.
			 */
			if (svdelta < 0.1)
				deltat /= 10.0;
			if (svdelta >= 0.1)
				deltat /= 10.0;
			if (svdelta >= 1.0)
				deltat /= 4.0;
			if (svdelta >= 7.0)
				deltat /= 4.0;
		}
	} else {
		/*
		 *  Full speed ahead.
		 */
		if (svdelta) {
			/*
			 *  If we've reversed directions during the
			 *  slowed interval, carry over the new
			 *  sign.
			 */
			if ((svdelta * deltat) < 0.0) /* different signs */
				deltat = -svdelta;
			else
				deltat = svdelta;
			svdelta = 0.0;
		}
	}
	/*
	 *  But is one the Moon's orbital nodes in the right place?
	 */
	if (ecltype == SOLAR) {
		echance = NOECL;
		if (nodediff <= 18.52)
			echance = MAYBE;
		if (nodediff <= 15.52)
			echance = MUST;
	}
	if (ecltype == LUNAR) {
		echance = NOECL;
		if (nodediff <= 12.25)
			echance = MAYBE;
		if (nodediff <= 9.5)
			echance = MUST;
	}
	/*
	 *  Now generate a message based on the flags.
	 */
	if ( ! print) {
		if (ecltype != NOECL && echance != NOECL) {
			if (eclmsg == 0) {
				standout();
				move(pt[SUN].row, 63);
				printw("%s",
				    ecltype == LUNAR ? "Lunar " : "Solar ");
				move(pt[SUN].row, 69);
				printw("Eclipse");
				eclmsg = 1;
				standend();
			}
			move(pt[SUN].row-1, 67);
			if (echance == MAYBE) {
				standout();
				printw("Maybe");
				standend();
			} else
				printw("     ");
		} else if (eclmsg) {
			move(pt[SUN].row-1, 67);
			printw("     ");
			move(pt[SUN].row, 63);
			printw("      ");
			move(pt[SUN].row, 69);
			printw("       ");
			eclmsg = 0;
		}
	}
}

/*
 *  (Sect. 66; Pg. 150)
 *
 *  (Oh, by the way, there's something wrong with the algorithm,
 *  or maybe the implementation of it ...)
 */
struct	rs *
moonriseset()
	{
	static	struct	rs	mrs;
	struct	equat	eq1, eq2;
	struct	rs	rs1, rs2;
	struct	tm	*tmp;
	REAL	jday;

	/*
	 *  If the Moon is very close the the horizon, the
	 *  rising and setting times will be way off; don't
	 *  do the calculations.  Just return the old values.
	 *
	 *  But make at least one calculation first time through.
	 */
	if (moon.hzn.el > -8.0 && moon.hzn.el < 8.0 && mrs.setaz != 0.0)
		return(&mrs);
	/*
	 *  First get today's julian date.
	 */
	jday = now.jd;
	/*
	 *  Convert it to midnight yesterday.
	 */
	jday -= 0.5;
	jday -= flrem(jday, 1.0);
	jday += 0.5;
	/*
	 *  Make a date out of it.
	 */
	tmp = jdtodate(jday);
	/*
	 *  Find the Moon's coords at that time.
	 */
	eq1 = *moonorbit(tmp, NOTNOW);
	/*
	 *  Advance the clock 12 hours and redo the calc.
	 */
	tmp = jdtodate(jday + 0.5);
	eq2 = *moonorbit(tmp, NOTNOW);
	/*
	 *  Apply precession correction.
	 */
	precess(&eq1);
	precess(&eq2);
	/*
	 *  Correct the coordinates for geocentric parallax.
	 */
	geopalx(&eq1);
	geopalx(&eq2);
	/*
	 *  Compute the rising and setting times and az's.
	 */
	rs1 = *riseset(&eq1);
	rs2 = *riseset(&eq2);
	/*
	 *  Interpolate to find the moon rise and set times.
	 */
	mrs.risetime = (12.03 * rs1.risetime)
		/ (12.03 + rs1.risetime - rs2.risetime);
	mrs.settime = (12.03 * rs1.settime)
		/ (12.03 + rs1.settime - rs2.settime);
	/*
	 *  Just average to find azimuths.
	 */
	mrs.riseaz = (rs1.riseaz + rs2.riseaz) / 2.0;
	mrs.setaz = (rs1.setaz + rs2.setaz) / 2.0;
	/*
	 *  Make a rough correction to allow for the Moon's
	 *  finite diameter.  (+- 2 minutes.)
	 */
	mrs.risetime -= 2.0/60.0;
	mrs.settime += 2.0/60.0;
	/*
	 *  The rising and setting times are LST.
	 */
	return(&mrs);
}
