
/* Prints the phase of the moon and generates LaTeX commands */
/* that produce lunisolar calendars. */
/* Construct with the command "cc -O -o lunisolar lunisolar.c -lm". */
/* Make a 1987 calendar with "lunisolar 1987 > 1987.tex; latex 1987" */
/* John D. Ramsdell - January 1987 */

#include <stdio.h>
#include <math.h>

/* Users must modify the following for their time zone. */
/* TIMEZONE is the number of minutes westward from Greenwich. */
#define TIMEZONE (5 * 60)
#define TZ "EST"		/* The name of the time zone. */

#define PI 3.141592653589793
#define NEGATIVE_PI (-1.0 * PI)
#define TWO_PI (2.0 * PI)
#define RADIANS_PER_DEGREE (PI / 180.0)

int leap_year (year)		/* True if year is a leap_year. */
     int year;
{
  return year % 4 == 0 && year % 100 != 0 || year % 400 == 0;
}

/* Time is most often represented as a double precision number */
/* in units of days.  Angles are in radians. */

/* J2000 is the number of days between January 1, 2000; 12:00:00 GMT */
/* and the local origin of time. */
double J2000;			/* This date is really called J2000.0. */

void make_J2000 ()		/* Assumes time origin of */
{				/* January 1, 1970; 00:00:00 GMT. */
  int year;
  J2000 = (2000 - 1970) * 365 + 0.5;
  for (year = 1970; year < 2000; year++) /* Account for leap years. */
    if (leap_year (year)) J2000 += 1.0;
}

double days_after_J2000 ()	/* Returns the current time, */
{				/* in units of days, after J2000.0. */
  long seconds = time((long *) 0);
  double seconds_per_day = 24 * 60 * 60;
  return seconds / seconds_per_day - J2000;
}

double normalize_angle (angle)	/* Returns the angle between */
     double angle;		/* -PI < angle <= PI. */
{
  if (angle > PI)
    do angle -= TWO_PI; while (angle > PI);
  else
    while (angle <= NEGATIVE_PI) angle += TWO_PI;
  return angle;
}

/*******************************************************************/

/* Astronomical almanac */

/*
 * All formulas are from:
 * The Astronomical Almanac for the Year 1984, 
 * US Naval Observatory and Royal Greenwich Observatory,
 * US Government Printing Office, Washington DC, 1984.
 */

/* Angular position of the sun to a */
/* precision of 0.01 degrees. (Page C24). */

#define SUN0 (RADIANS_PER_DEGREE * 280.460)
#define SUN1 (RADIANS_PER_DEGREE *   0.9856474)
#define SUN2 (RADIANS_PER_DEGREE * 357.528)
#define SUN3 (RADIANS_PER_DEGREE *   0.9856003)
#define SUN4 (RADIANS_PER_DEGREE *   1.915)
#define SUN5 (RADIANS_PER_DEGREE *   0.020)

double sun_position (days)
     double days;
{
  double mean_longitude_of_sun, mean_anomaly, ecliptic_longitude;
  mean_longitude_of_sun =
    normalize_angle (SUN0 + SUN1 * days);
  mean_anomaly =
    normalize_angle (SUN2 + SUN3 * days);
  ecliptic_longitude =
    normalize_angle (mean_longitude_of_sun
		     + SUN4 * sin (mean_anomaly)
		     + SUN5 * sin (2.0 * mean_anomaly));
  return ecliptic_longitude;
}

/* Angular velocity of the sun.  Derivative of sun_position. */

double sun_velocity (days)
     double days;
{
  double mean_anomaly =
    normalize_angle (SUN2 + SUN3 * days);
  return SUN1 + SUN4 * SUN3 * cos (mean_anomaly)
              + SUN5 * 2.0 * SUN3 * cos (2.0 * mean_anomaly);
}
    
/* Angular position of the moon to a */
/* precision of 0.3 degrees. (Page D46). */

#define DAYS_PER_JULEAN_CENTURY 36525.0
#define RADIAN_CENTURY (RADIANS_PER_DEGREE / DAYS_PER_JULEAN_CENTURY)

#define MOON0  (RADIANS_PER_DEGREE * 218.32)
#define MOON1  (RADIAN_CENTURY * 481267.883)
#define MOON2A (RADIANS_PER_DEGREE *   6.29)
#define MOON2B (RADIANS_PER_DEGREE * 134.9)
#define MOON2C (RADIAN_CENTURY * 477198.85)
#define MOON3A (RADIANS_PER_DEGREE *  -1.27)
#define MOON3B (RADIANS_PER_DEGREE * 259.2)
#define MOON3C (RADIAN_CENTURY * -413335.38)
#define MOON4A (RADIANS_PER_DEGREE *   0.66)
#define MOON4B (RADIANS_PER_DEGREE * 235.7)
#define MOON4C (RADIAN_CENTURY * 890534.23)
#define MOON5A (RADIANS_PER_DEGREE *   0.21)
#define MOON5B (RADIANS_PER_DEGREE * 269.9)
#define MOON5C (RADIAN_CENTURY * 954397.70)
#define MOON6A (RADIANS_PER_DEGREE *  -0.19)
#define MOON6B (RADIANS_PER_DEGREE * 357.5)
#define MOON6C (RADIAN_CENTURY * 035999.05)
#define MOON7A (RADIANS_PER_DEGREE *  -0.11)
#define MOON7B (RADIANS_PER_DEGREE * 186.6)
#define MOON7C (RADIAN_CENTURY * 966404.05)

double moon_position (days)
     double days;
{
  return normalize_angle (MOON0
			  + MOON1 * days
			  + MOON2A * sin (MOON2B + MOON2C * days)
			  + MOON3A * sin (MOON3B + MOON3C * days)
			  + MOON4A * sin (MOON4B + MOON4C * days)
			  + MOON5A * sin (MOON5B + MOON5C * days)
			  + MOON6A * sin (MOON6B + MOON6C * days)
			  + MOON7A * sin (MOON7B + MOON7C * days));
}

/****************************************************************/

/* Prints an English sentence giving the current phase of the moon. */
#define PHASE_LIMIT MOON1
void print_moon ()
{
  double days, phase;		/* Computes the moon's phase by */
  int percent;			/* computing the difference between */
  make_J2000 ();		/* the sun and moon's */
  days = days_after_J2000 ();	/* ecliptic longitude. */
  phase = sun_position (days);
  phase = normalize_angle (moon_position (days) - phase);
  percent = 50.0 * (1.0 - cos (phase)) + 0.5; /* Visable fraction. */
  printf("The moon is ");
  if (fabs (phase) < PHASE_LIMIT)
    printf ("new");
  else if (fabs (normalize_angle (phase + PI)) < PHASE_LIMIT)
    printf ("full");
  else if (fabs (phase - PI/2.0) < PHASE_LIMIT)
    printf ("first quarter (%d%% of full)", percent);
  else if (fabs (phase + PI/2.0) < PHASE_LIMIT)
    printf ("last quarter (%d%% of full)", percent);
  else if (phase > PI/2.0)
   printf ("waxing and gibbous (%d%% of full)", percent);
  else if (phase > 0.0)
   printf ("a waxing crescent (%d%% of full)", percent);
  else if (phase > PI/-2.0)
   printf ("a waning crescent (%d%% of full)", percent);
  else
   printf ("waning and gibbous (%d%% of full)", percent);
  printf (".\n");
}

/**********************************************************/

/* lunisolar calendar routines. */

int first_day_of_year (year)	/* Returns the integer number of days */
     int year;			/* between the start of year and */
{				/* J2000.0. */
  int days;
  days = 365 * (year - 2000);
  if (year > 2000)
    for (; year > 2000; year--)
      if (leap_year (year)) days += 1;
      else;			/*  Needed else! */
  else
    for (; year < 2000; year++)
      if (leap_year (year)) days -= 1;
  return days;
}

/* Routines that find the seasons. */

#define DIGITS 15
int zero (x, f, fp)		/* Root finder using */
     int x;			/* Newton's method. */
     double (*f) ();
     double (*fp) ();
{
  int i;
  double y, midnite, noon;
  y = x;
  for (i = 0; i < DIGITS; i++)
    y = y - f(y)/fp(y);
  noon = 0.5 + (TIMEZONE / 24.0 / 60.0);
  midnite = floor (y - noon) + noon;
  if (f (midnite) * f (midnite + 1.0) <= 0.0)
    return midnite;
  x = midnite;
  printf ("%%Not sure about the season change for day %d.\n", x);
  return x;
}
  
double phase;			/* sun_zero has a root at the */
double sun_zero (days)		/* desired day.  Used with zero */
     double days;		/* to find the seasons. */
{
  return normalize_angle (sun_position (days) - phase);
}

void find_seasons (first_day, seasons)
     int first_day, *seasons;
{				/* Remember Spring is the */
  int i;			/* first season of a year. */
  phase = PI/-2.0;		/* Find start of winters. */
  seasons[0] = zero (first_day - 11, sun_zero, sun_velocity);
  seasons[4] = zero (seasons[0] + 365, sun_zero, sun_velocity);
  phase = 0.0;			/* Find start of other seasons. */
  for (i = 1; i < 4; i++, phase += PI/2.0)
    seasons[i] = zero (seasons[i-1] + 91, sun_zero, sun_velocity);
  printf ("%% Seasons relative to January 1:");
  for (i = 0; i < 5; i++)
    printf (" %d", seasons[i] - first_day);
  printf (".\n");
}

void make_moon_table (seasons, moon) /* Computes the position of */
     int *seasons;		/* the moon for each day at */
     float *moon;		/* noon local time. */
{
  int i, day;
  for (i = 0, day = seasons[0]; day < seasons[4]; i++, day++) {
    double dday = day + TIMEZONE / (24.0 * 60.0);
    moon[i] = normalize_angle (moon_position (dday) - sun_position (dday));
  }
}

/* Routines that output LaTeX commands. */

/* Dates spiral inward by an amount DELTA_RADIUS. */
#define START_RADIUS 1.0
#define DELTA_RADIUS 0.005
float radius;

int month, day, moon_index;
int days_per_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void mark_dates (year, from, to, moon) /* Makes LaTeX statements */
     int year, from, to;	/* that place the dates. */
     float *moon;
{
  radius = START_RADIUS;
  for (; from < to; moon_index++, from++) {
    printf ("\\put(%1.6f,%1.6f){\\makebox(0,0){%d/%d}}\n",
	    radius * sin (moon[moon_index]),
	    radius * cos (moon[moon_index]),
	    month, day);
    radius -= DELTA_RADIUS;
    day++;
    if (day > days_per_month[month-1])
      if (month == 2 && leap_year (year) && day == 29);
      else {
	day = 1;
	month++;
	if (month > 12) month = 1;
      }
  }
}

void header (season, year)	/* Start of each season. */
     char *season;
     int year;
{
  printf ("\\begin{figure}\n");
  printf ("\\begin{center}\n");
  printf ("\\begin{picture}(2.0,2.0)(-1.0,-1.0)\n");
  printf ("\\tiny\n");
  printf ("\\put(0,0){\\makebox(0,0){\\Huge %s %d}}\n",
	  season, year);
}

void trailer ()			/* End of each season. */
{
  printf ("\\put(-1.0,0.0){\\line(1,0){0.5}}\n");
  printf ("\\put(0.5,0.0){\\line(1,0){0.5}}\n");
  printf ("\\put(0.0,-1.0){\\line(0,1){0.5}}\n");
  printf ("\\put(0.0,-0.4){\\circle{0.1}}\n");
  printf ("\\put(0.0,-0.3){\\makebox(0,0)[b]{\\large Full Moon}}\n");
  printf ("\\put(0.0,0.5){\\line(0,1){0.5}}\n");
  printf ("\\put(0.0,0.4){\\circle*{0.1}}\n");
  printf ("\\put(0.0,0.3){\\makebox(0,0)[t]{\\large New Moon}}\n");
  printf ("\\end{picture}\n");
  printf ("\\\\ {\\Large Lunisolar Calendar}\n");
  printf ("\\\\ {\\large Dates mark the lunar phase at noon %s.}\n", TZ);
  printf ("\\end{center}\n");
  printf ("\\end{figure}\n");
}

char *season_titles[4] =
{ "Winter", "Spring", "Summer", "Fall"};

void LaTeXize_tables (year, first_day, seasons, moon)
     int year, first_day, *seasons;
     float *moon;
{
  int a_season;
  printf ("\\documentstyle{article}\n");
  printf ("\\pagestyle{empty}\n");
  printf ("\\begin{document}\n");
  printf ("\\Large\n");
  printf ("\\setlength{\\unitlength}{60mm}\n");
  month = 12;			/* December */
  day = 32 - first_day + seasons[0];
  moon_index = 0;
  for (a_season = 0; a_season < 4; a_season++) {
    header (season_titles[a_season], a_season == 0 ? year - 1 : year);
    mark_dates (year, seasons[a_season], seasons[a_season+1], moon);
    trailer ();
  }
  printf ("\\end{document}\n");
}
    
/* Lunisolar master routine. */
  
int seasons[5];			/* Stores days that mark season changes. */
float moon[370];		/* Stores moon phases for each day. */

void lunisolar (year)		/* Constructs a LaTeX file that  */
     int year;			/* generates a lunisolar calendar */
{				/* for the year year. */
  if (year < 1950 || year > 2050)
    printf ("Program useful between the years 1950 and 2050.\n");
  else {
    int day_of_Jan1 = first_day_of_year (year);
    printf ("%% Lunisolar calendar for %d.\n", year);
    printf ("%% Constructed for %s, %1.2f hours west of Greenwich.\n",
	    TZ, TIMEZONE / 60.0);
    find_seasons (day_of_Jan1, seasons);
    make_moon_table (seasons, moon);
    LaTeXize_tables (year, day_of_Jan1, seasons, moon);
  }
}

main (argc, argv)		/* Invokes print_moon with */
     int argc;			/* no arguments, and */
     char **argv;		/* lunisolar with one. */
{
  int i;
  if (argc == 1) print_moon ();
  else {
    if (argc == 2) {
      int year;
      if (sscanf (argv[1], "%d", &year) == 1) {
	lunisolar (year);
	exit (0);
      }
    }
    fprintf (stderr, "Bad args:");
    for (i = 0; i < argc; i++)
      fprintf (stderr, " %s", argv[i]);
    fprintf (stderr,
	     "\nUsage: %s\ngives the phase of the moon,\n",
	     argv[0]);
    fprintf (stderr, "and: %s <year>\n", argv[0]);
    fprintf (stderr, "generates a lunisolar calendar for LaTeX.\n");
    exit (1);
  }
}
