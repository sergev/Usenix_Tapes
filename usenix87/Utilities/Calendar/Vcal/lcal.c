/*
 * Module:	lcal.c
 *
 * Purpose:	print month's appointments
 *
 * Author:	Mike Essex
 *
 * Date:	Sep. 16, 1986
 *
 * Includes:
 *	time.h, stdio.h, ctype.h, signal.h
 *
 * Discussion:
 *	Reads data from ~/.appointments and/or designated file and
 *      writes to standard *	out any appoinments which fall in
 *      the current or optionally specified month.
 * 
 *
 * Edit History
 * ==== =======
 *
 * Date      Who	What
 * ----      ---	----------------------------------------------
 *
 */

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>


char	*malloc();

int		monthdata[1000];	/* month data */
int		daydata[1000];		/* month data */
int		yeardata[1000];	/* month data */
char		*msgdata[1000];	/* message pointers */
char		msghold[40];
int		maxentries;
int		maxmsgs;
int		cmonth,cday,cyear;
int		argvindex;



/*
 * Procedure:	main()
 *
 * Function:	prints out listing of appointments
 *
 * Discussion:
 *	Parses command line for optional month and year, initializes
 *	varialbes, calls required funtions.
 */

main (argc,argv)

    int		argc;
    char	*argv[];
{

    int		month,day,year;
    int		i;
    int		tempmonth,tempyear;
    int		atoi();
    int		maxindex;
    extern int	abort();

    timeset();
    year = cyear;
    month = cmonth;
    day = cday;

    if (argc > 2) {
	tempmonth = atoi(argv[1]);
	tempyear = atoi(argv[2]);
	if (tempmonth && tempyear){
	    month = tempmonth;
	    year = tempyear;
	    if (year < 100) year += 1900;
	    if (argc > 3) {
		argvindex = 3;
		maxindex = argc;
	    }
	    else {
		argvindex = 0;
		maxindex = 1;
	    }
	}
	else {
	    if (tempmonth || tempyear) {
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

    
    signal(2,abort);
    signal(3,abort);

    day = 1;
    maxentries = 1000;
    maxmsgs = 18;
    msghold[0] = NULL;

    printf("                            C A L E N D A R   L I S T\n");
    while (argvindex < maxindex) {
	loaddata(argv[argvindex]);
	table(month,day,year,argv[argvindex]);
	argvindex++;
    }

    printf("------------------------------------------------------------------------------\n");
} /* main */


/*
 * Procedure:	abort()
 *
 * Function:	error exit routine
 */

abort()

{
    exit(1);
} /* abort */


/*
 * Procedure:	table(month,day,year)
 *
 * Function:	outputs appointments in tabular format
 *
 * Parameters:
 *	p1	- int - month
 *	p2	- int - day
 *	p3	- int - year
 *      p3      - char * - file name
 *
 * Discussion:
 *	Searches data arrays for appointments in the specified
 *	month and year and outputs them in tabular form to standard out.
 */

table(month,day,year,filename)

    int		month,day,year;
    char	*filename;

{
    int		i,j,k,d,dow,monthday,searchcnt,first;
    int		getdow();

    static char	*dayw[]= {
	"Sat   ","Sun   ","Mon   ", "Tue   ",
	"Wed    ", "Thu    ", "Fri    "
    };

    static char	*smon[]= {
    "JANUARY   ", "FEBRUARY   ", "MARCH    ", "APRIL    ",
    "MAY    ", "JUNE    ", "JULY    ", "AUGUST    ",
    "SEPTEMBER    ", "OCTOBER    ", "NOVEMBER    ", "DECEMBER    "
    };

    printf("------------------------------------------------------------------------------\n");
    if (argvindex) {
	printf("|  %-10.10s%u                       %30.30s       |\n", smon[month-1], year,filename);
    }
    else {
	printf("|  %-10.10s%u                                                            |\n", smon[month-1], year);
    }
    monthday = 0;
    while (++monthday <= 31) {
	searchcnt = 0;
	first = 1;
	while (searchcnt <= maxentries) {
	    if ((yeardata[searchcnt] == year) &&
	      (monthdata[searchcnt] == month) &&
	      (daydata[searchcnt] == monthday)) {
		if (first) {
		    dow = getdow(month,monthday,year);
		    first = 0;
		    /* printf("|----------------------------------------------------------------------------|\n"); */
		     printf("|                                                                            |\n"); 
		    printf("| %-7.7s%2d  %-64.64s|\n",dayw[dow],monthday,msgdata[searchcnt]);
		}
		else {
		    printf("|            %-64.64s|\n",msgdata[searchcnt]);
		}
	    }
	    searchcnt++;
	}
    }
} /* table */


/*
 * Procedure:	getdow(tmonth,tday,tyear)
 *
 * Function:	calculates day of week
 *
 * Parameters:
 *	p1	- int - month
 *	p2	- int - day
 *	p3	- int - year
 *
 * Return Values:
 *	interger value of day of week, sat=0, . . ., etc
 *
 * Discussion:
 *	calculates number of days from 1,1,1979 and determines dow
 */

getdow(tmonth,tday,tyear)

    int		tmonth,tday,tyear;
{

    static    int    mdays[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    int    month,day,year,mcnt;
    long   days;
    int    tdow;

    month = 1;
    day = 1;
    year = 79;

    if ((tmonth == month) && (tyear == year)) {
	days = abs(day - tday);
    }
    else {
        days = mdays[month] - day;
        if (tyear == year) {
            while (++month < tmonth) {
	        days += mdays[month];
	        if ((month == 2) && ((year % 4) == 0)) days++;
	    }
        }
        else {
            while (++month < 13) {
	        days += mdays[month];
                if ((month == 2) && ((year % 4) == 0)) days++;
            }
	    while (++year < tyear) {
	        days += 365;
	        if ((year % 4) == 0 ) days ++;
            }
    
	    mcnt = 0;
            while (++mcnt < tmonth) {
	        days += mdays[mcnt];
	        if ((mcnt == 2) && ((tyear % 4) == 0)) days++;
	    }
        }
        days += tday;
    }

    tdow = (days%7);

    return(tdow);
} /* get dow */


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
    char	home[80];
    FILE 	*fptr;
    int		i,j,k,l,field;

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
			    tmpbuf[k++] = ' ';
			    tmpbuf[k++] = ' ';
			    break;
			}
		    }
		    field++;
		}
		j++;
	    }
	    tmpbuf[k] = '\0';
	    msgdata[i] = malloc(40);
	    strcpy(msgdata[i],tmpbuf);

	    if (i >= maxentries) {
		printf("Warning:  Over 1000 entries in %s file.  Data truncated.\n",filename);
		break;
	    }
	    i++;
	}
	fclose(fptr);
    }
    else {
	printf("Error:  Cannot open %s file.\n",filename);
    }
} /* loaddata */


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

    cyear =	tp->tm_year;
    cmonth =	tp->tm_mon + 1;
    cday =	tp->tm_mday;

    cyear += 1900;

} /* timeset */
