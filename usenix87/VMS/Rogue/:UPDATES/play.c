#include curses.h
#include setjmp
#include "object.h"
#include "move.h"

short interrupted = 0;
extern short party_room, detect_monster;
extern char hit_message[];
int val;

play_level()
{
	short ch;
 	char getchartt();
	int count = 0;
	char com_string[80];
	char mess[24][82];
	int a,b;
	WINDOW *tempwin;
	int quit();
	
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
		count = 0;
CH:
		switch(ch) {
		case '\003':
			quit();
			break;
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
		case '\b':              /* ^H */
		case '\012':		/* ^J */
		case '\013':            /* ^K */
		case '\f':              /* ^L (form feed) */
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
		case '\032':             /* ^Z */
			tempwin=newwin(LINES,COLS,0,0);
			overlay(stdscr,tempwin);
			move(1,1);
			refresh();
			com_string[0]='\0';
			spawn_command(com_string);
			clear();
			overlay(tempwin,stdscr);
			delwin(tempwin);
			refresh();
			break;
			/* tstp(); */
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
			message("ve_rogue: VMS extended rogue , 1.0. ",0);
			break;
		case 'Q':
			quit();
			break;
		case '/':
			message("Identify what: ",1);
			identify_monst(getchartt());
			break;
		case '?':
			message("Help on what character: ",1);
			help(getchartt());
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


