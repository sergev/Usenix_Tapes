/*
 * Snake, from worm Written by Michael Toy
 * UCSC
 * modified by a.wiedemann 05.17.82
 * Siemens AG Munich
 * ZTI INF 212
 *
 * herpetology added by David Fiedler, InfoPro Systems
 * {astrovax, harpo, whuxcc}!infopro!dave
 * barriers and double-elongation added by Mark Wutka
 * Office of Computing Services
 * Georgia Institute of Technology
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
int skill_level =1;
char sl;
int barrier_skill = 0;
int start_len = LENGTH;
int pd=RIGHT;
int dir=RIGHT;
int grow_rate = 1;
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
                 skill_level=atoi(argv[1]);
		}
           else
                {
                 printf("Format for running snake:\n");
                 printf("snake n\n");
                 printf("where n= the skill level (1-4)\n");
                 exit(0);
                };
        if(skill_level<1) skill_level=1;
        if(skill_level>4) skill_level=4;
        switch(skill_level)
           {
             case 1:
                 start_len=7;
                 grow_rate=1;
                 barrier_skill = 1;
                 break;
             case 2:
                 start_len=10;
                 grow_rate=1;
                 barrier_skill = 2;
                 break;
             case 3:
                 start_len=10;
                 grow_rate=2;
                 barrier_skill=3;
                 break;
             case 4:
                 start_len=15;
                 grow_rate=2;
                 barrier_skill=4;
                 break;
           };
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
        barrier(barrier_skill,tv,stw);
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
	static char headchar;

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
                              { if (pd==LEFT || pd==RIGHT) twist='-';
                                if (pd==UP) twist='\\';
                                if (pd==DOWN) twist='/';}
		when 'J': y++; running = RUNLEN/2; ch = tolower(ch);dir=DOWN;
                              { if (pd==UP || pd==DOWN) twist='|';
                                if (pd==LEFT) twist='/';
                                if (pd==RIGHT) twist='\\';}
		when 'K': y--; running = RUNLEN/2; ch = tolower(ch);dir=UP;
                              { if (pd==UP || pd==DOWN) twist='|';
                                if (pd==LEFT) twist='\\';
                                if (pd==RIGHT) twist='/';}
		when 'L': x++; running = RUNLEN; ch = tolower(ch);dir=RIGHT;
                              { if (pd==LEFT || pd==RIGHT) twist='-';
                                if (pd==UP) twist='/';
                                if (pd==DOWN) twist='\\';};
		when '\f': setup(); return;
		when CNTRL(Z): suspend(); return;
		when CNTRL(C): exit_gracefully(); return;
		when CNTRL(D): exit_gracefully(); return;
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
		growing +=  ch-'0';
		prize();
		score += (growing*skill_level);
                growing = grow_rate * growing;
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
	clear();
	move(LINES - 1, 0);
	refresh();
	endwin();
	fflush(stdout);
	printf("Well you ran into something and the game is over.\n");
	printf("Your final score was %d\n", score);
	leave();
}
exit_gracefully ()
{
        clear();
        move(LINES - 1,0);
        refresh();
        endwin();
        fflush(stdout);
        printf("You quit with %d points.\n",score);
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
barrier(skill,tv,stw)
  WINDOW *tv,*stw;
  int skill;
 {
  int i,j=0;
  switch(skill)
  {
   case 1: break;
   case 2: for (i=COLS/5;i<4*COLS/5;++i)
           {
           for (j=2*LINES/5;j<3*LINES/5;++j)
           {
           barrier_print(j,i,tv,stw);
           }
           };
           break;
   case 3: for (i=2*COLS/5;i<3*COLS/5;++i)
           {
           for (j=LINES/5;j<4*LINES/5;++j)
           {
           barrier_print(j,i,tv,stw);
           }
           };
           for (i=COLS/5;i<4*COLS/5;++i)
           { for (j=2*LINES/5;j<3*LINES/5;++j)
           { barrier_print(j,i,tv,stw);}};
           break;
   case 4: for (j=LINES/7;j<3*LINES/7;++j)
           { for (i=1*COLS/7;i<2*COLS/7;i++)
           { barrier_print(j,i,tv,stw);};
             for (i=5*COLS/7;i<6*COLS/7;++i)
           { barrier_print(j,i,tv,stw);};};
           for (j=4*LINES/7;j<6*LINES/7;++j)
           { for (i=1*COLS/7;i<2*COLS/7;++i)
           { barrier_print(j,i,tv,stw);};
             for (i=5*COLS/7;i<6*COLS/7;++i)
           { barrier_print(j,i,tv,stw);};};
             for (j=2*LINES/7;j<3*LINES/7;++j)
             for (i=4*COLS/7;i<5*COLS/7;++i)
             barrier_print(j,i,tv,stw);
             for (j=4*LINES/7;j<5*LINES/7;++j)
             for (i=2*COLS/7;i<3*COLS/7;++i)
             barrier_print(j,i,tv,stw);
           break;
   };
}
barrier_print(j,i,tv,stw)
int j,i;
WINDOW *tv,*stw;
{
 wmove(tv,j,i);
 wmove(stw,j,i);
 waddch(tv,'*');
 waddch(stw,'*');
}
