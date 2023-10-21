/*
 * Modification History 
 *
 * Origional Author:  Tom Stoehn@Tektronics[zeus!tims] Modifications by:  Marc
 * Ries@TRW[trwrb!ries] 
 *
 */

#include <curses.h>
#include "month.h"
#include <sys/types.h>
#include <sys/file.h>

extern char *strcpy(), *strcat();
extern char *getenv(), *malloc();

#ifndef F_OK
#define F_OK	00
#endif

struct event_rec events = {0, 0, 0, 0, 0, 0, {0, 0, 0, 0, 0, 0, 0},
			   0, 0, 0, 0, 0, 0, 0, 0, 0};
struct event_rec current_event = {0, 0, 0, 0, 0, 0, {0, 0, 0, 0, 0, 0, 0},
				  1, 0, 0, 10, 0, 1, 0, "", 0};
short           update_schedule = 0;
short           updating = 0, edit_flag = 0, put_into_schedule, parsed_correctly;
char            schedule_file_name[75];
short           SCHEDULE_ROW, SKID_ROW;

char           *grids[] = {
"mid.  .  .  1a .  .  .  2a .  .  .  3a .  .  .  4a .  .  .  5a .  .  .  6a",
			   "6a .  .  .  7a .  .  .  8a .  .  .  9a .  .  .  10a.  .  .  11a.  .  .  noon",
"noon  .  .  1p .  .  .  2p .  .  .  3p .  .  .  4p .  .  .  5p .  .  .  6p",
"6p .  .  .  7p .  .  .  8p .  .  .  9p .  .  .  10p.  .  .  11p.  .  .  mid"
};
extern short    month, day, year, days, message_line_filled;

post_event(month, day, year)
	short           month, day, year;
{
	goto_schedule();
	current_event.event_month = month;
	current_event.event_day = day;
	current_event.event_year = year;
	display_event(&current_event);
}

read_schedule()
{
char            *s;
int             fd, rec_size;
struct event_rec *event_ptr, *chain_ptr;

	chain_ptr = events.next_event;	/* free old events */
	while (chain_ptr)
	{
		event_ptr = chain_ptr;
		chain_ptr = chain_ptr->next_event;
		free((char *)event_ptr);
	}
	events.next_event = 0;

	if (!(s = getenv("HOME")))
	{
		s = ".";
	}
	(void) strcpy(schedule_file_name, s);
	(void) strcat(schedule_file_name, "/.month");

	rec_size = sizeof(struct event_rec);

	if ((fd = open(schedule_file_name, 0)) != -1)
	{

		chain_ptr = &events;

		for (;;)
		{
			if ((!(event_ptr = (struct event_rec *) malloc(rec_size))) ||
			    (read(fd, (char *)event_ptr, rec_size) != rec_size))
			{
				break;
			}
			chain_ptr->next_event = event_ptr;
			chain_ptr = event_ptr;
			chain_ptr->next_event = (struct event_rec *) 0;
		}
		(void) close(fd);
	}
}

write_schedule()
{
int             fd;
struct event_rec *chain_ptr;

	updating = 1;

	if ((fd = creat(schedule_file_name, 0640)) == -1)
	{
		return (-1);
	}
	chain_ptr = events.next_event;

	while (chain_ptr)
	{
		if (!is_passed_event(chain_ptr))
		{
			(void) write(fd, (char *) chain_ptr, sizeof(struct event_rec));
		}
		chain_ptr = chain_ptr->next_event;
	}
	(void) close(fd);
	updating = 0;
	return (0);
}

select_regularity_col(col)
	register        col;
{
short           i, hflag;

	switch (col)
	{
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
	short           is_accept;
{
	if (is_accept)
	{
		accept_current_event();
	} else
	{
		cancel_current_event();
		display_event(&current_event);
	}
	if (edit_flag)
	{
		clear_message_line();
	} else
	{
		message_line_filled = 1;
	}
	goto_day(day);
}

accept_current_event()
{
	if ((parse_event(&current_event) != -1))
	{
		if (get_answer("Put into schedule? ", "change cancelled", "done",
			       'n', 'y')
		    == 'y')
		{
			if (!edit_flag)
			{
				link_event(&current_event);
			}
			put_into_schedule = 1;
		}
	}
}

cancel_current_event()
{
	if (!edit_flag)
	{
		current_event = events;
	}
}

link_event(event)
	struct event_rec *event;
{
struct event_rec *t, *ptr;

	if (!(ptr = (struct event_rec *)
	      malloc(sizeof(struct event_rec))))
	{
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
short           hs;

	hs = has_smtwtfs(event->smtwtfs);

	if ((event->every || event->nth_is_on || event->last) && !hs)
	{
		error_message("missing day of week", 1);
		return (-1);
	}
	if (hs && !event->every && !event->nth_is_on && !event->last)
	{
MQ:		error_message("missing qualifier", 1);
		return (-1);
	}
	if (!event->every &&
	    (event->monthly || event->yearly) &&
	    (event->nth_is_on || event->last))
	{
		error_message("need 'every'", 1);
		return (-1);
	}
	if (event->last && !event->monthly && !event->yearly)
	{
		error_message("monthly or yearly?", 1);
		return (-1);
	}
	if ((event->nth_is_on || event->last) &&
	    (!event->monthly && !event->yearly && !event->every))
	{
		goto MQ;
	}
	parsed_correctly = 1;
	return (0);
}


overview()
{
short           i, j, row, col, hour, minute, span, n;
struct event_rec *event_list[MAX_EVENTS];
char           *grid;

	(void) get_daily_events(event_list);

	clear_schedule_area();

	for (i = 0; i < 4; i++)
	{
		mvaddstr((SCHEDULE_ROW + i + i), 1, grids[i]);
	}

	standout();
	i = 0;

	while (event_list[i])
	{

		hour = event_list[i]->hour;
		minute = event_list[i]->minute;

		row = SCHEDULE_ROW + ((hour / 6) * 2);

		if (row > (LINES - 1))
		{
			break;
		}
		span = (event_list[i]->span_hours * 60) +
			event_list[i]->span_minutes;
		col = 1 + (12 * (hour % 6)) + (3 * (minute / 15));
		n = hour / 6;
		grid = grids[n];
		span /= 15;

		move(row, col);

		for (j = 0; j < span; j++)
		{
			addch(grid[col - 1]);
			addch(grid[col]);
			addch(grid[col + 1]);

			col += 3;
			if (col > 72)
			{
				col = 1;
				row += 2;

				if (row > (SCHEDULE_ROW + 6))
				{
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

get_daily_events(event_list)
	struct event_rec *event_list[];
{
short           i = 0;
struct event_rec *eptr;

	eptr = events.next_event;

	while (eptr && (i < (MAX_EVENTS - 1)))
	{
		if (event_matches_date(eptr))
		{
			event_list[i++] = eptr;
		}
		eptr = eptr->next_event;
	}
	event_list[i] = 0;
	return (i);
}

get_every_event(event_list)
	struct event_rec *event_list[];
{
short           i = 0;
struct event_rec *eptr;

	eptr = events.next_event;

	while (eptr && (i < (MAX_EVENTS - 1)))
	{
		event_list[i++] = eptr;
		eptr = eptr->next_event;
	}
	event_list[i] = 0;
	return (i);
}

scan_events(schar)
	int             schar;
{
struct event_rec *event_list[MAX_EVENTS];
short           i, j, k, ch, n;

	if (schar == 'S')
	{			/* scan todays events */
		if ((n = get_daily_events(event_list)) <= 0)
		{
			error_message("No events this day", 0);
			clear_schedule_area();
			return;
		}
	} else
	{			/* scan all events */
		if ((n = get_every_event(event_list)) <= 0)
		{
			error_message("No events", 0);
			clear_schedule_area();
			return;
		}
	}
	sort_events(event_list, n);

	for (i = 0; i < n; i++)
	{
		current_event = *event_list[i];
		display_event(event_list[i]);
GETCH:
		ch = get_npdeq();

		switch (ch)
		{
		case '\0':
			i--;
			break;
		case '\033':
		case 'q':
			goto OUT;
		case 'n':
			j = i + 1;
			while ((j < n) && (!event_list[j]))
			{
				j++;
			}
			if (j >= n)
			{
				/*
				 * sound_bell(); goto GETCH; 
				 */
				goto OUT;
			}
			i = j - 1;
			break;
		case 'p':
			j = i - 1;
			while ((j >= 0) && (!event_list[j]))
			{
				j--;
			}
			if (j < 0)
			{
				sound_bell();
				goto GETCH;
			}
			i = j - 1;
			break;
		case 'd':
			delete_event(event_list[i]);
			event_list[i] = 0;
			for (k = i + 1; k < n; k++)
			{
				if (event_list[k] != 0)
				{
					i = k;
					break;
				}
			}
			if (event_list[i] == 0)
			{
				for (k = i - 1; k >= 0; k--)
				{
					if (event_list[k] != 0)
					{
						i = k;
						break;
					}
				}
			}
			if (event_list[i] != 0)
			{
				i--;
			} else
			{
				goto OUT;
			}
			break;
		case 'e':
	EDIT:		goto_schedule();
			edit_flag = 1;
			parsed_correctly = 0;
			put_into_schedule = 0;
			if (user() == ACCEPT)
			{
				if (parsed_correctly && put_into_schedule)
				{
					*event_list[i] = current_event;
					update_schedule++;
				} else if (!parsed_correctly)
				{
					goto EDIT;
				}
			} else
			{
				display_event(event_list[i]);
			}
			edit_flag = 0;
			i--;
			break;
		}
	}
OUT:	goto_day(day);
}

delete_event(event)
	struct event_rec *event;
{
struct event_rec *ptr;

	ptr = &events;

	while (ptr && (ptr->next_event != event))
	{
		ptr = ptr->next_event;
	}
	if (ptr)
	{
		ptr->next_event = ptr->next_event->next_event;
		free((char *) event);
	}
	update_schedule++;
}

sort_events(e, n)
	register struct event_rec *e[];
int             n;
{
register struct event_rec *t;
register        i, j;
short           f;

	for (i = 0; i < n; i++)
	{
		for (j = (n - 1), f = 0; j > 0; j--)
		{
			if ((e[j]->hour < e[j - 1]->hour) ||
			    ((e[j]->hour == e[j - 1]->hour) &&
			     (e[j]->minute < e[j - 1]->minute)))
			{
				t = e[j];
				e[j] = e[j - 1];
				e[j - 1] = t;
				f++;
			}
		}
		if (f == 0)
		{
			break;
		}
	}
}

show_all_events(month, year)
	register        month, year;
{
register struct event_rec *eptr;
short           i;
char            match_list[32];
short           tday;

	tday = day;

	for (i = 1; i <= days; i++)
	{

		eptr = events.next_event;
		match_list[i] = 0;
		day = i;

		while (eptr)
		{
			if (event_matches_date(eptr))
			{
				if (!eptr->monthly && !eptr->yearly && !eptr->every)
				{
					match_list[i] = 2;
					break;
				} else
				{
					match_list[i] = 1;
				}
			}
			eptr = eptr->next_event;
		}
	}
	day = tday;
	print_cal(month, year, match_list);
}

file_events()
{
struct event_rec *event_list[MAX_EVENTS];
short           i, ch, n;
char            buf[256];
int             fd;

	if ((ch = get_answer("all, or current? [ac] ",
			     "", "", 'a', 'c')) == 'c')
	{
		if ((n = get_daily_events(event_list)) <= 0)
		{
			error_message("No events this day", 0);
			return;
		}
	} else if (ch == 'a')
	{			/* scan all events */
		if ((n = get_every_event(event_list)) <= 0)
		{
			error_message("No events", 0);
			return;
		}
	} else
	{
		return;
	}
	if (!get_input_line("file: ", buf))
	{
		return;
	}
	if (!access(buf, F_OK))
	{
		if (get_answer("file already exists, use it? ", "aborted", "OK",
			       'n', 'y') == 'n')
		{
			return;
		}
	}
	if ((fd = creat(buf, 0640)) < 0)
	{
		error_message("permission denied", 1);
	} else
	{
		if (ch == 'c')
		{
			(void) strcpy(buf, "\n\t\t\tEvents for ");
			print_date(event_list[0]->event_month, event_list[0]->event_day,
			       event_list[0]->event_year, buf + strlen(buf));
		} else
		{
			(void) strcpy(buf, "\n\t\t\tAll Events Listing");
		}
		(void) strcat(buf, "\n\n\n");
		(void) write(fd, buf, strlen(buf));
		for (i = 0; i < n; i++)
		{
			(void) strcpy(buf, "    ");
			print_time(event_list[i], buf + strlen(buf));
			(void) strcat(buf, "\n");
			print_span(event_list[i], buf + strlen(buf));
			(void) strcat(buf, "\n   ");
			print_event_description(event_list[i], buf + strlen(buf));
			(void) strcat(buf, "\n\n* * * * * * * * * *\n\n");
			(void) write(fd, buf, strlen(buf));
		}
		(void) close(fd);
		error_message("done", 0);
	}
}

scan_every_event_for_appt()
{
struct event_rec *ptr;

	ptr = events.next_event;

	while (ptr)
	{
		current_event = *ptr;
		display_appt(ptr);

		ptr = ptr->next_event;
	}
}
