#include curses.h
#include "object.h"
#include "move.h"

char message_cleared = 1;
char message_line[SCOLS] = "";
short message_col;

extern short cant_int, did_int;

extern short interrupted;

message(msg, intrpt)
char *msg;
short intrpt;
{
	if (intrpt) {
		interrupted = 1;
	}
	cant_int = 1;
	/* slurp(); */

	if (!message_cleared) {
		mvaddstr(MIN_ROW-1, message_col, MORE);
		refresh_vms();
		wait_for_ack(0);
		check_message();
	}
	strcpy(message_line, msg);
	mvaddstr(MIN_ROW-1, 0, msg);
	addch(' ');
	refresh_vms();
	message_cleared = 0;
	message_col = strlen(msg);

	if (did_int) {
/*		onintr(); */
	}
	cant_int = 0;
}

remessage()
{
	if (message_line[0]) {
		message(message_line, 0);
	}
}

check_message()
{
	register i;

	if (message_cleared) {
		return;
	}
	move(MIN_ROW-1, 0);
	clrtoeol();
	move(rogue.row, rogue.col);
	refresh_vms();
	message_cleared = 1;
}

get_input_line(buf, if_cancelled)
char *buf;
char *if_cancelled;
{
	short ch;
	short i = 0;

	message("call it:", 0);

	while (((ch = getchar()) != '\n') && (ch != CANCEL)) {
		if ((ch >= ' ') && (ch <= '~') && (i < MAX_TITLE_LENGTH-2)) {
			buf[i++] = ch;
			addch(ch);
		}
		if ((ch == '\b') && (i > 0)) {
			mvaddch(0, --i + 9, ' ');
			move(MIN_ROW-1, i+9);
		}
		refresh_vms();
	}
	check_message();
	if (ch == CANCEL) {
		if (if_cancelled) {
			message(if_cancelled, 0);
		}
		return(0);
	}
	buf[i++] = ' ';
	buf[i] = 0;
	return(1);
}

/*
slurp()
{
	long ln;
	short i, n;

	do {
		ioctl(0, FIONREAD, &ln);
		n = stdin->_cnt + ln;

		for (i = 0; i < ln; i++) getchar();
	} while (ln > 0L);
}
*/
