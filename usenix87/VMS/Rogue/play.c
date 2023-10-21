#include curses.h
#include "object.h"
#include "move.h"

short interrupted = 0;
extern short party_room, detect_monster;
extern char hit_message[];

play_level()
{
	short ch;
 	char getchartt();
	int count = 0;
	char com_string[80];
	char mess[24][82];
	int a,b;
	WINDOW *tempwin;
	for (;;) {
		interrupted = 0;
		if (hit_message[0]) {
			message(hit_message);
			hit_message[0] = 0;
		}
		move(rogue.row, rogue.col);
		refresh_vms();
		ch = getchartt();
		check_message();
CH:
		switch(ch) {
		case '.':
			rest((count > 0) ? count : 1);
			break;
		case 'i':
			inventory(&rogue.pack, IS_OBJECT);
			break;
		/*case 'p':
			inventory(&level_objects, IS_OBJECT);
			break;*/
		case 'f':
			fight(0);
			break;
		case 'F':
			fight(1);
			break;
		case 'h':
		case 'j':
		case 'k':
		case 'l':
		case 'y':
		case 'u':
		case 'n':
		case 'b':
			single_move_rogue(ch, 1);
			break;
		case 'H':
		case 'J':
		case 'K':
		case 'L':
		case 'B':
		case 'Y':
		case 'U':
		case 'N':
		case '':
		case '\012':		/* ^J */
		case '':
		case '':
		case '\031':
		case '\025':
		case '\016':
		case '\002':
			multiple_move_rogue(ch);
			break;
		case 'e':
			eat();
			break;
		case 'q':
			quaff();
			break;
		case 'r':
			read_scroll();
			break;
		case 'm':
			move_onto();
			break;
		case 'd':
			drop();
			break;
		case '\020':
			remessage();
			break;
		case '>':
			if (check_down()) {
				return;
			}
			break;
		case '<':
			if (check_up()) {
				return;
			}
			break;
		case 'I':
			single_inventory();
			break;
		case 'R':  /* again vms curses refuses to redraw the 
				nice way */
			tempwin=newwin(LINES,COLS,0,0);
			overlay(stdscr,tempwin);
			refresh();
			clear();
			overlay(tempwin,stdscr);
			delwin(tempwin);
			refresh();
			break;
		case 'T':
			take_off();
			break;
		case 'W':
		case 'P':
			wear();
			break;
		case 'w':
			wield();
			break;
		case 'c':
			call_it();
			break;
		case 'z':
			zapp();
			break;
		case 't':
			throw();
			break;
		case '\032':
			/* tstp(); */
			break;
		case '!':
			tempwin=newwin(LINES,COLS,0,0);
			overlay(stdscr,tempwin);
			move(1,1);
			refresh();

			spawn_command(gets(com_string));
			clear();
			overlay(tempwin,stdscr);
			delwin(tempwin);
			refresh();
			break;
		case 'v':
			message("i_rogue: VMS, Version 1.0. (stoehr was here)", 0);
			break;
		case 'Q':
			quit();
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
			count = 0;
			do {
				count = (10 * count) + (ch - '0');
				ch = getchartt();
			} while ((ch >= '0') && (ch <= '9'));
			goto CH;
			break;
		case ' ':
			break;
		default:
			message("unknown command");
			break;
		}
	}
}
