#include <curses.h>
#include "defs.h"

/*
 * This implements the player (client) portion of blackjack.  It's
 * pretty stupid, basically a big case statement.  It listens to
 * the dealer (server), and updates the screen, getting input from
 * the player when necessary.  Here are the commands it sends and
 * receives:
 *
 * commands to client:
 *
 * Pnplayername		add player playername as player n
 * Mstring		display string as a message
 * Rstring		display string and get single char response
 * rstring		display string and get string response
 * Cnhstring		display string as cards for player n hand h
 *
 * commands to server:
 *
 * H			hit with another card
 * S			stand
 * T			split
 * D			double down
 * Q			quit
 * (plus plain old string)
 */
main (argc, argv)
int argc;
char **argv;
{
	char buf[SLEN], host[20], pbuf[SLEN];
	int s, make_con();		/* socket I talk and listen on */
	void send_name();
	char *gets();
	int i, arrows = 0, gothost = 0;
	static char blank[] = "                                   ";
	int x, y, standoutflag = 0;		/* display stuff */

	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGQUIT, SIG_IGN);
	if (argc > 3)  {
		(void) fprintf(stderr, "usage: %s [hostname] [-arrows]\n", *argv);
		exit(1);
	}
	for (i = 1; i < argc; i++)
		if (!strcmp(*++argv, "-arrows"))
			arrows = 1;
		else  {
			if (gothost)  {
				fputs("usage: bj [hostname] [-arrows]\n", stderr);
				exit(1);
			}
			gothost = 1;
			(void) strcpy(host, *argv);
		}
	if (!gothost)
		(void) gethostname(host, SLEN);	/* host is this machine */
	s = make_con(host);
	send_name(s);			/* send name, get player info */
	initscr();			/* curses stuff */
	noecho();
	crmode();
	refresh();
	for (;;)  {
		sockread(s, buf);	/* get command from dealer */
		switch(buf[0])  {
		case 'P':		/* display player names */
			if (buf[1] == 'S')
				if (arrows)  {
					(void) strcpy(pbuf, "-->");
					(void) strcat(pbuf, buf + 3);
				}
				else  {
					(void) strcpy(pbuf, buf + 3);
					standoutflag = 1;
				}
			else
				(void) strcpy(pbuf, buf + 3);
			switch(buf[2])  {
			case '0':
				x = 5;
				y = 55;
				break;
			case '1':
				x = 11;
				y = 50;
				break;
			case '2':
				x = 17;
				y = 45;
				break;
			case '3':
				x = 17;
				y = 10;
				break;
			case '4':
				x = 11;
				y = 5;
				break;
			case '5':
				x = 5;
				y = 0;
				break;
			case '6':
				x = 2;
				y = 30;
				(void) strcpy(pbuf, "Dealer");
				break;
			}
			mvaddstr(x, y, blank);
			if (standoutflag)
				standout();
			mvaddstr(x, y, pbuf);
			refresh();
			standend();
			standoutflag = 0;
			break;
		case 'M':		/* display a message */
			move(0, 0);
			clrtoeol();
			mvaddstr(0, 0, buf + 1);
			refresh();
			sleep(1);	/* delete or increase for different delay reading messages */
			break;
		case 'r':		/* display a message, then read a string */
			move(0, 0);
			clrtoeol();
			mvaddstr(0, 0, buf + 1);
			refresh();
			echo();
			nocrmode();
			(void) gets(buf);	/* getstr() screws up tty bits */
			sockwrite(s, buf);
			noecho();
			crmode();
			break;
		case 'R':		/* display a message, then read a char */
			move(0,0);
			clrtoeol();
			mvaddstr(0, 0, buf + 1);
			refresh();
			*buf = getch();
			buf[1] = '\0';
			sockwrite(s, buf);
			break;
		case 'C':		/* display a players hand */
			switch(buf[1])  {
			case '0':	/* the different players */
				move(6, 55);
				break;
			case '1':
				move(12, 50);
				break;
			case '2':
				move(18, 45);
				break;
			case '3':
				move(18, 10);
				break;
			case '4':
				move(12, 5);
				break;
			case '5':
				move(6, 0);
				break;
			case '6':
				move(3, 30);
			}
			/* this places the hands under the player name in the proper row */
			move(stdscr->_cury + buf[2] - '0', stdscr->_curx);
			if (buf[1] == '6')
				addstr(buf + 3);
			else  {
				printw("#%c: ", buf[2] + 1);
				addstr(buf + 3);
			}
			refresh();
			break;
		case 'c':		/* clear screen */
			clear();
			refresh();
			break;
		case 'Q':		/* quit */
			endwin();
			exit(0);
		default:
			puts("Error");
			puts(buf);
		}
		(void) fflush(stdout);
	}
}
