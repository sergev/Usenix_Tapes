/* Copyright 1983 - University of Maryland */

/*
 *
 * calend - An appointment calendar maintainer
 *
 * calend is used to remind a user about appointments.
 *
 * When calend is called, it looks at the users appointments
 * in the .calrc file in his home directory.  This file is made
 * up of lines of the form:
 *
 * <opts> <start-date> <end-date> <message string>
 *
 * where <opts> is a string made up of
 * any of 'm','a','1','r','d', 'x', '+', '*' and <start-date> and
 * and <end-date> are either days of the week, (e.g. 'Wed') or
 * a date of the year of the form:
 * mm/dd (eg 11/20), month dd (eg Nov 20), or dd Month (eg 20 Nov)
 *
 * In addition, <end-date> can be an '*' instead, signifying the
 * same day or date as start-date.  start-date and end-date must
 * both be either dates or days.
 *
 * When the current date is between <start-date> and <end-date>
 * inclusive (or if they are days then the day of the week is between
 * them) then the message will be processed according to the specified
 * options.  The meanings of the various options are as follows:
 *
 * 'm' : mail the message to the user.  The first time that 'calend'
 *	 is called during that period, send mail to the user using
 * 	 the message-string as mail-text.
 * 'a' : always print the message.  Every time 'calend' is called
 * 	 during that period, print the message string on the terminal.
 * '1' : print the message once.  Print the message on the terminal
 * 	 the first time 'calend' is run during that period.
 * 'r' : run the remind program.  The first four non-blank digits
 * 	 of the message string should be a time-of-day of the form
 * 	 'hhmm' which remind can use.
 * 'd' : delete this entry from the file when 'calend' is finished
 * 	 with it.
 * 'x' : the message is executed as a process.  The '1' or 'a' says
 *	 whether to do it once or always during a period.  If neither,
 *	 the default is 'a'.
 * '+' : move the message dates when it's done.  The first word of
 *	 the message should be the offset of the form:
 *			+ [ <months> {m,/} ] [ <days> [d] ]
 *	 where '[' & ']' mean optional, and '{' & '}' mean a choice.
 *	 For example, +14 (two weeks), +3m (three months), +3/2
 *	 (three months and two days), +3m2d (same).
 * '*' : 'pending' flag used by the program internally to indicate
 * 	 that more processing needs to be done.
 * '#' : comment flag to indicate that this line shouldn't be processed
 *	 at all.
 * ':' : comment flag, not deleted by 'calend -c'.
 *
 * Author: Bruce Israel, umcp-cs!israel, israel@Maryland
 */

#include <signal.h>
#include "globals.h"
#include "process.h"

struct tm	*localtime();

onintr()
{
    char tmname[BUFSIZ];
    int i;

	for (i = 0; i <= level; i++) {
	    sprintf(tmname,"%s-%c",tmplate,'A' + level);
	    unlink(tmname);
	}

	exit(1);
}

main(argc, argv)
int argc; char *argv[];
{
	char *ptr;
	char	fname[BUFSIZ];	/* file name */
	int f_opt = NO;
	long	tmptime;	/* place to put time */
	struct tm *curtim;	/* current time structure */
	bool	debugdate;	/* user-specified date? */
	int	userdate,userday; /* specifiable date for debugging */

	argc--, argv++;
	while (argc > 0) {
		ptr = *argv;
		while (*ptr) switch (*ptr++) {

		case '-':
			break;

		case 'C':
		case 'c':
			clean = YES;
			break;

		case 'f':
		case 'F':
			f_opt = YES;
			if (*ptr == 0) {
			    argv++;
			    if (*argv == 0) {
				fprintf(stderr,
					"calend: no file given with '-f'.\n");
				exit(1);
			    }
			    strcpy(fname,*argv);
			}
			else {
			    strcpy(fname,ptr);
			    *ptr = 0;
			}
			break;

		case 'I':
		case 'i':
			i_opt = YES;
			break;

		case 'R':
		case 'r':
			remonly = YES;
			break;

		case 'D':
		case 'd':
			debugdate = YES;
			if (*ptr == 0) {
			    fprintf(stderr,
					"calend: no date given with '-D'.\n");
			    exit(1);
			}
			userday = *ptr++ - '0';
			userdate = atoi(ptr);
			*ptr = 0;
			break;

		default:
			fprintf(stderr,
			"Unknown option '%c' - ignored\n",ptr[-1]);
		}
		argc--, argv++;
	}

	/* get current date */

	if (debugdate) {
	    cdate = userdate;
	    cwday = userday;
	}
	else {
	    time(&tmptime);
	    curtim = localtime(&tmptime);
	    cwday = curtim -> tm_wday;
	    cdate = (curtim -> tm_mon) * 100 + curtim -> tm_mday + 100;
	}

	setgid(getegid());
	setuid(geteuid());
	
	/* get dates file name */

	if (! f_opt) sprintf(fname,"%s/%s",getenv("HOME"),RCFILE);

	sprintf(tmplate,"/tmp/cal-%d",getpid());

	level = 0;

	signal(SIGINT,onintr);

	process_file(fname,1);
}
