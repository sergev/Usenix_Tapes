/*
 * Snake, from worm Written by Michael Toy
 * UCSC
 * modified by a.wiedemann 05.17.82
 * Siemens AG Munich
 * ZTI INF 212
 *
 * herpetology added by David Fiedler, InfoPro Systems
 * {astrovax, harpo, whuxcc}!infopro!dave
 *
 * compile with "cc -O -s -o snake snake.c -lcurses -ltermcap
 *
 */

#include <ctype.h>
#include "curses.h"
#include <signal.h>

char *getenv();

#define newlink() (struct body *) malloc(sizeof (struct body));
#define HEAD '@'
#define BODY 'o'
#define	UP	1
#define	DOWN	2
#define	LEFT	3
#define	RIGHT	4
#define LENGTH 7
#define RUNLEN 8
#define when break;case
#define otherwise break;default
#define CNTRL(p) ('p'-'A'+1)

WINDOW *tv;
WINDOW *stw;
struct body {
	int x;
	int y;
	struct body *prev;
	struct body *next;
} *head, *tail, goody, *malloc();

int growing = 0;
int running = 0;
int score = 0;
int start_len = LENGTH;
int pd=RIGHT;
char twist;
char lastch;
char outbuf[BUFSIZ];

main(argc, argv)
char **argv;
{
	int leave(), wake(), suspend();
	char ch;

	if (argc == 2)
		{
		start_len = atoi(argv[1]);
		score += (start_len - LENGTH);
		}
	setbuf(stdout, outbuf);
	srand(getpid());
	signal(SIGALRM, wake);
	signal(SIGINT, leave);
	signal(SIGQUIT, leave);
#ifdef SIGTSTP
	signal(SIGTSTP, suspend);	/* process control signal */
#endif
	initscr();
	crmode();
	noecho();
	clear();
	stw = newwin(1, COLS-1, 0, 0);
	tv = newwin(LINES-1, COLS-1, 1, 0);
	/* snake cannot be bigger than the inner width of tv window !! */
	if ((start_len <= 0) || (start_len > COLS - 6))
		start_len = LENGTH;
	box(tv, '*', '*');
	scrollok(tv, FALSE);
	scrollok(stw, FALSE);
	wmove(stw, 0, 0);
	wprintw(stw, " Snake");
	refresh();
	wrefresh(stw);
	wrefresh(tv);
	life();                 /* Create the snake */
	prize();                /* Put up a goal */
	for (;;)
	{
		if (running)
		{
			running--;
			process(lastch);
		}
		else
		{
		    fflush(stdout);
		    if (read(0, &ch, 1) >= 0)
			process(ch);
		}
	}
}

life()
{
	register struct body *bp, *np;
	register int i;

	head = newlink();
	head->x = start_len+2;
	head->y = 12;
	head->next = NULL;
	display(head, HEAD);
	for (i = 0, bp = head; i < start_len; i++, bp = np) {
		np = newlink();
		np->next = bp;
		bp->prev = np;
		np->x = bp->x - 1;
		np->y = bp->y;
		display(np, '-');
	}
        display(np,' ');
        display(np->next,' ');
	tail = np;
	tail->prev = NULL;
	wrefresh(tv);
}

display(pos, chr)
struct body *pos;
char chr;
{
	wmove(tv, pos->y, pos->x);
	waddch(tv, chr);
}

leave()
{
	move(LINES - 1, 0);
	refresh();
	endwin();
	exit(0);
}

wake()
{
	signal(SIGALRM, wake);
	fflush(stdout);
	process(lastch);
}

rnd(range)
{
	return abs((rand()>>5)+(rand()>>5)) % range;
}

newpos(bp)
struct body * bp;
{
	do {
		bp->y = rnd(LINES-3)+ 2;
		bp->x = rnd(COLS-3) + 1;
		wmove(tv, bp->y, bp->x);
	} while(winch(tv) != ' ');
}

prize()
{
	int value;

	value = rnd(9) + 1;
	newpos(&goody);
	waddch(tv, value+'0');
	wrefresh(tv);
}

process(ch)
char ch;
{
	register int x,y;
	struct body *nh;
	static char headchar, dir;

	alarm(0);
	x = head->x;
	y = head->y;
        pd = dir;
	switch(ch)
	{
		when 'h': x--; dir=LEFT;
                              {
                                if(pd==LEFT || pd==RIGHT) twist='-';
                                if (pd==UP) twist='\\';
                                if (pd==DOWN) twist='/';};
		when 'j': y++; dir=DOWN; twist='|';
                              { if (pd==UP || pd==DOWN) twist='|';
                                if (pd==LEFT) twist='/';
                                if (pd==RIGHT) twist='\\';};
		when 'k': y--; dir=UP; twist ='|';
                              { if (pd==UP || pd==DOWN) twist='|';
                                if (pd==LEFT) twist='\\';
                                if (pd==RIGHT) twist='/';};
		when 'l': x++; dir=RIGHT; twist ='-';
                              { if (pd==LEFT || pd==RIGHT) twist='-';
                                if (pd==UP) twist='/';
                                if (pd==DOWN) twist='\\';};
		when 'H': x--; running = RUNLEN; ch = tolower(ch);dir=LEFT;
		when 'J': y++; running = RUNLEN/2; ch = tolower(ch);dir=DOWN;
		when 'K': y--; running = RUNLEN/2; ch = tolower(ch);dir=UP;
		when 'L': x++; running = RUNLEN; ch = tolower(ch);dir=RIGHT;
		when '\f': setup(); return;
		when CNTRL(Z): suspend(); return;
		when CNTRL(C): crash(); return;
		when CNTRL(D): crash(); return;
		otherwise: if (! running) alarm(1);
			   return;
	}
	lastch = ch;
	if (growing <= 0)
	{
		tail->next->prev = NULL;
		nh = tail->next;
		free(tail);
		tail = nh;
		if (growing < 0)
			growing = 0;
                display(tail->next,' ');
	}
	else growing--;
        display(head,twist);
	wmove(tv, y, x);
	if (isdigit(ch = winch(tv)))
	{
		growing += ch-'0';
		prize();
		score += growing;
		running = 0;
		wmove(stw, 0, 68);
		wprintw(stw, "Score: %3d", score);
		wrefresh(stw);
	}
	else if(ch != ' ') crash();
	nh = newlink();
	nh->next = NULL;
	nh->prev = head;
	head->next = nh;
	nh->y = y;
	nh->x = x;
	switch (dir) {
		case UP:
			headchar = '^'; break;
		case DOWN:
			headchar = 'V'; break;
		case LEFT:
			headchar = '<' ; break;
		case RIGHT:
			headchar = '>' ; break;
		default:
			headchar = HEAD ;
		}
	display(nh, headchar);
	head = nh;
	wrefresh(tv);
	if (! running) alarm(1);
}

crash()
{
        sleep(2);
	clear();
	move(LINES - 1, 0);
	refresh();
	endwin();
	fflush(stdout);
	printf("Well you ran into something and the game is over.\n");
	printf("Your final score was %d\n", score);
	leave();
}

suspend()
{
	char *sh;

	move(LINES-1, 0);
	refresh();
	endwin();
	fflush(stdout);
#ifdef SIGTSTP
	kill(getpid(), SIGTSTP);
	signal(SIGTSTP, suspend);
#else
	sh = getenv("SHELL");
	if (sh == NULL)
		sh = "/bin/sh";
	system(sh);
#endif
	crmode();
	noecho();
	setup();
}

setup()
{
	clear();
	refresh();
	touchwin(stw);
	wrefresh(stw);
	touchwin(tv);
	wrefresh(tv);
	alarm(1);
}

