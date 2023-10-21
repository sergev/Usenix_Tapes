#include <curses.h>
#include <signal.h>
#include <utmp.h>
#include "month.h"

short initialized = 0;
short dhour, dminute, dsecond;
extern short crow, ccol, update_schedule, updating;
extern short this_month, this_day, this_year, SCHEDULE_ROW;
extern struct event_rec events;
extern struct mdate mdates[];

main(argc, argv)
int argc;
char *argv[];
{
	check_args(argc, argv);
	initialize();
	print_screen();
	start_display();
	user();
	terminate();
}

initialize()
{
	int blast_out(), i;

	signal(SIGINT, blast_out);
	signal(SIGQUIT, blast_out);
	get_current_date();
	read_schedule();

	for (i = 0; i < 12; i++) {
		mdates[i].month = this_month;
		mdates[i].year = this_year;
	}
	initscr();
	SCHEDULE_ROW = (LINES < 24) ? 17 : 18;
	initialized = 1;
	crmode();
	noecho();
}

terminate()
{
	if (updating) {
		return;
	}
	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	if (initialized) {
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
	}
	move(LINES-1, 0);
	clrtoeol();
	refresh();
	endwin();
	exit(0);
}

blast_out()
{
	update_schedule = 0;
	terminate();
}

check_args(argc, argv)
int argc;
char *argv[];
{
	short i, daemon = 0;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch(argv[i][1]) {
			case 'd':
				daemon = i;
				break;
			default:
				goto BA;
			}
		} else {
BA:			printf("Bad argument: %s\n", argv[i]);
			terminate();
		}
	}
	if (daemon) {
		if (!fork()) {
			signal(SIGINT, SIG_IGN);
			signal(SIGQUIT, SIG_IGN);
			signal(SIGTSTP, SIG_IGN);
			daemonize();
		}
		exit(0);
	}
}

daemonize()
{
	int do_nothing();
	struct event_rec *eptr;
	short minutes, eminutes, diff, seconds;

	printf("daemon started\n");
	fflush(stdout);
AGAIN:
	get_current_date();
	minutes = (60 * dhour) + dminute;

	seconds = (60 * (15 - (dminute % 15) - 1)) + (60 - dsecond);
	if (seconds < 60) {
		seconds = 900;
	}
	signal(SIGALRM, do_nothing);
	alarm(seconds);
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
	while (read(fd, &t_buf, sizeof(struct utmp)) > 0) {
		if (!called_before) {
			if (!strcmp(tname, t_buf.ut_line)) {
				u_buf = t_buf;
				break;
			}
		} else if (byte_comp(&u_buf, &t_buf, sizeof(struct utmp))) {
				close(fd);
				retval = 1;
				break;
		}
	}
	close(fd);
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
