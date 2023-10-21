/*			Modification History
 *
 *  Origional Author:  Tom Stoehn@Tektronics[zeus!tims]
 *  Modifications by:  Marc Ries@TRW[trwrb!ries]
 *
 */

#include <curses.h>
#include <signal.h>
#include <sys/types.h>
#include <utmp.h>
#include "month.h"

short initd = 0;
short dhour, dminute, dsecond;
extern short crow, ccol, update_schedule, updating;
extern short this_month, this_day, this_year, SCHEDULE_ROW, SKID_ROW;
extern struct event_rec events;
extern struct mdate mdates[];
extern char *strcpy();

extern short start_day;

short calflag, nagflag;
char *nagprog = "";

int lflag = 0;

main(argc, argv)
int argc;
char **argv;
{
    extern int optind;
    extern char *optarg;
    int argch;

    while ( (argch=getopt(argc,argv,"ABCN:Ldhv?")) != EOF ) {
	switch (argch) {
	    case 'd': /* Place in background */ 
                if (!fork()) {
		    (void) signal(SIGINT, SIG_IGN);
		    (void) signal(SIGQUIT, SIG_IGN);
		    daemonize();
		}
		exit(0);
            case 'N': /* Output in nag format  */
	        nagflag++;
		if (optind > argc) {
		    (void) strcpy(nagprog, "echo");
		} else {
		    nagprog = optarg;
		}
            case 'B': /* Output Time and Event */
                get_current_date();
 	        read_schedule();
                psched2();
                exit(0);
                break;
            case 'C': /* Output in calendar format  */
		calflag++;
            case 'A': /* Output in APPT format */
                get_current_date();
	        read_schedule();
                scan_every_event_for_appt(); 
                exit(0);
                break;
	    case 'L': /* Output Lunar display only */
		init();
                get_current_date();
		++lflag;
		lunar();
		exit(0);
            case 'v': /* Output Version Info */
	        (void) fprintf(stderr, "  MONTH [version %s]\n", VERSION);
		sleep(3);
                break;
	    case '?':
	    case 'h':
	    default:
  	 	(void) printf("Usage: %s [-A] [-B] [-C] [-L] [-d] [-N [arg]] [-v]\n", *argv);
  		(void) printf("              |    |    |    |    |    |    |     |\n");
  	 	(void) printf("              |    |    |    |    |    |    |     +-- version id\n");
  	 	(void) printf("              |    |    |    |    |    |    +-- program to \"nag\" with\n");
  	 	(void) printf("              |    |    |    |    |    +-- .nag format\n");
  	 	(void) printf("              |    |    |    |    +-- daemonize\n");
		(void) printf("              |    |    |    +-- lunar display\n");
  	 	(void) printf("              |    |    +-- calendar format\n");
  	 	(void) printf("              |    +-- today's events\n");
  	 	(void) printf("              +-- .appt format\n");
		terminate();
        }
    }
    init();
    print_screen();
    start_display();
    user();
    terminate();
}

init()
{
	int blast_out(), i;

	(void) signal(SIGINT, blast_out);
	(void) signal(SIGQUIT, blast_out);
	get_current_date();
	read_schedule();

	for (i = 0; i < 12; i++) {
		mdates[i].month = this_month;
		mdates[i].year = this_year;
	}
	initscr();
	SCHEDULE_ROW = (LINES < 25) ? 17 : 18;
	SKID_ROW = 18;
	initd = 1;
	crmode();
	noecho();
}

terminate()
{
	if (updating) {
		return;
	}
	(void) signal(SIGINT, SIG_IGN);
#ifdef  BSD
	(void) signal(SIGTSTP, SIG_IGN);
#endif
	(void) signal(SIGQUIT, SIG_IGN);
	(void) signal(SIGHUP, SIG_IGN);

	if (initd) {
		if (update_schedule) {
			mvaddstr(0, 0, "updating schedule\t");
		} else {
			mvaddstr(0, 0, "schedule unchanged\t");
		}
		refresh();
		if (update_schedule) {
			if (write_schedule() == -1) {
				sound_bell();
				mvaddstr(0, 0, "cannot create .month");
			}
		}
	move(LINES-1, 0);
	clrtoeol();
	refresh();
	endwin();
	}
	exit(0);
}

blast_out()
{
	update_schedule = 0;
	terminate();
}

daemonize()
{
	int do_nothing();
	struct event_rec *eptr;
	short minutes, eminutes, diff;
	unsigned short seconds;

	(void) fflush(stdout);
AGAIN:
	get_current_date();
	minutes = (60 * dhour) + dminute;

	seconds = (60 * (15 - (dminute % 15) - 1)) + (60 - dsecond);
	if (seconds < 60) {
		seconds = 900;
	}
	(void) signal(SIGALRM, do_nothing);
	(void) alarm(seconds);
	if (!logged_in()) {
		terminate();
	}
	read_schedule();

	eptr = events.next_event;

	while (eptr) {

		if (event_matches_date(eptr)) {
			eminutes = (((short)60) * ((short)eptr->hour)) +
			    ((short)eptr->minute);
			diff = eminutes - minutes;
			if ((diff >= 0) && (diff <= 15)) {
				remind(eptr->event_string, diff);
			}
		}
		eptr = eptr->next_event;
	}
	pause();
	goto AGAIN;
}

logged_in()
{
	static struct utmp u_buf;
	struct utmp t_buf;
	int fd, retval = 0;
	static short called_before = 0;
	char *ttyname(), *tname;
AGAIN:
	if ((fd = open("/etc/utmp", 0)) < 0) {
		return(0);
	}
	if (!called_before) {
		tname = ttyname(0) + 5;
	}
	while (read(fd, (char *)&t_buf, sizeof(struct utmp)) > 0) {
		if (!called_before) {
			if (!strcmp(tname, t_buf.ut_line)) {
				u_buf = t_buf;
				break;
			}
		} else if (byte_comp((char *)&u_buf, (char *)&t_buf, sizeof(struct utmp))) {
				(void) close(fd);
				retval = 1;
				break;
		}
	}
	(void) close(fd);
	if (!called_before) {
		called_before = 1;
		goto AGAIN;
	}
	return(retval);
}

do_nothing()
{
}

byte_comp(s1, s2, n)
register char *s1, *s2;
register int n;
{
	short i;

	for (i = 0; i < n; i++) {
		if (*(s1++) != *(s2++)) {
			return(0);
		}
	}
	return(1);
}

#ifdef BSD
/*  File   : getopt.c
    Author : Henry Spencer, University of Toronto
    Updated: 28 April 1984
    Purpose: get option letter from argv.
*/

extern char *index();

#ifndef	NullS
#define	NullS	(char*)0
#endif	NullS

char	*optarg;	/* Global argument pointer. */
int	optind = 0;	/* Global argv index. */

int getopt(argc, argv, optstring)
    int argc;
    char *argv[];
    char *optstring;
    {
	register int c;
	register char *place;
	static char *scan = NullS;	/* Private scan pointer. */

	optarg = NullS;

	if (scan == NullS || *scan == '\0') {
	    if (optind == 0) optind++;
	    if (optind >= argc) return EOF;
	    place = argv[optind];
	    if (place[0] != '-' || place[1] == '\0') return EOF;
	    optind++;
	    if (place[1] == '-' && place[2] == '\0') return EOF;
	    scan = place+1;
	}

	c = *scan++;
	place = index(optstring, c);
	if (place == NullS || c == ':') {
	    (void) fprintf(stderr, "%s: unknown option %c\n", argv[0], c);
	    return '?';
	}
	if (*++place == ':') {
	    if (*scan != '\0') {
		optarg = scan, scan = NullS;
	    } else {
		optarg = argv[optind], optind++;
	    }
	}
	return c;
    }
#endif BSD
