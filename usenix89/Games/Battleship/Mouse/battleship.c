
/*
 * Battleship game, written by der Mouse (mcgill-vision!mouse).
 *
 * You have a fleet of ships to place on a grid, the computer has a similar
 *  fleet, which it places as it pleases.  You shoot alternately, the first
 *  one to knock out all the other player's ships wins.  The computer really
 *  plays hardball in the shooting phase; to help compensate, it has
 *  absolutely no intellegence about placing its ships.
 *
 * When you run the game, you will be faced with a rather cryptic screen
 *  showing two boards and some letters at the top.  The two boards are yours
 *  and the computer's (yours is on the left); the strings at the top are the
 *  ships you have available.  The game is waiting for you to place your
 *  ships on your board.  To do so, move around (see below) and mark (see
 *  below) first one end and then the other, for each ship in your fleet.
 *  The game is smart enough to figure out which ship you are trying to place
 *  (this stage considers all ships of equal length to be identical).  There
 *  is no way to `unplace' a ship, once you mark the second point you are
 *  stuck with it.  If you mark the first point and you don't want the ship
 *  there, you can either mark the same point again, which clears the mark,
 *  or you can do something else illegal (like try to place the ship in an
 *  illegal direction).  Ships can be placed in the 8 principal directions,
 *  and there is no prohibition on diagonal ships being interlaced.
 *
 * To move, use (a) Rogue/Hack keys, (b) the 3x3 keypad centered on the 's'
 *  key ('q', 'w', 'e', 'd', 'c', 'x', 'z', 'a'), or (on terminals with a
 *  numeric keypad), the 3x3 keypad centered on the '5' key.  To select a
 *  point, use the escape key, the '.' key, LINEFEED, or RETURN.
 *
 * Once all your ships are placed, the computer places its ships (actually,
 *  it places them before you get to place yours, but the difference is
 *  irrelevant).  Then you get to start shooting.  Which of you shoots first
 *  is random; thereafter you shoot alternately.  When you shoot, the pixel
 *  you shot is displayed in whatever `standout' mode your terminal provides;
 *  if it was a miss, the '.' which was there is just highlighted; if it was
 *  a hit, a character indicating the ship type appears (highlighted).  That
 *  is, when you hit, you know what type of ship you hit.  This was done
 *  because the algorithm I use for the computer requires this information as
 *  well, and of course I had to keep the game fair.
 *
 * Eventually, one of you will lose.  When this happens, your score is
 *  updated and you are asked whether you want to play again.  If you say no,
 *  your score is updated in the global file (if one is configured into the
 *  game); if you say yes, things restart from the point you were at just
 *  after starting the game.
 *
 * --------------Installation
 *
 * Edit the configuration parameters below to suit you, then compile with
 *
 *	cc -o <whatever> <this file> -lcurses -ltermcap
 *
 *  (-ltermcap is known as -ltermlib on some systems, I believe).
 */

/*
 * Configuration parameters.
 *
 * BDSIZE is the size of the board.  Each board is this many pixels square.
 *  Battleship does not check that the boards fit on the screen (this could
 *  be considered a bug).
 *
 * ship_lens[] is an array listing the ships, by size.  Battleship can go
 *  into an infinite loop if there are enough ships here that it is possible
 *  to place some such that another cannot be placed.
 *
 * ship_chars[] is an array of characters used for displaying the ships.
 *  Order must match ship_lens[].  These characters should all be distinct;
 *  otherwise the computer will have more information than the player.
 *
 * score_file[] is the pathname of the global score file, which is used to
 *  remember scores between games.  If you do not want to keep a score file,
 *  you can either set this to /dev/null or you can configure it as a
 *  non-existant file.  The game will not create the file if it does not
 *  exist; to start with an empty list, create the score file with zero size
 *  (eg, use cp /dev/null to it).  The game must be able to write the score
 *  file; if this is not the case it will behave as if no score file were
 *  configured.  This means that either the score file must be writable by
 *  all users who will play battleship or that battleship must be setuid (or
 *  setgid) to a uid (gid) which can write the score file.  The setuid/setgid
 *  solution is more secure, but if you are worried about making some code
 *  you just pulled off USEnet setuid (even though it need not be to root),
 *  you can just go with a mode 666 score file (though then any joker who
 *  pleases can modify their or anyone's score, though why they'd want to I
 *  dunno).
 */

#define BDSIZE 10

int ship_lens[] = { 5, 4, 3, 3, 2 };
char ship_chars[] = "ABDSP";
char score_file[] = "/u1/mouse/games/battleship/scores";

/*
 * End of configuration parameters
 */

#include <curses.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/file.h>

char *getenv();

#define ARRAYSIZE(arr) (sizeof(arr)/sizeof(arr[0]))

#define NSHIPS (ARRAYSIZE(ship_lens))

typedef struct {
	  unsigned char x;
	  unsigned char y; } POS;

typedef struct {
	  POS scr;
	  POS pos;
	  char shipno;
	  char force_lit;
	  char showhidden;
	  char shot_at; } PIXEL;

typedef struct _board {
	  int s_o_x;
	  int s_o_y;
	  POS (*getshot)();
	  int (*aftermath)();
	  int (*he_sunk)();
	  int (*you_sunk)();
	  struct _board *target;
	  struct _board *nextturn;
	  char hits_left[NSHIPS];
	  PIXEL b[BDSIZE][BDSIZE]; } BOARD;

BOARD Human;
#define human (&Human)
BOARD Computer;
#define computer (&Computer)
BOARD *turn;
int min_ship_len;
int max_ship_len;
int nwon_human;
int nwon_computer;
jmp_buf done_jmp;
FILE *scoref;
char *username;
char human_name[512];

/* sigh, the problems of distributing code.... */
int mth$int_random()
{
 static long int init = 0;

 if (init == 0)
  { time(&init);
    srandom((int)init);
  }
 return(random());
}

int rnd(min,max)
register int min;
register int max;
{
 return((mth$int_random()%(max+1-min))+min);
}

int max(a,b)
register int a;
register int b;
{
 return((a>b)?a:b);
}

char char_for(p)
register PIXEL *p;
{
 if (p->shipno >= 0)
  { return(ship_chars[p->shipno]);
  }
 else
  { return('.');
  }
}

int oob(x,y)
register int x;
register int y;
{
 return((x<0)||(y<0)||(x>=BDSIZE)||(y>=BDSIZE));
}

int ship_ok(board,shipno,x,y,dx,dy)
BOARD *board;
int shipno;
int x;
int y;
int dx;
int dy;
{
 int len;

 if ((dx == 0) && (dy == 0))
  { return(0);
  }
 for (len=ship_lens[shipno];len>0;len--)
  { if (oob(x,y) || (board->b[x][y].shipno >= 0))
     { return(0);
     }
    x += dx;
    y += dy;
  }
 return(1);
}

POS getpos(board)
BOARD *board;
{
 int ox;
 int oy;
 static int x = BDSIZE / 2;
 static int y = BDSIZE / 2;
 char c;
 POS pos;

 ox = board->s_o_x;
 oy = board->s_o_y;
 while (1)
  { move(oy+y,ox+x+x);
    refresh();
    c = getch();
    clear_message();
    switch (c)
     { default:
	  putchar('\7');
	  fflush(stdout);
	  break;
       case '.': case '\33': case '\r': case '\n':
	  pos.x = x;
	  pos.y = y;
	  return(pos);
	  break;
       case 'h': case 'a': case '4':
	  x --;
	  break;
       case 'j': case 'x': case '2':
	  y ++;
	  break;
       case 'k': case 'w': case '8':
	  y --;
	  break;
       case 'l': case 'd': case '6':
	  x ++;
	  break;
       case 'y': case 'q': case '7':
	  x --;
	  y --;
	  break;
       case 'u': case 'e': case '9':
	  x ++;
	  y --;
	  break;
       case 'b': case 'z': case '1':
	  x --;
	  y ++;
	  break;
       case 'n': case 'c': case '3':
	  x ++;
	  y ++;
	  break;
     }
    if (x < 0)
     { x = 0;
     }
    if (x >= BDSIZE)
     { x = BDSIZE - 1;
     }
    if (y < 0)
     { y = 0;
     }
    if (y >= BDSIZE)
     { y = BDSIZE - 1;
     }
  }
}

POS get_human_shot();		/* these are defined at the end */
int human_aftermath();
int human_he_sunk();
int human_you_sunk();
POS get_computer_shot();
int computer_aftermath();
int computer_he_sunk();
int computer_you_sunk();

int yes(msg)
char *msg;
{
 char resp;

 message("%s",msg);
 while (1)
  { refresh();
    resp = getch();
    switch (resp)
     { case 'y': case 'Y': case '1': case 't': case 'T':
	  return(1);
	  break;
       case 'n': case 'N': case '0':
	  return(0);
	  break;
     }
    message("%s (Y/N) ",msg);
  }
}

int taketurn()
{
 POS shot;
 PIXEL *p;
 BOARD *t;

 shot = (*turn->getshot)(turn);
 t = turn->target;
 p = &t->b[shot.x][shot.y];
 (*turn->aftermath)(p->pos.x,p->pos.y,p->shipno);
 if (! p->shot_at)
  { p->shot_at = 1;
    if (p->shipno >= 0)
     { t->hits_left[p->shipno] --;
       if (t->hits_left[p->shipno] <= 0)
	{ (*t->he_sunk)(p->shipno);
	  (*turn->you_sunk)(p->shipno);
	}
     }
  }
 update_pixel(p);
 turn = turn->nextturn;
}

int gamewon()
{
 int i;
 int h;
 int c;

 h = 0;
 c = 0;
 for (i=0;i<NSHIPS;i++)
  { h += human->hits_left[i];
    c += computer->hits_left[i];
  }
 if ((h != 0) && (c != 0))
  { return(0);
  }
 if ((h == 0) && (c == 0))
  { message("INTERNAL ERROR, both human and computer done at once!");
    refresh();
    endwin();
    abort();
  }
 if (h == 0)
  { nwon_computer ++;
  }
 else
  { nwon_human ++;
  }
 drawscore();
 showallships();
}

int another_game()
{
 return(yes("Care for another? "));
}

message(fmt,a1,a2,a3,a4,a5,a6,a7,a8)
char *fmt;
int a1;
int a2;
int a3;
int a4;
int a5;
int a6;
int a7;
int a8;
{
 move(LINES-1,0);
 printw(fmt,a1,a2,a3,a4,a5,a6,a7,a8);
 clrtoeol();
}

clear_message()
{
 move(LINES-1,0);
 clrtoeol();
}

topline(fmt,a1,a2,a3,a4,a5,a6,a7,a8)
char *fmt;
int a1;
int a2;
int a3;
int a4;
int a5;
int a6;
int a7;
int a8;
{
 move(0,0);
 printw(fmt,a1,a2,a3,a4,a5,a6,a7,a8);
 clrtoeol();
}

clear_topline()
{
 topline("");
}

init_ships()
{
 int i;

 min_ship_len = ship_lens[0];
 max_ship_len = ship_lens[0];
 for (i=1;i<NSHIPS;i++)
  { if (ship_lens[i] < min_ship_len)
     { min_ship_len = ship_lens[i];
     }
    else if (ship_lens[i] > max_ship_len)
     { max_ship_len = ship_lens[i];
     }
  }
}

init_boards()
{
 int i;
 int j;
 PIXEL *p;

 human->s_o_x = (COLS - ((4 * BDSIZE) - 2)) / 3;
 human->s_o_y = (LINES - BDSIZE) / 2;
 human->getshot = get_human_shot;
 human->aftermath = human_aftermath;
 human->he_sunk = human_he_sunk;
 human->you_sunk = human_you_sunk;
 human->nextturn = computer;
 human->target = computer;
 computer->s_o_x = COLS - human->s_o_x - ((2 * BDSIZE) - 1);
 computer->s_o_y = human->s_o_y;
 computer->getshot = get_computer_shot;
 computer->aftermath = computer_aftermath;
 computer->he_sunk = computer_he_sunk;
 computer->you_sunk = computer_you_sunk;
 computer->nextturn = human;
 computer->target = human;
 for (i=0;i<BDSIZE;i++)
  { for (j=0;j<BDSIZE;j++)
     { p = &human->b[i][j];
       p->shipno = -1;
       p->shot_at = 0;
       p->force_lit = 0;
       p->pos.x = i;
       p->pos.y = j;
       p->scr.x = human->s_o_x + (2 * i);
       p->scr.y = human->s_o_y + j;
       p->showhidden = 1;
       p = &computer->b[i][j];
       p->shipno = -1;
       p->shot_at = 0;
       p->force_lit = 0;
       p->pos.x = i;
       p->pos.y = j;
       p->scr.x = computer->s_o_x + (2 * i);
       p->scr.y = computer->s_o_y + j;
       p->showhidden = 0;
     }
  }
 for (i=0;i<NSHIPS;i++)
  { human->hits_left[i] = ship_lens[i];
    computer->hits_left[i] = ship_lens[i];
  }
 computer_init();
}

showallships()
{
 int i;
 int j;
 PIXEL *p;

 for (i=0;i<BDSIZE;i++)
  { for (j=0;j<BDSIZE;j++)
     { p = &computer->b[i][j];
       p->showhidden = 1;
     }
  }
 drawboard(computer);
}

complete_redraw()
{
 int i;
 int j;
 PIXEL *p;

 clear();
 drawboard(human);
 drawboard(computer);
 drawscore();
}

drawscore()
{
 topline("%s %d     Computer %d",human_name,nwon_human,nwon_computer);
}

drawboard(board)
BOARD *board;
{
 int i;
 int j;
 PIXEL *p;

 for (i=0;i<BDSIZE;i++)
  { for (j=0;j<BDSIZE;j++)
     { update_pixel(&board->b[i][j]);
     }
  }
}

update_pixel(p)
PIXEL *p;
{
 if (p->shot_at || p->force_lit)
  { standout();
  }
 mvaddch( (int)p->scr.y,
	  (int)p->scr.x,
	  (p->shot_at||p->showhidden) ? char_for(p) : '.' );
 standend();
}

place_a_ship_randomly(shipno,board)
int shipno;
BOARD *board;
{
 int x;
 int y;
 int dx;
 int dy;

 do
  { x = rnd(0,BDSIZE-1);
    y = rnd(0,BDSIZE-1);
    dx = rnd(-1,1);
    dy = rnd(-1,1);
  } while (!ship_ok(board,shipno,x,y,dx,dy));
 enter_ship(board,shipno,x,y,dx,dy,0);
}

enter_ship(board,shipno,x,y,dx,dy,update)
BOARD *board;
int shipno;
int x;
int y;
int dx;
int dy;
int update;
{
 int len;

 for (len=ship_lens[shipno];len>0;len--)
  { board->b[x][y].shipno = shipno;
    if (update)
     { update_pixel(&board->b[x][y]);
     }
    x += dx;
    y += dy;
  }
}

place_computer_ships()
{
 int i;

 for (i=0;i<NSHIPS;i++)
  { place_a_ship_randomly(i,computer);
  }
}

place_human_ships()
{
 char done[NSHIPS];
 char scrx[NSHIPS];
 int i;
 int x;
 POS end;
 PIXEL *p1;
 PIXEL *p2;
 int dx;
 int dy;
 int len;
 int shipsleft;
 char c;

 move(0,0);
 clrtoeol();
 x = 0;
 for (i=0;i<NSHIPS;i++)
  { done[i] = 0;
    scrx[i] = x;
    x += ship_lens[i] + 1;
  }
 shipsleft = NSHIPS;
 while (shipsleft > 0)
  { for (i=0;i<NSHIPS;i++)
     { c = done[i] ? ' ' : ship_chars[i];
       move(0,scrx[i]);
       for (x=0;x<ship_lens[i];x++)
	{ addch(c);
	}
     }
    end = getpos(human);
    p1 = &human->b[end.x][end.y];
    p1->force_lit = 1;
    update_pixel(p1);
    p1->force_lit = 0;
    end = getpos(human);
    p2 = &human->b[end.x][end.y];
    update_pixel(p1);
    dx = p2->pos.x - p1->pos.x;
    dy = p2->pos.y - p1->pos.y;
    len = max(abs(dx),abs(dy));
    if (len == 0)
     { continue;
     }
    if ( (dx && (dx != len) && (dx != -len)) ||
	 (dy && (dy != len) && (dy != -len)) )
     { message("Ships must be placed orthogonally or diagonally!");
       continue;
     }
    dx /= len;
    dy /= len;
    len ++;
    if (len > max_ship_len)
     { message("There are no ships that long!");
       continue;
     }
    if (len < min_ship_len)
     { message("There are no ships that short!");
       continue;
     }
    for (i=0;i<NSHIPS;i++)
     { if ((ship_lens[i] == len) && !done[i])
	{ break;
	}
     }
    if (i >= NSHIPS)
     { message("You have already placed all your ships of that length!");
       continue;
     }
    if (! ship_ok(human,i,p1->pos.x,p1->pos.y,dx,dy))
     { message("You can't put that ship there!");
       continue;
     }
    enter_ship(human,i,p1->pos.x,p1->pos.y,dx,dy,1);
    done[i] = 1;
    shipsleft --;
  }
 move(0,0);
 clrtoeol();
 drawscore();
}

pause()
{
 refresh();
 getch();
}

done()
{
 longjmp(done_jmp,1);
}

main()
{
 int havescores;

 initscr();
 noecho();
 nonl();
 crmode();
 havescores = 0;
 if (setjmp(done_jmp))
  { move(LINES-1,0);
    refresh();
    endwin();
    printf("\n");
    if (havescores)
     { savescores();
     }
    exit(0);
  }
 signal(SIGINT,done);
 init_scores();
 havescores = 1;
 do
  { init_ships();
    init_boards();
    complete_redraw();
    place_computer_ships();
    place_human_ships();
    turn = (mth$int_random() & 1) ? human : computer;
    while (! gamewon())
     { taketurn();
     }
  } while (another_game());
 done();
}

lock_scoref()
{
 flock(fileno(scoref),LOCK_EX);
}

unlock_scoref()
{
 flock(fileno(scoref),LOCK_UN);
}

capitalize(str)
char str[];
{
 char *cp;
 int inword = 0;

 for (cp=str;*cp;cp++)
  { if (isascii(*cp) && isalnum(*cp))
     { if (!inword)
	{ if (islower(*cp))
	   { *cp = toupper(*cp);
	   }
	}
       inword = 1;
     }
    else
     { inword = 0;
     }
  }
}

init_scores()
{
 char sbuf[512];
 char user[512];
 int hwon;
 int cwon;

 username = getenv("USER");
 if (username == 0)
  { username = "anonymous";
  }
 strcpy(human_name,username);
 capitalize(human_name);
 scoref = fopen(score_file,"r+");
 if (scoref == 0)
  { scoref = fopen("/dev/null","r+");
    if (scoref == 0)
     { message("Cannot open scorefile or /dev/null, sorry");
       done();
     }
  }
 rewind(scoref);
 lock_scoref();
 while (fgets(sbuf,sizeof(sbuf),scoref) == sbuf)
  { if (sscanf(sbuf,"%s%d%d",user,&hwon,&cwon) == 3)
     { if (strcmp(user,username) == 0)
	{ rewind(scoref);
	  unlock_scoref();
	  nwon_human = hwon;
	  nwon_computer = cwon;
	  return;
	}
     }
  }
 rewind(scoref);
 unlock_scoref();
 nwon_human = 0;
 nwon_computer = 0;
}

savescores()
{
 char sbuf[512];
 char user[512];
 int hwon;
 int cwon;
 long int rbegin;

 rewind(scoref);
 lock_scoref();
 while (1)
  { rbegin = ftell(scoref);
    if (fgets(sbuf,sizeof(sbuf),scoref) != sbuf)
     { break;
     }
    if (sscanf(sbuf,"%s%d%d",user,&hwon,&cwon) == 3)
     { if (strcmp(user,username) == 0)
	{ fseek(scoref,rbegin,0);
	  fprintf(scoref,"%s %10d %10d\n",username,nwon_human,nwon_computer);
	  rewind(scoref);
	  unlock_scoref();
	  return;
	}
     }
  }
 fseek(scoref,0L,2);
 fprintf(scoref,"%s %10d %10d\n",username,nwon_human,nwon_computer);
 rewind(scoref);
 unlock_scoref();
}

POS get_human_shot(b)
BOARD *b;
{
 return(getpos(b->target));
}

int human_aftermath(x,y,t)
int x;
int y;
int t;
{
}

int human_he_sunk(sno)
int sno;
{
 message("Gotcha!");
}

int human_you_sunk(sno)
int sno;
{
 message("Ouch!");
}

char known[BDSIZE][BDSIZE];
#define UNKNOWN (-2)
#define EMPTY (-1)
char shiphot[NSHIPS];
char shipdead[NSHIPS];
int count[BDSIZE][BDSIZE];

computer_init()
{
 int i;
 int j;

 for (i=0;i<BDSIZE;i++)
  { for (j=0;j<BDSIZE;j++)
     { known[i][j] = UNKNOWN;
     }
  }
 for (i=0;i<NSHIPS;i++)
  { shipdead[i] = 0;
    shiphot[i] = 0;
  }
}

computer_aftermath(x,y,sno)
int x;
int y;
int sno;
{
 if (sno >= 0)
  { known[x][y] = sno;
    shiphot[sno] = 1;
  }
 else
  { known[x][y] = EMPTY;
  }
}

computer_he_sunk(sno)
int sno;
{
}

computer_you_sunk(sno)
int sno;
{
 shipdead[sno] = 1;
 shiphot[sno] = 0;
}

maybe_count(b,sno,sx,sy,dx,dy)
BOARD *b;
int sno;
int sx;
int sy;
int dx;
int dy;
{
 int n;
 int i;
 int x;
 int y;

 x = sx;
 y = sy;
 n = 0;
 for (i=ship_lens[sno];i>0;i--,(x+=dx),(y+=dy))
  { if (known[x][y] == EMPTY)
     { return;
     }
    if (known[x][y] == UNKNOWN)
     { continue;
     }
    if (known[x][y] == sno)
     { n ++;
     }
    else
     { return;
     }
  }
 if (shiphot[sno] && (n != ship_lens[sno]-b->hits_left[sno]))
  { return;
  }
 x = sx;
 y = sy;
 for (i=ship_lens[sno];i>0;i--,(x+=dx),(y+=dy))
  { count[x][y] ++;
  }
}

drawcountpixel(x,y)
int x;
int y;
{
 PIXEL *p;

 p = &human->b[x][y];
 mvaddch(p->scr.y,p->scr.x-1,
" -~=+*#@123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
				[count[x][y]]);
}

POS get_computer_shot(b)
BOARD *b;
{
 int sno;
 int i;
 int j;
 int best;
 int n;
 POS shot;
 BOARD *target;
 char c;
 PIXEL *p;
 char interesting[NSHIPS];

 for (i=0;i<NSHIPS;i++)
  { interesting[i] = ! shipdead[i];
  }
 for (i=0;i<NSHIPS;i++)
  { if (shiphot[i])
     { for (i=0;i<NSHIPS;i++)
	{ interesting[i] &= shiphot[i];
	}
       break;
     }
  }
/*
 move(1,0);
 printw("Dead: ");
 for(i=0;i<NSHIPS;i++)
  { addch(shipdead[i]?ship_chars[i]:'-');
  }
 printw("    Hot: ");
 for(i=0;i<NSHIPS;i++)
  { addch(shiphot[i]?ship_chars[i]:'-');
  }
 printw("    Interesting: ");
 for(i=0;i<NSHIPS;i++)
  { addch(interesting[i]?ship_chars[i]:'-');
  }
 clrtoeol();
 for (i=0;i<BDSIZE;i++)
  { for (j=0;j<BDSIZE;j++)
     { p = &human->b[i][j];
       switch (known[i][j])
	{ case UNKNOWN:
	     c = '?';
	     break;
	  case EMPTY:
	     c = ' ';
	     break;
	  default:
	     c = ship_chars[known[i][j]];
	     break;
	}
       mvaddch(p->scr.y,p->scr.x-1,c);
     }
  }
*/
 for (i=0;i<BDSIZE;i++)
  { for (j=0;j<BDSIZE;j++)
     { count[i][j] = 0;
     }
  }
 target = b->target;
 for (sno=0;sno<NSHIPS;sno++)
  { if (interesting[sno])
     { for (i=ship_lens[sno]-1;i<BDSIZE;i++)
	{ for (j=0;j<BDSIZE;j++)
	   { maybe_count(target,sno,j,i,0,-1);
	     maybe_count(target,sno,i,j,-1,0);
	   }
	  for (j=ship_lens[sno]-1;j<BDSIZE;j++)
	   { maybe_count(target,sno,j,i,-1,-1);
	   }
	  for (j=BDSIZE-ship_lens[sno];j>=0;j--)
	   { maybe_count(target,sno,j,i,1,-1);
	   }
	}
     }
  }
 for (i=0;i<BDSIZE;i++)
  { for (j=0;j<BDSIZE;j++)
     { if (known[i][j] != UNKNOWN)
	{ count[i][j] = 0;
	}
     }
  }
 best = 0;
 do
  { shot.x = mth$int_random() % BDSIZE;
    shot.y = mth$int_random() % BDSIZE;
  } while (known[shot.x][shot.y] != UNKNOWN);
 n = 0;
 for (i=0;i<BDSIZE;i++)
  { for (j=0;j<BDSIZE;j++)
     { if (count[i][j] > best)
	{ best = count[i][j];
	  n = 1;
	  shot.x = i;
	  shot.y = j;
	}
       else if (count[i][j] == best)
	{ n ++;
	}
     }
  }
 n = mth$int_random() % n;
 for (i=0;i<BDSIZE;i++)
  { for (j=0;j<BDSIZE;j++)
     { if (count[i][j] == best)
	{ n --;
	  if (n < 0)
	   { shot.x = i;
	     shot.y = j;
	     return(shot);
	   }
	}
     }
  }
 message("Error, cannot find shot for computer!");
 pause();
 return(shot);
}
