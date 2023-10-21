/*
 * Modification History 
 *
 * Origional Author:  Jeff Bauer@ETA Systems[eta!bauer] 
 * Modifications by:  Marc Ries@TRW[trwrb!ries] 
 *
 */

#include <curses.h>

help()
{
	clear();
	standout();
	mvaddstr(0, 25, " Keys and their Functions");
	standend();
	mvaddstr(1, 2, "h, l, k, j       - move cursor left, right, up, down");
	mvaddstr(2, 2, "<CR> and <LF>    - select items/commands/scroll-areas");
	mvaddstr(3, 2, "0-9              - direct entry of date or marker numbers");
	mvaddstr(4, 2, "/  - directly type in complete date (i.e. 5/6/86) | <ESC> - abort function");
	mvaddstr(5, 2, "<SPACE>/<BACKSPACE> - scroll hours/minutes forward/backward");
	mvaddstr(6, 2, "m, d - and y     - move into the months, days, years areas");
	mvaddstr(7, 2, "n or +, p or '-' - go to the next or previous month, day, or year");
	mvaddstr(8, 2, "M - mark the currently displayed date  | G - go to a previously marked date");
	mvaddstr(9, 2, ";                - go to last visited date outside of current displayed month");
	mvaddstr(10, 2, "T                - go to today's actual date (initial date displayed)");
	mvaddstr(11, 2, "O                - display schedule grid for currently displayed day");
	mvaddstr(12, 2, "A                - show days in current month that have items scheduled");
	mvaddstr(13, 2, "S                - scan the events for the currently displayed day");
	mvaddstr(14, 2, "  n : go to next event      p : go to previous event    d : delete this event");
	mvaddstr(15, 2, "  e : edit this event       q : quit the scan           <ESC>: same as 'q'");
	mvaddstr(16, 2, "E                - scan every stored event");
	mvaddstr(17, 2, "F                - save events on disk file");
	mvaddstr)18, 2, "P                - post an event (see man page)");
	mvaddstr(19, 2, "L                - print picture of moon for 11:00 PM of current day");
	mvaddstr(20, 2, "Q                - quit, store any event changes");
	mvaddstr(21, 2, "^L, ^R           - redraw screen (including this page)");
	mvaddstr(22, 2, "^C, ^\           - quit, ignore any event changes");
	standout();
	mvaddstr(23, 18, " Press any key to return to your calendar.");
	standend();
	refresh();
	get_char();
	clear();
	print_screen();
}
