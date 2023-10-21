#include <math.h>
#include "nav.h"
/*
 * Corrected Magnetic procedure for course < 0     09-24-85
 *
 */
/*
 *	Returns radians given decimal degrees
 */
double Radians(degrees)
double degrees;
{
	return degrees * (pi/180.);
}

/*
 *	Returns decimal degrees given radians
 */
double Degrees(radians)
double radians;
{
	return radians / (pi/180.);
}

/*
 *	Returns distance in Nautical Miles given two sets of latitude
 *	and longitude in radians.
 *	Latitudes are positive for North, negative for South (N1, N2)
 *	Longitudes are positive for West, negative for East  (W1, W2)
 *
 *	Algorithm from HP-25 Applications Programs, Rhumbline Navigation,
 *	page 65.
 */
double Distance(N1,W1,N2,W2)
double N1,W1,N2,W2;
{
	register double C;

	C = atan2 (
	    ( 2. * asin(sin(.5 * (W1 - W2))) )
		,
	    ( log(tan(pi*.25 + .5*N2)) - log(tan(pi*.25 + .5*N1)) )
		 );

	if (N1 == N2)
	      return (60. * fabs(Degrees(2. * asin(sin(.5 * (W1 - W2)))))
			  * cos(N1));
	   else
	      return (60. * Degrees(N2 - N1) / cos(fabs(C)));
} 

/*
 *	Returns true bearing from North in decimal degrees given two sets of
 *	latitude and longitude in radians.
 *	Latitudes are positive for North, negative for South (N1, N2)
 *	Longitudes are positive for West, negative for East  (W1, W2)
 *
 *	Algorithm from HP-25 Applications Programs, Rhumbline Navigation,
 *	page 65.
 */
double Bearing(N1,W1,N2,W2)
double N1,W1,N2,W2;
{
	register double C;

	C = atan2 (
	    ( 2. * asin(sin(.5 * (W1 - W2))) )
		,
	    ( log(tan(pi*.25 + .5*N2)) - log(tan(pi*.25 + .5*N1)) )
		 );
	return ( (asin(sin(W1-W2)) >= 0.)
			? Degrees(fabs(C))
			: 360. - Degrees(fabs(C)) );


}

/*
 *	Returns magnetic bearing in decimal degrees given true bearing and the
 *	magnetic variation in decimal degrees.  East variation is expressed
 *	by a negative value.	Pretty fancy, eh?
 */
double Magnetic(true,variation)
double true,variation;
{
	register double course;

	course = true + variation;
	if ( course < 0. ) course = 360. + course;
	if ( course >= 360. ) course = course - 360.;
	return course;
}

/*
 *	Returns latitude in radians (Ni) of the intercept at longitude
 *	Wi for the great circle course from (N1, W1) to (N2, W2).
 *	Latitude and longitude in radians.
 *	Latitudes are positive for North, negative for South (N1, N2)
 *	Longitudes are positive for West, negative for East  (W1, W2)
 *
 *	Algorithm from HP-25 Applications Programs, Great Circle Ploting,
 *	page 62.
 */
double LatIntercept(N1,W1,N2,W2,Wi)
double N1,W1,N2,W2,Wi;
{
	if (W1 == W2)
		oops("Sorry, True N/S from/to's break nav");

	return (
		atan(
		     (tan(N2)*sin(Wi-W1) - tan(N1)*sin(Wi-W2))
				/
		     (sin(W2-W1))
		)
	);
}
WindCorrection (tas, crs, speed, direction, gs, var)
int tas, speed, direction, *gs;
double *crs, var;
{
double B, wca;

	if (*crs == direction) /* heading is INTO the wind */
        {
            *gs = *gs - speed;
        }
        else
        if (((abs(*crs - direction)) % 180) == 0) /* heading WITH the wind */
        {
            *gs = *gs + speed;
        }
        else /* compute effect of wind on heading and ground speed */
        {
            B = Radians (*crs - (direction+var));
            *gs = sqrt (tas*tas + speed*speed -  2.*tas*speed*cos(B));
            wca = Degrees(asin (speed / (*gs / sin(B))));
            *crs = *crs - wca;
            if (*crs < 0) *crs = *crs + 360.0;
        }
}

