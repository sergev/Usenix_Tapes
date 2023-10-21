#include <curses.h>
#include "month.h"

struct event_rec events = {0, 0, 0, 0, 0, 0, {0, 0, 0, 0, 0, 0, 0},
0, 0, 0, 0, 0, 0, 0, 0, 0};
struct event_rec current_event = {0, 0, 0, 0, 0, 0, {0, 0, 0, 0, 0, 0, 0},
1, 0, 0, 10, 0, 1, 0, "", 0};
short update_schedule = 0;
short updating = 0, edit_flag = 0, put_into_schedule, parsed_correctly;
char schedule_file_name[75];
short SCHEDULE_ROW;

char *grids[] = {
"mid.  .  .  1a .  .  .  2a .  .  .  3a .  .  .  4a .  .  .  5a .  .  .  6a",
"6a .  .  .  7a .  .  .  8a .  .  .  9a .  .  .  10a.  .  .  11a.  .  .  noon",
"noon  .  .  1p .  .  .  2p .  .  .  3p .  .  .  4p .  .  .  5p .  .  .  6p",
"6p .  .  .  7p .  .  .  8p .  .  .  9p .  .  .  10p.  .  .  11p.  .  .  mid"
};
extern short month, day, year, days;

post_event(month, day, year)
short month, day, year;
{
	goto_schedule();
	current_event.event_month = month;
	current_event.event_day = day;
	current_event.event_year = year;
	display_event(&current_event);
}

read_schedule()
{
	char *getenv(), *malloc(), *s;
	int fd, rec_size;
	struct event_rec event_buf, *event_ptr, *chain_ptr;

	chain_ptr = events.next_event;	/* free old events */
	while (chain_ptr) {
		event_ptr = chain_ptr;
		chain_ptr = chain_ptr->next_event;
		free(event_ptr);
	}
	events.next_event = 0;

	if (!(s = getenv("HOME"))) {
		s = ".";
	}
	strcpy(schedule_file_name, s);
	strcat(schedule_file_name, "/.month");

	rec_size = sizeof(struct event_rec);

	if ((fd = open(schedule_file_name, 0)) != -1) {

		chain_ptr = &events;

		while (read(fd, &event_buf, rec_size) == rec_size) {
			if (event_ptr = (struct event_rec *) malloc(rec_size)) {
				chain_ptr->next_event = event_ptr;
				chain_ptr = event_ptr;
				*chain_ptr = event_buf;
				chain_ptr->next_event = (struct event_rec *)0;
			} else {
				break;
			}
		}
		close(fd);
	}
}

write_schedule()
{
	int fd;
	struct event_rec *chain_ptr;

	updating = 1;

	if ((fd = creat(schedule_file_name, 0640)) == -1) {
		return(-1);
	}
	chain_ptr = events.next_event;

	while (chain_ptr) {
		if (!is_passed_event(chain_ptr)) {
			write(fd, (char *)chain_ptr, sizeof(struct event_rec));
		}
		chain_ptr = chain_ptr->next_event;
	}
	close(fd);
	updating = 0;
	return(0);
}

select_regularity_col(col)
register col;
{
	short i, hflag;

	switch(col) {
	case MONTHLY_COL:
		hl_all(&current_event, (current_event.monthly ? 0 : 1),
		    0, -1, -1, -1, -1);
		break;
	case YEARLY_COL:
		hl_all(&current_event, 0, (current_event.yearly ? 0 : 1),
		    -1, -1, -1, -1);
		break;
	case EVERY_COL:
		hl_all(&current_event, -1, -1,
		   (current_event.every ? 0 : 1), -1, -1, -1);
		break;
	case SMTWTFS_COL:
	case SMTWTFS_COL + 3:
	case SMTWTFS_COL + 6:
	case SMTWTFS_COL + 9:
	case SMTWTFS_COL + 12:
	case SMTWTFS_COL + 15:
	case SMTWTFS_COL + 18:
		i = (col - SMTWTFS_COL) / 3;
		hflag = (current_event.smtwtfs[i] = !current_event.smtwtfs[i]);
		hl_schedule(col, hflag);
		hl_all(&current_event, -1, -1, -1, -1, -1, -1);
		break;
	case NTH_COL:
		hl_all(&current_event, -1, -1, -1,
		    (current_event.nth_is_on ? 0 : 1), -1, -1);
		break;
	case LAST_COL:
		hl_all(&current_event, -1, -1, -1, -1,
		    (current_event.last ? 0 : 1), -1);
		break;
	}
}

accept_cancel(is_accept)
short is_accept;
{
	if (is_accept) {
		accept_current_event();
	} else {
		cancel_current_event();
		display_event(&current_event);
	}
	goto_day(day);
}

accept_current_event()
{
	if ((parse_event(&current_event) != -1)) {
		if (get_answer("Put into schedule? ", "done", "change cancelled")
		    == 'y') {
			if (!edit_flag) {
				link_event(&current_event);
			}
			put_into_schedule = 1;
		}
	}
}

cancel_current_event()
{
	if (!edit_flag) {
		current_event = events;
	}
}

link_event(event)
struct event_rec *event;
{
	struct event_rec *t, *ptr;
	char *malloc();

	if (!(ptr = (struct event_rec *)
	    malloc(sizeof(struct event_rec)))) {
		return;
	}
	*ptr = *event;
	t = events.next_event;
	events.next_event = ptr;
	ptr->next_event = t;
	update_schedule++;
}

parse_event(event)
struct event_rec *event;
{
	short hs;

	hs = has_smtwtfs(event->smtwtfs);

	if ((event->every || event->nth_is_on || event->last) && !hs) {
		error_message("missing day of week", 1);
		return(-1);
	}
	if (hs && !event->every && !event->nth_is_on && !event->last) {
MQ:		error_message("missing qualifier", 1);
		return(-1);
	}
	if (!event->every &&
	    (event->monthly || event->yearly) &&
		(event->nth_is_on || event->last)) {
		error_message("need 'every'", 1);
		return(-1);
	}
	if (event->last && !event->monthly && !event->yearly) {
		error_message("monthly or yearly?", 1);
		return(-1);
	}
	if ((event->nth_is_on || event->last) &&
	    (!event->monthly && !event->yearly && !event->every)) {
		goto MQ;
	}
	parsed_correctly = 1;
	return(0);
}


overview()
{
	short i, j, row, col, hour, minute, duration, n;
	struct event_rec *events_today[MAX_DAILY_EVENTS];
	char *grid;

	get_daily_events(events_today);

	clear_schedule_area();

	for (i = 0; i < 4; i++) {
		mvaddstr((SCHEDULE_ROW + i + i), 1, grids[i]);
	}

	standout();
	i = 0;

	while (events_today[i]) {

		hour = events_today[i]->hour;
		minute = events_today[i]->minute;

		row = SCHEDULE_ROW + ((hour / 6) * 2);

		duration = (events_today[i]->duration_hours * 60) +
		    events_today[i]->duration_minutes;
		col = 1 + (12 * (hour % 6)) + (3 * (minute / 15));
		n = hour / 6;
		grid = grids[n];
		duration /= 15;

		move(row, col);

		for (j = 0; j < duration; j++) {
			addch(grid[col - 1]);
			addch(grid[col]);
			addch(grid[col + 1]);

			col += 3;
			if (col > 72) {
				col = 1;
				row += 2;

				if (row > (SCHEDULE_ROW + 6)) {
					row = SCHEDULE_ROW;
				}
				move(row, col);
				grid = grids[++n % 4];
			}
		}
		i++;
	}
	standend();
}

get_daily_events(events_today)
struct event_rec *events_today[];
{
	short i = 0;
	struct event_rec *eptr;

	eptr = events.next_event;

	while (eptr && (i < (MAX_DAILY_EVENTS - 1))) {
		if (event_matches_date(eptr)) {
			if (!events_today) {
				return(1);
			}
			events_today[i++] = eptr;
		}
		eptr = eptr->next_event;
	}
	if (events_today) {
		events_today[i] = 0;
	}
	return(i);
}

scan_today_events()
{
	struct event_rec *events_today[MAX_DAILY_EVENTS];
	short i, j, k, ch, n;

	if ((n = get_daily_events(events_today)) <= 0) {
		error_message("No events this day", 0);
		clear_schedule_area();
		return;
	}
	sort_events(events_today, n);

	for (i = 0; i < n; i++) {
		current_event = *events_today[i];
		display_event(events_today[i]);
GETCH:
		ch = get_npdeq();

		switch(ch) {
		case '\0':
			i--;
			break;
		case '\033':
		case 'q':
			goto OUT;
		case 'n':
			j = i + 1;
			while ((j < n) && (!events_today[j])) {
				j++;
			}
			if (j >= n) {
				/*sound_bell();
				goto GETCH;*/
				goto OUT;
			}
			i = j - 1;
			break;
		case 'p':
			j = i - 1;
			while ((j >= 0) && (!events_today[j])) {
				j--;
			}
			if (j < 0) {
				sound_bell();
				goto GETCH;
			}
			i = j - 1;
			break;
		case 'd':
			delete_event(events_today[i]);
			events_today[i] = 0;
			for (k = i+1; k < n; k++) {
				if (events_today[k] != 0) {
					i = k;
					break;
				}
			}
			if (events_today[i] == 0) {
				for (k = i-1; k >= 0; k--) {
					if (events_today[k] != 0) {
						i = k;
						break;
					}
				}
			}
			if (events_today[i] != 0) {
				i--;
			} else {
				goto OUT;
			}
			break;
		case 'e':
EDIT:		goto_schedule();
			edit_flag = 1;
			parsed_correctly = 0;
			put_into_schedule = 0;
			if (user() == ACCEPT) {
				if (parsed_correctly && put_into_schedule) {
					*events_today[i] = current_event;
					update_schedule++;
				} else if (!parsed_correctly) {
					goto EDIT;
				}
			} else {
				display_event(events_today[i]);
			}
			edit_flag = 0;
			i--;
			break;
		}
	}
OUT:	goto_day(day);
}

scan_every_event()
{
	register short ch;
	struct event_rec *ptr;

	ptr = events.next_event;

	while (ptr) {
		current_event = *ptr;
		display_event(ptr);

		ch = get_nq();

		switch(ch) {
			case 'n':
				ptr = ptr->next_event;
				break;
			case 'q':
				goto RET;
				break;
		}
	}
RET: ;
}

delete_event(event)
struct event_rec *event;
{
	struct event_rec *ptr;

	ptr = &events;

	while (ptr && (ptr->next_event != event)) {
		ptr = ptr->next_event;
	}
	if (ptr) {
		ptr->next_event = ptr->next_event->next_event;
		free((char *)event);
	}
	update_schedule++;
}

sort_events(e, n)
register struct event_rec *e[];
int n;
{
	register struct event_rec *t;
	register i, j;
	short f;

	for (i = 0; i < n; i++) {
		for (j = (n - 1), f = 0; j > 0; j--) {
			if ((e[j]->hour < e[j-1]->hour) ||
			    ((e[j]->hour == e[j-1]->hour) &&
			    (e[j]->minute < e[j-1]->minute))) {
				t = e[j];
				e[j] = e[j-1];
				e[j-1] = t;
				f++;
			}
		}
		if (f == 0) {
			break;
		}
	}
}

show_all_events(month, year)
register month, year;
{
	register struct event_rec *eptr;
	short i;
	char match_list[32];
	short tday;

	tday = day;

	for (i = 1; i <= days; i++) {

		eptr = events.next_event;
		match_list[i] = 0;
		day = i;

		while (eptr) {
			if (event_matches_date(eptr)) {
				match_list[i] = 1;
				break;
			}
			eptr = eptr->next_event;
		}
	}
	day = tday;
	print_cal(month, year, match_list);
}
