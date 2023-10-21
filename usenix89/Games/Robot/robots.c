/* Written by Allan R. Black, Strathclyde University.
 * This program may be freely distributed provided that
 * this comment is not removed and that
 * no profit is gained from such distribution.
 */

/*
 *	@(#)robots.c	1.7	3/21/84
 */

/* 1.7a modified by Stephen J. Muir at Lancaster University. */

# include <curses.h>
# include <signal.h>
# include <pwd.h>
# include <ctype.h>
# include <sys/file.h>

# define MIN_ROBOTS	10
# define MAX_ROBOTS	500
# define MIN_VALUE	10
# define MAX_FREE	3

# define VERT	'|'
# define HORIZ	'-'
# define ROBOT	'='
# define SCRAP	'@'
# define ME	'I'
# define MUNCH	'*'
# define DOT	'.'

# define LEVEL		(level+1)

# define MSGPOS		39
# define RVPOS		51

# define HOF_FILE	"/usr/games/lib/robots_hof"
# define TMP_FILE	"/usr/games/lib/robots_tmp"

# define NUMSCORES	20
# define NUMNAME	"Twenty"

# define TEMP_DAYS	7
# define TEMP_NAME	"Week"

# define MAXSTR		100

extern char	*getlogin ();
extern struct passwd	*getpwnam ();

struct scorefile {
	int	s_uid;
	int	s_score;
	char	s_name[MAXSTR];
	bool	s_eaten;
	int	s_level;
	int	s_days;
};

# define FILE_SIZE	(NUMSCORES*sizeof(struct scorefile))

/*# define ALLSCORES*/

# define SECSPERDAY	86400

# define BEL	007
# define CTLH	010
# define CTLJ	012
# define CTLK	013
# define CTLL	014
# define CTLY	031
# define CTLU	025
# define CTLB	002
# define CTLN	016
# define CTLR	022

char	whoami[MAXSTR];

int	my_x, my_y;
int	new_x, new_y;

int	level = 0;
int	score = 0;
int	free_teleports = 0;
int	free_per_level = 1;
bool	dead = FALSE;
bool	last_stand;

int	count;
char	com, cmd_ch;
bool	running, adjacent, first_move, bad_move;
int	dots = 0;

int	robot_value = MIN_VALUE;
int	max_robots = MIN_ROBOTS;
int	robots_alive;
struct passwd	*pass;

struct robot {
	bool	alive;
	int	x;
	int	y;
} robot_list[MAX_ROBOTS];

int	seed;

struct ltchars	ltc;

char	dsusp;

int	_putchar();
int	interrupt();

char	*forbidden [] =
	{ "root",
	  "daemon",
	  "uucp",
	  "pop",
	  "ingres",
	  "demo",
	  "chessel",
	  "saturn",
	  "sjm",
	  "cosmos",
	  0
	};

main(argc,argv)
	int argc;
	char *argv[];
{
	register char *x, **xx;
	if(argc > 1) {
		if(argv[1][0] == '-') {
			switch(argv[1][1]) {
			case 's':
				scoring(FALSE);
				exit(0);
			}
		}
	}
	if ((x = getlogin ()) == 0 || (pass = getpwnam (x)) == 0)
	{ printf ("Who the hell are you?\n");
	  exit (1);
	}
	strcpy(whoami,x);
	for (xx = forbidden; *xx; ++xx)
		if (strcmp (whoami, *xx) == 0)
		{ printf ("only individuals may play robots\n");
		  exit (1);
		}
	seed = time(0)+pass->pw_uid;
	signal(SIGQUIT,interrupt);
	signal(SIGINT,interrupt);
	initscr();
	crmode();
	noecho();
	ioctl(1,TIOCGLTC,&ltc);
	dsusp = ltc.t_dsuspc;
	ltc.t_dsuspc = ltc.t_suspc;
	ioctl(1,TIOCSLTC,&ltc);
	for(;;) {
		count = 0;
		running = FALSE;
		adjacent = FALSE;
		last_stand = FALSE;
		if(rnd(free_per_level) < free_teleports) {
			free_per_level++;
			if(free_per_level > MAX_FREE) free_per_level = MAX_FREE;
		}
		free_teleports += free_per_level;
		leaveok(stdscr,FALSE);
		draw_screen();
		put_robots();
		do {
			my_x = rndx();
			my_y = rndy();
			move(my_y,my_x);
		} while(winch(stdscr) != ' ');
		addch(ME);
		for(;;) {
			scorer();
			if(robots_alive == 0) break;
			command();
			robots();
			if(dead) munch();
		}
		if (!level)
			nice (2);
		msg("%d robots are now scrap heaps",max_robots);
		leaveok(stdscr,FALSE);
		move(my_y,my_x);
		refresh();
		readchar();
		level++;
	}
}

draw_screen()
{
	register int x, y;
	clear();
	for(y = 1; y < LINES-2; y++) {
		mvaddch(y,0,VERT);
		mvaddch(y,COLS-1,VERT);
	}
	for(x = 0; x < COLS; x++) {
		mvaddch(0,x,HORIZ);
		mvaddch(LINES-2,x,HORIZ);
	}
}

put_robots()
{
	register struct robot *r, *end;
	register int x, y;
	robot_value += level*5+rnd(level*10);
	max_robots += level*3+rnd(level*5);
	if(max_robots > MAX_ROBOTS) max_robots = MAX_ROBOTS;
	robots_alive = max_robots;
	end = &robot_list[max_robots];
	for(r = robot_list; r < end; r++) {
		for(;;) {
			x = rndx();
			y = rndy();
			move(y,x);
			if(winch(stdscr) == ' ') break;
		}
		r->x = x;
		r->y = y;
		r->alive = TRUE;
		addch(ROBOT);
	}
}

command()
{
	register int x, y;
retry:
	move(my_y,my_x);
	refresh();
	if(last_stand) return;
	bad_move = FALSE;
	if(!running) {
		cmd_ch = read_com();
		switch(cmd_ch) {
		case CTLH:
		case CTLJ:
		case CTLK:
		case CTLL:
		case CTLY:
		case CTLU:
		case CTLB:
		case CTLN:
			cmd_ch |= 0100;
			adjacent = TRUE;
		case 'H':
		case 'J':
		case 'K':
		case 'L':
		case 'Y':
		case 'U':
		case 'B':
		case 'N':
			cmd_ch |= 040;
			running = TRUE;
			first_move = TRUE;
		case 't':
		case 'T':
		case 's':
		case 'S':
		case 'm':
		case 'M':
		case '?':
		case 'd':
		case 'D':
		case CTLR:
			count = 0;
		}
	}
	switch(cmd_ch) {
	case '.':
		return;
	case 'h':
	case 'j':
	case 'k':
	case 'l':
	case 'y':
	case 'u':
	case 'b':
	case 'n':
		do_move(cmd_ch);
		break;
	case 't':
	case 'T':
	teleport:
		new_x = rndx();
		new_y = rndy();
		move(new_y,new_x);
		switch(winch(stdscr)) {
		case ROBOT:
		case SCRAP:
		case ME:
			goto teleport;
		}
		if(free_teleports > 0) {
			for(x = new_x-1; x <= new_x+1; x++) {
				for(y = new_y-1; y <= new_y+1; y++) {
					move(y,x);
					if(winch(stdscr) == ROBOT) goto teleport;
				}
			}
			free_teleports--;
		}
		break;
	case 's':
	case 'S':
		last_stand = TRUE;
		leaveok(stdscr,TRUE);
		return;
	case 'm':
	case 'M':
	case '?':
		good_moves();
		goto retry;
	case 'd':
	case 'D':
		if(dots < 2) {
			dots++;
			put_dots();
		} else {
			erase_dots();
			dots = 0;
		}
		goto retry;
	case 'q':
	case 'Q':
		quit(FALSE);
	case CTLR:
		clearok(curscr,TRUE);
		wrefresh(curscr);
		goto retry;
	default:
		bad_move = TRUE;
	}
	if(bad_move) {
		if(running) {
			if(first_move) putchar(BEL);
			running = FALSE;
			adjacent = FALSE;
			first_move = FALSE;
		} else {
			putchar(BEL);
		}
		refresh();
		count = 0;
		goto retry;
	}
	first_move = FALSE;
	mvaddch(my_y,my_x,' ');
	my_x = new_x;
	my_y = new_y;
	move(my_y,my_x);
	if(winch(stdscr) == ROBOT) munch();
	mvaddch(my_y,my_x,ME);
	refresh();
}

read_com()
{
	if(count == 0) {
		if (dots)
		{ put_dots ();
		  move (my_y, my_x);
		  refresh ();
		}
		if(isdigit(com = readchar())) {
			count = com-'0';
			while(isdigit(com = readchar())) count = count*10+com-'0';
		}
		if (dots)
			erase_dots ();
	}
	if(count > 0) count--;
	return(com);
}

readchar()
{
	static char buf[1];
	while(read(0,buf,1) <= 0);
	return(buf[0]);
}

do_move(dir)
	char dir;
{
	register int x, y;
	new_x = my_x+xinc(dir);
	new_y = my_y+yinc(dir);
	move(new_y,new_x);
	switch(winch(stdscr)) {
	case VERT:
	case HORIZ:
	case SCRAP:
		bad_move = TRUE;
	}
	if(adjacent & !first_move) {
		for(x = new_x-1; x <= new_x+1; x++) {
			for(y = new_y-1; y <= new_y+1; y++) {
				move(y,x);
				switch(winch(stdscr)) {
				case ROBOT:
				case SCRAP:
					bad_move = TRUE;
					return;
				}
			}
		}
	}
}

put_dots()
{
	register int x, y;
	for(x = my_x-dots; x <= my_x+dots; x++) {
		if (x < 1 || x > COLS - 2)
			continue;
		for(y = my_y-dots; y <= my_y+dots; y++) {
			if (y < 1 || y > LINES - 3)
				continue;
			move(y,x);
			if(winch(stdscr) == ' ') addch(DOT);
		}
	}
}

erase_dots()
{
	register int x, y;
	for(x = my_x-dots; x <= my_x+dots; x++) {
		if (x < 1 || x > COLS - 2)
			continue;
		for(y = my_y-dots; y <= my_y+dots; y++) {
			if (y < 1 || y > LINES - 3)
				continue;
			move(y,x);
			if(winch(stdscr) == DOT) addch(' ');
		}
	}
}

good_moves()
{
	register int x, y;
	register int test_x, test_y;
	register char *m, *a;
	static char moves[] = "hjklyubn.";
	static char ans[sizeof moves];
	a = ans;
	for(m = moves; *m; m++) {
		test_x = my_x+xinc(*m);
		test_y = my_y+yinc(*m);
		move(test_y,test_x);
		switch(winch(stdscr)) {
		case ' ':
		case ME:
			for(x = test_x-1; x <= test_x+1; x++) {
				for(y = test_y-1; y <= test_y+1; y++) {
					move(y,x);
					if(winch(stdscr) == ROBOT) goto bad;
				}
			}
			*a++ = *m;
		}
	bad:;
	}
	*a = 0;
	if(ans[0]) {
		a = ans;
	} else {
		a = "Forget it!";
	}
	mvprintw(LINES-1,MSGPOS,"%*.*-s",RVPOS-MSGPOS,RVPOS-MSGPOS,a);
}

xinc(dir)
	char dir;
{
	switch(dir) {
	case 'h':
	case 'y':
	case 'b':
		return(-1);
	case 'l':
	case 'u':
	case 'n':
		return(1);
	case 'j':
	case 'k':
	default:
		return(0);
	}
}

yinc(dir)
	char dir;
{
	switch(dir) {
	case 'k':
	case 'y':
	case 'u':
		return(-1);
	case 'j':
	case 'b':
	case 'n':
		return(1);
	case 'h':
	case 'l':
	default:
		return(0);
	}
}

robots()
{
	register struct robot *r, *end, *find;
	end = &robot_list[max_robots];
	for(r = robot_list; r < end; r++) {
		if(r->alive) {
			mvaddch(r->y,r->x,' ');
		}
	}
	for(r = robot_list; r < end; r++) {
		if(r->alive) {
			r->x += sign(my_x-r->x);
			r->y += sign(my_y-r->y);
			move(r->y,r->x);
			switch(winch(stdscr)) {
			case ME:
				addch(MUNCH);
				dead = TRUE;
				break;
			case SCRAP:
				r->alive = FALSE;
				score += robot_value;
				robots_alive--;
				break;
			case ROBOT:
				for(find = robot_list; find < r; find++) {
					if(find->alive) {
						if(r->x == find->x && r->y == find->y) {
							find->alive = FALSE;
							break;
						}
					}
				}
				r->alive = FALSE;
				addch(SCRAP);
				score += robot_value*2;
				robots_alive -= 2;
				break;
			case MUNCH:
				break;
			default:
				addch(ROBOT);
			}
		}
	}
}

munch()
{
	scorer();
	msg("MUNCH! You're robot food");
	leaveok(stdscr,FALSE);
	mvaddch(my_y,my_x,MUNCH);
	move(my_y,my_x);
	refresh();
	readchar();
	quit(TRUE);
}

quit(eaten)
	bool eaten;
{
	move(LINES-1,0);
	refresh();
	endwin();
	putchar('\n');
	ltc.t_dsuspc = dsusp;
	ioctl(1,TIOCSLTC,&ltc);
	scoring(eaten);
	exit(0);
}

scoring(eaten)
	bool eaten;
{
	static char buf[MAXSTR];
	sprintf(buf,"for this %s",TEMP_NAME);
	record_score(eaten,TMP_FILE,TEMP_DAYS,buf);
	printf("[Press return to continue]");
	fflush(stdout);
	gets(buf);
	record_score(eaten,HOF_FILE,0,"of All Time");
}

record_score(eaten,fname,max_days,type_str)
	bool eaten;
	char *fname;
	int max_days;
	char *type_str;
{
	int fd;
	int (*action)();
	action = signal(SIGINT,SIG_IGN);
	if((fd = open(fname,2)) < 0) {
		perror(fname);
	} else {
		if(flock(fd,LOCK_EX) < 0) {
			perror(fname);
		} else {
			do_score(eaten,fd,max_days,type_str);
			flock(fd,LOCK_UN);
		}
		close(fd);
	}
	signal(SIGINT,action);
}

do_score(eaten,fd,max_days,type_str)
	bool eaten;
	int fd, max_days;
	char *type_str;
{
	register struct scorefile *position;
	register int x;
	register struct scorefile *remove, *sfile, *eof;
	struct scorefile *oldest, *this;
	char *so, *ts;
	int uid, this_day, limit;
	static char buf[20];
	ts = buf;
	this_day = max_days ? time(0)/SECSPERDAY : 0;
	limit = this_day-max_days;
	sfile = (struct scorefile *)(malloc(FILE_SIZE));
	eof = &sfile[NUMSCORES];
	this = 0;
	for(position = sfile; position < eof; position++) {
		position->s_score = 0;
		position->s_days = 0;
	}
	read(fd,sfile,FILE_SIZE);
	remove = 0;
	if(score > 0) {
		uid = pass->pw_uid;
		oldest = 0;
		x = limit;
		for(position = eof-1; position >= sfile; position--) {
			if(position->s_days < x) {
				x = position->s_days;
				oldest = position;
			}
		}
		position = 0;
		for(remove = sfile; remove < eof; remove++) {
			if(position == 0 && score > remove->s_score) position = remove;
# ifndef ALLSCORES
			if (remove->s_uid == uid)
			{ if (remove->s_days < limit)
				oldest = remove;
			  else
			 	break;
			}
# endif ALLSCORES
		}
		if(remove < eof) {
			if(position == 0 && remove->s_days < limit) position = remove;
		} else if(oldest) {
			remove = oldest;
			if(position == 0) {
				position = eof-1;
			} else if(remove < position) {
				position--;
			}
		} else if(position) {
			remove = eof-1;
		}
		if(position) {
			if(remove < position) {
				while(remove < position) {
					*remove = *(remove+1);
					remove++;
				}
			} else {
				while(remove > position) {
					*remove = *(remove-1);
					remove--;
				}
			}
			position->s_score = score;
			strncpy(position->s_name,whoami,MAXSTR);
			position->s_eaten = eaten;
			position->s_level = LEVEL;
			position->s_uid = uid;
			position->s_days = this_day;
			this = position;
			lseek(fd,0,0);
			write(fd,sfile,FILE_SIZE);
			close(fd);
		}
	}
	printf(
# ifdef ALLSCORES
		"\nTop %s Scores %s:\n",
# else ALLSCORES
		"\nTop %s Robotists %s:\n",
# endif ALLSCORES
		NUMNAME,
		type_str
	);
	printf("Rank   Score    Name\n");
	count = 0;
	for(position = sfile; position < eof; position++) {
		if(position->s_score == 0) break;
		if(position == this && SO && SE) {
			tputs(SO,0,_putchar);
		}
		if (position->s_days >= limit)
		{ printf ("%-6d %-8d %s: %s on level %d.",
			  ++count,
			  position->s_score,
			  position->s_name,
			  position->s_eaten ? "eaten" : "chickened out",
			  position->s_level
			 );
# ifdef OLDSCORES
		} else
		{ printf ("**OLD** %-8d %s: %s on level %d.",
			  position->s_score,
			  position->s_name,
			  position->s_eaten ? "eaten" : "chickened out",
			  position->s_level
			 );
# endif OLDSCORES
		}
		if(position == this && SO && SE) {
			tputs(SE,0,_putchar);
		}
# ifndef OLDSCORES
		if (position->s_days >= limit)
# endif OLDSCORES
			putchar ('\n');
	}
}

scorer()
{
	static char	infobuf [6];
	register int	x, y;
	if ((y = free_teleports-free_per_level) < 0)
		y = 0;
	x = free_teleports-y;
	sprintf(infobuf,"%d+%d",x,y);
	if (y == 0)
		infobuf[1] = '\0';
	move(LINES-1,0);
	clrtoeol();
	printw("<%s> level: %d    score: %d",infobuf,LEVEL,score);
	mvprintw(LINES-1,RVPOS,"robots: %d    value: %d",robots_alive,robot_value);
}

rndx()
{
	return(rnd(COLS-2)+1);
}

rndy()
{
	return(rnd(LINES-3)+1);
}

rnd(mod)
	int mod;
{
	if(mod <= 0) return(0);
	return((((seed = seed*11109+13849) >> 16) & 0xffff) % mod);
}

sign(x)
	register int x;
{
	if(x < 0) return(-1);
	return(x > 0);
}

msg(message,args)
	char *message;
	int args;
{
	register int x;
	static FILE msgpr;
	static char msgbuf[1000];
	msgpr._flag = _IOSTRG | _IOWRT;
	msgpr._ptr = msgbuf;
	msgpr._cnt = 1000;
	_doprnt(message,&args,&msgpr);
	putc(0,&msgpr);
	mvaddstr(LINES-1,MSGPOS,msgbuf);
	clrtoeol();
	refresh();
}

interrupt()
{
	quit(FALSE);
}
