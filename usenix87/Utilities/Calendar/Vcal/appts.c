/*
 * Module:	appts.c
 *
 * Purpose:	Displays current or specified date's appointments
 *
 * Author:	Mike Essex
 *
 * Date:	Sep. 16, 1986
 *
 * Includes:
 *	time.h, stdio.h, ctype.h
 *
 * Discussion:
 *	This program reads a designated data file or, by default,
 *	the .appointments file in the user's home directory and
 *	displays the current days appointments if no command line
 *	arguement is given.  An optional numeric month, day and
 *	year arguement may be specified to display appointments
 *	for a different day.  Output is standard out.
 *
 * Edit History
 * ==== =======
 *
 * Date      Who	What
 * ----      ---	----------------------------------------------
 * 11/26/86  me		added multi-datafile capability
 * 11/26/86  me		fixed 'home' bug
 * 12/08/86  me		modified "No appointments" to print filenames
 *
 */

#include <time.h>
#include <stdio.h>
#include <ctype.h>

int		monthdata[1000];
int		daydata[1000];
int		yeardata[1000];
int		timedata[1000];
long		juliedata[1000];
char		*msgdata[1000];
int		month,day,year,hour,mhour,min,sec,dow,ampm;
int		argvindex;
long		julie;
int		maxentries = 1000;
int		cntr;
char	*dlist[8] = {"x","Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
int	mdays[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};



/*
 * Procedure:	main()
 *
 * Function:	initializes values, gets current day, parses command
 *		line arguements, loads file data, outputs table
 */

main(argc,argv)

int		argc;
char		*argv[];

{
    int		go,none,cntr,cnt,timeout,status,nowtime;
    int		loaddata();
    int		wday;
    int		atoi();
    int		setdow();
    int		tempmonth,tempday,tempyear;
    int		maxindex;
    int		holdargvi,holdmaxi;
    long	setjulie();

    timeset();

    if (argc > 3) {
	tempmonth = atoi(argv[1]);
	tempday = atoi(argv[2]);
	tempyear = atoi(argv[3]);
	if (tempmonth && tempday && tempyear){
	    month = tempmonth;
	    day = tempday;
	    year = tempyear;
	    if (year < 100) year += 1900;
	    if (argc > 4) {
		argvindex = 4;
		maxindex = argc;
	    }
	    else {
		argvindex = 0;
		maxindex = 1;
	    }
	}
	else {
	    if (tempmonth || tempday || tempyear) {
		printf("Syntax error:  Incorrect date arguments\n");
		exit(1);
	    }
	    else {
		argvindex = 1;
		maxindex = argc;
	    }
	}
    }
    else {
        if (argc == 1) {
	    argvindex = 0;
	    maxindex = argc;
	}
	else {
	    argvindex = 1;
	    maxindex = argc;
	}
    }

    julie = setjulie(month,day,year);
    wday = setdow(month,day,year);


    go = 1;
    holdargvi = argvindex;
    holdmaxi = maxindex;
    while (go) {
	printf("-------------------------------------------------------------------------------\n");
	printf("|                       Appointments for %-3.3s  %2d/%2d/%4d                      |\n",dlist[wday],month,day,year);


	while (argvindex < maxindex) {
	    cnt = loaddata(argv[argvindex]);
	    cntr = 0;
	    none = 1;
	    while (cntr < cnt) {
		if (juliedata[cntr] == julie) {
		    printf("| %4d  %-69.69s |\n",timedata[cntr],msgdata[cntr]);
		    none = 0;
		};
		cntr++;
	    }
	    if (none) {
		if (argvindex == 0) {
		    printf("| No appointments today.                                                      |\n");
		}
		else {
		    printf("| No appointments in:  %-40.40s               |\n",argv[argvindex]);
		}
	    }
	    argvindex++;
	}
	printf("-------------------------------------------------------------------------------\n");
	if ((wday == 5) || (wday == 6)) {
	    wday++;
	    julie++;
	    date(julie);
	    go = 1;
	}
	else {
	    go = 0;
	}
    argvindex = holdargvi;
    maxindex = holdmaxi;
    }
    exit(0);
} /* main */


/*
 * Procedure:	timeset()
 *
 * Function:	Gets current time and date
 *
 * Return Values:
 *	loads time and date info into global variables
 *
 */

timeset()

{
    struct	tm *localtime();

    struct tm *tp;		/* time structure */
    long	tloc;		/* number of seconds since 1970 */

    time(&tloc);	/* fills tloc */

    tp = localtime(&tloc);

    dow =	tp->tm_wday;
    year =	tp->tm_year;
    month =	tp->tm_mon + 1;
    day =	tp->tm_mday;
    hour = tp->tm_hour;
    mhour = tp->tm_hour;
    min = tp->tm_min;
    sec = tp->tm_sec;

    year += 1900;

    if (sec >= 30)
    {
	min += 1;	/* round up minutes */
	if (min == 60)
	{
	mhour += 1;
	min = 0;
	}
    }

    if (hour > 12)
    {
	hour -= 12;
    }
    if (hour >= 12)
    {
	ampm = 0;
    }
    else {
	ampm = 1;
    }
} /* timeset */


/*
 * Procedure:	loaddata()
 *
 * Function:	loads appointment data from ~/.appointments file
 *
 * Return Values:
 *	various global arrays loaded with appointment data
 *
 */

loaddata(filename)

char	*filename;
{
    char	basedata[80];
    char	tmpbuf[80];
    char	*getenv();
    FILE 	*fptr;
    char	home[80];
    int		i,j,k,l,field;
    long	setjulie();
    char	*malloc();

    i = 0;
    while (i < maxentries) {
	daydata[i] = 0;
	monthdata[i] = 0;
	yeardata[i] = 0;
	msgdata[i] = 0;
	i++;
    }

    if (argvindex == 0) {
	strcpy(home,getenv("HOME"));
	strcat(home,"/.appointments");
    }
    else {
	strcpy(home,filename);
    }

    if ((fptr = fopen(home,"r")) != NULL) {
	i = 0;
	while((fgets(basedata,80,fptr) != NULL)) {

	    basedata[strlen(basedata)-1] = NULL;

	    j = 0;
	    k = 0;
	    field = 0;
	    while (basedata[j] != NULL ) {
                 
                if (basedata[j] != ',') {

		    tmpbuf[k++] = basedata[j];
		}
		else {
		    switch (field) {

			case 0 : {
			    tmpbuf[k] = NULL;
			    monthdata[i] = atoi(tmpbuf);
			    k = 0;
			    break;
			}
			case 1 : {
			    tmpbuf[k] = NULL;
			    daydata[i] = atoi(tmpbuf);
			    k = 0;
			    break;
			}
			case 2 : {
			    tmpbuf[k] = NULL;
			    yeardata[i] = atoi(tmpbuf);
			    k = 0;
			    break;
			}
			case 3 : {
			    tmpbuf[k] = NULL;
			    timedata[i] = atoi(tmpbuf);
			    k = 0;
			    break;
			}
		    }
		    field++;
		}
		j++;
	    }
	    tmpbuf[k] = NULL;
	    msgdata[i] = malloc(40);
	    strcpy(msgdata[i],tmpbuf);
	    juliedata[i] = setjulie(monthdata[i],daydata[i],yeardata[i]);

	    if (i >= maxentries) {
		printf("Warning:  Over 1000 entries in %s file.  Data truncated.\n",filename);
		break;
	    }
	    i++;
	}
	fclose(fptr);
    }
    else {
	printf("Error:  cannot open %s file\n",filename);
	exit(1);
    }
    return(i);
} /* loaddata */


/*
 * Procedure:	setdow(tmonth,tday,tyear)
 *
 * Function:	<short functional description>
 *
 * Parameters:
 *	p1	- int - month
 *	p2	- int - day
 *	p3	- int - year
 *
 * Return Values:
 *	interger value representing day of week with 1=sat, . . ., etc
 *
 */

setdow(tmonth,tday,tyear)

int 		tmonth,tday,tyear;

{

    int		mcnt;
    long	days;
    int		tdow;
    int		smonth,sday,syear;

    smonth = 1;
    sday = 1;
    syear = 1979;

    if ((tmonth == smonth) && (tyear == syear)) {
	days = abs(sday - tday);
    }
    else {
        days = mdays[smonth] - sday;
        if (tyear == syear) {
            while (++smonth < tmonth) {
	        days += mdays[smonth];
	        if ((smonth == 2) && ((syear % 4) == 0)) days++;
	    }
        }
        else {
            while (++smonth < 13) {
	        days += mdays[smonth];
                if ((smonth == 2) && ((syear % 4) == 0)) days++;
            }
	    while (++syear < tyear) {
	        days += 365;
	        if ((syear % 4) == 0 ) days ++;
            }
    
	    mcnt = 0;
            while (++mcnt < tmonth) {
	        days += mdays[mcnt];
	        if ((mcnt == 2) && ((tyear % 4) == 0)) days++;
	    }
        }
        days += tday;
    }

    tdow = ((days%7) + 1);

    return(tdow);
} /* setdow */


/*
 * Procedure:	setjulie(tmonth,tday,tyear)
 *
 * Function:	calculates the julian date
 *
 * Parameters:
 *	p1	- int - month
 *	p2	- int - day
 *	p2	- int - year
 *
 * Return Values:
 *	julian date as a long
 *
 */

long setjulie(tmonth,tday,tyear)

int		tmonth,tday,tyear;

{

    int		mcnt;
    long	days;
    int		tdow;
    int		smonth,sday,syear;

    smonth = 1;
    sday = 1;
    syear = 1979;

    if ((tmonth == smonth) && (tyear == syear)) {
	days = abs(sday - tday);
    }
    else {
        days = mdays[smonth] - sday;
        if (tyear == syear) {
            while (++smonth < tmonth) {
	        days += mdays[smonth];
	        if ((smonth == 2) && ((syear % 4) == 0)) days++;
	    }
        }
        else {
            while (++smonth < 13) {
	        days += mdays[smonth];
                if ((smonth == 2) && ((syear % 4) == 0)) days++;
            }
	    while (++syear < tyear) {
	        days += 365;
	        if ((syear % 4) == 0 ) days ++;
            }
    
	    mcnt = 0;
            while (++mcnt < tmonth) {
	        days += mdays[mcnt];
	        if ((mcnt == 2) && ((tyear % 4) == 0)) days++;
	    }
        }
        days += tday;
    }

    return(days);
} /* setjulie */


/*
 * Procedure:	date(days)
 *
 * Function:	calculates the current month, day and year
 *
 * Parameters:
 *	p1	- int - number days offset from 1,1,1979
 *
 * Return Values:
 *	sets global variable, month, day and year to calc value
 *
 */

date(days)

long   days;

{

    int    cnt;

    month = 1;
    day = 1;
    year = 1979;

    while (days--) {
	if (++day > mdays[month]) {
	    day = 1;
	    if (++month > 12) {
		month = 1;
		year++;
	    }
	    if ((month == 2) && !(year % 4)) {
		day--;
	    }
	}
    }
} /* date */
