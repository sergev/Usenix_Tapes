#include	"p.h"

int dmsize[12] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

char	month[12][3] = {
	"Jan", "Feb", "Mar", "Apr",
	"May", "Jun", "Jul", "Aug",
	"Sep", "Oct", "Nov", "Dec"
};

char	days[7][3] = {
	"Sun", "Mon", "Tue", "Wed",
	"Thu", "Fri", "Sat"
};

/*
 *  Everything worth knowing about Sol.  The ecliptic field must
 *  be kept up to date by calling sunorbit();
 */
struct	sun	sun;

/*
 *  Everything worth knowing about Luna.
 */
struct	moon	moon;

/*
 *  Planetary equatorial coordinates; kept and used for angular
 *  separation calculations.
 */
struct	equat	wandeq[8];

/*
 *  Planetary orbital data.
 */
struct	pod	pod[] =	{

	/*	Mercury		*/
	{	0.24085,	231.2973,	77.1442128,	0.2056306,
			0.3870986,	7.0043579,	48.0941733,
				6.74,		1.918e-6	},

	/*	Venus		*/
	{	0.61521,	355.73352,	131.2895792,	0.0067826,
			0.7233316,	3.394435,	76.4997524,
				16.92,		1.721e-5	},

	/*	Mars		*/
	{	1.88089,	126.30783,	335.6908166,	0.0933865,
			1.5236883,	1.8498011,	49.4032001,
				9.36,		4.539e-6	},

	/*	Jupiter		*/
	{	11.86224,	146.966365,	14.0095493,	0.0484658,
			5.202561,	1.3041819,	100.2520175,
				196.74,		1.994e-4	},

	/*	Saturn		*/
	{	29.45771,	165.322242,	92.6653974,	0.0556155,
			9.554747,	2.4893741,	113.4888341,
				165.60,		1.740e-4	},

	/*	Uranus		*/
	{	84.01247,	228.0708551,	172.7363288,	0.0463232,
			19.21814,	0.7729895,	73.8768642,
				65.80,		7.768e-5	},

	/*	Neptune		*/
	{	164.79558,	260.3578998,	47.8672148,	0.0090021,
			30.10957,	1.7716017,	131.5606494,
				62.20,		7.597e-5	},

	/*	Pluto (osculating elements for 1/2/1980)	*/
	{	250.9,		209.439,	222.972,	0.25387,
			39.78459,	17.137,		109.941,
				8.20,		4.073e-6	}
};

/*
 *  Keep Earth orbital data separate for ease in subscripting the others.
 */
struct	pod	epod	=	{
	{	1.00004,	98.833540,	102.596403,	0.016718,
			1.0,		0.0,		0.0,
				0.0,		0.0		}
};

/*
 *  CRT screen coordinates for displaying the results.
 */
struct	pt	pt[] = {
	{ 1, 8 },			/* Mercury */
	{ 2, 8 },			/* Venus */
	{ 3, 8 },			/* Mars */
	{ 4, 8 },			/* Jupiter */
	{ 5, 8 },			/* Saturn */
	{ 6, 8 },			/* Uranus */
	{ 7, 8 },			/* Neptune */
	{ 8, 8 },			/* Pluto */
	{ 10, 8 },			/* Sun */
	{ 11, 8 },			/* Moon */
	{ 13, 0 },			/* Separation matrix */
	{ 22, 0 },			/* Time */
};

char	*name[]	=	{
	"Mercury", "Venus", "Mars", "Jupiter", "Saturn",
	"Uranus", "Neptune", "Pluto", "Sun", "Moon", "Angsep", ""
};

/*
 *  This is the now structure.  Its contents should be kept up to date
 *  by calling tempus().
 */
struct	now	now;

/*
 *  Orbital position of Earth (heliocentric coodinates) computed
 *  from the time in the now structure.  Kept up to date by calling
 *  earthorbit().
 */
struct	planpos	earth;

int	iter;		/* iteration count for Keplerian calculations */
int	flags;		/* file control flags */
int	print;		/* hard copy output flag */
int	loop;		/* keep looping */
int	ritenow;	/* compute results for today's date and time */
int	request;	/* user has request pending */
extern	int	daylight;	/* enable DST conversions */
int	eltime;		/* sleep away the actual interval */
REAL	deltat;		/* loop increment in REAL days */
int	donin;		/* input line is complete; parse it */
