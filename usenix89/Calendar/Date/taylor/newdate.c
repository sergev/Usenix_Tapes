/**				newdate.c				**/

/** This program is implemented based on the manual entry in HP-UX
    (Hewlett-Packards *reliable* version of Unix) for the 'date(1)'
    program.  The main improvement is that the user now has a set
    of format commands that they can use to get output in a different
    format than the normal (ugly) date(1) command.  Note to system V
    and HP-UX users - I've also added %A and %N for full day and
    month name, respectively.

    Please see the manual entry for more information.

    (C) Copyright 1986, Dave Taylor
**/

/** If you're on a System V machine, then compile with -DSYSV as a flag. **/

#include <stdio.h>
#include <sys/time.h>

char *short_dayname[]   = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
char *short_monthname[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
		            "Aug", "Sep", "Oct", "Nov", "Dec" };

char *long_dayname[]    = { "Sunday", "Monday", "Tuesday", "Wednesday", 
			    "Thursday", "Friday", "Saturday" };
char *long_monthname[]  = { "January", "February", "March", "April", "May",
			    "June", "July", "August", "September", "October",
			    "November", "December" };

struct tm *localtime();		/* forward declare for compiler happiness */

#ifdef SYSV
  extern char *tzname[2];
#else
  char *timezone();		/* another forward declaration...  	*/
#endif

main(argc, argv)
int argc;
char *argv[];
{
	char buffer[200], 	/* our output buffer... 		*/
	     tempbuf[100];	/* and a temp one for formatting stuff  */
	int  loc;		/* our location in the format string    */
	long thetime;		/* the current time, in seconds!        */
	struct tm *t;		/* the time record structure		*/
#ifndef SYSV
	struct timeval	tp;	/* for storing yet-another-format       */
	struct timezone tz;	/* to figure out our timezone...        */
#endif

	if (argc == 1 || argv[1][0] != '+')   /* go to 'real' date prog */
	  execv("/bin/date", argv);

	if (argc > 2) {
	  fprintf(stderr,"Usage: %s [new time] [ + format string]\n",
		  argv[0]);
	  exit(1);
	}

	/* if we're here we're doing okay... */

	thetime = time( (long *) 0);
	t = localtime(&thetime);

#ifndef SYSV
	/** now let's get the timezone that we're in... **/

	gettimeofday(&tp, &tz);

#endif

	/* we have the time...now let's parse and build the output! */

	for (loc = 1; loc < strlen(argv[1]);) {
	  tempbuf[0] = '\0';

	  if (argv[1][loc] == '%') {	/* a format string! */

	    switch (argv[1][loc+1]) {

	      case 'n' : strcat(buffer, "\n");		break;
	      case 't' : strcat(buffer, "\t");		break;
	      case '%' : strcat(buffer, "%");		break;

	      case 'm' : sprintf(tempbuf, "%d", t->tm_mon);	break;
	      case 'd' : sprintf(tempbuf, "%d", t->tm_mday);	break;
	      case 'y' : sprintf(tempbuf, "%d", t->tm_year);	break;
	      case 'D' : sprintf(tempbuf, "%d/%d/%d",
			 t->tm_mon, t->tm_mday, t->tm_year);	break;
	      case 'H' : sprintf(tempbuf, "%d", t->tm_hour);	break;
	      case 'M' : sprintf(tempbuf, "%d", t->tm_min);	break;
	      case 'S' : sprintf(tempbuf, "%d", t->tm_sec);	break;
	      case 'T' : sprintf(tempbuf, "%02d:%02d:%02d",
			 t->tm_hour, t->tm_min, t->tm_sec);	break;
	      case 'j' : sprintf(tempbuf, "%d", t->tm_yday);	break;
	      case 'w' : sprintf(tempbuf, "%d", t->tm_wday);	break;
	      case 'a' : sprintf(tempbuf, "%s", 
				 short_dayname[t->tm_wday]); 	break;
	      case 'h' : sprintf(tempbuf, "%s", 
				short_monthname[t->tm_mon]); 	break;
	      case 'A' : sprintf(tempbuf, "%s", 
				 long_dayname[t->tm_wday]); 	break;
	      case 'N' : sprintf(tempbuf, "%s", 
				long_monthname[t->tm_mon]); 	break;
	      case 'r' : sprintf(tempbuf, "%d:%02d %s",
			 t->tm_hour > 12? t->tm_hour - 12:t->tm_hour,
		         t->tm_min,
			 t->tm_hour > 12? "pm":"am");		break;
	      case 'z' : sprintf(tempbuf, "%s",
#ifdef SYSV
			 t->tm_isdst? tzname[1] : tzame[0]);	break;
#else
			 timezone(tz.tz_minuteswest, t->tm_isdst));  break;
#endif

	      case '\0': fprintf(stderr,
			"%s: unexpected end of format instructions!\n",argv[0]);
			 exit(1);

	      default  : fprintf(stderr,
		         "%s: don't understand %%%c as a format instruction!\n",
			 argv[0], argv[1][loc+1]);
			 exit(1);
	    }

	    loc += 2;	/* skip the percent and the char we just dealt with */

	    if (tempbuf[0] != '\0') 
	      strcat(buffer, tempbuf);

	  }
	  else { 	/* not a percent sign... */

	    tempbuf[0] = argv[1][loc++];
	    tempbuf[1] = '\0';
	    strcat(buffer, tempbuf);

	  }
	}
	
	/* and print the buffer out! */

	printf("%s\n", buffer);
	
	exit(0);			/* bye! */
}
