#include <curses.h>
#include "month.h"

extern short crow, ccol, current_area, month, day, year, edit_flag;
extern short this_month, this_day, this_year, message_line_filled, days;
extern struct event_rec current_event;
extern struct mdate mdates[];
struct mdate mdates[12];

user()
{
	register short ch;
	short n, m, y;
	char *prompt;
	struct mdate tmp;

	for (;;) {
MR:		move(crow, ccol);
		refresh();

		m = month; y = year;
GETCH:
		ch = get_char();

		switch(ch) {
		case '/':
			date_search();
			break;
		case ';':
			if (current_area != SCHEDULE) {
				goto_this_day(mdates[10].month, 1, mdates[10].year);
				tmp = mdates[10];
				mdates[10] = mdates[11];
				mdates[11] = tmp;
				goto MR;
				}
			break;
		case 'Q':
			if (current_area != SCHEDULE) {
				terminate();
			}
			break;
		case '\033':
			if (edit_flag) {
				return(CANCEL);
			} else if (current_area == SCHEDULE) {
				accept_cancel(0);
			}
			break;
		case 'L':
			if (current_area == DAYS) {
				lunar();
			} else if ((current_area == MONTHS) || (current_area == YEARS)) {
				warn_day();
			}
			break;
		case 'j':
		case 'k':
		case 'l':
		case 'h':
		case 'y':
		case 'm':
		case 'd':
		case '\t':
			move_cursor(ch);
			break;
		case 'n':
		case '+':
		case 'p':
		case '-':
			incr(ch);
			break;
		case '\n':
		case '\r':
			if ((n = selection()) != NOTHING) {
				return(n);
			}
			break;
		case ' ':
		case '\b':
			scroll_time(ch);
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			switch(current_area) {
			case DAYS:
				prompt = "day: ";
				break;
			case MONTHS:
				prompt = "month: ";
				break;
			case YEARS:
				prompt = "year: ";
				break;
			case SCHEDULE:
				switch(ccol) {
				case SMONTH_COL:
					prompt = "month: ";
					break;
				case SDAY_COL:
					prompt = "day: ";
					break;
				case SYEAR_COL:
					prompt = "year: ";
					break;
				case NTH_COL:
					prompt = "nth: ";
					break;
				default:
				    goto GETCH;
				}
				break;
			default:
			    goto GETCH;
			}
			if ((n = enter_number(ch, prompt)) >= 0) {
				do_number(n);
			}
			break;
		case 'P':
			if (current_area != SCHEDULE) {
				post_event(month, day, year);
			}
			break;
		case 'A':
			if (current_area != SCHEDULE) {
				show_all_events(month, year);
			}
			break;
		case 'S':
			if (current_area == DAYS) {
				scan_today_events();
			} else if ((current_area == MONTHS) || (current_area == YEARS)) {
				warn_day();
			}
			break;
		case 'E':
			if (current_area != SCHEDULE) {
				scan_every_event();
			}
			break;
		case 'O':
			if (current_area == DAYS) {
				overview();
			} else if ((current_area == MONTHS) || (current_area == YEARS)) {
				clear_schedule_area();
				warn_day();
			}
			break;
		case 'T':
			if (current_area != SCHEDULE) {
				goto_this_day(this_month, this_day, this_year);
			}
			break;
		case 'M':
		case 'G':
			if (current_area != SCHEDULE) {
				mark_day((ch == 'G'));
			}
			break;
		default:
			goto GETCH;
			break;
		}
		if ((m != month) || (y != year)) {		/* changed months */
			mdates[10] = mdates[11];
			mdates[11].month = month;
			mdates[11].year = year;
		}
	}
}

enter_number(ch, prompt)
short ch;
char *prompt;
{
	char nbuf[6];
	short col, first_col, i = 0;
	int retval = -1;

	first_col = col = strlen(prompt);

	mvaddstr(0, 0, prompt);

	for (;;) {
		if ((ch >= '0') && (ch <= '9')) {
			if ((col < (first_col + 2)) ||
			    (((current_area == YEARS) ||
			    ((current_area == SCHEDULE)) &&
			    (ccol == SYEAR_COL)) &&
			    (col < (first_col + 4)))) {
				nbuf[i++] = ch;
				mvaddch(0, col++, ch);
				refresh();
			}
		} else if ((ch == '\n') || (ch == '\r')) {
			nbuf[i] = 0;
			retval = atoi(nbuf);
			break;
		} else if (ch == '\b') {
			if (col > first_col) {
				i--; col--;
				mvaddch(0, col, ' ');
				move(0, col);
				refresh();
			}
		} else if (ch == '\033') {
			break;
		}
		ch = get_char();
	}
	clear_message_line();
	return(retval);
}

do_number(n)
short n;
{
	if (current_area == YEARS) {
		if (n > 0) {
			shift_years(n - year);
			goto_year(year);
		}
	} else if (current_area == MONTHS) {
		if ((n <= 12) && (n >= 1)) {
			crow = TOP_MONTH_ROW + n;
			selection();
		}
	} else if (current_area == DAYS) {
		if ((n >= 1) && (n <= days)) {
			goto_day(n);
		}
	} else if (current_area == SCHEDULE) {
		switch(ccol) {
		case SMONTH_COL:
			if ((n >= 1) && (n <= 12)) {
				current_event.event_month = n;
			}
			break;
		case SDAY_COL:
			if ((n >= 1) && (n <= days)) {
				current_event.event_day = n;
			}
			break;
		case SYEAR_COL:
			if (n > 0) {
				current_event.event_year = n;
			}
			break;
		case NTH_COL:
				if ((n > 0) && (n <= 53)) {
					current_event.nth = n;
					current_event.nth_is_on = 1;
					print_nth(&current_event);
				}
			break;
		default:
			return;
		}
		if (ccol != NTH_COL) {
			print_date(current_event.event_month,
			current_event.event_day, current_event.event_year);
		}
	}
}

handle_event_description()
{
	short ch;

	for (;;) {
		move(crow, ccol);
		refresh();
		ch = get_char();

		if ((ch >= ' ') && (ch <= '~')) {
			if (ccol <= 78) {
				addch(ch);
				current_event.event_string[ccol-TIME_COL] = ch;
				ccol++;
				current_event.event_string[ccol-TIME_COL] = 0;
			}
		} else {
			switch(ch) {
			case '\n':
			case '\r':
			case '\t':
				schedule_move_cursor(ch);
				return;
				break;
			case '\b':
				if (ccol > TIME_COL) {
					ccol--;
					mvaddch(crow, ccol, ' ');
					current_event.event_string
					    [ccol-TIME_COL] = 0;
				}
				break;
			case '\004':
			case '\025':
			case '\030':
				current_event.event_string[0] = 0;
				ccol = TIME_COL;
				move(crow, TIME_COL);
				clrtoeol();
				break;
			case '\027':
				while ((ccol > TIME_COL) && (current_event.event_string
					[--ccol - TIME_COL] == ' ')) ;
				while ((ccol > TIME_COL) && (current_event.event_string
					[(ccol - 1) - TIME_COL] != ' ')) {
					ccol--;
				}
				move(crow, ccol);
				clrtoeol();
				break;
			}
		}
	}
}

get_answer(prompt, yes_string, no_string)
char *prompt, *yes_string, *no_string;
{
	char *s;
	short retval, col;

	mvaddstr(0, 0, prompt);
	refresh();

	retval = get_char();

	s = (retval == 'y') ? yes_string : no_string;
	mvaddstr(0, 0, s);
	col = strlen(s);

	if (edit_flag) {
		addstr(" -more-");
		move(0, col + 7);
		blank_out(col + 7);
		move(0, col + 7);
		refresh();
		get_char();
	} else {
		blank_out(col);
	}
	return(retval);
}

get_nq()
{
	register ch;

	mvaddstr(0, 0, "[n,q] ");
	refresh();

	for (;;) {
		ch = get_char();

		switch(ch) {
		case 'q':
		case '\033':
			clear_message_line();
			return('q');
			break;
		case 'n':
		case '\n':
		case '\r':
			clear_message_line();
			return('n');
			break;
		}
	}
}

get_npdeq()
{
	register ch;

	mvaddstr(0, 0, "[n,p,d,e,q] ");
	refresh();

	for (;;) {
		ch = get_char();

		switch(ch) {
		case '\n':
		case '\r':
			ch = 'n';		/* no break statement */
		case 'n':
		case 'p':
		case 'e':
		case '\033':
		case 'q':
			clear_message_line();
			return(ch);
			break;
		case 'd':
			ch = get_answer("really delete? ", "", "");
			if (ch == 'y') {
				mvaddstr(0, 0, "deleted");
			} else {
				mvaddstr(0, 0, "not deleted");
			}
			addstr(" -more-");
			refresh();
			get_char();
			clear_message_line();
			return((ch == 'y') ? 'd' : '\0');
			break;
		}
	}
}

mark_day(goto_flag)
short goto_flag;
{
	short ch;

	mvaddstr(0, 0, "mark: ");
	refresh();

	for (;;) {
		ch = get_char();

		switch(ch) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':

			mvaddch(0, 6, ch);
			refresh();
			message_line_filled = 1;

			ch -= '0';

			if (goto_flag) {
				goto_this_day(mdates[ch].month, 1, mdates[ch].year);
			} else {
				mdates[ch].month = month;
				mdates[ch].year =  year;
			}
			return;
			break;
		case '\033':
		case '\n':
		case '\r':
			clear_message_line();
			return;
			break;
		}
	}
}

get_char()
{
	register int ch;
GETCH:
	ch = getchar() & 0377;

	if (message_line_filled) {
		clear_message_line();
	}

	switch(ch) {
	case '\014':
	case '\022':
		wrefresh(curscr);
		goto GETCH;
		break;
	}
	return(ch);
}

date_search()
{
	register short ch;
	short col, slash_count;
	char buf[16];
	int i = 0, m, d, y;

STARTOVER:

	mvaddstr(0, 0, "date: ");
	buf[col = 0] = 0;
	slash_count = 0;

	for (;;) {
		refresh();
GETCH:
		ch = get_char();

		switch(ch) {
		case '\b':
			if (col > 0) {
				if (buf[col - 1] == '/') {
					slash_count--;
				}
				buf[--col] = 0;
				mvaddch(0, col + 6, ' ');
				move(0, col + 6);
			}
			break;
		case '/':
			if ((slash_count >= 2) || ((col > 0) && (buf[col-1] == '/')) ||
				(col == 0)) {
				sound_bell();
			} else {
				addch(ch);
				buf[col++] = ch;
				buf[col] = 0;
				slash_count++;
			}
			break;
		case '0':
			if ((col == 0) || (buf[col-1] == '/')) {
				goto GETCH;
			}					/* no break statement */
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (
			((col == 2) && (buf[col-1] != '/')) ||
			((slash_count == 1)&&(buf[col-1] != '/')&&(buf[col-2] != '/'))||
			((slash_count == 2) && (buf[col-1] != '/') && (buf[col-2] != '/') &&
				(buf[col-3] != '/') && (buf[col-4] != '/')) ||
			((col == 1) && ((ch > '2') || (buf[0] > '1')))
			) {
				sound_bell();
				goto GETCH;
			} else {
				addch(ch);
				buf[col++] = ch;
				buf[col] = 0;
			}
			break;
		case '\n':
		case '\r':
			if ((slash_count < 2) || (buf[col-1] == '/')) {
				sound_bell();
				goto GETCH;
			} else {
				sscanf(buf, "%d/%d/%d", &m, &d, &y);
				if (y < 100) {
					y += 1900;
				}
				goto_this_day(m, d, y);	/* no break statement! */
			}
		case '\033':
			goto RET;
			break;
		case '\004':
		case '\025':
		case '\030':
			clear_message_line();
			goto STARTOVER;
			break;
		default:
			goto GETCH;
			break;
		}
	}
RET:
	clear_message_line();
}

warn_day()
{
	error_message("put cursor on day", 1);
}
