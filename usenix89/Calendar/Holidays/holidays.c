

/*********************************************************************
 *  
 *                A Christian and Holiday Calandar Routine
 *  
 *  
 * Program Copyright (c) 1987 Tigertail Associates.  All rights reserved.
 *  
 *
 *   Synopsis:
 *              holidays [flags] [ year]
 *
 *   For flags see usage printf below.
 *   If no year is specified then the current year is printed.
 *   If a year is specified then a holiday calendar for that year is printed.
 *
 *********************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include "taxc.h"

#define DAY		        (24L*3600L)
#define SIDEREAL_MO		((long) (29.5306*DAY))
#define FIRST_Q		    ((long) ((29.5306*DAY)/4L))
#define FULL   		    ((long) ((29.5306*DAY)/2L))
#define LAST_Q 		    ((long) ((29.5306*3L*DAY)/4L))
#define HALF_DAY	    (DAY/2L)
#define D(m,w,d)	    ((m)*100+(w)*10+(d))


/*********************************************************************
 *
 *      Miscellaneous GLOBAL Values:
 *
 *********************************************************************/

char *usage[] = { "usage: %s [flags] [year]",
                  "           Calendar Content (Default: -h)",
                  "      -h Holidays only printed",
                  "      -a All, one line for each day of year",
                  "      -u Print this usage statement",
                  "           Annotation Content (Default: -beLp1)",
                  "      -b Both religious and secular calendar",
                  "      -B Neither religious nor secular calendar",
                  "      -r Religious feasts",
                  "      -s Secular holidays and memoria",
                  "      -e Historical events",
                  "      -p <0-9> Priority limit on annotation",
                  "      -l Lunar phases",
                  "           Date Format (Default: -NwmdF)",
                  "      -n Day number of year",
                  "      -w Week day",
                  "      -m Month name",
                  "      -d Day of month",
                  "      -t Two digit year",
                  "      -f Four digit year",
                  "      -z todo file format",
                  "Capital letters reverse meaning of entry",
                  "If year is not specified the current year is used.",
                  NULLS };

typedef struct { int w1, w2, w3, w4; char *desc; } hday;


/* Events in History Based on Fixed Days in a Particular Year */


hday events[] =
        /*  start   end   day pri string to print */
        {  { 1939, 1939,   -1,  0, "Start of WW II" },           /*          */
		   { 1945, 1945,  132,  0, "V-E Day" },                  /* May 13th */
		   { 1915, 1915,  127,  0, "Sinking of the Lusitania" }, /* May  7th */
           { 1914, 1914,  178,  0, "Start of WW I" },            /* Jun 28th */
           { 1919, 1919,  178,  0, "Treaty of Versailles" },     /* Jun 28th */
           { 1945, 1945,  225,  0, "V-J Day" },                  /* Aug 14th */
           { 1929, 1929,  296,  0, "Wall Steet Panic" },         /* Oct 24th */
           { 1941, 1941,  325,  0, "Pearl Harbor" },             /* Nov 22nd */
           { 1968, 1968,  325,  0, "JFK Assassinated" },         /* Nov 22th */
		   {   -1,   -2,  366, 10, NULLS } };


/* Holidays in Secular Calendar Based on Fixed Days */

hday secular[] =
       /*  start   end   day pri  string to print */
		{ { 1896,    0,   32,  4, "Ground Hog Day" },            /* Feb  2th */
		  { 1896,    0,   42,  2, "Lincoln's Day (US)" },        /* Feb 12th */
		  { 1755,    0,   52,  2, "Washington's Day (US)" },     /* Feb 22th */
          {    0,    0,   90,  3, "April Fool's Day" },          /* Apr  1st */
          { 1890,    0,  103,  3, "Pan American Day" },          /* Apr 14th */
          {    0,    0,  120,  3, "May Day / Labor Day" },       /* May  1st */
          { 1853,/*?*/0, 124,  4, "Independence Day (Mexico)" }, /* May  5th */ 
          { 1819,    0,  144,  3, "National Maritime Day" },     /* May 22th */
          { 1880,    0,  146,  4, "British Empire Day" },        /* May 24th */
          { 1866, 1940,  149,  3, "Decoration Day (US)" },       /* May 30th */
          { 1941, 1976,  149,  3, "Memorial Day (US)" },         /* May 30th */
          { 1789,    0,  164,  3, "Flag Day (US)" },             /* Jun 14th */
          {    0,    0,  181,  4, "Canada Day" },                /* July 1th */
          { 1776,    0,  184,  1, "Independence Day (US)" },     /* Jul  4th */
          { 1778,    0,  194,  4, "Bastille Day" },              /* Jul 14th */
          { 1850,    0,  246,  5, "Los Angeles Day" },           /* Sep  4th */
          { 1854,    0,  251,  4, "California Admission Day" },  /* Sep  9th */
          { 1779, 1951,  256,  3, "Constitution Day (US)" },     /* Sep 14th */
          { 1952,    0,  259,  3, "Citizenship Day" },           /* Sep 17th */
          { 1940,    0,  271,  9, "Buz' Day" },                  /* Sep 29th */
          { 1493, 1978,  284,  2, "Columbus Day (US)" },         /* Oct 12th */
          { 1945,    0,  296,  2, "United Nations Day" },        /* Oct 24th */
          { 1920,    0,  314,  2, "Veteran's Day" },             /* Nov 11th */
          { 1803,    0,  308,  3, "Guy Fawkes' Day" },           /* Nov  5th */
          { 1941,    0,  348,  5, "Bill of Rights Day" },        /* Dec 15th */
          { 1903,    0,  350,  5, "Wright Bros. Day" },          /* Dec 17th */
          {    0,    0,  359,  5, "Boxing Day" },                /* Dec 26th */
		  {   -1,   -2,  366, 10, NULLS } };



/* Holidays in Christian Calendar Based on Fixed Days */

hday  reldays[] =
       /*  start   end   day pri  string to print */
		{ {    0,    0,    5,  3, "Epiphany" },                  /* Jan  6th */
          {    0,    0,   32,  3, "Candelmas" },                 /* Feb  2nd */
          {    0,    0,   83,  4, "Mary's Annunciation" },       /* Mar 25th */
          {    0,    0,  179,  4, "Saint Peter's Day" },         /* Jun 29th */
          {    0,    0,  180,  4, "Saint Paul's Day" },          /* Jun 30th */
          {    0,    0,  217,  3, "Feast of the Transfiguration" },/* Aug 6th */
          {    0,    0,  226,  4, "Mary's Assumption" },         /* Aug 15th */
          {    0,    0,  250,  4, "Mary's Nativity" },           /* Sep  8th */
          {    0,    0,  256,  5, "Exaltation of the Cross" },   /* Sep 14th */
          {    0,    0,  271,  4, "Michaelmas" },                /* Sep 29th */
          {    0,    0,  304,  4, "All Martyr's Day" },          /* Nov  1st */
          {    0,    0,  330,  2, "(Advent Begins)" },           /* Nov 27th */
		  {   -1,   -2,  366, 10, NULLS } };

/* Holidays in Christian Calendar based on Easter Sunday */

hday eastdays[] =
       /*  start   end   day pri  string to print */
        { {    0,    0,  -56,  2, "Septuagesima" }, /* 70 days before Easter? */
          {    0,    0,  -49,  2, "Sexagesima" },   /* 60 days before Easter? */
		  {    0,    0,  -50,  3, "(Carnival Begins)" },
          {    0,    0,  -42,  2, "Quinquagesima" },/* 50 days before Easter? */
          {    0,    0,  -40,  3, "Shrove Tuesday (Carnival Ends)" },
  /*      {    0,    0,  -40,  3, "Mardi Gras Day (Carnival Ends)" }, /* Alt  */
          {    0,    0,  -39,  1, "Ash Wednesday" },/* 40 days before Easter  */
          {    0,    0,   -7,  0, "Palm Sunday" },  /* 10 days before Easter? */
          {    0,    0,   -3,  2, "Maundy Thursday" },
          {    0,    0,   -2,  1, "Good Friday" },
  /*      {    0,    0,    0,  0, "Easter Sunday" },/* Handled directly in pgm*/
                                                    /* 40 days after Easter   */
          {    0,    0,   39,  3, "Holy Thursday (Feast of the Ascension)" },
                                                    /* 50 days after Easter   */
          {    0,    0,   49,  0, "Pentecost (Whitsunday)" },
                                                    /* 60 days after Easter?  */
          {    0,    0,   56,  1, "Trinity Sunday" },
		  {   -1,   -2,  366, 10, NULLS } };

/* Holidays in Both Christian and Secular Calendars Based on Fixed Days */

hday bothdays[] =
       /*  start   end   day pri  string to print */
		{ {    0,    0,    0,  0, "New Year's Day" },            /* Jan  1st */ 
    /*    { 1983,   -1,   14,  2, "Martin Luther King Jr Day" }, /* Jan 15th */
          { 1974, 1974,    5,  0, "Daylight Savings Begins" },   /* Jan  6th */
          { 1942, 1942,   39,  0, "Daylight Wartime Begins" },   /* Feb  9th */
          {    0,    0,   44,  1, "Saint Valentines' Day" },     /* Feb 14th */
          { 1975, 1975,   53,  0, "Daylight Savings Begins" },   /* Feb ??th */
          {    0,    0,   73,  3, "Ides of March (Beware!!)" },  /* Mar 15th */
          {    0,    0,   75,  3, "Saint Patrick's Day" },       /* Mar 17th */
          {    0,    0,   79,  3, "Vernal Equinox" },            /* Mar 21st */
    /*    { 1914,    0,  104,  0, "Ceasar's Rendering Day" },    /* Apr 15th */
          {    0,    0,  171,  3, "Summer Solistice" },          /* Jun 21st */
          {    0,    0,  246,  5, "Kid's Day" },                 /* Sep  4th */
          {    0,    0,  263,  3, "Autumnal Equinox" },          /* Sep 21st */
          { 1942, 1942,  271,  0, "Standard Time Begins" },      /* Sep 30th */
          { 1918, 1919,  277,  0, "Standard Time Begins" },      /* Oct  5th */
          {    0,    0,  303,  0, "Halloween"},                  /* Oct 31th */
          {    0,    0,  354,  3, "Winter Solistice" },          /* Dec 21st */
          {    0,    0,  357,  0, "Christmas Eve" },             /* Dec 24th */ 
          {    0,    0,  358,  0, "Christmas" },                 /* Dec 25th */
          {    0,    0,  364,  0, "New Year's Eve" },            /* Dec 31st */
		  {   -1,   -2,  366, 10, NULLS } };


/* Holidays in Both Christian and Secular Calendars Based on Weekday */

hday daydays[] =
              /*    month  * 100 */
              /*  + wkinx  * 10  */
  /*  start   end + wday pri  string to print */
   { { 1967,    0,   10,   1, "Superbowl Sunday" },         /* 2nd Sun Jan */
     { 1983,    0,   21,   1, "Martin Luther King Jr Day" },/* 3rd Mon Jan */
  /* { 1960,    0,  250,   0, "Daylight Savings (Europe)" },/* LastSun Mar */
     { 1918, 1919,  250,   0, "Daylight Wartime" },         /* LastSun Mar */
     { 1967, 1973,  350,   0, "Daylight Savings" },         /* LastSun Apr */
     { 1976, 1986,  350,   0, "Daylight Savings" },         /* LastSun Apr */
                                                            /* 1st Sun Apr */
     { 1987,    0,  300,   0, "Daylight Savings (Spring Forward)" },
     { 1911,    0,  410,   0, "Mother's Day" },             /* 2nd Sun May */
     {    0,    0,  520,   0, "Father's Day" },             /* 3rd Sun Jun */
     { 1882,    0,  801,   0, "Labor Day" },                /* 1st Mon Sep */
  /* { 1960,    0,  850,   0, "Standard Time (Europe)" },   /* LastSun Sep */
     { 1967,    0,  950,   0, "Standard Time (Fall Back)" },/* LastSun Oct */
     {    0,    0, 1033,   0, "Thanksgiving Day (US)" },    /* 4th Thu Nov */
     {   -1,   -2, 1268,  10, NULLS } };




/* Holidays in Secular Calendar Based on Weekday */

hday dscdays[] =
              /*    month  * 100 */
              /*  + wkinx  * 10  */
  /*  start   end + wday pri  string to print */
   { { 1975,    0,  131,   1, "President's Day" },          /* 3rd Mon Feb */
     { 1950,    0,  426,   3, "Armed Forces' Day (US)" },   /* 3rd Sat May */
     { 1940, 1951,  420,   3, "I'm an American Day" },      /* 3rd Sun May */
     { 1977,    0,  451,   3, "Memorial Day" },             /* LastMon May */
     { 1865,    0,  510,   3, "Children's Day" },           /* 2nd Sun Jun */
     { 1979,    0,  810,   0, "Grandparent's Day" },        /* 2nd Sun Sep */
     { 1979,    0,  901,   2, "Child Health Day" },         /* 1st Mon Oct */
     { 1979,    0,  911,   2, "Columbus Day (Obs-US)" },    /* 2nd Mon Oct */
     { 1863,    0,  911,   4, "Thanksgiving Day (Canada)" },/* 2nd Mon Oct */
     {   -1,   -2, 1268,  10, NULLS } };

















char *mon_name[] ={" Jan", " Feb", " Mar", " Apr", " May", " Jun",
                   " Jul", " Aug", " Sep", " Oct", " Nov", " Dec" };
int   mon_days[] ={  31,     28,     31,     30,     31,     30,
                     31,     31,     30,     31,     30,     31 };
char *day_name[] ={" Sun", " Mon", " Tue", " Wed", " Thu", " Fri", " Sat"};


char	*myname;			/* How program was invoked	*/

int  yrday();
int  weeknum();
int  daynum();
long uxeaster();
long uxdate();
long uxtime();
void datex();


/************************************************************************
 *
 *     Holidays -- Print a holiday calendar for a given year.
 *
 ************************************************************************/

int PROC main (argc, argv)
	int	argc;
	char	**argv;

{   int y;
	long time();
	char j;
	struct tm *localtime(), *gmtime(), *tp;
	int ds, ids, syd, ly, i, nday, wi, di, cy, cm, cyd, cmd, cwd;
	int ep, sp, bp, dp, rp, cp;     /* pointers into the annotation arrays */
	int e_month;
	long daysold, lbase, now, nd, cd;
#ifdef EBUG
	int  nfd;
	long ncd;
#endif
	Bool lwdm = FALSE;
	long e_s;
	int e_sun;
	char dateline[80], info[80];
	int mnum, mnum2;                      /* Magic numbers for checking days */
	int pri = 1;                          /* Priority of Holiday (important) */
	Bool sechf    = TRUE;				  /* Print secular holidays */
	Bool relhf    = TRUE;				  /* Print religious holidays */
	Bool eventf   = TRUE;				  /* Print history events */
	Bool holf;				              /* True if any holidays printed */
	Bool alldaysf = FALSE;                /* Print every day in year */
	Bool daynumf  = FALSE;                /* Print day number of day in year */
	Bool wdayf    = TRUE;                 /* Print week day of date */
	Bool monthf   = TRUE;                 /* Print month of date */
	Bool mdayf    = TRUE;                 /* Print month day of date */
	Bool syrf     = FALSE;                /* Print year in 2 digit format */
	Bool lyrf     = FALSE;                /* Print year in 4 digit format */
	Bool moif     = FALSE;                /* Print week day index in month */
	Bool nyrf;							  /* True if no year to be printed */
	Bool lunarf   = FALSE;                /* Print moon in quarter phases */
	Bool seasonf  = FALSE;                /* Print start of seasons */
	Bool todof    = FALSE;                /* Print todo file format */

    extern	int	     optind;	          /* from getopt()	*/
    extern	char	*optarg;	          /* from getopt()	*/
    int	     option;    				  /* option "letter"	*/
	char *type;							  /* Religious or Holiday */


/*********************************************************************
 *
 *   Process Arguments
 *
 *********************************************************************/

	myname = *argv;

	optind = 1;
	while ((option = getopt (argc, argv, "ABCDEFHILMNRSTWZabcdefhilmnp:rstwz")) != EOF)
	{   switch (option)
	    {   case 'a':	alldaysf = TRUE;	break; /* All days printed */
	        case 'A':	alldaysf = FALSE;	break; /* Only Holidays printed*/
	        case 'b':	relhf    = TRUE;	       /* both relig & secular Hol*/
						eventf   = TRUE;
	                 	sechf    = TRUE;    break;
	        case 'B':	relhf    = FALSE;	       /* neither rel | secu Hol*/
						eventf   = FALSE;
	                 	sechf    = FALSE;   break;
	        case 'd':	mdayf    = FALSE;	break; /* Day of month */
	        case 'D':	mdayf    = FALSE;	break; /* Day of month deleted */
	        case 'e':	eventf   = TRUE;	break; /* Historical events prt */
	        case 'E':	eventf   = FALSE;	break; /* Historical events del */
	        case 'f':	lyrf     = TRUE;	break; /* Four digit year printed */
	        case 'F':	lyrf     = FALSE;	       /* No year printed */
	                 	syrf     = FALSE;   break;
	        case 'h':	alldaysf = FALSE;	break; /* Only holidays printed */
	        case 'H':	alldaysf = TRUE;	break; /* All days Printed */
	        case 'i':	moif     = TRUE;	break; /* Week and Day indexes */
	        case 'I':	moif     = FALSE;	break; /* No Week and Day indexes */
	        case 'l':	lunarf   = TRUE;	break; /* Phases of the moon */
	        case 'L':	lunarf   = FALSE;	break; /* No Phases of the moon */
	        case 'm':	monthf   = TRUE;	break; /* Month name printed */
	        case 'M':	monthf   = FALSE;	break; /* Month name deleted */
	        case 'n':	daynumf  = TRUE;	break; /* Day of year printed */
	        case 'N':	daynumf  = FALSE;	break; /* Day of year deleted */
	        case 'p':	pri      = atoi(optarg);	break; /* Prio of Holiday */
			case 'c':							   /* Christian festivals prt */
	        case 'r':	relhf    = TRUE;    break; /* Religious festivals prt */
			case 'C':							   /* Christian festivals del */
	        case 'R':	relhf    = FALSE;   break; /* Religious festivals del */
	        case 's':	sechf    = TRUE;    break; /* Secular holidays printed*/
	        case 'S':	sechf    = FALSE;   break; /* Secular holidays deleted*/
	        case 't':	syrf     = TRUE;	break; /* Two digit years */
	        case 'T':	lyrf     = FALSE;	       /* No year printed */
	                 	syrf     = FALSE;   break;
	        case 'w':	wdayf    = TRUE;	break; /* Week day printed */
	        case 'W':	wdayf    = FALSE;	break; /* Week day deleted */
	        case 'z':	todof    = TRUE;	break; /* Date in to do format */
	        case 'Z':	todof    = FALSE;	break; /* Date in to do format */
	        default:	Usage(); }				   /* Usage message */
	}

	argc -= optind;			/* skip options	*/
	argv += optind;

	now = time( (long *)0 );
	tp  = localtime( &now );
	y   = tp->tm_year + 1900;

	if ( argc == 1 )
	then sscanf ( argv[0], " %d%c", &y, &j );

	if ( y < 1913 || y > 2037 )
	then
	{    printf ( "Unix calendar out of range.  Calendar undefined.\n" );
		 return ( 0 ); }



/*********************************************************************
 *
 *   Initial Setup
 *
 *********************************************************************/

	cd  = uxdate   ( y, 1, 1);
	e_s = uxeaster ( y );  /* Get what day easter is */

	if ( lunarf )
	then
	{   lbase  = uxdate( 1979, 2, 26 ); /* Date and time of new moon */
	    lbase += uxtime( 16, 0, 0 ); }

	if (relhf && sechf)
	then type = "Christian and Holiday";
	else if ( relhf)
	then type = "Christian's";
	else type = "Holiday";
	e_sun = (e_s - cd)       / DAY;  /* Get what day easter is */
	printf ("\nA %s Calendar for Year %d\n\n", type, y);

	ly = leap(y);
	if ( ly )
	then nd = 366;
	else nd = 365;

	if (relhf || sechf || lunarf || eventf )
	then holf = TRUE;
	else holf = FALSE;

	if ( syrf || lyrf )
	then nyrf = FALSE;
	else nyrf = TRUE;



/*********************************************************************
 *
 *   Loop Through Days in Year
 *
 *********************************************************************/

	ep = sp = bp = dp = rp = cp = 0;

	for (nday=0; nday<nd; nday++)
	{   /* Get current date info out of datelib */

		datex( cd, &cy, &cm, &cyd, &cmd, &cwd );

#ifdef EBUG
	    /* Check year */
		if ( cy != y )
		then
		{    printf ("year check failed: y = %d, cy = %d\n", y, cy);
		     printf ("     cm = %d, cyd = %d, cmd = %d, cwd =%d\n",
			                    cm,      cyd,      cmd,     cwd); }

	    /* Reverse Engineer the Unix date */
		ncd = uxdate( cy, cm+1, cmd+1 );
		if ( cd != ncd )
		then printf ("uxdate check failed: cd = %ld, ncd = %ld\n", cd, ncd);

	    /* Reverse Engineer the Number of Full Days Sofar this Year */
		nfd = yrday( cy, cm+1, cmd+1 );
		if ( cyd != nfd || cyd != nday)
		then printf ("yrday check failed: nday = %d, cyd = %d, nfd = %d\n",
		                                       nday,      cyd,       nfd);
#endif

	    wi = weeknum( cd );
		di = daynum ( cd );


/*********************************************************************
 *
 *   Format Date
 *
 *********************************************************************/

		dateline[0] = NULLC;
		if ( todof )
		then /* >Sat Jun  6 87     +! */
			sprintf ( dateline, ">%s%s %2d %2d     +!",
			          day_name[cwd]+1, mon_name[cm], cmd+1, cy-1900 );
		else
		{   if ( daynumf )
	    	then (sprintf( info, "%3d", nday+1 ),    strcat ( dateline, info ));
			if ( wdayf )
	    	then ( strcat ( dateline, day_name[cwd] ));
			if ( monthf )
	    	then (strcat ( dateline, mon_name[cm] ));
			if ( mdayf )
	    	then (sprintf( info, " %2d", cmd+1 ),    strcat ( dateline, info ));
			if ( syrf )
	    	then (sprintf( info, " %2d ", cy-1900 ), strcat ( dateline, info ));
			if ( lyrf )
	    	then (sprintf( info, ", %4d ", cy ),     strcat ( dateline, info ));
			if ( nyrf )
	    	then  (strcat ( dateline, " " ));
			if ( moif )
	    	then (sprintf( info, "[%d-%d] ", wi,di), strcat ( dateline, info ));
            }

	    if ( !lunarf ) strcat ( dateline, "  " ); 

		if ( cyd == 0 ) /* ds is a count characters in the date string */
		then ids = strlen( dateline ) + (lunarf ? 5 : 0);

		ds = ids;

		syd = (ly&&cyd>60) ? cyd-1 : cyd; /* Create a standard date */

		e_month = (cm == 1 && ly) ? mon_days[cm]+1 : mon_days[cm];

		lwdm = ((cmd+9)>e_month) ? TRUE : FALSE; /* 7 + 1 for cmd + 1 to pass */

		/* Todays magic numbers: -1 is not equal to anything else */
		mnum  = cm*100+di*10+cwd;               /* any other weekday */
		mnum2 = ( lwdm ? (cm*100+50+cwd) : -1); /* last weekday of mo */



/*********************************************************************
 *
 *   Annotate Date with Holidays
 *
 *********************************************************************/

		if ( holf )
		then /* Mark Holidays */

		/* ANNOTATE WITH LUNAR AND SEASONAL EVENTS */

		{    if ( lunarf )
			 then
	    	 {	 if ( lbase < cd )
			 	 then daysold =  (cd - lbase) % SIDEREAL_MO;
			 	 else daysold =  SIDEREAL_MO - ((lbase - cd) % SIDEREAL_MO);

			 	 /**/ if ( (daysold > (FIRST_Q - HALF_DAY)) &&
				           (daysold < (FIRST_Q + HALF_DAY)) )
			 	 then strcat( dateline, " |)  ");

			 	 else if ( (daysold > FULL - HALF_DAY) &&
				           (daysold < FULL + HALF_DAY) )
			 	 then strcat( dateline, " ()  ");

			 	 else if ( (daysold > (LAST_Q - HALF_DAY)) &&
				           (daysold < (LAST_Q + HALF_DAY)) )
			 	 then strcat( dateline, " (|  ");

			 	 else if ( (daysold > SIDEREAL_MO-HALF_DAY) ||
				           (daysold < HALF_DAY) )
			 	 then strcat( dateline, " []  ");
				 else strcat( dateline, "     "); }


		/* ANNOTATE WITH GENERAL INFORMATION */

		    if ( eventf )  /* Mark Events */
			then
			{
				for ( ; events[ep].w1 >= 0; ep++ )
				{   if ( events[ep].w3 > syd )
					then goto secstuff;
				    else if ( ( events[ep].w1 <= cy  &&
					            events[ep].w2 >= cy  &&
						        events[ep].w3 == syd  ) ||
				              ( events[ep].w1 <= cy  &&
					            events[ep].w2 == 0   &&
						        events[ep].w3 == syd  ) ||
				              ( events[ep].w1 == 0   &&
						        events[ep].w3 == syd  ) )
					     then if (events[ep].w4 <= pri)
					          then
						      {    strcat( dateline, events[ep].desc );
			                       strcat( dateline, "  " );
							       goto secstuff; }
					}

				}


		/* ANNOTATE WITH SECULAR INFORMATION */

			secstuff:
		    if ( sechf )  /* Mark Secular Holidays */
			then
			{
				for ( ; secular[sp].w1 >= 0; sp++ )
				{   if ( secular[sp].w3 > syd )
					then goto wdays;
				    else if ( ( secular[sp].w1 == 0   &&
						        secular[sp].w3 == syd  ) ||
				              ( secular[sp].w1 <= cy  &&
					            secular[sp].w2 == 0   &&
						        secular[sp].w3 == syd  ) ||
				              ( secular[sp].w1 <= cy  &&
					            secular[sp].w2 >= cy  &&
						        secular[sp].w3 == syd  ) )
					     then if (secular[sp].w4 <= pri)
					          then
						      {    strcat( dateline, secular[sp].desc );
			                       strcat( dateline, "  " );
							       goto wdays; }
				}

				wdays:

				/* Check for Holidays Days based on Weekday */

				for ( dp=0; dscdays[dp].w1 != -1; dp++ )
				if ( ( dscdays[dp].w1 == 0    &&  /* no date restrict */
			          (dscdays[dp].w3 == mnum ||
				       dscdays[dp].w3 == mnum2 )) ||
		             ( dscdays[dp].w1 <= cy   &&  /* no ending date   */
			           dscdays[dp].w2 == 0    &&
				      (dscdays[dp].w3 == mnum ||
				       dscdays[dp].w3 == mnum2 )) ||
		             ( dscdays[dp].w1 <= cy   &&  /* both yrs present */
			           dscdays[dp].w2 >= cy   &&
				      (dscdays[dp].w3 == mnum || 
				       dscdays[dp].w3 == mnum2 )))
			    then if (dscdays[dp].w4 <= pri)
			         then
				     {    strcat( dateline, dscdays[dp].desc );
	                      strcat( dateline, "  " ); }

			/* Special Cases: Victoria Day, Easter Monday, Sadie Hawkin's Day */

		    if ( cm == 4 && cwd == 1 && cmd<25 && cmd>18 ) /* Victoria Day */
		    then if ( 4 <= pri )
			     then strcat( dateline, "Victoria Day (Canada)  " );

		    if ( cyd == e_sun+1 ) /* Easter Monday */
		    then if ( 4 <= pri )
			     then strcat( dateline, "Easter Monday (Canada)  " );

		    if ( cm == 10 && cwd == 0 && cmd>11 && cmd<18 )
		    then if ( 4 <= pri )
				 then strcat( dateline, "Sadie Hawkin's Day  " );

			}


		/* ANNOTATE WITH IMPORTANT HOLIDAYS TO BOTH CALENDARS */

		    if ( sechf || relhf )  /* Mark Important Holidays */
			then
			{
				/* Check for Fixed Secular Holidays in both Calendars */

				for ( ; bothdays[bp].w1 >= 0; bp++ )
				{   if ( bothdays[bp].w3 > syd )
					then goto bigdays;
				    else if ( ( bothdays[bp].w1 == 0   &&    /* no dates */
						        bothdays[bp].w3 == syd  ) ||
				              ( bothdays[bp].w1 <= cy  &&    /* no end date */
					            bothdays[bp].w2 == 0   &&
						        bothdays[bp].w3 == syd  ) ||
				              ( bothdays[bp].w1 <= cy  &&    /* date limits */
					            bothdays[bp].w2 >= cy  &&
						        bothdays[bp].w3 == syd  ) )
					then if (bothdays[bp].w4 <= pri)
					     then
						 {    strcat( dateline, bothdays[bp].desc );
			                  strcat( dateline, "  " );
							  goto bigdays; }
					}


				bigdays:

				/* Check for Holidays Days based on Weekday */

				for ( dp=0; daydays[dp].w1 != -1; dp++ )
				if ( ( daydays[dp].w1 == 0    &&  /* no date restrict */
			          (daydays[dp].w3 == mnum ||
				       daydays[dp].w3 == mnum2 )) ||
		             ( daydays[dp].w1 <= cy   &&  /* no ending date   */
			           daydays[dp].w2 == 0    &&
				      (daydays[dp].w3 == mnum ||
				       daydays[dp].w3 == mnum2 )) ||
		             ( daydays[dp].w1 <= cy   &&  /* both yrs present */
			           daydays[dp].w2 >= cy   &&
				      (daydays[dp].w3 == mnum || 
				       daydays[dp].w3 == mnum2 )))
			    then if (daydays[dp].w4 <= pri)
			         then
				     {    strcat( dateline, daydays[dp].desc );
	                      strcat( dateline, "  " ); }

			/* Special_cases: Election Day and Easter */

								   /* First Tue in Nov excep if Tue the 1st */
		    /**/ if ( cm == 10 && cwd == 2 && di == 0 && cmd != 0)
		    then strcat( dateline, "Election Day (US)  " );

		    else if ( cm == 10 && cwd == 2 && cmd == 7) /* else Nov 8th */
		    then strcat( dateline, "Election Day (US)  " );

		    if ( cyd == e_sun ) /* Easter Sunday */
			then strcat( dateline, "Easter Sunday  " );

		    if ( syd == 104 ) /* April 15th */
		    then if ( relhf )
				 then strcat( dateline, "Ceasar's Rendering Day  " );
				 else strcat( dateline, "IRS' Day (US)  " );

			} /* End of Secular Holidays Important to Both Calendars */


		/* ANNOTATE WITH CHRISTIAN HOLIDAYS */

			if ( relhf )
			then
			{   for ( ; reldays[rp].w1 >= 0; rp++ )
				{   if ( reldays[rp].w3 > syd )
					then goto east;
				    else if ( ( reldays[rp].w1 == 0   &&
						        reldays[rp].w3 == syd  ) ||
				              ( reldays[rp].w1 <= cy  &&
					            reldays[rp].w2 == 0   &&
						        reldays[rp].w3 == syd  ) ||
				              ( reldays[rp].w1 <= cy  &&
					            reldays[rp].w2 >= cy  &&
						        reldays[rp].w3 == syd  ) )
					     then if (reldays[rp].w4 <= pri)
					          then
						      {    strcat( dateline, reldays[rp].desc );
			                       strcat( dateline, "  " );
							       goto east; }
					}

		/* ANNOTATE WITH FLOATING CHRISTIAN HOLIDAYS */

				east:

				/* Check for Feast days based on Easter */

				for ( ; eastdays[cp].w1 >= 0; cp++ )
				{   if ( eastdays[cp].w3 > (cyd - e_sun) )
					then goto printit;
				    else if ( ( eastdays[cp].w1 == 0   &&
						        eastdays[cp].w3 == (cyd - e_sun)  ) ||
				              ( eastdays[cp].w1 <= cy  &&
					            eastdays[cp].w2 == 0   &&
						        eastdays[cp].w3 == (cyd - e_sun)  ) ||
				              ( eastdays[cp].w1 <= cy  &&
					            eastdays[cp].w2 >= cy  &&
						        eastdays[cp].w3 == (cyd - e_sun)  ) )
					     then if (eastdays[cp].w4 <= pri)
					          then
						      {    strcat( dateline, eastdays[cp].desc );
							       goto printit; }
					}

			} /* End of Christian Festivals */

			} /* End of Holidays */





/*********************************************************************
 *
 *     Print Date and Annotations if Interesting
 *
 *********************************************************************/

		printit:

		/* Trim blanks off the end of the line */

		for ( i=strlen(dateline)-1; ((dateline[i]==' ')&&(i>0)); i-- );
		if (i > 0) then dateline[i+1] = NULLC;

	    if ( alldaysf                ||           /* all days to be printed */
		    (strlen(dateline) > ids) ||           /* Holiday info on line */
			(lunarf && dateline[ids-3] != ' ') )  /* A lunar event */
		then printf( "%s\n", dateline );



	    /* Advance yet another day */
		cd += DAY;
		}


/*********************************************************************
 *
 *     Finish up:
 *
 *********************************************************************/

	return (0);

} /* test of datelib */






/************************************************************************
 *
 *  USAGE
 *
 * Print usage messages (char *usage[]) to stderr and exit nonzero.
 * Each message is followed by a newline.
 *
 ************************************************************************/

PROC Usage()
{
    register int	which = 0;		/* current line */

	while (usage [which] != NULLS)
	{   fprintf (stderr, usage [which++], myname);
	    putc ('\n', stderr);
	}

	exit (1);

} /* Usage */
