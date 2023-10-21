/* snake2
 *
 * Copyright (C) 1981 by Don Libes.  This software may be freely copied
 * and distributed for noncommercial purposes provided that this notice
 * remains intact.  Commercial use of this software requires my prior
 * written permission.
 *
 * HISTORY
 * 20-Sep-85  libes at National Bureau of Standards
 *      Modified to support BSD 4.2 and Eunice (BSD 4.1)
 *	Modified to run on any size screen.
 *
 * 07-Aug-81  don at University of Rochester
 *	Fixed lack of rollover bug.  
 *	ioctl(FIONREAD,...) gives # of NEW unread chars added to input
 *	buffer since last call, not just # of unread chars in buffer.
 *
 * 18-Jul-81  don at University of Rochester
 *	Ring structure for snake body was being corrupted.  It seems C's
 *	idea on how to mod negative numbers is backwards.  Hard to tell
 *	if this is a bug, because reference manual is fuzzy on this.
 *
 * 16-Jul-81  don at University of Rochester
 *	Created after playing similar game on DG at Xerox.
 *	Different scoring.  Diagonal moves allowed.  Boxes are people
 *	rather than just points.  Various other hacks.  See man file.
 *	cc snake2.c -lcurses -ltermcap -lipc
 */

#include <sys/types.h>
#include <curses.h>
#include <ctype.h>
#include <pwd.h>

#include "snake2.h"

#define MAXUSERS	255		/* A large number */
#define MAXLEN		((LINES-2)*(COLS-2))	/* Maximum length of snake */
#define MAXBOXES	6
#define SKILL		(12-6)		/* How often we grow */
#define bell		putchar('\07')	/* Ring my chimes */

time_t time();				/* For random factor */

int *snakex, *snakey;			/* position of snake */
int sptr;				/* ptr to snake in its array */
int length;				/* length of snake */
int turn = 0;				/* number of turns we've had */
int points = 0;				/* Points by eating */
int diry = 1;				/* direction we are moving */
int dirx = 1;				/* initially diagonally and down */

/* Data structures for boxes */
struct {
	int y;
	int x;
	int inuse;
	char *name;
	int length;
	int points;			/* point value of box */
} boxes[MAXBOXES];

char names[MAXUSERS][USERNAME_LENGTH+1];
int numnames = 0;			/* number of names in /etc/passwd */

/* Data structures for log */
struct logtype {
	char name[USERNAME_LENGTH+1];
	int length;
	int points;
	int rows, columns;
} log[MAXUSERS];
long our_offset;			/* position of our name in log file */

/* scoring algorithm */
#define score(length,points,rows,columns)	((points)+(length))

/* parameters for lseek */
#define L_SET	0
#define L_INCR	1
#define L_XTND	2

char *getlogin(), *whoami;
char *strcpy(), *malloc();
int sortlog();
FILE *fopen();
FILE *lf;
int reluid;				/* uid relative to log file */
int usersknown;				/* number of users in log */
int userknown = FALSE;				/* if user in log file */

WINDOW *tmpscr;				/* save screen for clear */
WINDOW *newwin();

main()
{
	init();
	while (TRUE) {
		if (!snake()) break;	/* get user input */
		quick_sleep(TIMEOUT);
		ranbox();		/* randomly box */
		ranbox2();		/* randomly unbox */
	}
	quit();
}

init()
{
	int i = 0;
	extern struct passwd *getpwent(), *getpwuid();
	struct passwd *pwent;
	long last_offset;	/* keep track of offsets in log file */

#ifdef EUNICE
	/* if invoked from raw VMS, getlogin() screws up by returning */
	/* an empty string rather than a null string */
	if (!((whoami = getlogin()) && whoami[0])) {
#else
	if (!(whoami = getlogin())) {
#endif
		/* can't just copy ptr from getpwuid since this is */
		/* static data which will be reused when we read in */
		/* /etc/passwd! */
		whoami = malloc(USERNAME_LENGTH+1);
		strcpy(whoami,getpwuid(getuid())->pw_name);
	}
	setuid(geteuid());

	initscr();
	tmpscr = newwin(LINES,COLS,0,0);
	box(stdscr,'|','-');
	noecho();			/* Don't let directions echo! */
	raw();
	refresh();

	snakex = (int *)malloc(MAXLEN*sizeof(int));
	snakey = (int *)malloc(MAXLEN*sizeof(int));
	snakex[0] = snakey[0] = 2;
	move(2,2);
	addch('s');
	snakey[1] = 3; snakex[1] = 3;
	move(3,3);
	addch('s');
	length = 2;
	sptr = 1;

	srand((int) time((time_t *)0));
	/* read in names */
	while (pwent = getpwent()) {
		strcpy(names[numnames++],pwent->pw_name);
	}
	if (!(lf = fopen(LOG,"r+"))) {
		cleanup();
		printf("can't read/update score file (%s)\n",LOG);
		exit(0);
	}
	last_offset = our_offset = 0L;
	while (5 == fscanf(lf,"%s%d%d%d%d",log[i].name,&log[i].length,
					&log[i].points,
					&log[i].rows,
					&log[i].columns)) {
		if (!userknown) {
			if (!strcmp(whoami,log[i].name)) {
				our_offset = last_offset;
				reluid = i;
				userknown = TRUE;
				/* Give'm something to shoot for */
				move(LINES-1,50);
				printw("Personal high score: %d",
					score(log[i].length,log[i].points,
						log[i].rows,log[i].columns));
			} else last_offset = ftell(lf);
		}
		i++;
	}

	if (!userknown) {	/* New entry for this user */
		our_offset = last_offset;
		strcpy(log[i].name,whoami);
		log[i].length = 0;
		log[i].points = 0;
		log[i].rows = LINES;
		log[i].columns = COLS;
		reluid = i;
		userknown = TRUE;

		/* make new entry in log file */
		fseek(lf,0L,L_XTND);	/* EOF */
		fprintf(lf,"%*s%8d%8d%8d%8d\n",USERNAME_LENGTH,
					log[i].name,
					log[i].length,
					log[i].points,
					log[i].rows,
					log[i].columns);
		i++;
	}
	fclose(lf);
	usersknown = i;
}

quit()
{
	int i = 0;

	if (score(length,points,LINES,COLS) >
		score(log[reluid].length,
			log[reluid].points,
			log[reluid].rows,
			log[reluid].columns)) { /* new personal high score! */

		log[reluid].length = length;
		log[reluid].points = points;
		log[reluid].rows = LINES;
		log[reluid].columns = COLS;

		if (!(lf = fopen(LOG,"r+"))) {
			cleanup();
			printf("can't update score file (%s)\n",LOG);
			exit(0);
		}
		fseek(lf,our_offset+1,L_SET);
		fprintf(lf,"%*s%8d%8d%8d%8d\n",USERNAME_LENGTH,
			whoami,length,points,LINES,COLS);
		fclose(lf);
	}
	/* the 36 here is just the width of the stripe we are painting */
	/* i.e. it equals the width of the printw */
	move(1,(COLS-36)/2);
	printw("%*s%8s%8s%8s",USERNAME_LENGTH,
		"Who","Length","Points","Score");
	/* print out top LINES-4 scorers (right on top of game) */
	qsort(log,usersknown,sizeof(struct logtype),sortlog);
	for (i=0;i<(LINES-4) && i<usersknown;i++) {
		move(2+i,(COLS-36)/2);
		printw("%*s%8d%8d%8d",USERNAME_LENGTH,
			log[i].name,log[i].length,log[i].points,
			score(log[i].points,log[i].length,
				log[i].rows,log[i].columns));
	}
	cleanup();
	/* move UNIX prompt to bottom line (from where?) */
	mvcur(0,0,LINES-1,0);
	endwin();
}
	
snake()
{
	int key;			/* last key struck */

	turn++;

	if (-1 != (key = getkey())) {
		switch (key) {
		case 'i': case '8': case 'e':
			if (diry == 1 && dirx == 0) bell;
			else {
				diry = -1;
				dirx = 0;
			}
			break;
		case 'l': case '6': case 'f':
			if (diry == 0 && dirx == -1) bell;
			else {
				diry = 0;
				dirx = 1;
			}
			break;
		case ',': case '2': case 'c':
			if (diry == -1 && dirx == 0) bell;
			else {
				diry = 1;
				dirx = 0;
			}
			break;
		case 'j': case '4': case 's':
			if (diry == 0 && dirx == 1) bell;
			else {
				diry = 0;
				dirx = -1;
			}
			break;
		case '9': case 'o': case 'r':
			if (diry == 1 && dirx == -1) bell;
			else {
				diry = -1;
				dirx = 1;
			}
			break;
		case '3': case '.': case 'v':
			if (diry == -1 && dirx == -1) bell;
			else {
				diry = 1;
				dirx = 1;
			}
			break;
		case '1': case 'm': case 'x':
			if (diry == -1 && dirx == 1) bell; 
			else {
				diry = 1;
				dirx = -1;
			}
			break;
		case '7': case 'u': case 'w':
			if (diry == 1 && dirx == 1) bell;
			else {
				diry = -1;
				dirx = -1;
			}
			break;
		case 003:	/* Ctrl-C */
		case 004:	/* Ctrl-D */
		case 0177:	/* Rubout */
		case 034:	/* FS	  */
			return(0);
			break;
		case 014:	/* Ctrl-L */
			/* Really refresh screen */
			wclear(tmpscr);
			overlay(stdscr,tmpscr);
			clear();
			refresh();
			overlay(tmpscr,stdscr);
			break;
		}
	}
	return movesnake(diry,dirx);
}

movesnake(y,x)    /* check if snake hits money or edge of window, too */
{
	int c;
	int newsptr;	/* temp snake pointer */
	newsptr = (1+sptr)%MAXLEN;

	/* keep snake at right length */
	if (turn%SKILL) {
		move(snakey[(MAXLEN+1+sptr-length)%MAXLEN],
		     snakex[(MAXLEN+1+sptr-length)%MAXLEN]);
		addch(' ');
	} else {
		length++;
		move(0,15);
		printw("Length = %d",length);
		move(0,33);
		printw("Points = %d",points);
		move(0,51);
		printw("Score = %d",score(length,points,LINES,COLS));
	}

	/* move head of snake one step further */
	snakey[newsptr] = snakey[sptr]+y;
	snakex[newsptr] = snakex[sptr]+x;

	/* check for a hit! */
	move(snakey[newsptr],snakex[newsptr]);
	if (snakey[newsptr] == 0 || snakey[newsptr] == LINES-1 ||
		snakex[newsptr] == 0 || snakex[newsptr] == COLS-1) {
		move(LINES-2,2);
		addstr("You hit the edge, you snake!");
		return(0);
	}
	c = inch();
	if (c != ' ') { /* musta hit sumptin, duh... */
		int i;
		for (i=0;i<MAXBOXES;i++) {
			if (!boxes[i].inuse) continue;
			if (boxes[i].x <= snakex[newsptr] &&
			    boxes[i].x + boxes[i].length > snakex[newsptr] &&
			    boxes[i].y <= snakey[newsptr] &&
			    boxes[i].y + boxes[i].length > snakey[newsptr]) {
				/* hit box i */
				delbox(i);
				/* double points after 250 */
				if (points+length>=250)
					points += 2*boxes[i].points;
				else points += boxes[i].points;
				move(0,33);
				printw("Points = %d",points);
				move(0,51);
				printw("Score = %d",points+length);
				break;
			}
		}
		if (i == MAXBOXES) {
			move(LINES-2,2);
			addstr("You hit yourself, you snake!");
			return(0);
		}
	}

	move(snakey[sptr],snakex[sptr]);
	addch('s');
	move(snakey[newsptr],snakex[newsptr]);
	addch('S');

	sptr = newsptr;
	refresh();
	return(TRUE);
}

ranbox() /* randomly draw a box */
{
	int i, j, y, x, y2, x2, s;
	int length;

	i = rand()%MAXBOXES;
	if (boxes[i].inuse) return;
	j = rand()%numnames;
	length = strlen(names[j]);
	y = 1 + rand()%((LINES-1)-length);
	x = 1 + rand()%((COLS-1)-length);
	/* check if area is clear */
	for(y2=y;y2<y+length;y2++) {
		for (x2=x;x2<x+length;x2++) {
			move(y2,x2);
			if (' ' != inch()) return;
		}
	}
	boxes[i].inuse = TRUE;
	boxes[i].x = x;
	boxes[i].y = y;
	boxes[i].name = names[j];
	boxes[i].length = length;
	if (!(s = rand()%9)) boxes[i].points = length*3;
		else boxes[i].points = length;
	if (names[j] == names[0] ) { /* root! */
		boxes[i].points = (rand()%40) - 20;
	}
	/* draw box */
	/* top */
	move(y,x);
	addstr(names[j]);
	/* show triple value by capitalizing first letter */
	if ((!s) && (islower(names[j][0]))) {
		move(y,x);
		addch(names[j][0]-('a'-'A'));
	}
	for (y2=y+1;y2<y+length;y2++) {
		/* left side */
		move(y2,x);
		addch(names[j][y2-y]);
		/* right side */
		move(y2,x+length-1);
		addch(names[j][length-(y2-y)-1]);
	}
	/* bottom */
	for (x2=x+1;x2<x+length-1;x2++) {
		move(y+length-1,x2);
		addch(names[j][length-(x2-x)-1]);
	}
}

ranbox2() /* randomly delete a box */
{
	/* about every 10 times try and delete a randomly selected box */
	if (!(rand()%10)) {
		delbox(rand()%MAXBOXES);
	}
}

delbox(i) /* delete a box */
int i;
{
	int x2, y2, x, y;
	int length;

	if (!boxes[i].inuse) return;
	x = boxes[i].x;
	y = boxes[i].y;
	length = boxes[i].length;
	boxes[i].inuse = FALSE;
	/* delete box (this should probably be written as one loop) */
	move(y,x);
	/* delete top of box */
	for (x2=x;x2<x+length;x2++) {
		/* to */
		move(y,x2);
		addch(' ');
	}
	/* sides */
	for (y2=y+1;y2<y+length;y2++) {
		move(y2,x);
		addch(' ');
		move(y2,x+length-1);
		addch(' ');
	}
	/* bottom */
	for (x2=x+1;x2<x+length-1;x2++) {
		move(y+length-1,x2);
		addch(' ');
	}
}

sortlog(foo,bar)	/* sort function for log */
struct logtype *foo;
struct logtype *bar;
{
	int x, y;

	x = score(foo->length,foo->points,foo->rows,foo->columns);
	y = score(bar->length,bar->points,bar->rows,bar->columns);
	if (x == y) return(0);
	else if (x > y) return(-1);
	else return(1);
}

cleanup()
{
	echo();
	noraw();
	refresh();
}
