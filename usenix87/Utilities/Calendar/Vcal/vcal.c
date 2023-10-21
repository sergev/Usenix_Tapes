/*
 * Module:	vcal.c
 *
 * Purpose:	visual appointment calendar
 *
 * Author:	Mike Essex
 *
 * Date:	Sep. 16, 1986
 *
 * Includes:
 *	time.h, stdio.h, ctype.h, signal.h
 *
 * Discussion:
 *	displays a calendar to the screen for the current or optionally
 *	specified month.  User may move the cursor any day of the month
 *	and view or enter appointments for that date.
 * 
 *
 * Edit History
 * ==== =======
 *
 * Date      Who	What
 * ----      ---	----------------------------------------------
 *11/25/86   me         added multiple data file capability
 *11/25/86   me         fixed array "home" from *home[80] to home[80]
 *11/25/86   me         fixed "space" and "area" to be global
 *11/26/86   me		changed help message routines
 * 2/06/87   me		changed calendar header from printing a default
 *			NULL for .appointments file to not printing
 *02/19/87   me		reworked terminal handler to make correct use
 *			of termcap(3) functions for greater terminal
 *			portability.  modified so that cursor comes to
 *			current day of month on startup.  Fixed minor
 *                      bugs and changed a few messages.
 *02/19/87   me		lengthened notes line printout
 *
 */

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>
#include <sgtty.h>

#define		LF	'\012'
#define		BS	'\010'
#define		ESC	'\033'

int		mon[] = {
    0,
    31, 29, 31, 30,
    31, 30, 31, 31,
    30, 31, 30, 31,
};

int	tputc();		/* termcap dependent output */
char	space[BUFSIZ];		/* used by area */
char	*area = space;		/* termcap required variable */
char	*BC;			/* backspace string pointer */
char	*backstring = "\b";	/* backspace string */
char	PC;			/* number pad characters */
char	*UP;			/* cursor up string */
extern	short	ospeed;		/* output speed */
char	*CM;			/* cursor motion */
char	*CL;			/* clear screen */
char	*TI;			/* begin cm */
char	*TE;			/* end cm */
char	*SO;			/* standout*/
char	*SE;			/* end standout*/
char	*tgoto();
char	*malloc();
char	tcolumns = 0;		/* terminal columns */
char	tlines = 0;		/* terminal lines */

int		xposition[32];		/* calendar x axis position */
int		yposition[32];		/* calendar y axis position */
int		active[33];		/* highlight day array */

int		monthdata[1000];	/* month data */
int		daydata[1000];		/* month data */
int		yeardata[1000];	/* month data */
char		*msgdata[1000];	/* message pointers */
char		msghold[80];
int		dayindex[18];		/* index to day's msgs */
int		maxentries;
int		maxmsgs;
int		tmonth,tday,tyear;

int		maxchars;
int		notclear;
int		help1,help2;		/* help  message flags */

char	dayw[] = {
	"  S    M    T    W    T    F    S   "
};

char	*smon[]= {
    "JANUARY   ", "FEBRUARY   ", "MARCH    ", "APRIL    ",
    "MAY    ", "JUNE    ", "JULY    ", "AUGUST    ",
    "SEPTEMBER    ", "OCTOBER    ", "NOVEMBER    ", "DECEMBER    ",
};



/*
 * Procedure:	main(argv[1],argv[2],argv[3])
 *
 * Function:	Front end for calendar program
 *
 * Parameters:
 *	p1	- character pointer - optional month
 *	p2	- character pointer - optional year
 *	p3      - character pointer - optional data file name
 *
 * Discussion:
 *	This module parses command line, intializes variables and calls
 *	init functions.  It then drops into a character interpreter
 *	loop, taking input from standard in.  Each input either changes
 *	cursor position or invokes a function.
 */

main (argc,argv)

    int		argc;
    char	*argv[];
{

    int		month,day,year;
    char	key;
    char	*datafile;
    extern int	abort();
    int i;

    datafile = NULL;
    day = 1;
    if (argc == 1) {
	timeset();
	month = tmonth;
	day = tday;
	year = tyear;
    }
    else {
	if (argc == 3) {
	    month = atoi(argv[1]);
	    year = atoi(argv[2]);
	    if (year < 100) year += 1900;
	    if (!month || !year) {
		printf("Syntax Error:  non-numeric argument\n");
		abort(1);
	    }
	}
	else {
	    if (argc == 2) {
		timeset();
		month = tmonth;
		day = tday;
		year = tyear;
		datafile = argv[1];
	    }
	    else {
		if ( argc == 4) {
		    month = atoi(argv[1]);
		    year = atoi(argv[2]);
		    if (year < 100) year += 1900;
		    if (!month || !year) {
			printf("Syntax Error:  non-numeric argument\n");
			abort(1);
		    }
		    datafile = argv[3];
		}
		else {
		    printf("Syntax Error:  incorrect number of arguments\n");
		    abort(1);
		}
	    }
	}
    }
    signal(2,abort);
    signal(3,abort);

    system("stty cbreak -echo");

    tinit();
    tputs(TI,1,tputc);
    maxchars = 38;
    maxentries = 1000;
    maxmsgs = 18;
    msghold[0] = NULL;
    help1 = 0;
    help2 = 0;

    loaddata(datafile);
    cal(month,day,year,datafile);
    movcur(day);

    key = 'a';
    while (key != LF) {

	key = getchar();

	switch (key) {

            case 'H' :

		helpcal();

	        break;

	    case 'p' :

		if (--month < 1) {
		    month = 12;
		    year--;
		}
		day = 1;
		notclear = 0;
		cal(month,day,year,datafile);

		break;

	    case 'n' :
		    
		if (++month == 13) {
		    month = 1;
		    year++;
		}
		day = 1;
		notclear = 0;
		cal(month,day,year,datafile);

		break;
		    
	    case 'h' :

		if (--day <= 0) {
		    day = mon[month];
		}
		if (notclear) {
		    clearmsgs();
		}

		break;

	    case 'l' :

		if (++day > mon[month]) {
		    day = 1;
		}
		if (notclear) {
		    clearmsgs();
		}

		break;

	    case 'j' :

		if ((day += 7) > mon[month]) {
		    day = day % 7;
		    if (day == 0) {
			day = 7;
		    }
		}
		if (notclear) {
		    clearmsgs();
		}

		break;

	    case 'k' :

		if ((day -= 7) <= 0) {
		    day += 35;
		    if (day > mon[month]) {
			day -= 7;
		    }
		}
		if (notclear) {
		    clearmsgs();
		}

		break;

	    case 'e' :

		day = mon[month];
		if (notclear) {
		    clearmsgs();
		}

		break;

	    case 'b' :

		day = 1;
		if (notclear) {
		    clearmsgs();
		}

		break;

	    case 'c' :

		clearday(month,day,year);

		break;

	    case ' ' :

		notes(month,day,year);

		break;

	    case 'm' :

		modnotes(month,day,year);

		break;


	}
	movcur(day);
    }
    closefile(datafile);
    tputs(TE,1,tputc);
    system("stty -cbreak echo");
    if (help1 || help2) {
	helpclr();
    }
    mov(1,24);
} /* main */


/*
 * Procedure:	<routine name>
 *
 * Function:	abort()
 *
 * Discussion:
 *	Reset tty parameters and exits with an error code.
 */

abort()

{
    system("stty -cbreak echo");
    tputs(SE,1,tputc);
    if (help1 || help2) {
	helpclr();
    }
    mov(1,24);
    tputs(TE,1,tputc);
    exit(1);
} /* abort */



/*
 * Procedure:	cal(month,day,year,datafile)
 *
 * Function:	produces the visual calendar to the screen
 *
 * Parameters:
 *	p1	- int - month
 *	p2	- int - day
 *	p3	- int - year
 *	p4      - char * - datafile name
 *
 * Discussion:
 *	Calculates current months parameters, such as, starting
 *	day of week, number of days and days on which there are
 *	appointments.  This data is then used to draw a calendar
 *	to the screen.
 */

cal(month,day,year,datafile)

    int		month,day,year;
    char	*datafile;

{
    int		i,j,k,d;

    tputs(CL,24,tputc);
    if (datafile == NULL) {
	printf("V I S U A L   C A L E N D A R\n");
    }
    else {
	printf("V I S U A L   C A L E N D A R\t\t%s\n",datafile);
    }
    printf("-------------------------------------------------------------------------------\n");
    printf("\t\t\t\t%s%u\n", smon[month-1], year);
    printf("-------------------------------------------------------------------------------\n\n");
    printf("%s\n\n", dayw);


    d = jan1(year);
    mon[2] = 29;
    mon[9] = 30;


    i = 1;
    while (i <= 32) {
	active[i++] = 0;
    }

    i = 0;
    while (i < maxentries) {
	if ((yeardata[i] == year) && (monthdata[i] == month) ) {
	    active[daydata[i]] = 1;
	}
	i++;
    }

    switch((jan1(year+1)+7-d)%7) {

    /*
     *	non-leap year
     */
	case 1:
	    mon[2] = 28;
	    break;

    /*
     *	1752
     */
	default:
	    mon[9] = 19;
	    break;

    /*
     *	leap year
     */
	case 2:
		;
    }

    /* calculate days from beginning of year */

    for(i=1; i<month; i++) {
	d += mon[i];
    }

    d %= 7;
    i = 0;

    /* inset to first day of week position */

    while (i++ < (5*d)) {
	printf(" ");
    }

    k = 0;
    i = d;

    for (j=1;j<=31;j++) {
	xposition[j] = (i*5) + 2;
	yposition[j] = (k*2) + 7;
	if (++i == 7) {
	    i = 0;
	    k++;
	}
    }

    for(i=1; i<=mon[month]; i++) {
	if(i==3 && mon[month]==19) {
	    i += 11;
	    mon[month] += 11;
	}
	if (active[i]) {
	    if(i > 9) {
		printf(" ");
		tputs(SO,1,tputc);
		printf("%d",i/10);
		tputs(SE,1,tputc);
	    }
	    else {
		printf(" ");
		tputs(SO,1,tputc);
		printf(" ");
		tputs(SE,1,tputc);
	    }
	    tputs(SO,1,tputc);
	    printf("%d",i%10);
	    tputs(SE,1,tputc);
	    if (*SO == NULL) {
		printf("* ");
	    }
	    else {
		printf("  ");
	    }
	}
	else {
	    if(i > 9) {
		printf(" %d",i/10);
	    }
	    else {
		printf("  ");
	    }
	    printf("%d  ",i%10);
	}

	if(++d == 7) {
	    d = 0;
	    printf("  \n\n");
	}
    }
    
    helpclr();

    mov(42,5);
    printf ("TIME      MESSAGE");
} /* cal */


/*
 * Procedure:	jan1(year)
 *
 * Function:	calculates day of week of Jan 1 on specified year
 *
 * Parameters:
 *	p1	- int - year
 *
 * Return Values:
 *	integer representation day of the week
 */

jan1(year)

    int		year;
{

    register y, d;
/*
 *	normal gregorian calendar
 *	one extra day per four years
 */

    y = year;
    d = 4+y+(y+3)/4;

/*
 *	julian calendar
 *	regular gregorian
 *	less three days per 400
 */

    if(y > 1800) {
	d -= (y-1701)/100;
	d += (y-1601)/400;
    }

/*
 *	great calendar changeover instant
 */

    if(y > 1752)
	d += 3;

    return(d%7);
} /* jan1 */



/*
 * Procedure:	movcur(day)
 *
 * Function:	moves the cursor to the specified day's position
 *
 * Parameters:
 *	p1	- int - day
 *
 * Discussion:
 *	uses termcap values to move the cursor to a matrix position
 *	stored in a global array
 */

movcur(day)

    int	day;
{
    tputs( tgoto( CM, xposition[day],yposition[day]), 1, tputc );
} /* movcur */


/*
 * Procedure:	setday(day)
 *
 * Function:	places day in calendar display with appointment
 *		days highlighted
 *
 * Parameters:
 *	p1	- int - day
 *
 * Discussion:
 *	Uses day arguement, termcap values and calendar position
 *	matrices to write a day to the screen with days with appointment
 *	days highlighted.
 */

setday(day)

    int		day;
{
    tputs( tgoto( CM, xposition[day]-1,yposition[day]), 1, tputc );
    if (active[day]) {
	if(day > 9) {
	    tputs(SO,1,tputc);
	    printf("%d",day/10);
	    tputs(SE,1,tputc);
	}
	else {
	    tputs(SO,1,tputc);
	    printf(" ");
	    tputs(SE,1,tputc);
	}
	tputs(SO,1,tputc);
	printf("%d",day%10);
	tputs(SE,1,tputc);
	if (*SO == NULL) {
	    printf("*");
	}
	else {
	    printf(" ");
	}
    }
    else {
	if(day > 9) {
	    printf("%d",day/10);
	}
	else {
	    printf(" ");
	}
	printf("%d ",day%10);
    }
} /* setday */



/*
 * Procedure:	tinit()
 *
 * Function:	gets termcap entries
 *
 * Return Values:
 *	loads global variables with termcap parameters
 *
 * Discussion:
 *	Initial various termcap variables so that they can later
 *	be used to move the cursor and highlight certain displays.
 */

tinit()	/* termcap initalization */

{
    struct	sgttyb speed;

    char	term[BUFSIZ];		/* holds termcap entry */
    char	name[16];		/* terminal name */
    char	*np;			/* termcap name pointer */
    char	*NADA = { "\0" };	/* null string */
    char	*tgetstr();
    char	*getenv();

    if( (np = getenv( "TERM" )) != NULL ) {
	    strncpy( name, np, 16 );
    }
    else {
	    printf("TERM environment variable does not exist\n");
	    abort();
    }

    if( tgetent( term, name ) != 1 ) {
	    printf("Termcap for %s does not exist\n",name);
	    abort();
    }

    /* if a capability does not exist, point it to a null
    *  string not NULL
    */

    gtty(stdout,&speed);
    ospeed = speed.sg_ospeed;

    tlines = tgetnum( "li" );
    tcolumns = tgetnum( "co" );

    if( (CM = tgetstr( "cm", &area )) == NULL) {
	CM = NADA;	
	printf("Error:  No termcap 'cm' entry for this terminal type\n");
	abort();
    }
    if( (CL = tgetstr( "cl", &area )) == NULL)
	CL = NADA;
    if( (UP = tgetstr( "up", &area )) == NULL)
	UP = NADA;
    if( (SO = tgetstr( "so", &area )) == NULL)
	SO = NADA;
    if( (SE = tgetstr( "se", &area )) == NULL)
	SE = NADA;
    if( (TI = tgetstr( "ti", &area)) == NULL)
	TI = NADA;
    if( (TE = tgetstr( "te", &area)) == NULL)
	TE = NADA;
    if( tgetnum( "sg", &area)  > 0)  {
	SO = NADA;
	SE = NADA;
    }
    if ( (BC = tgetstr( "bc", &area)) ==  NULL) {
	BC = backstring;
    }

} /* tinit */



/*
 * Procedure:	loaddata(datafile)
 *
 * Function:	loads data file
 *
 * Return Values:
 *	Various global arrays loaded with appointment data
 *
 * Discussion:
 *	Opens user's data file and reads it in, placing
 *	the data in appropriate arrays.
 */

loaddata(datafile)

    char	*datafile;
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

    if (datafile == NULL) {
	strcpy(home,getenv("HOME"));
	strcat(home,"/.appointments");
    }
    else {
	strcpy(home,datafile);
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
	    tmpbuf[k] = NULL;
	    msgdata[i] = malloc(80);
	    strncpy(msgdata[i],tmpbuf,80);
	    msgdata[79] = NULL;

	    if (i >= maxentries) {
		printf("Warning:  Over 1000 entries in data file.  Data truncated.\n");
		break;
	    }
	    i++;
	}
	fclose(fptr);
    }
} /* loaddata */


/*
 * Procedure:	closefile(datafile)
 *
 * Function:	writes appointment data to file
 *
 * Discussion:
 *	Opens the user's designated data file and writes the current
 *	data into it.
 */

closefile(datafile)

char	*datafile;
{
    FILE	*fptr;
    char	tmpbuf[80];
    int		i;
    char	*getenv();
    char	home[80];

    if (datafile == NULL) {
	strcpy(home,getenv("HOME"));
	strcat(home,"/.appointments");
    }
    else {
	strcpy(home,datafile);
    }
    if ((fptr = fopen(home,"w")) == NULL) {
	printf("Error:  Cannot open %s file",datafile);
	abort();
    }	
    i = 0;
    while (i < maxentries) {
	if (daydata[i]) {
	    strcpy(tmpbuf,msgdata[i]);
	    tmpbuf[4] = NULL;
	    fprintf(fptr,"%d,%d,%d,%4.4s,%s\n",monthdata[i],daydata[i],yeardata[i],tmpbuf,&tmpbuf[6]);
	}
	i++;
    }
    fclose(fptr);
} /* closefile */




/*
 * Procedure:	notes(month,day,year)
 *
 * Function:	displays the current day's appointments
 *
 * Parameters:
 *	p1	- int - month
 *	p2	- int - day
 *	p3	- int - year
 *
 * Discussion:
 *	Writes to notes section of screen notes for day to
 *	which the cursor was pointing.
 */

notes(month,day,year)

    int		month,day,year;
{
    int		i,j,k;

    if (help1) {
	helpclr();
	help1 = 0;
    }

    i = 0;
    while (i < 18) {
	dayindex[i++] = 0;
    }

    mov(64,5);
    printf("%2d/%2d/%2d",month,day,year);

    i = 0;
    j = 0;
    k = 6;

    while (i < maxentries) {
	if ((yeardata[i] == year) && (monthdata[i] == month) && (daydata[i] == day)) {
	    dayindex[j++] = i;
	    notclear++;
	    mov(42,k++);
	    printf("%-38.38s",msgdata[i]);
	}
	if (j > maxmsgs) {
	    mov(42,24);
	    printf("* too many entries to print *");
	    break;
	}
	i++;
    }
    i = 0;
    while (i < maxentries) {
	if (daydata[i] == 0) {
	    dayindex[j++] = i;
	    if (j >= maxmsgs) {
		break;
	    }
	}
	i++;
    }
} /* notes */



/*
 * Procedure:	modnotes(month,day,year)
 *
 * Function:	add, delete and modify appointment list
 *
 * Parameters:
 *	p1	- int - month
 *	p2	- int - day
 *	p3	- int - year
 *
 * Return Values:
 *	changes data in global message and date arrays
 *
 * Discussion:
 *	Allows the user to add, delete and modify a specified day's
 *	appointment list.  Routine contains a small character
 *	interpreter which moves the cursor through the notes list
 *	and selects routines to modify the list.
 */

modnotes (month,day,year)

    int		month,day,year;
{

    char	key,c;
    int		xcoord,ycoord;
    int		i,cnt,total;
    char	tmpmsg[39];

    notes(month,day,year);
    xcoord = 42;
    ycoord = 6;
    i = 0;
    mov(xcoord,ycoord);
    key = 'a';

    while (key != LF) {

	key = getchar();

	switch (key) {

            case 'H' :

		helpnotes();

	        break;

	    case 'j' :
		i++;
		if (++ycoord > 23) {
		    ycoord = 6;
		    i = 0;
		}
		break;

	    case 'k' :
		i--;
		if (--ycoord < 6) {
		    ycoord = 23;
		    i = 17;
		}
		break;

	    case 'd' :

		tputs(SO,1,tputc);
		printf("                                      ");
		tputs(SE,1,tputc);
		mov (xcoord,ycoord);
		monthdata[dayindex[i]] = 0;
		daydata[dayindex[i]] = 0;
		yeardata[dayindex[i]] = 0;
		strcpy(msghold,msgdata[dayindex[i]]);
		if (msgdata[dayindex[i]]) {
		    free(msgdata[dayindex[i]]);
		    msgdata[dayindex[i]] = 0;
		}
		break;

            case 'y' :

		strcpy(msghold,msgdata[dayindex[i]]);

		break;

            case 'p' :

		    if (msgdata[dayindex[i]] == 0) {
			msgdata[dayindex[i]] = malloc(80);
		    }
		    strncpy(msgdata[dayindex[i]],msghold,80);
		    yeardata[dayindex[i]] = year;
		    daydata[dayindex[i]] = day;
		    monthdata[dayindex[i]] = month;
		    printf("%s",msgdata[dayindex[i]]);

		break;

	    case 'i' :

		tputs(SO,1,tputc);
		printf("                                      ");

		mov (xcoord,ycoord);

		c = '0';
		cnt = 0;
		while (c != ESC) {
		    c = getchar();
		    if (c != ESC ) {
			if (c == BS) {
			    if (cnt > 0) {
				cnt--;
				if (cnt == 5) {
				    cnt = 3;
				    mov(46,ycoord);
				}
				printf("%s %s",BC,BC);
			    }
			}
			else {
			    if (c != LF) {
				if ( cnt < maxchars) {
				    tmpmsg[cnt] = c;
				    cnt++;
				    printf("%c",c);
				}
				if (cnt == 4) {
				    tmpmsg[4] = ' ';
				    tmpmsg[5] = ' ';
				    cnt = 6;
				    mov(48,ycoord);
				}
			    }
			    else {
				c = ESC;
			    }
			}
		    }
		}
		tputs(SE,1,tputc);
		if (cnt) {
		    tmpmsg[cnt] = NULL;
		    if (msgdata[dayindex[i]] == 0) {
			msgdata[dayindex[i]] = malloc(80);
		    }
		    strncpy(msgdata[dayindex[i]],tmpmsg,80);
		    monthdata[dayindex[i]] = month;
		    daydata[dayindex[i]] = day;
		    yeardata[dayindex[i]] = year;
		}
		else {
		    monthdata[dayindex[i]] = 0;
		    daydata[dayindex[i]] = 0;
		    yeardata[dayindex[i]] = 0;
		    if (msgdata[dayindex[i]]) {
			free(msgdata[dayindex[i]]);
			msgdata[dayindex[i]] = 0;
		    }
		}

		break;

	    default :;
	}
	mov (xcoord,ycoord);
    }

    active[day] = 0;
    i = 0;
    while (i < maxentries) {
	if ((yeardata[i] == year) && (monthdata[i] == month)) {
	    notclear = 18;
	    active[daydata[i]] = 1;
	}
	i++;
    }

    notclear = 18;
    clearmsgs();
    if (active[day]) {
	notes(month,day,year);
    }
    setday(day);
    if (help2) {
	helpclr();
	help2 = 0;
    }
} /* modnotes */


/*
 * Procedure:	tputc(character)
 *
 * Function:	moves cursor on screen
 *
 * Parameters:
 *	p1	- int - character
 *
 * Discussion:
 *	Outputs a character to crt.  Used as an argument to tputs().
 */

tputc(c)
    int c;
{
    putchar(c);
}


/*
 * Procedure:	mov(column,row)
 *
 * Function:	moves cursor on screen
 *
 * Parameters:
 *	p1	- int - crt column
 *	p2	- int - crt row
 *
 * Discussion:
 *	Moves the cursor to specified column and row on screen using
 *	termcap data.
 */

mov(col, row)

int	col,row;

{
    tputs( tgoto( CM, (col - 1),(row - 1)), 1, tputc );
} /* move */


/*
 * Procedure:	clearday(month,day,year)
 *
 * Function:	deletes specified day's appointments
 *
 * Parameters:
 *	p1	- int - month
 *	p2	- int - day
 *	p2	- int - year
 *
 * Return Values:
 *	changes various global arrays
 *
 * Discussion:
 *	Removes appointment entries for the specified day from
 *	message and date tables and from the crt
 */

clearday (month,day,year)

    int		month,day,year;
{
    int		i;

    i = 0;
    while (i < maxentries) {
	if ((yeardata[i] == year) && (monthdata[i] == month) && (daydata[i] == day)) {
	    active[day] = 0;
	    monthdata[i] = 0;
	    daydata[i] = 0;
	    yeardata[i] = 0;
	    if (msgdata[i])  {
		free(msgdata[i]);
		msgdata[i] = 0;
	    }
	}
	i++;
    }
    clearmsgs();
    setday(day);
} /* clearday */


/*
 * Procedure:	clearmsgs()
 *
 * Function:	clears the notes section of the crt
 */

clearmsgs()

{
    int		i;

    mov(64,5);
    printf("          ");

    if (notclear > maxmsgs) notclear = maxmsgs;

    i = 6;
    while (i < (notclear+6)) {
	mov(42,i++);
	printf("                                      ");
    }
    mov(42,24);
    printf("                                   ");
    notclear = 0;
} /* clearmsgs */


/*
 * Procedure:	helpcal()
 *
 * Function:	displays calendar help messages to crt
 */

helpcal()

{
    mov(1,19);
    printf("j/k/h/l:  cursor down/up/left/right     \n");
    printf("b:  first day         e: last day       \n");
    printf("space: display notes  m: modify notes   \n");
    printf("c: clear notes        CR: exit program  \n");
    printf("p: previous month     n: next month     \n");
    printf("BRK: exit w/o changes                   ");
    help1++;
} /* helpcal */


/*
 * Procedure:	helpnotes()
 *
 * Function:	displays notes help messages to crt
 */

helpnotes() 

{
    mov(1,19);
    printf("j/k:  cursor down/up                    \n");
    printf("i: enter insert      ESC/CR: exit insert\n");
    printf("d: delete entry      CR: exit notes     \n");
    printf("y/p: yank/put a line                    \n");
    printf("                                        \n");
    printf("                                        ");
    help2++;
} /* helpnotes */


/*
 * Procedure:	helpclr()
 *
 * Function:	clears notes area of screen
 */

helpclr ()
{
    mov(1,19);
    printf("                                        \n");
    printf("                                        \n");
    printf("                                        \n");
    printf("                                        \n");
    printf("    Type 'H' for help                   \n");
    printf("                                        ");
} /* helpclr */


/*
 * Procedure:	timeset()
 *
 * Function:	sets current month, day and year variables
 *
 * Return Values:
 *	tday, tmonth and tyear variables set to current date
 */

timeset()

{
    struct	tm *localtime();

    struct tm *tp;		/* time structure */
    long	tloc;		/* number of seconds since 1970 */

    time(&tloc);	/* fills tloc */

    tp = localtime(&tloc);

    tyear =	tp->tm_year;
    tmonth =	tp->tm_mon + 1;
    tday =	tp->tm_mday;

    tyear += 1900;

} /* timeset */
