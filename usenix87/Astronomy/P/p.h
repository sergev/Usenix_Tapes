#include	<stdio.h>
#include	<time.h>
#include	<math.h>
#include	<curses.h>

/*
 *	Compute and display various data on the Sun, Moon, and
 *	planets.  The source book for this program is
 *
 *		"Practical astronomy with your calculator"
 *			by
 *			  Peter Duffet-Smith
 *
 *					R. C. Kukuk
 *					11/83
 */

typedef	double	REAL;

extern	int dmsize[];

extern	char	month[12][3];

extern	char	days[7][3];

/*
 *  The time structure, tm, is used as the basic data structure for
 *  holding time and date information.  The following differences
 *  are noted:
 *
 *	tm_year contains the actual year.
 *
 *	tm_mon ranges between 1 and 12.
 *
 *	tm_yday ranges between 1 and 365 (366).
 *
 *	tm_isdst contains more than just a daylight savings time flag.
 *	It also contains an indicator describing what kind of time is
 *	currently being stored in the structure.  The kinds are listed
 *	below.
 *
 *  When conversions are made between the various time formats, not all
 *  of the fields may be valid.  For example, only the fields relating
 *  to the time of day are valid for GMT, GST, and LST.
 */
#define	DST	1		/* Local daylight savings time */
#define	LT	0		/* Local time */
#define	LST	-1		/* Local siderial time */
#define	GST	-2		/* Greenwich siderial time */
#define	GMT	-3		/* Greenwich mean time */

#define	NAPLONG		88.1			/* west long is pos */
#define	NAPLAT		41.7			/* west lat is pos */
#define	NAPELEV		198.0			/* meters */
#define	NAPZONE		6.0			/* Central Standard Timezone */

#define	HERELONG	NAPLONG
#define	HERELAT		NAPLAT
#define	HERELEV		NAPELEV
#define	HEREZONE	NAPZONE

#define	NORISE	25.0
#define	NOSET	-1.0

#define	PI	3.141592654
#define	RAD	(360.0/(2.0*PI))

#define	NOW	1
#define	NOTNOW	0

/*
 *  Coordinate structures.
 */
struct	equat	{
	REAL	ra;			/* in hours, min, and sec */
	REAL	dec;
};

struct	ae	{
	REAL	az;
	REAL	el;
};

struct	eclipt	{
	REAL	lat;
	REAL	lon;
};

struct	deg	{
	char	sg;			/* sign for neg angles */
	int	deg;			/* or hour angle or ra */
	int	min;
	int	sec;
};

struct	rs	{
	REAL	risetime;
	REAL	settime;
	REAL	riseaz;
	REAL	setaz;
};

/*
 *  CRT screen coordinates for displaying the results.
 */
struct	pt	{
	short	row;
	short	col;
};

extern	struct	pt	pt[];

/*
 *  Planetary orbital position data structure.
 */
struct	planpos	{
	REAL	M;		/* Mean anonaly */
	REAL	v;		/* True anomaly */
	REAL	L;		/* Heliocentric longitude */
	REAL	R;		/* Radius vector (AU) */
};

/*
 *  Everything you wanted to know about a planet.
 */
struct	planet	{
	struct	equat	equat;	/* Location */
	struct	ae	hzn;	/* Current horizon location */
	struct	rs	rs;	/* Rising & setting times and az's */
	REAL	rho;		/* Distance (AU) */
	struct	deg	ltt;	/* Light travel time (hr, min) */
	REAL	th;		/* Angular diameter */
	REAL	F;		/* Phase */
	REAL	m;		/* Apparent brightness */
};

/*
 *  Everything worth knowing about Sol.  The ecliptic field must
 *  be kept up to date by calling sunorbit();
 */
struct	sun	{
	struct	equat	equat;	/* Location */
	struct	eclipt	eclipt;	/* "Funny" heliocentric coordinates */
	struct	ae	hzn;	/* If you've lost it */
	struct	rs	rs;	/* "Sunrise, sunset, swiftly go the days ..." */
	REAL	th;		/* Angular diameter */
	REAL	dist;		/* Distance in AU */
	REAL	M;		/* Needed for Lunar calculations */
};

extern	struct	sun	sun;

/*
 *  Everything worth knowing about Luna.
 */
struct	moon	{
	struct	equat	equat;	/* Location */
	struct	ae	hzn;	/* If you've lost it */
	struct	rs	rs;	/* Moon rise and set */
	REAL	dist;		/* Distance in km */
	REAL	th;		/* Angular diameter */
	REAL	age;		/* Age of phase */
	REAL	F;		/* Phase */
	REAL	lpp;		/* Needed for phase calculations */
	REAL	Mpm;		/* Needed for distance calcs */
	REAL	Ec;		/*   "     "     "       "   */
	REAL	heliolon;	/* Needed for eclipse calculations */
	REAL	Np;		/*   "     "     "         " */
};

extern	struct	moon	moon;

/*
 *  Planetary equatorial coordinates; kept and used for angular
 *  separation calculations.
 */
extern	struct	equat	wandeq[];

/*
 *  Orbital data for Sol.
 *  (Pg. 82)
 */
#define	S_EPSG		278.83354
#define	S_OMEGG		282.596403
#define	S_E		0.016718
#define	S_R0		1.495985e8
#define	S_TH0		0.533128

/*
 *  Orbital data for Luna.
 *  (Pg. 140)
 */
#define	L_L0		64.975464
#define	L_P0		349.383063
#define	L_N0		151.950429
#define	L_I		5.145396
#define	L_E		0.05490
#define	L_TH0		0.5181
#define	L_A		384401.0
#define	L_PI0		0.9507

/*
 *  Appropriate subscripts.
 */
#define	MERCURY		0
#define	VENUS		1
#define	MARS		2
#define	JUPITER		3
#define	SATURN		4
#define	URANUS		5
#define	NEPTUNE		6
#define	PLUTO		7
#define	SUN		8
#define	MOON		9
#define	SEPMAT		10
#define	TIME		11


#define	SECCOL		9

/*
 *  Planetary orbital data.
 */
struct	pod	{
	REAL	Tp;			/* Period in tropical years */
	REAL	epsilon;		/* Longitude at epoch (1/0/1980) */
	REAL	w;			/* Longitude of perihelion */
	REAL	e;			/* Eccentricity of orbit */
	REAL	a;			/* Semi-major axis (AU) */
	REAL	i;			/* Inclination of orbit */
	REAL	omega;			/* Longitude of ascending node */
	REAL	th0;			/* Angular size at 1 AU */
	REAL	A;			/* Brightness factor */
};

extern	struct	pod	pod[];

/*
 *  Keep Earth orbital data separate for ease in subscripting the others.
 */
extern	struct	pod	epod;

extern	char	*name[];

REAL	flrem(), flday(), julianday(), hmstohr(), dmstodeg(), epoch80();
REAL	s12b();
REAL	hatora(), ratoha();
REAL	obliq(), kepler(), dcanon(), tcanon();
REAL	angsep(), atan2a(), parsint();
double	atof();

struct	tm	*jdtodate(), *lsttolt();
struct	ae	*eqtohzn();
struct	equat	*getequat(), *hzntoeq(), *ectoeq(), *moonorbit();
struct	eclipt	*eqtoec();
struct	rs	*riseset(), *sunriseset(), *atmref(), *moonriseset();
struct	planpos	*orbit();
struct	planet	*plandata();

/*
 *  This is the now structure.  Its contents should be kept up to date
 *  by calling tempus().
 */
struct	now	{
	struct	tm	lt;
	struct	tm	gmt;
	struct	tm	gst;
	struct	tm	lst;
	REAL	jd;
};
extern	struct	now	now;

/*
 *  Orbital position of Earth (heliocentric coodinates) computed
 *  from the time in the now structure.  Kept up to date by calling
 *  earthorbit().
 */
extern	struct	planpos	earth;

extern	int	iter;		/* iteration count for Keplerian calculations */
extern	int	flags;		/* file control flags */
extern	int	print;		/* hard copy output flag */
extern	int	loop;		/* keep looping */
extern	int	ritenow;	/* compute results for today's date and time */
extern	int	request;	/* user has request pending */
extern	int	daylight;	/* enable DST conversions */
extern	REAL	deltat;		/* loop increment in REAL days */
extern	int	eltime;		/* sleep away the actual interval */
extern	int	donin;
