#include <curses.h>
#include "month.h"

short days;
short crow, ccol;
short current_area;
short message_line_filled;
char *blankout = "                        ";
extern short SCHEDULE_ROW;

char *days_of_week =
"Sunday    Monday   Tuesday Wednesday  Thursday    Friday  Saturday";

short schedule_cols[] = {
	0, SMONTH_COL, SDAY_COL, SYEAR_COL, MONTHLY_COL, YEARLY_COL,
	EVERY_COL, NTH_COL, LAST_COL, SMTWTFS_COL,
	SMTWTFS_COL+3, SMTWTFS_COL+6, SMTWTFS_COL+9, SMTWTFS_COL+12,
	SMTWTFS_COL+15, SMTWTFS_COL+18, -1
};

char *month_names[] = {
	"         ",
	"JANUARY  ",
	"FEBRUARY ",
	"MARCH    ",
	"APRIL    ",
	"MAY      ",
	"JUNE     ",
	"JULY     ",
	"AUGUST   ",
	"SEPTEMBER",
	"OCTOBER  ",
	"NOVEMBER ",
	"DECEMBER " ,
	"         "
};

char *smtwtfs_names[] = {
	"Sun",
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat",
};

extern short month, day, year, start_day, edit_flag;
extern short this_month, this_day, this_year;
extern struct event_rec current_event;
short first_year;

print_screen()
{
	print_month(month);
	print_year(year);
	print_day_headers();
	print_cal(month, year, 0);
	print_all_months();
	print_all_years(year);
	hl_month_year(month, 1, year, 1);
}

print_month(month)
int month;
{
	mvaddstr(0, 35, month_names[month]);
}

print_year(year)
int year;
{
	char nbuf[8];

	sprintf(nbuf, "%4d", year);
	mvaddstr(0, 47, nbuf);
}

print_day_headers()
{
	mvaddstr(2, 12, days_of_week);
}

print_cal(month, year, all_events_list)
register month, year;
char *all_events_list;
{
	short i, month_is_current, cday;
	short row = 4, col = 13, standing_out = 0;
	char nbuf[6];

	start_day = get_start_day(month, year);
	days = days_in(month, year);
	if (day > days) {
		day = days;
	} else if (day < 1) {
		day = 1;
	}

	month_is_current = ((month == this_month) && (year == this_year));

	for (i = 1, cday = 1; i <= 42; i++) {

		if ((cday <= days) && (i >= (start_day + 1))) {
			if (all_events_list && all_events_list[cday]) {
				sprintf(nbuf, "(%2d)", cday);
			} else {
				sprintf(nbuf, " %2d ", cday);
			}
			cday++;
		} else {
			strcpy(nbuf, "    ");
		}
		if (month_is_current && ((cday-1) == this_day)) {
			standout();
			standing_out = 1;
			month_is_current = 0;
		}
		mvaddstr(row, col, nbuf);

		if (standing_out) {
			standing_out = 0;
			standend();
		}
		if ((i % 7) == 0) {
			row += 2;
			col = 13;
		} else {
			col += 10;
		}
	}
}

print_all_months()
{
	short i;

	standout();
	for (i = 0; i <= 13; i++) {
		mvaddstr(TOP_MONTH_ROW + i, 0, month_names[i]);
	}
	standend();
}

print_all_years(year)
int year;
{
	short i;
	char nbuf[8];

	first_year = year - 4;
	standout();
	move(YEAR_ROW, YEAR_COL);
	addstr("<<");
	for (i = first_year; i < (first_year + 10); i++) {
		sprintf(nbuf, " %4d ", i);
		addstr(nbuf);
	}
	addstr(">>");
	standend();
}

hl_month_year(month, mflag, year, yflag)
short month, mflag, year, yflag;
{
	short i;

	if (mflag != -1) {
		if (!mflag) {
			standout();
		}
		mvaddstr(TOP_MONTH_ROW + month, 0, month_names[month]);
		if (!mflag) {
			standend();
		}
	}
	if (yflag != -1) {
		if (!yflag) {
			standout();
		}
		move(YEAR_ROW, 14 + (6 * (year - first_year)));
		for (i = 0; i < 6; i++) {
			addch(inch());
		}
		if (!yflag) {
			standend();
		}
	}
}

start_display()
{
	goto_day(day);
}

goto_this_day(gmonth, gday, gyear)
int gmonth, gday, gyear;
{
	month = gmonth;
	year = gyear;
	day = gday;

	print_screen();

	switch(current_area) {
	case MONTHS:
		goto_month(month);
		break;
	case DAYS:
		goto_day(day);
		break;
	case YEARS:
		goto_year(year);
		break;
	}
}

goto_month(month)
int month;
{
	crow = TOP_MONTH_ROW + month;
	ccol = 9;
	current_area = MONTHS;
}

goto_day(tday)
short tday;
{
	day = tday;
	get_row_col_from_day(&crow, &ccol, day);
	current_area = DAYS;
}

goto_year(year)
int year;
{
	crow = YEAR_ROW;
	ccol = YEAR_COL + 3 + (6 * (year - first_year));
	current_area = YEARS;
}

goto_schedule()
{
	current_area = SCHEDULE;
	crow = SCHEDULE_ROW;
	ccol = MONTHLY_COL;
}

move_cursor(dir)
register short dir;
{
	short mday, row, col;

	if ((current_area != SCHEDULE) &&
	    ((dir == 'm') || (dir == 'y') || (dir == 'd'))) {
		if (dir == 'm') {
			goto_month(month);
		} else if (dir == 'y') {
			goto_year(year);
		} else {
			goto_day(day);
		}
		return;
	}
	switch (current_area) {
	case MONTHS:
		switch(dir) {
		case 'j':
			if (crow <= (TOP_MONTH_ROW + 12)) {
				crow++;
			}
			break;
		case 'k':
			if (crow > TOP_MONTH_ROW) {
				crow--;
			}
			break;
		}
		break;
	case YEARS:
		switch(dir) {
		case 'h':
			if (ccol > YEAR_COL) {
				if (ccol == (YEAR_COL + 3)) {
					ccol = YEAR_COL;
				} else {
					ccol -= 6;
				}
			} else {
				shift_years(-1);
			}
			break;
		case 'l':
			if (ccol < LAST_YEAR_COL) {
				if (ccol == (LAST_YEAR_COL - 6)) {
					ccol = LAST_YEAR_COL;
				} else if (ccol == YEAR_COL) {
					ccol = YEAR_COL + 3;
				} else {
					ccol += 6;
				}
			} else {
				shift_years(1);
			}
			break;
		}
		break;
	case DAYS:

		row = crow;
		col = ccol;

		switch(dir) {
		case 'h':
			if (col > 15) {
				col -= 10;
			}
			break;
		case 'j':
			if (row < 14) {
				row += 2;
			}
			break;
		case 'k':
			if (row > 4) {
				row -= 2;
			}
			break;
		case 'l':
			if (col < 74 ) {
				col += 10;
			}
			break;
		}
		if ((mday = get_day_from_row_col(row, col)) > 0) {
			day = mday;
			crow = row;
			ccol = col;
		}
		break;
	case SCHEDULE:
		schedule_move_cursor(dir);
		break;
	}
}

schedule_move_cursor(dir)
short dir;
{
	short i;

	switch(dir) {
	case 'H':
		if (crow == SCHEDULE_ROW) {
			ccol = DATE_COL;
		}
		break;
	case 'L':
		if (crow == SCHEDULE_ROW) {
			ccol = SMTWTFS_COL + 18;
		}
		break;
	case 'h':
	case 'l':
		if (crow == SCHEDULE_ROW) {
			i = 0;
			while (schedule_cols[++i] != ccol) ;
			i += ((dir == 'h') ? -1 : 1);
			if (schedule_cols[i] != -1) {
				ccol = schedule_cols[i];
			}
		} else if ((crow == TIME_ROW) || (crow == DURATION_ROW)) {
			ccol = (dir == 'h') ? TIME_COL : MINUTE_COL;
		} else if (crow == ACCEPT_ROW) {
			ccol = (dir == 'h') ? ACCEPT_COL : CANCEL_COL;
		}
		break;
	case '\t':
	case '\n':
	case '\r':
		if (crow == SCHEDULE_ROW) {
			crow += 2;
			ccol = TIME_COL;
		} else if (crow == DESCRIPTION_ROW) {
			crow = ACCEPT_ROW;
			ccol = ACCEPT_COL;
		} else if (crow == ACCEPT_ROW)  {
			crow = SCHEDULE_ROW;
			ccol = MONTHLY_COL;
		} else if (crow == DURATION_ROW) {
			crow = DESCRIPTION_ROW;
			ccol = TIME_COL + strlen(current_event.event_string);
			handle_event_description();
		} else {
			crow++;
		}
		break;
	case '\033':
		goto_day(day);
		break;
	}
}

selection()
{
	short new_year;
	int x;

	switch(current_area) {
		case MONTHS:
			if ((crow - TOP_MONTH_ROW) != month) {
				if (crow <= TOP_MONTH_ROW) {
					hl_month_year(month, 0, 0, -1);
					month = 12;
					shift_years(-1);
					hl_month_year(month, 1, 0, -1);
					crow = TOP_MONTH_ROW + 13;
				} else if (crow > (TOP_MONTH_ROW + 12)) {
					hl_month_year(month, 0, 0, -1);
					month = 1;
					shift_years(1);
					hl_month_year(month, 1, 0, -1);
					crow = TOP_MONTH_ROW;
				} else {
					hl_month_year(month, 0, 0, -1);
					month = crow - TOP_MONTH_ROW;
					hl_month_year(month, 1, 0, -1);
					print_cal(month, year, 0);
				}
				print_month(month);
			}
			break;
		case YEARS:
			if (ccol == YEAR_COL) {
				shift_years(-10);
			} else if (ccol == LAST_YEAR_COL) {
				shift_years(10);
			} else {
				new_year = first_year +
					   ((ccol - (YEAR_COL + 3)) / 6);
				if (new_year != year) {
					hl_month_year(0, -1, year, 0);
					year = new_year;
				}
				print_cal_hl_year(month, year);
			}
			break;
		case SCHEDULE:
			if (crow == SCHEDULE_ROW) {
				select_regularity_col(ccol);
			} else if (crow == ACCEPT_ROW) {
				x = (ccol == ACCEPT_COL) ? ACCEPT : CANCEL;
				accept_cancel(ccol == ACCEPT_COL);
				if (edit_flag) {
					return(x);
				}
			} else {
				move_cursor('\t');
			}
			break;
	}
	return(NOTHING);
}

shift_years(shift)
short shift;
{
	if (((year + shift) < (first_year + 10)) &&
	    ((year + shift) >= first_year)) {
		hl_month_year(0, -1, year, 0);
		year += shift;
		hl_month_year(0, -1, year, 1);
	} else {
		year += shift;
		print_all_years(first_year + shift + 4);
	}
	print_cal_hl_year(month, year);
}

print_cal_hl_year(month, year)
int month, year;
{
	print_cal(month, year, 0);
	print_year(year);
	hl_month_year(0, -1, year, 1);
}

get_row_col_from_day(row, col, day)
short *row, *col, day;
{
	*row =  4 + (((start_day + day - 1) / 7) *  2);
	*col = 15 + (((start_day + day - 1) % 7) * 10);
}

get_day_from_row_col(row, col)
short row, col;
{
	short mday;

	mday = (7 * ((row - 4)) / 2) +
	       ((col - 14) / 10) - start_day + 1;

	if ((mday <= days) && (mday > 0)) {
		return(mday);
	}
	return(0);
}

print_event_regularity(event)
struct event_rec *event;
{
	if (event->monthly) {
		standout();
	}
	mvaddstr(SCHEDULE_ROW, MONTHLY_COL, "monthly");
	standend();
	if (event->yearly) {
		standout();
	}
	mvaddstr(SCHEDULE_ROW, YEARLY_COL, "yearly");
	standend();
	if (event->every) {
		standout();
	}
	mvaddstr(SCHEDULE_ROW, EVERY_COL, "every");
	standend();
	print_smtwtfs(event->smtwtfs);
	print_nth(event);
	if (event->last) {
		standout();
	}
	mvaddstr(SCHEDULE_ROW, LAST_COL, "last");
	standend();
}

print_smtwtfs(smtwtfs)
char smtwtfs[];
{
	short i;
	char *s;

	move(SCHEDULE_ROW, SMTWTFS_COL);

	for (i = 0; i < 7; i++) {
		if (smtwtfs[i]) {
			standout();
		}
		addstr(smtwtfs_names[i]);
		if (smtwtfs[i]) {
			standend();
		}
	}
}

hl_schedule(col, hflag)
register col, hflag;
{
	register int ch;
	short i;

	move(SCHEDULE_ROW, col);

	if (hflag) {
		standout();
	}

	if ((col < SMTWTFS_COL) || (col > (SMTWTFS_COL+18))) {
		while((ch = inch()) != ' ') {
			move(SCHEDULE_ROW, col);
			addch(ch);
			col++;
		}
	} else {
		move(SCHEDULE_ROW, col);

		for (i = 0; i < 3; i++) {
			addch(inch());
		}
	}
	standend();
}

display_event(event)
struct event_rec *event;
{
	clear_schedule_area();
	print_date(event->event_month, event->event_day, event->event_year);
	print_event_regularity(event);
	print_time(event);
	print_duration(event);
	print_event_description(event);
	print_accept();
}

print_accept()
{
	mvaddstr(ACCEPT_ROW, TIME_COL, "Accept/Cancel");
}

print_time(event)
struct event_rec *event;
{
	char buf[32];
	short hour;
	char *apm;

	hour = event->hour;
	apm = (hour < 12) ? "AM" : "PM";

	if (hour > 12) {
		hour = hour % 12;
	}
	if (hour == 0) {
		hour = 12;
	}
	sprintf(buf, "time:  %2d:%02d %s", hour, event->minute, apm);
	mvaddstr(TIME_ROW, 4, buf);
}

print_duration(event)
struct event_rec *event;
{
	char buf[32];

	sprintf(buf, "duration:  %2d:%02d", event->duration_hours,
		event->duration_minutes);
	mvaddstr(DURATION_ROW, 0, buf);
}

print_event_description(event)
struct event_rec *event;
{
	char buf[100];

	sprintf(buf, "event:  %s", event->event_string);
	mvaddstr(DESCRIPTION_ROW, 3, buf);
}

scroll_time(dir)
register dir;
{
	short hour, minute, d;

	if (crow == TIME_ROW) {
		hour = current_event.hour;
		minute = current_event.minute;
	} else if (crow == DURATION_ROW) {
		hour = current_event.duration_hours;
		minute = current_event.duration_minutes;
	} else if (crow != SCHEDULE_ROW) {
		return;
	}
	if (ccol == TIME_COL) {
		if (dir == ' ') {
			hour = (hour + 1) % 24;
		} else {
			hour = (hour + 23) % 24;
		}
	} else if (ccol == MINUTE_COL) {
		if (dir == ' ') {
			minute = (minute + 15) % 60;
		} else {
			minute = (minute + 45) % 60;
		}
	} else  {
		if (ccol == SMONTH_COL) {
			current_event.event_month += ((dir == ' ') ? 1 : -1);
			if (current_event.event_month > 12) {
				current_event.event_month = 1;
			} else if (current_event.event_month <= 0) {
				current_event.event_month = 12;
			}
		} else if (ccol == SDAY_COL) {
			d = days_in(current_event.event_month,
			    current_event.event_year);
			current_event.event_day += ((dir == ' ') ? 1 : -1);
			if (current_event.event_day > d) {
				current_event.event_day = 1;
			} else if (current_event.event_day <= 0) {
				current_event.event_day = d;
			}
		} else if (ccol == SYEAR_COL) {
			current_event.event_year += ((dir == ' ') ? 1 : -1);
			if (current_event.event_year < 0) {
				current_event.event_year = this_year;
			}
		} else if (ccol == NTH_COL) {
			current_event.nth += ((dir == ' ') ? 1 : -1);
			if (current_event.nth < 1) {
				current_event.nth = 53;
			} else if (current_event.nth > 53) {
				current_event.nth = 1;
			}
		}
	}
	if (crow == TIME_ROW) {
		current_event.hour = hour;
		current_event.minute = minute;
		print_time(&current_event);
	} else if (crow == DURATION_ROW) {
		current_event.duration_hours = hour;
		current_event.duration_minutes = minute;
		print_duration(&current_event);
	} else if (ccol <= SYEAR_COL) {
		print_date(current_event.event_month,
		    current_event.event_day, current_event.event_year);
	} else if (ccol == NTH_COL) {
		current_event.nth_is_on = 1;
		print_nth(&current_event);

	}
}

hl_all(event, mf, yf, ef, nf, lf, xf)
struct event_rec *event;
register mf, yf, ef, nf, lf, xf;
{
	short i;

	toggle_char(&(event->monthly), mf, MONTHLY_COL);
	toggle_char(&(event->yearly), yf, YEARLY_COL);
	toggle_char(&(event->every), ef, EVERY_COL);
	toggle_char(&(event->nth_is_on), nf, NTH_COL);
	toggle_char(&(event->last), lf, LAST_COL);

	if (xf != -1) {
		for (i = 0; i < 7; i++) {
			toggle_char(&(event->smtwtfs[i]), xf,
			    (SMTWTFS_COL+(3*i)));
		}
	}
}

toggle_char(c, f, col)
char *c;
register f, col;
{
	if (f == 1) {
		if (!(*c)) {
			*c = 1;
			hl_schedule(col, f);
		}
	} else if (!f) {
		if (*c) {
			*c = 0;
			hl_schedule(col, f);
		}
	}
}

print_date(month, day, year)
int month, day, year;
{
	char buf[64];

	standout();
	sprintf(buf, "%2d/%2d/%4d", month, day, year);
	mvaddstr(SCHEDULE_ROW, DATE_COL, buf);
	standend();
}

error_message(str, sound)
char *str;
int sound;
{
	mvaddstr(0, 0, blankout);
	mvaddstr(0, 0, str);
	refresh();
	if (sound) {
		sound_bell();
	}
	message_line_filled = 1;
}

sound_bell()
{
	putchar(7);
	fflush(stdout);
}

clear_schedule_area()
{
	move(SCHEDULE_ROW, 0);
	clrtobot();
}

blank_out(col)
int col;
{
	while (col < 35) {
		addch(' ');
		col++;
	}
}
print_nth(event)
struct event_rec *event;
{
	char *ns, buf[10];
	short n;

	if (event->nth_is_on) {
		standout();
	}
	n = event->nth;
	if (event->nth > 13) {
		n %= 10;
	}

	switch(n) {
	case 0:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
		ns = "th";
		break;
	case 1:
		ns = "st";
		break;
	case 2:
		ns = "nd";
		break;

	case 3:
		ns = "rd";
		break;
	}
	sprintf(buf, "%02d%s", event->nth, ns);
	mvaddstr(SCHEDULE_ROW, NTH_COL, buf);
	standend();
}

remind(str, minutes)
char *str;
int minutes;
{
	char s[50];

	if (minutes > 1) {
		sprintf(s, "In %d minutes", minutes);
	} else if (minutes == 1) {
		strcpy(s, "In 1 minute");
	} else {
		strcpy(s, "Right now");
	}
	printf("\n%s   \n%s.   \n", str, s);
	sound_bell();
}

clear_message_line()
{
	message_line_filled = 0;
	mvaddstr(0, 0, blankout);
}

incr(ch)
int ch;
{
	short inc;

	inc = ((ch == 'n') || (ch == '+')) ? 1 : -1;

	switch (current_area) {
	case MONTHS:
		hl_month_year(month, 0, 0, -1);
		if ((month == 1) && (inc < 0)) {
			shift_years(-1);
			month = 12;
		} else if ((month == 12) && (inc > 0)) {
			shift_years(1);
			month = 1;
		} else {
			month += inc;
		}
		hl_month_year(month, 1, 0, -1);
		print_month(month);
		goto_month(month);
		print_cal(month, year, 0);
		break;
	case DAYS:
		if (((inc > 0) && (day < days)) || ((inc < 0) && (day > 1))) {
			goto_day(day + inc);
		}
		break;
	case YEARS:
		shift_years(inc);
		goto_year(year);
		break;
	}
}
