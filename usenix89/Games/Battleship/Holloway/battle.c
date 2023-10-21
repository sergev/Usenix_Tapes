#include <stdio.h>
#include <curses.h>
#include <signal.h>

/* #define	BSD	1	/* Define BSD if compiling for BSD Unix */

#ifdef BSD
#define	beep()	write(1,"\007",1);
#define	cbreak	crmode
#define	saveterm savetty
#define	resetterm resetty
#define	nocbreak nocrmode
#endif

#define	OTHER	1-turn

#define	NOBLITZ	0		/* Defined if HITs don't mean you get to 
				   continue wiping out your opponent. */
/*#define	NOASK	0 */	/* Defined if the computer figures out whether
				   it hits without bothering yout about it. */
/*#define	SEEMISS	0 */	/* Defined if the player sees the computer's
				   misses. */

char numbers[] = "   0  1  2  3  4  5  6  7  8  9";

char carrier[] = "Aircraft Carrier";
char battle[] = "Battleship";
char sub[] = "Submarine";
char destroy[] = "Destroyer";
char ptboat[] = "PT Boat";

char name[40];
char dftname[] = "Stranger";

struct _ships {
    char *name;
    char symbol;
    char length;
    char start;		/* Coordinates - 0,0=0; 10,10=100. */
    char dir;		/* Direction - 0 = right; 1 = down. */
    char hits;		/* How many times has this ship been hit? (-1==sunk) */
    };

struct _ships plyship[] = {
    { carrier,'A',5,0,0,0 },
    { battle,'B',4,0,0,0 },
    { destroy,'D',3,0,0,0 },
    { sub,'S',3,0,0,0 },
    { ptboat,'P',2,0,0,0 },
};

struct _ships cpuship[] = {
    { carrier,'A',5,0,0,0 },
    { battle,'B',4,0,0,0 },
    { destroy,'D',3,0,0,0 },
    { sub,'S',3,0,0,0 },
    { ptboat,'P',2,0,0,0 },
};

char hits[2][100], board[2][100];	/* "Hits" board, and main board. */

int srchstep;
int cpuhits;
int cstart, cdir;
int plywon=0, cpuwon=0;			/* How many games has each won? */
int turn;				/* 0=player, 1=computer */
int huntoffs;				/* Offset on search strategy */

main(){
    intro();
    do{
	initgame();
	while(awinna() == -1){
#ifdef NOBLITZ
	    if(turn) cputurn(); else plyturn();
#else
	    while((turn) ? cputurn() : plyturn());
#endif
	    turn = OTHER;
	    }
	} while(playagain());
    uninitgame();
}

#define	PR	addstr

intro(){
    int uninitgame();
    extern char *getlogin();
    char *tmpname;

    srand(time(0L));			/* Kick the random number generator */

    signal(SIGINT,uninitgame);
    if(signal(SIGQUIT,SIG_IGN) != SIG_IGN) signal(SIGQUIT,uninitgame);

    if(tmpname = getlogin())
	strcpy(name,tmpname);
    else
	strcpy(name,dftname);
    name[0] = toupper(name[0]);

    initscr();
    saveterm();
    nonl(); cbreak(); noecho();
    clear();
    mvaddstr(4,29,"Welcome to Battleship!");
    move(8,0);
PR("                                                  \\\n");
PR("                           \\                     \\ \\\n");
PR("                          \\ \\                   \\ \\ \\_____________\n");
PR("                         \\ \\ \\_____________      \\ \\/            |\n");
PR("                          \\ \\/             \\      \\/             |\n");
PR("                           \\/               \\_____/              |__\n");
PR("           ________________/                                       |\n");
PR("           \\  S.S. Penguin                                         |\n");
PR("            \\                                                     /\n");
PR("             \\___________________________________________________/\n");
    mvaddstr(20,27,"Hit any key to continue..."); refresh();
    getch();
}

initgame(){
    int i;

    clear();
    mvaddstr(0,35,"BATTLESHIP");
    mvaddstr(4,12,"Main Board");
    mvaddstr(6,0,numbers);
    move(7,0);
    for(i=0; i<10; ++i){
	printw("%c  .  .  .  .  .  .  .  .  .  .  %c\n",i+'A',i+'A');
	}
    mvaddstr(17,0,numbers);
    mvaddstr(4,55,"Hit/Miss Board");
    mvaddstr(6,45,numbers);
    for(i=0; i<10; ++i){
	mvprintw(7+i,45,"%c  .  .  .  .  .  .  .  .  .  .  %c",i+'A',i+'A');
	}
    mvaddstr(17,45,numbers);
    for(turn=0; turn<2; ++turn)
	for(i=0; i<100; ++i){
	    hits[turn][i] = board[turn][i] = 0;
	    }
    for(turn=0; turn<2; ++turn){
	for(i=0; i<5; ++i)
	    if(!turn) plyplace(&plyship[i]);
	    else cpuplace(&cpuship[i]);
	}
    turn = rnd(2);
    cstart = cdir = -1;
    cpuhits = 0;
    srchstep = 3;
    huntoffs = rnd(srchstep);
}

rnd(n)
int n;
{
    return(((rand() & 0x7FFF) % n));
}

plyplace(ss)
struct _ships *ss;
{
    int c, d;

    do{
	prompt();
	printw("Place your %s (ex.%c%d) ? ",ss->name,rnd(10)+'A',rnd(10));
	c = getcoord();
	d = getdir();
	} while(!checkplace(ss,c,d));
    placeship(ss,c,d);
}

getdir(){
    int d;

    prompt(); addstr("What direction (0=right, 1=down) ? ");
    return(sgetc("01")-'0');
}

placeship(ss,c,d)
struct _ships *ss;
int c, d;
{
    int x, y, l, i;

    for(l=0; l<ss->length; ++l){
	i = c + l * ((d) ? 10 : 1);
	board[turn][i] = ss->symbol;
	x = (i % 10) * 3 + 3;
	y = (i / 10) + 7;
	if(!turn) mvaddch(y,x,ss->symbol);
	}
    ss->start = c;
    ss->dir = d;
    ss->hits = 0;
}

checkplace(ss,c,d)
struct _ships *ss;
int c, d;
{
    int x, y, l;

    x = c%10; y = c/10;
    if(((x+ss->length) > 10 && !d) || ((y+ss->length) > 10 && d==1)){
	if(!turn)
	    switch(rnd(3)){
		case 0:
		    error("Ship is hanging from the edge of the world");
		    break;
		case 1:
		    error("Try fitting it on the board");
		    break;
		case 2:
		    error("Figure I won't find it if you put it there?");
		    break;
		}
	return(0);
	}
    for(l=0; l<ss->length; ++l){
	x = c + l * ((d) ? 10 : 1);
	if(board[turn][x]){
	    if(!turn)
		switch(rnd(3)){
		    case 0:
			error("There's already a ship there");
			break;
		    case 1:
			error("Collision alert! Aaaaaagh!");
			break;
		    case 2:
			error("Er, Admiral, what about the other ship?");
			break;
		    }
	    return(0);
	    }
	}
    return(1);
}

error(s)
char *s;
{
    prompt(); beep();
    printw("%s -- hit any key to continue --",s);
    refresh();
    getch();
}

prompt(){
    move(22,0); clrtoeol();
}

toupper(ch)
int ch;
{
    return((ch >= 'a' && ch <= 'z') ? ch-'a'+'A' : ch);
}

getcoord(){
    int ch, x, y, oldx, oldy;

redo:
    y = sgetc("ABCDEFGHIJ");
    do{
	ch = getch();
	if(ch == 0x7F || ch == 8){
	    addstr("\b \b"); refresh();
	    goto redo;
	    }
	} while(ch < '0' || ch > '9');
    addch(x=ch); refresh();
    return((y-'A')*10+x-'0');
}

cpuplace(ss)
struct _ships *ss;
{
    int c, d;

    do{
	c = rnd(100); d = rnd(2);
	} while(!checkplace(ss,c,d));
    placeship(ss,c,d);
}

awinna(){
    int i, j;
    struct _ships *ss;

    for(i=0; i<2; ++i){
	ss = (i) ? cpuship : plyship;
	for(j=0; j<5; ++j, ++ss)
	    if(ss->length != ss->hits)
		break;
	if(j == 5) return(OTHER);
	}
    return(-1);
}

plyturn(){
    int c, res, i;
    char *m;

    prompt();
    addstr("Where do you want to shoot? ");
    c = getcoord();
    if(!(res = hits[turn][c])){
	hits[turn][c] = res = (board[OTHER][c]) ? 'H' : 'M';
	mvaddch(7+c/10,48+3*(c%10),(res=='H') ? 'H' : 'o');
	if(c = hitship(c)){
	    prompt();
	    switch(rnd(3)){
		case 0:
		    m = "You sank my %s!";
		    break;
		case 1:
		    m = "I have this sinking feeling about my %s....";
		    break;
		case 2:
		    m = "Have some mercy for my %s!";
		    break;
		}
	    move(23,0); clrtoeol(); beep();
	    printw(m,cpuship[c-1].name); refresh();
	    return(awinna() == -1);
	    }
	}
    prompt();
    move(23,0); clrtoeol();
    printw("You %s.",(res=='M')?"missed":"scored a hit"); refresh();
    return(res == 'H');
}

hitship(c)
int c;
{
    struct _ships *ss;
    int sym, i, j;

    ss = (turn) ? plyship : cpuship;
    if(!(sym = board[OTHER][c])) return(0);
    for(i=0; i<5; ++i, ++ss)
	if(ss->symbol == sym){
	    j = ss->hits; ++j; ss->hits = j;
	    if(j == ss->length) return(i+1);
	    return(0);
	    }
}

cputurn(){
    int c, res, x, y, i, d;

redo:
    if(cstart == -1){
	if(cpuhits){
	    for(i=0, c=rnd(100); i<100; ++i, c = (c+1) % 100)
		if(hits[turn][c] == 'H')
		    break;
	    if(i != 100){
		cstart = c;
		cdir = -1;
		goto fndir;
		}
	    }
	do{
	    i = 0;
	    do{
		while(hits[turn][c=rnd(100)]);
		x = c % 10; y = c / 10;
		if(++i == 1000) break;
		} while(((x+huntoffs) % srchstep) != (y % srchstep));
	    if(i == 1000) --srchstep;
	    } while(i == 1000);
	}
    else if(cdir == -1){
fndir:	for(i=0, d=rnd(4); i++ < 4; d = (d+1) % 4){
	    x = cstart%10; y = cstart/10;
	    switch(d){
		case 0: ++x; break;
		case 1: ++y; break;
		case 2: --x; break;
		case 3: --y; break;
		}
	    if(x<0 || x>9 || y<0 || y>9) continue;
	    if(hits[turn][c=y*10+x]) continue;
	    cdir = -2;
	    break;
	    }
	if(i == 4){
	    cstart = -1;
	    goto redo;
	    }
	}
    else{
	x = cstart%10; y = cstart/10;
	switch(cdir){
	    case 0: ++x; break;
	    case 1: ++y; break;
	    case 2: --x; break;
	    case 3: --y; break;
	    }
	if(x<0 || x>9 || y<0 || y>9 || hits[turn][y*10+x]){
	    cdir = (cdir+2) % 4;
	    for(;;){
		switch(cdir){
		    case 0: ++x; break;
		    case 1: ++y; break;
		    case 2: --x; break;
		    case 3: --y; break;
		    }
		if(x<0 || x>9 || y<0 || y>9){ cstart = -1; goto redo; }
		if(!hits[turn][y*10+x]) break;
		}
	    }
	c = y*10 + x;
	}

#ifdef NOASK
    res = (board[OTHER][c]) ? 'H' : 'M';
    move(21,0); clrtoeol();
    printw("I shoot at %c%d. I %s!",c/10+'A',c%10,(res=='H')?"hit":"miss");
#else
    for(;;){
	prompt();
	printw("I shoot at %c%d. Do I (H)it or (M)iss? ",c/10+'A',c%10);
	res = sgetc("HM");
	if((res=='H' && !board[OTHER][c]) || (res=='M' && board[OTHER][c])){
	    error("You lie!");
	    continue;
	    }
	break;
	}
    addch(res);
#endif
    hits[turn][c] = res;
    if(res == 'H'){
	++cpuhits;
	if(cstart == -1) cdir = -1;
	cstart = c;
	if(cdir == -2) cdir = d;
	mvaddch(7+(c/10),3+3*(c%10),'*');
	}
    else{
#ifdef SEEMISS
	mvaddch(7+(c/10),3+3*(c%10),' ');
#endif
	if(cdir == -2) cdir = -1;
	}
    if(c=hitship(c)){
	cstart = -1;
	cpuhits -= plyship[c-1].length;
	x = plyship[c-1].start;
	d = plyship[c-1].dir;
	y = plyship[c-1].length;
	for(i=0; i<y; ++i){
	    hits[turn][x] = '*';
	    x += (d) ? 10 : 1;
	    }
	}
    if(awinna() != -1) return(0);
    return(res == 'H');
}

playagain(){
    int i, x, y, dx, dy, j;

    for(i=0; i<5; ++i){
	x = cpuship[i].start; y = x/10+7; x = (x % 10) * 3 + 48;
	dx = (cpuship[i].dir) ? 0 : 3;
	dy = (cpuship[i].dir) ? 1 : 0;
	for(j=0; j < cpuship[i].length; ++j){
	    mvaddch(y,x,cpuship[i].symbol);
	    x += dx; y += dy;
	    }
	}

    if(awinna()) ++cpuwon; else ++plywon;
    i = 18 + strlen(name);
    if(plywon >= 10) ++i;
    if(cpuwon >= 10) ++i;
    mvprintw(2,(80-i)/2,"%s: %d     Computer: %d",name,plywon,cpuwon);

    prompt();
    printw((awinna()) ? "Want to be humiliated again, %s? "
		  : "Going to give me a chance for revenge, %s? ",name);
    return(sgetc("YN") == 'Y');
}

uninitgame(){
    refresh();
    resetterm(); echo(); endwin();
    exit(0);
}

sgetc(s)
char *s;
{
    char *s1;
    int ch;

    refresh();
    for(;;){
	ch = toupper(getch());
	if(ch == 3) uninitgame();
	for(s1=s; *s1 && ch != *s1; ++s1);
	if(*s1){
	    addch(ch); refresh();
	    return(ch);
	    }
	}
}
