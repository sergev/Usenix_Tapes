/*vi:set tabstops=4 */
/*
 *                    S E T C L O C K    C O M M A N D 
 *
 *                                Version 2
 *
 *
 *
 *                setclock MMddhhmm[yy]
 *                setclock [-{lds}{x} [offset] | -{zu}[x]]
 *
 *   
 *  The first form of setclock takes a date and time to set the hardware
 *  clock in an IBM PC/AT.  If the year is not specified the current year
 *  in the hardware clock is used.  Alternatively, and more commonly,
 *  setclock takes a flag describing the kind of time stored in the PC/AT
 *  hardware clock and it converts it to something useful to set the 
 *  internal UNIX clock which runs in GMT.  The AT hardware clock returns:
 *                           0216141486
 *                           mmddhhmmyy
 *  If this is not local time then the hours need to be incremented or
 *  decremented depending on daylight savings time and an adjustment to
 *  local time if the clock is running on some other timezone.  This process
 *  asumes that the PC/AT clock runs on one time all year around.  It can be:
 *
 *          flag   Time
 *           u     GMT (universal time)
 *           z     GMT (zooloo)
 *           g     GMT
 *           l     Local Time (hardware changes when going on DST)
 *           d     Daylight Time (hardware always runs on DST)
 *           s     Standard Time (hardware always runs of ST)
 *
 *  For the l, d, and s flags there is an offset argument giving the
 *  number of hours and fractions of an hour from Greewich.  Both d
 *  and s are treated identically so the offset must be set properly
 *  to make them work. An offset should be positive for places west of
 *  Greenwich.  Therefore European offsets will be negative and American
 *  offsets positive.  The default offset is 8.0 h or PST.
 *
 *  If the x modifier is given then the program waits until the minutes change
 *  in the hardware clock before the function returns its value.  This means
 *  that the clock is set within a second or so provided the AT hardware clock
 *  has been set accurate to the second.  It also means that the function may
 *  take up to a minute to return a value.
 *
 *  Added kluge to handle new 1987 DST rules to compensate for local time
 *  not knowing the real time DST goes into effect.  Should be recompiled
 *  when a new local time shows up.  BADDST is the compile time flag.
 *
 *  Author:       Robert Uzgalis
 *  Organization: Tigertail Associates
 *                320 North Bundy Drive
 *                Los Angeles, Califonia 90049
 *                USA
 *
 *  Program Copyright (c) 1986, 1987 Tigertail Associates. All rights reserved.
 *
 */

#include <time.h>
#include <stdio.h>

#define PROC
#define then
#define Bool	int
#define TRUE	1
#define FALSE	0
#define NULLF   ((FILE *) NULL)
#define RES     250L   /* 1/4 second */
#define sec_min (long)60
#define sec_hr  (long)60*60
#define sec_day (long)60*60*24
#define sec_yr  (long)60*60*24*365

long nap();
char *strcat();
struct tm *localtime();
long time();





long PROC yrday (m, d, y)
int        m, d, y;
{  static int dtab[] =
           { /* jan, feb, mar, apr, may, jun, jul, aug, sep, oct, nov, dec );
              *  31,  28,  31,  30,  31,  30,  31,  31,  30,  31,  30,  31 */
                  0,  31,  59,  90, 120, 151, 181, 212, 243, 273, 304, 334, 365
           };
    long days;

    days = dtab[m-1];
    if (((y+1900)%4)==0 && ((m>2) || (m==2 && d>28)))
    then days += d;
    else days += (d - 1);

    return ( days );
}



PROC main (argc, argv)
int        argc;
char           **argv;

{ int        mo, day, hr, imin, min, yr, rc;
  float      offset;
#ifdef BADDST
  long       dst_beg, sys_dst_beg, aprilfools, first_sun_apr;
  struct tm *gmtime();
  Bool       dst;
#endif
  long       off, sec_sofar, yrs_sofar, ysec, nld;
  static char d[30] = " ";
  static char y[2] = " ";
  char cr;
  struct tm *tn;
  FILE      *fd;

  /*  Pickup AT/PC battery backup clock time */

  fd = fopen ("/dev/clock", "r");
  if (fd == NULLF)
  {  fprintf( stderr, "setclock: no access to /dev/clock\n" );
     exit( -1 ); }
  fscanf ( fd, "%2d%2d%2d%2d%2d", &mo, &day, &hr, &min, &yr );
  fclose (fd);



/*        SET IBM PC/AT HARDWARE CLOCK TIME       */

  if (argc >= 2 && argv[1][0] >= '0' && argv[1][0] <= '9')
  {    rc = sscanf( argv[1], "%[0-9]%c", d, &cr );
	   if (rc == 1)
	   {  if (strlen(d) == 8)
		  {   sprintf( y, "%2d", yr );
		      strcat( d, y ); }
	      if (strlen(d) == 10)
          {   fd = fopen( "/dev/clock", "w" );
              if (fd == NULLF)
              {  fprintf( stderr, "setclock: no write access to /dev/clock\n" );
                 exit( -1 ); }
			  fprintf( fd, "%s", d );
			  fclose (fd);
			  exit (0); }
		  }
	  fprintf ( stderr, "setclock: wrong length for date/time\n" );
	  fprintf ( stderr, "usage: setclock MMddhhmm[yy]\n" );
	  exit (1);
	  }

/*        GET IBM PC/AT HARDWARE TIME TO SET UNIX CLOCK     */


  /* Convert PC/AT time to GMT in seconds since 1970 */

  /* If no argument then assume that clock is running on PST */

  offset = 8.0;  /* hours from GMT; default PST */

  /*  Synchronize processing with change in minute */

  if (argc >= 2 && (argv[1][2] == 'x'))
  {  imin = min;
     while (min == imin)
     {  fd = fopen ("/dev/clock", "r");
	    if (fd == NULLF)
  	    {  fprintf( stderr, "setclock: error on /dev/clock" );
   	       exit( -1 ); }
        fscanf ( fd, "%2d%2d%2d%2d%2d", &mo, &day, &hr, &min, &yr );
        fclose (fd);
        nap ( RES );
	 }                   }

	
  if (argc >= 2 )
  switch ( argv[1][1] )
  { case 'l':   /* Local Time -- assume AT clock changes properly itself */
                   printf ( "%02d%02d%02d%02d%02d\n", mo, day, hr, min, yr );
                   exit( 0 );

    case 'd':   /* Daylight time (offset in hours next argument) */
    case 's':   /* Standard time (offset in hours next argument) */
                   if (argc >= 2) rc = sscanf ( argv[2], "%f%c", &offset, &cr);
                   break;

    default:    /* Greenwich Mean Time (GMT) any other character */
                   offset = 0.0; /* hours from Greenwich */ }


  offset     *= sec_hr;
  off         = (long) offset;

  nld         = (yr - 69) / 4;  /* num of full leap days excluding this year */

  sec_sofar   = yrday(mo,day,yr) * sec_day ;   /* do days this year      */
  sec_sofar  +=              hr  * sec_hr  ;   /* hours in today         */
  sec_sofar  +=              min * sec_min ;   /* minutes in today       */
  ysec        = sec_sofar;

  yrs_sofar   =       (yr - 70)  * sec_yr  ;   /* full years so far      */
  yrs_sofar  +=             nld  * sec_day ;   /* account for leap days  */


  /* Output Local Time corrected for Daylight Savings etc */

#ifdef BADDST

  /* This code is dedicated to the fools in the US Congress who changed when
   * daylight savings time starts.... when a new correct version of
   * localtime shows up that takes care of the 1987 rules this code should
   * go away.
   */

  sec_sofar     = ysec + yrs_sofar;              /* play date/time as gmt */

  aprilfools    = yrday(4,1,yr) * sec_day;

  tn            = gmtime( &aprilfools );
  first_sun_apr = aprilfools + ((7L - (tn->tm_wday))%7L) * sec_day;

  dst_beg       = first_sun_apr + 1L * sec_hr;  /* Make it 1am */

  sys_dst_beg   = dst_beg + ((tn->tm_wday==0)?28L:21L) * sec_day + sec_hr; 

  if ( ysec >= dst_beg && ysec < sys_dst_beg )
  then dst = TRUE;

  if (dst)
  { sec_sofar += sec_hr;            /* correct for when localtime doesn't work*/
  	tn         = gmtime( &sec_sofar );
  	printf ( "%02d%02d%02d%02d%02d\n", tn->tm_mon+1, tn->tm_mday,
                                       tn->tm_hour, tn->tm_min, tn->tm_year ); }
  else
  { sec_sofar += off;               /* correct for timezone     */
  	tn         = localtime( &sec_sofar );
  	printf ( "%02d%02d%02d%02d%02d\n", tn->tm_mon+1, tn->tm_mday,
                                       tn->tm_hour, tn->tm_min, tn->tm_year ); }

#else
  sec_sofar   = ysec + yrs_sofar + off;   /* correct for timezone     */

  tn = localtime( &sec_sofar );
  printf ( "%02d%02d%02d%02d%02d\n", tn->tm_mon+1, tn->tm_mday,
                                     tn->tm_hour, tn->tm_min, tn->tm_year );
#endif

  return( 0 );
}
