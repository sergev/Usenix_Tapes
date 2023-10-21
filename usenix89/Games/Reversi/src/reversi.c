/************************************************
 *                                         	*
 *           REVERSI                       	*
 *                                         	*
 *   Copyright: Chris Miller, 1979, 1981, 1984  *
 *                                         	*
 ************************************************/

/*	The program requires the curses library. */

/*      Notes on efficiency.

	The best way to speed up the program would probably be to
	improve the heuristic ordering of moves in the search. This
	may require generating all legal successor positions and doing
	a quick and dirty static evaluation on them; coding this would
	be a lot of work.

	Most of the program's time is spent in the routines:
		sandwich   (c.20%)
		legal         15%
		domove        15%
		static_eval   15%
	so that any tweaking should be done in these procedures. A certain
	amount has already been done which explains the rather hacked code,
	especially in sandwich
*/

#include <curses.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/times.h>

long time();
char *sprintf();

#define SKIP ;

#define FOR_BOARD(i)  for (i=0; i<64; i++)

#define NOMOVE -1
#define ismove(pos,player) (nextmove(pos,NOMOVE,(POSITION *) NULL, player) != NOMOVE)

#define INFINITY 10000

#define WHITE -1
#define BLACK 1
#define FREE  0

#define MAXDEPTH 6
#define INIT_DEPTH 2
int depth = INIT_DEPTH;

#define MAXASPIRE 6
#define INIT_ASPIRE 3
int aspire = INIT_ASPIRE;
int expected, margin;        /* Bounds on aspiration for alphabeta search */

char input[20];
int machine_piece, human_piece;
int trace_eval = FALSE, remark = TRUE, cretin_flag;
int last_remark;
#define FIRST_REMARK (INFINITY+1)
long ab_count, se_count;        /* Calls of alfabeta and static_eval */

#define	libpath(l,c) l/c"
char infofil[] = libpath(LIB,reversi.doc);
char rem_file[] = libpath(LIB,reversi.remark);

typedef struct {
	char    SQUARE[64];
	int	MOVES_LEFT;
	} POSITION;

jmp_buf newgame, undomove, intrundo;

WINDOW	*wboard, *wmoves, *wtrace, *wremarks, *wmessages;

main ()
{
	int finish_up ();
	int oninterrupt ();
	int onquit ();
	int exit ();
	long initrand;

	signal (SIGQUIT, exit);

	initscr();
	initwindows();
	crmode();
	noecho();
	move(0, 16);
	addstr ("R E V E R S I\n\n");

	while (TRUE)
	    {
		message ("Do you want instructions? ");
		*input = getch();
		if (*input == 'y' || *input == 'Y')
		    {	info();
			break;
		    }
		if (*input == 'n' || *input == 'N') break;
		bell();
	    }

	initrand = time ((long *) NULL);        /* Initialise random number generator */
	srand ((int) (initrand & 0377));

	get_remarks ();

	signal (SIGINT, oninterrupt);
	signal (SIGQUIT, onquit);

	while (TRUE)
	    {   setjmp(newgame);
		setjmp(undomove);
		setjmp(intrundo);
		playgame();
		setjmp(intrundo);
		setjmp(undomove);
		message("Would you like another game? ");
		*input = getch();
		if (*input == 'n' || *input == 'N') break;
		if (*input != 'y' && *input != 'Y') {
			message ("I assume that grunt means yes!\n");
			touchwin(wmoves);
			touchwin(wtrace);
			touchwin(wremarks);
		}
	    }

	finish_up ();
}

finish_up ()
{
	wclear(wtrace);
	wrefresh(wtrace);
	move(LINES-3, 0);
	refresh();
	endwin();
	puts ("Thank you for your company.\nAu revoir ...");
	exit (0);
}

oninterrupt ()
{       char ch;
	int oninterrupt();

	signal (SIGINT, SIG_IGN);      /* Ignore interrupts within interrupts */

	wclear(wtrace);
	waddstr(wtrace, "            *** Interrupt ***\n");
        waddstr(wtrace, "x to exit program        c to continue execution\n");
	waddstr(wtrace, "n to start a new game    u to undo last move\n");
	waddstr(wtrace, "i for instructions\n");
	wrefresh(wtrace);
	while (TRUE)
	    {   
		while (whitesp (ch=getch())) SKIP
		switch (ch)
		    {
			case 'x':       message ("*** EXIT ***\n");
					finish_up();

			case 'u':
					signal (SIGINT, oninterrupt);
					wclear(wmessages);
					wclear(wtrace);
					redraw();
					longjmp(intrundo, 1);

			case 'n':
					signal (SIGINT, oninterrupt);
					wclear(wmessages);
					wclear(wtrace);
					redraw();
					longjmp(newgame, 0);

			case 'c':       
					wclear(wmessages);
					waddstr(wmessages, "*** Continue ***");
					wclear(wtrace);
					redraw();
					signal(SIGINT, oninterrupt);
					return;

			case 'i':	info();
					wclear(wtrace);
					redraw();
					signal(SIGINT, oninterrupt);
					return;
		    }
	    }
}

onquit ()
{       message ("*** QUIT ***");
	finish_up ();
}

playgame ()
{
	POSITION board, saveboard, oldboard;
	register int  i;
	int colour = BLACK, white = 0, black = 0;

	wclear(stdscr);
	wrefresh(stdscr);
	wclear(wboard);
	wclear(wtrace);
	wclear(wremarks);
	wclear(wmessages);
	wclear(wmoves);
	touchwin(wboard);
	touchwin(wtrace);
	touchwin(wremarks);
	touchwin(wmessages);
	touchwin(wmoves);
	while (TRUE)
	    {   
		message ("What colour would you like? ");
		*input = getch();
		if (*input == 'w' || *input == 'W')
		    {	human_piece = WHITE;
			machine_piece = BLACK;
			break;
		    }
		if (*input == 'b' || *input == 'B')
		    {	human_piece = BLACK;
			machine_piece = WHITE;
			break;
		    }
		bell();
	    }

	FOR_BOARD (i) board.SQUARE[i] = FREE;
	board.MOVES_LEFT = 64;
	saveboard = board;
	cretin_flag = FALSE;
	last_remark = FIRST_REMARK;     /* Force a remark first time */

	clear();
	display (&board);

	while (!game_over(&board))
	    {   if (colour==human_piece)
		    {   oldboard = saveboard;
			saveboard = board;
			if (setjmp(undomove)!=0)
			    {   board = oldboard;
				colour = human_piece;
				display (&board);
				continue;
			    }
			if (setjmp(intrundo)!=0)
			    {   board = saveboard;
				colour = human_piece;
				display (&board);
				continue;
			    }
		    }

		if (!makemove(&board, &colour)) return;
			/* Pass a pointer to colour so that restore
				can reset it (this is ugly!)
			*/
		colour = -colour;
	    }

	if (machine_piece==colour)
	    {   
		display (&board);
	    }

	FOR_BOARD (i)
		switch (board.SQUARE[i])
		    {	case WHITE:	white++; break;
			case BLACK:	black++;
		    }

	setjmp(intrundo);
	clear();
	refresh();
	touchwin(wboard);
	display(&board);
	move(13, 0);
	printw ("Game over: I have %d pieces, you have %d pieces",
		(machine_piece==WHITE? white: black),
		(human_piece==BLACK? black: white));
	if (white==black)
	    {   addstr ("\n\n\t*** Game drawn! ***\n");
		refresh();
		return;
	    }
	if ((machine_piece==WHITE && white>black) ||
	    (machine_piece==BLACK && black>white))
	    {   addstr("\n\n\t*** I win ***\n");
		if (cretin_flag) addstr ("\n\t*** CRETIN! ***\n");
	    }
	   else addstr("\n\n\t*** You win ***\n");
	refresh();
}

makemove (board, colour)
	POSITION *board;
	int *colour;
{
	char letter; int number;
	int mv = NOMOVE, value;
	int d;
	struct tms timesbefore, timesafter;
	long realtime, cputime;

    getmove:
	if (*colour == human_piece)
	    {   if (!ismove (board, *colour))
		    {
			wmove(wmoves, 0, 0);
			waddstr (wmoves, "** You have no legal move **");
			wclrtoeol(wmoves);
			wrefresh(wmoves);
			sleep (3);
			return (TRUE);
		    }

		wmove(wmoves, 0, 0);
		wprintw(wmoves, "Your move (%s): __",
			human_piece==BLACK? "XX": "OO");
		wclrtoeol(wmoves);
		waddstr(wmoves, "\b\b");
		wmove(wmoves, 0, 16);
		wrefresh(wmoves);
		getreply (wmoves, input);
		decode (input, &letter, &number);
		if (('h'<letter) || (1>number) || (8<number) || ('a'>letter))
			{ if (!do_command(letter, number, board, colour))
				return (FALSE);
			}
		   else if (okmove(board, letter, number, *colour))
			    return(TRUE);
		goto getmove;
	    }

	if (*colour == machine_piece)
	    {   if ((mv = nextmove (board, NOMOVE, 0, *colour)) == NOMOVE)
		    {   
			wmove(wmoves, 1, 0);
			wprintw (wmoves, "%s","** I have no legal move **");
			wclrtoeol(wmoves);
			wrefresh(wmoves);
			if (remark) {
				wmove (wremarks, 0, 0);
				waddstr (wremarks, "I'm not dead, just resting");
				wclrtoeol(wremarks);
				wrefresh(wremarks);
			}
			display (board);
			return (TRUE);
		    }

		ab_count = se_count = 0;
		if (trace_eval)
		    {   times(&timesbefore);
			realtime = time((long *) NULL);
		    }
		if ((!remark) && (!trace_eval) &&
			(nextmove(board, mv, 0, colour) == NOMOVE))
		    {		/* Only one legal move - make it */
				/* (provided we don't need dynamic value) */
			value = static_eval (board);
			goto movefound;
		    }

		if ((board->MOVES_LEFT <= 10) &&
			(board->MOVES_LEFT <= 3*depth))
			/* Exhaustive for last 3*depth ply (at most 10!) */
			d = board->MOVES_LEFT;
		   else d = depth;

		set_aspiration (board);
		value = alfabeta (board, d, (*colour)*INFINITY, *colour, &mv);

	    movefound:
		wmove(wmoves, 1, 0);
		wprintw(wmoves, "My move   (%s): ",
			machine_piece==BLACK? "XX": "OO");
		wclrtoeol(wmoves);
		wmove(wmoves, 1, 16);
		wprintw(wmoves, "%c%c", (mv&7)+'a', (mv>>3)+'1');
		wrefresh(wmoves);
		domove (board, mv, board, *colour);
		if (remark) make_remark(value);
		if (trace_eval)
		    {
			wmove (wtrace, 0, 0);
			wprintw(wtrace, "Depth: %d; Level: %d",
					depth, aspire);
			wclrtoeol(wtrace);
			wprintw(wtrace, "\nValue: %5d", value);
			wclrtoeol(wtrace);
			wprintw(wtrace,
			    "\nBoards evaluated - static: %5D, dynamic: %5D",
				se_count, ab_count);
			wclrtoeol(wtrace);

			times (&timesafter);
			cputime = timesafter.tms_utime + timesafter.tms_stime
				   - timesbefore.tms_utime - timesbefore.tms_stime;

			wprintw(wtrace, "\nTime - elapsed: %4D, cpu: %4D.%1D",
				(time(0) - realtime),
				cputime/60,             /* Seconds */
				(cputime/6) % 10);      /* Tenths */
			wclrtoeol(wtrace);
			wprintw(wtrace, "\nBoards per cpu second: %.2f",
				(double) (se_count+ab_count) * 60.0
					/ (double) cputime);
			wclrtoeol(wtrace);
			wrefresh(wtrace);
		    }
		display (board);
		return (TRUE);
	    }
	return(FALSE);
}

okmove (board, letter, number, colour)
	POSITION *board;
	char letter;
	int number;
	int colour;
{
	int mv;

	if ((number<1) || (number>8) || (letter<'a') || (letter>'h'))
	    {   
		message ("Column should be in range a-h, row in range 1-8");
		bell();
		return (FALSE);
	    }

	mv = letter - 'a' + 8*(--number);
	if (!legal(mv, colour, board))
	    {   
		message ("Illegal move");
		bell();
		return (FALSE);
	    }

	domove (board, mv, board, colour);
	display (board);
	return (TRUE);
}

make_remark (value)
	int value;
{
	value *= machine_piece;
	wmove(wremarks, 0, 0);
	wclrtobot(wremarks);
	if (value < -INFINITY+64)
	    {
		waddstr (wremarks, "You should win");
		last_remark = FIRST_REMARK; /* Always remark next time */
		if (aspire==MAXASPIRE)
			wprintw(wremarks, 
				"        (by at most %d)", -value-INFINITY+64);
	    }
	 else if (value < -1000)
	    {   if (last_remark != -INFINITY)
		    {   cretin_flag = TRUE;
			last_remark = -INFINITY;
			waddstr(wremarks,
				"\nOnly a cretin could lose from your position");
		    }
	    }
	 else if (value <= 1000) printr(value);
	 else if (value < INFINITY-63)
	    {   if (last_remark!=INFINITY)
		    {   waddstr(wremarks,
			"Resign, you dolt! Further resistance is futile");
			last_remark = INFINITY;
		    }
	    }
	 else
	    {   waddstr (wremarks, "I shall win");
		last_remark = FIRST_REMARK;
		if (aspire==MAXASPIRE)
		       wprintw (wremarks,
			"        (by at least %d)", value-INFINITY+64);
	    }
	wrefresh(wremarks);
}

#define MAX_REMARKS 50
FILE *rem_fp;
long good_remarks[MAX_REMARKS], bad_remarks[MAX_REMARKS];
int max_good = 0, max_bad = 0;

get_remarks ()
/*      Set up file pointers to remarks; the format of the remarks file
	is

		Remarks to the effect that machine is winning, in
			increasing order of strength, one per line.
		A line beginning with the character "@".
		Remarks to the effect that human is winning, in increasing
			order of strength, one per line.
*/
{       long fp = 0l;
	int ch;

	rem_fp = fopen (rem_file, "r");
	if (rem_fp==NULL)
	    {   message ("Remarks not available");
		bell();
		sleep(1);
		return;
	    }

	while ((ch=getc(rem_fp))!=EOF)
	    {   if (ch=='@') break;
		good_remarks[max_good++]=fp++;
		if (max_good==MAX_REMARKS)
		    {   message ("Too many remarks");
			bell();
			sleep(1);
			return;
		    }
		while (ch!='\n')
		    {   ch = getc(rem_fp);
			fp++;
		    }
	    }
	max_good--;

	fp++;
	while (ch!='\n')
	    {   ch = getc(rem_fp);
		fp++;
	    }

	while ((ch=getc(rem_fp))!=EOF)
	    {   bad_remarks[max_bad++]=fp++;
		if (max_bad==MAX_REMARKS)
		    {   message("Too many remarks");
			bell();
			sleep(1);
			return;
		    }
		while (ch!='\n')
		    {   ch=fgetc(rem_fp);
			fp++;
		    }
	    }
	max_bad--;
}

printr (n)
	int n;
/*      Locate file-pointer and print appropriate line. The strange
	computation is to cluster remarks towards low evaluations,
	since the evaluation function changes more rapidly with small
	changes in position as the evaluation gets larger
*/


{       int ch;
	int index;
	float findex;
	int sign_n = (n<0? -1: 1);
	char string[100], *stringp = string;

	if (rem_fp==NULL) return;

	findex = (float) abs(n)/1000.0;
	index = findex * (2-findex) * (n<0? max_bad: max_good) + 0.5;
	if (index*sign_n == last_remark)
			/* Don't make the same remark twice in a row */
	    {   
		return;
	    }

	last_remark = index*sign_n;

	fseek (rem_fp, n<0? bad_remarks[index]: good_remarks[index], 0);
	while ((ch=getc(rem_fp))!=EOF)
	    {   if (ch=='\n') break;
		*stringp++ = ch;
	    }
	*stringp = '\0';
	waddstr(wremarks, string);
}

do_command (letter, number, board, colour)
	POSITION *board;
	char letter;
	int number;
	int *colour;
{
	int mv = NOMOVE;

	switch (letter)
	    {	case 'q':	return(FALSE);

		case 'h':
		case '?':       print_help ();
				wclear(wmessages);
				redraw();
				break;

		case 'p':       redraw();
				break;

		case 't':	trace_eval = !trace_eval;
				message("Tracing now %s",
					   trace_eval? "on": "off");
				if (!trace_eval) {
					wclear(wtrace);
					wrefresh(wtrace);
				}
				break;

		case 'r':       if (rem_fp==NULL)
				    {   
				   	 message("Remark file not available");
					 break;
				    }
				remark = !remark;
				if (remark) {
					message ("OK, I'll make remarks");
				} else {
					message ("OK, I'll shut up");
					wclear(wremarks);
					wrefresh(wremarks);
				   }
				break;

		case 's':	if (number==0) {
					message ("Current search depth: %d",
						    depth);
				} else if (number<0 || number>MAXDEPTH) {
					message ("Search depth must be in range 1-%d",
						    MAXDEPTH);
					bell();
				}
				else {	depth = number;
					message ("Search depth set to %d",
						    number);
				     }
				break;

		case 'l':	if (number==0)
					message ("Current aspiration level: %d",
						    aspire);
				else if (number<0 || number>MAXASPIRE) {
					message ("Aspiration level must be 1-%d",
						    MAXASPIRE);
					bell();
				}
				else {	aspire = number;
					message ("Aspiration level set to %d",
						    number);
				     }
				break;

		case 'e':       message("Static evaluation: %d",
					     static_eval (board));
				break;

		case 'v':	if (number==0) number=depth;
				if (number<1 || number>9) {
					message("Invalid depth");
					bell();
				}
				else {  set_aspiration (board);
					message("Dynamic evaluation: %d",
				  	alfabeta (board, number,
						 human_piece*INFINITY,
						 human_piece, &mv));
				     }
				break;

		case 'm':	if (number==0) number=depth;
				if (number<1 || number>9)
				    {   message ("Invalid depth");
					bell();
					break;
				    }
				set_aspiration (board);
				alfabeta (board, number, human_piece*INFINITY,
						human_piece, &mv);
				message("I would move at %c%c",
					 'a'+(mv&7), '1'+(mv>>3));
				break;

		case '>':       save(board, colour);
				break;

		case '<':       restore(board, colour);
				break;

		case 'u':       longjmp(undomove, 1);
				break;

		default:        message ("Type ? for help");
				bell();
				break;
	    }
	return (TRUE);
}

alfabeta (pos, depth, parent_opt, sign, parent_best)

/* Alpha-beta pruning algorithm.

	If sign=1, then try to MAXIMISE score, else MINIMISE.
	Parent node wants to MINIMISE/MAXIMISE, so as soon as
	we exceed parent_opt, give up and return INFINITY.
	Return best move in parent_best.

	Externals used:
		static_eval (position)
		ismove (position, player) [does player have a legal move?]
		game_over (pos)
		typedef POSITION
		aspiration (player, depth, position)  [return the aspiration limit for
					       next level of search]
		nextmove (position, mv, nextposition, player)
			[give the next legal move for player in position;
			   put the resulting position in nextposition]

*/

	POSITION *pos;
	int depth, parent_opt, sign, *parent_best;

{
	POSITION nextpos;
	int value, this_opt, this_best = NOMOVE, mv = NOMOVE, asp = 0;

	if ((depth==0) || game_over(pos))
	    {	value = static_eval (pos);
		*parent_best = NOMOVE;
		if ((sign*value) > (sign*parent_opt))
			return (sign*INFINITY);
		   else return (value);
	    }

	ab_count++;     /* Record boards dynamically evaluated */
	this_opt = (sign==1? -INFINITY: INFINITY);

	if (!ismove(pos,sign))		/* No legal move */
	    {   value = alfabeta (pos, depth, this_opt, -sign, &this_best);
		goto valfound;
	    }

	asp = sign * aspiration (sign, depth);

	while ((mv=nextmove (pos, mv, &nextpos, sign)) != NOMOVE)
	    {	value = alfabeta (&nextpos, depth-1, this_opt, -sign, &this_best);
	      valfound:
		if ((sign*value) >= (sign*parent_opt))
			return (sign*INFINITY);

		if ((sign*value) > asp)
		    {   *parent_best = mv;
			return (value);
		    }

		if ((sign*value) > (sign*this_opt))
		    {	this_opt = value;
			*parent_best = mv;
		    }

			/* If two moves have same evaluation, choose
			   uniformly randomly between them (of course,
			   where several moves have same value, this
			   is biased towards the later ones
			*/

		if ((value == this_opt) && (rand() & 020))
			*parent_best = mv;
	    }

	return (this_opt);
}

set_aspiration (position)
	POSITION *position;
/*	Aspirations are as follows:
	   Aspiration-level		Aspiration
		1		The worse of 0, static value
		2		The static value
		3		10% better than the static value
		4		25% better than the static value
		5		Any winning move
		6		The move winning by the greatest margin

	It is assumed that the opponent has
	the same level of aspiration as the program.
*/

{
	if (aspire==MAXASPIRE)
	    {	expected = 0;
		margin = INFINITY;
		return;
	    }

	if (aspire==(MAXASPIRE-1))
	    {	expected = 0;
		margin = INFINITY-64;
		return;
	    }

	expected = static_eval (position);

	switch (aspire)
	    {   case 1: expected /= 2;
			margin = -abs(expected);
			return;
		case 2: margin = 0;  return;
		case 3: margin = abs (expected/10);  return;
		case 4: margin = abs (expected/4);   return;
	    }
}

aspiration (player, depth)
	int player, depth;
{
	if (aspire==MAXASPIRE) return (player*INFINITY);
	if (depth<3 && aspire>1) return (player*(INFINITY-64));
	return (expected+player*margin);
}

static_eval (pos)
	POSITION *pos;

/*	Square values (only valid when a "vulnerable" piece occupies the
	square in question)
*/

#define	VCORN	100
		/* Corner */
#define VORTH	-30
		/* Orthogonally next to corner */
#define VDIAG	-50
		/* Diagonally     "   "    "   */
#define VEDGE	20
		/* On edge */
#define VNEXT	-7
		/* Next to edge */
#define VNORM	1
		/* Elsewhere */

{
	static int model[64] = {
	  VCORN, VORTH, VEDGE, VEDGE, VEDGE, VEDGE, VORTH, VCORN,
	  VORTH, VDIAG, VNEXT, VNEXT, VNEXT, VNEXT, VDIAG, VORTH,
	  VEDGE, VNEXT, VNORM, VNORM, VNORM, VNORM, VNEXT, VEDGE,
	  VEDGE, VNEXT, VNORM, VNORM, VNORM, VNORM, VNEXT, VEDGE,
	  VEDGE, VNEXT, VNORM, VNORM, VNORM, VNORM, VNEXT, VEDGE,
	  VEDGE, VNEXT, VNORM, VNORM, VNORM, VNORM, VNEXT, VEDGE,
	  VORTH, VDIAG, VNEXT, VNEXT, VNEXT, VNEXT, VDIAG, VORTH,
	  VCORN, VORTH, VEDGE, VEDGE, VEDGE, VEDGE, VORTH, VCORN
	};

	register int i;
	register int value = 0;
	int scores[64];

	se_count++;     /* Record number of static evaluations */
	if (game_over(pos))
	    {   FOR_BOARD(i) value += pos->SQUARE[i];
		return (value>0? INFINITY+value-64:
			value<0? -INFINITY+value+64: 0);
	    }

	FOR_BOARD(i) scores[i] = 0;

	find_invulnerable (pos, scores);
		/* scores now contains VINVUL (or -VINVUL)
			for each invulnerable piece, and 0 elsewhere;
			now fill in other evaluations (special cases:
			next to corner [bad!], on edge[good!], next to
			edge[poor], anywhere else[boring])
		*/

	FOR_BOARD(i)
		value += (scores[i]? scores[i]: model[i]*pos->SQUARE[i]);

	return (value);
}

game_over (pos)
	POSITION *pos;
{
	if (!(pos->MOVES_LEFT)) return (TRUE);

	return ((!ismove(pos, WHITE)) && !(ismove(pos, BLACK)));
}

#define VINVUL 50

find_invulnerable (board, scores)
	POSITION *board;
	int scores[64];

/*	This function finds invulnerable pieces, and scores them
	appropriately; it does not find ALL invulnerable pieces -
	in fact, only concave blocks including a corner - but
	nevertheless should provide a good approximation.
*/

{
	int hwm, corner, value, i, j;

	if ((corner=board->SQUARE[0]) != 0)
	    {	value = corner*VINVUL;
		hwm = 7;

		for (i=0; i<56; i+= 8)
		    {   if (board->SQUARE[i] != corner) break;
			scores[i] = value;
			for (j=1; j<hwm; j++)
			    {   if (board->SQUARE[i+j] != corner)
				    {	hwm = j;
					break;
				    }

				scores[i+j] = value;
			    }
		    }
		scores[0] = corner*VCORN;
	    }

	if ((corner=board->SQUARE[7]) != 0)
	    {	value = corner*VINVUL;
		hwm = 0;

		for (i=0; i<56; i+= 8)
		    {   if (board->SQUARE[i+7] != corner) break;
			scores[i+7] = value;
			for (j=6; j>hwm; j--)
			    {   if (board->SQUARE[i+j] != corner)
				    {	hwm = j;
					break;
				    }

				scores[i+j] = value;
			    }
		    }
		scores[7] = corner*VCORN;
	    }

	if ((corner=board->SQUARE[56]) != 0)
	    {	value = corner*VINVUL;
		hwm = 7;

		for (i=56; i>0; i-= 8)
		    {   if (board->SQUARE[i] != corner) break;
			scores[i] = value;
			for (j=1; j<hwm; j++)
			    {   if (board->SQUARE[i+j] != corner)
				    {	hwm = j;
					break;
				    }

				scores[i+j] = value;
			    }
		    }
		scores[56] = corner*VCORN;
	    }

	if ((corner=board->SQUARE[63]) != 0)
	    {	value = corner*VINVUL;
		hwm = 0;

		for (i=56; i>0; i-= 8)
		    {   if (board->SQUARE[i+7] != corner) break;
			scores[i+7] = value;
			for (j=6; j>hwm; j--)
			    {   if (board->SQUARE[i+j] != corner)
				    {	hwm = j;
					break;
				    }

				scores[i+j] = value;
			    }
		    }
		scores[63] = corner*VCORN;
	    }
}

whitesp (ch)
char ch;
{
	return (ch==' ' || ch=='\n' || ch=='\t');
}


decode (s, ch, num)
char *s,*ch;
int *num;
{
	if (*s=='\0') return;
	*ch = ('A'<= *s && *s <='Z'? *s-'A'+'a': *s);
	s++;
	*num = 0;
	sscanf(s, " %d", num);
}

/*************************************/

nextmove (pos, mv, nextpos, player)
	POSITION *pos, *nextpos;
	int mv, player;
{
	/* On first entry for a given position, move is set to NOMOVE,
	   i.e. -1.  Moves are generated in a rough plausibility order
	   given by the list structure move_table, whose head is
	   move_table[-1], and whose tail-pointer is set to NOMOVE.
	   The order is purely heuristic, and corresponds roughly to
	   the values associated by static_eval with different squares.
	*/

	static int m_table[65] =
	    {	0,  7,  6,  5,  4, 24, 16,  8, 56, 15,
		14, 13, 12, 25, 17, 49, 48, 23, 22, 21,
		20, 26, 42, 41, 40, 31, 30, 29, 28, 35,
		34, 33, 32, 39, 38, 37, 36, 10, 43, 51,
		59, 47, 46, 45, 44, 27, 19, 50, 58, 55,
		54, 53, 52,  1, 11, -1, 57, 63, 62, 61,
		60, 18,  3,  9,  2
	    };

	static int *move_table = &(m_table[1]);

	while ((mv=move_table[mv])!=NOMOVE)
		if (legal(mv, player, pos))
		    {	if (nextpos) domove (pos, mv, nextpos, player);
			   /* Next position generated only if pointer is
				non-zero */
			return (mv);
		    }
	return (NOMOVE);	/* No more legal moves */
}

/**************************************/

/*      Globals for passing arguments to "sandwich";
	this is to save time putting arguments on and off the
	stack in a very heavily used piece of code
*/

int s_move, s_row, s_col, s_player, s_opponent, s_flip;
POSITION *s_pos;

domove (pos, mv, nextpos, player)
	POSITION *pos, *nextpos;
	int mv, player;
{
	register int i;
	if (pos != nextpos) FOR_BOARD(i) nextpos->SQUARE[i] = pos->SQUARE[i];

	s_move = mv;
	s_row = mv >> 3;
	s_col = mv & 7;
	s_player = player;
	s_opponent = -player;
	s_flip = TRUE;
	s_pos = nextpos;

	nextpos->SQUARE[s_move] = player;

	sandwich (/* mv, */ -9 /* , player, nextpos, TRUE */ );
	sandwich (/* mv, */ -8 /* , player, nextpos, TRUE */ );
	sandwich (/* mv, */ -7 /* , player, nextpos, TRUE */ );
	sandwich (/* mv, */ -1 /* , player, nextpos, TRUE */ );
	sandwich (/* mv, */  1 /* , player, nextpos, TRUE */ );
	sandwich (/* mv, */  7 /* , player, nextpos, TRUE */ );
	sandwich (/* mv, */  8 /* , player, nextpos, TRUE */ );
	sandwich (/* mv, */  9 /* , player, nextpos, TRUE */ );

	nextpos->MOVES_LEFT = (pos->MOVES_LEFT) - 1;
}

/*************************************/

legal (mv, player, pos)
	POSITION *pos;
	int mv, player;
{
	if (pos->SQUARE[mv]) return(FALSE);
					   /* Already occupied */

	if (pos->MOVES_LEFT > 60)	/* Initial 4 moves */
		return ((mv==27) || (mv==28) || (mv==35) || (mv==36));
				/* Central four squares */

	s_move = mv;
	s_row = mv >> 3;
	s_col = mv & 7;
	s_player = player;
	s_opponent = -player;
	s_flip = FALSE;
	s_pos = pos;

	return ( sandwich ( /* mv, */ -9 /* , player, pos, FALSE */ ) ||
		 sandwich ( /* mv, */ -8 /* , player, pos, FALSE */ ) ||
		 sandwich ( /* mv, */ -7 /* , player, pos, FALSE */ ) ||
		 sandwich ( /* mv, */ -1 /* , player, pos, FALSE */ ) ||
		 sandwich ( /* mv, */  1 /* , player, pos, FALSE */ ) ||
		 sandwich ( /* mv, */  7 /* , player, pos, FALSE */ ) ||
		 sandwich ( /* mv, */  8 /* , player, pos, FALSE */ ) ||
		 sandwich ( /* mv, */  9 /* , player, pos, FALSE */ ));
}

/**********************************/

sandwich ( /* s_move, */ increment /* , s_player, s_pos, s_flip */ )
	register int increment;
	/* int s_player, s_move, s_flip; */
	/* POSITION *s_pos; */

/*	Test whether the square move sandwiches a line
	of enemy pieces in the direction [row_inc, col_inc];
	If (s_flip) then update position by capturing such pieces
*/

{
	register int square, offset;
	int row_offset, col_offset, piece, piece_count;

	if (s_pos->SQUARE[s_move+increment] != s_opponent) return(FALSE);

			/*  Quick test to catch most failures -
			    note that the tested square may not even
			    be on the board, but the condition is a
			    sufficient one for failure */

	row_offset = (increment<-1 ? s_row:	/* inc -1: -9, -8, -7 */
		      increment> 1 ? 7-s_row:      /* inc  1:  7,  8,  9 */
		      8);
	col_offset = (increment & 4? s_col:	/* inc -1: -9, -1,  7 */
		      increment & 1? 7-s_col:      /* inc  1: -7,  1,  9 */
		      8);

	offset = (row_offset>col_offset? col_offset: row_offset);
			/* offset = shortest distance to an edge in the
			   direction of search */
	if (2 > offset) return (FALSE);

	piece_count = 1;
	square = s_move+increment;

	while (--offset)
	    {   if (!(piece=s_pos->SQUARE[square += increment]))
			return (FALSE);	 /* If empty square, give up */

		if (piece==s_player) break;
		    else piece_count++;	 /* Count opponent's pieces
						   encountered */
	    }

	if (!offset) return (FALSE);

	if (s_flip)
		while (piece_count--)
			s_pos->SQUARE[square -= increment] = s_player;

	return (TRUE);
}

print_help ()
{
	clear();
	move(3, 0);
	addstr ("    <column><row> to make a move\n");
	addstr ("    ? or h        to obtain this message\n");
	addstr ("    q             to terminate this game at once\n");
	addstr ("    p             to re-display the current position\n");
	addstr ("    t             to switch on or off tracing of evaluation\n");
	addstr ("    r             to switch remarks on or off\n");
	addstr ("    s<n>          to set depth of search to n\n");
	addstr ("    l<n>          to set the level of aspiration to n\n");
	addstr ("    e             to get the static evaluation of the position\n");
	addstr ("    v<n>          to get the dynamic evaluation (depth n)\n");
	addstr ("    m<n>          to get a suggested next move (depth n)\n");
	addstr ("    u             to undo your last move\n");
	addstr ("    >             to save the position in a file\n");
	addstr ("    <             to restore from a file\n");
	refresh();
	message("Type SPACE to continue ... ");
	*input = getch();
}

info()
{
	char cmd[256];

	clear();
	refresh();
	if (access(infofil, 0444) == -1) {
		message("Unfortunately, there is no information available\n");
		bell();
		sleep(1);
		return;
	}
	sprintf(cmd, "/usr/ucb/more %s", infofil);
	system(cmd);
	message("Hit new line when ready...");
	*input = getch();
	clear();
	refresh();
	crmode();
	noecho();

	return;
}

display (board)
	POSITION *board;
{
	register int i, j;
	char *piece;

	wclear(wmessages);
	wrefresh(wmessages);
	wmove(wboard, 0, 0);

	waddstr(wboard, "   a  b  c  d  e  f  g  h\n");
	for (i=0; i<8; i++)
	    {	wprintw(wboard, "%1d|", i+1);
		for (j=0; j<8; j++)
		    {   switch (board->SQUARE[8*i+j])
			    {	case WHITE: piece = "OO"; break;
				case BLACK: piece = "XX"; break;
				case FREE : piece = "  ";  break;
			    }
			wprintw(wboard, "%s|", piece);
		    }
		wclrtoeol(wboard);
		waddch(wboard, '\n');
	    }
	wrefresh(wboard);
}

getreply (w, str)
	WINDOW *w;
	char *str;
{       int ch;
	char *s = str;

	while ((ch=getch()) == ' ' || ch == '\t') SKIP
	while (ch!='\n' && ch!='\r')
	    {   if (ch==EOF)        /* End-of-file or read error */
		    {   if ((ch=getch())==EOF)    /* Re-try; if failure
						   assume EOF */
			    {   message ("Unexpected end of input");
				bell();
				finish_up();
			    }
			  else {
				continue;
			  }
		    }
		if (ch==0177) {
			if (s>str) {
				s--;
				waddstr(w, "\b \b");
				wrefresh(w);
			}
			ch = getch();
			continue;
		}
		*s++ = ch;
		waddch(w, ch);
		wrefresh(w);
		ch = getch();
	    }

	*s = 0;
}

#define MAGIC   0501    /* Header word */

save(board, colour)
	POSITION *board;
	int *colour;
{       FILE *fp;
	char fname[128];

	message("Save into: ");
	getreply(wmessages, fname);
	if ((fp=fopen(fname,"w"))==NULL)
	    {   message("Can't open %s\n", fname);
		bell();
		return;
	    }
	putw(MAGIC, fp);
	fwrite(board, sizeof(POSITION), 1, fp);
	putw(*colour, fp);
	fclose(fp);
}

restore(board, colour)
	POSITION *board;
	int *colour;
{       FILE *fp;
	char fname[128];

	message("Restore from: ");
	getreply(wmessages, fname);
	if ((fp=fopen(fname,"r"))==NULL)
	    {   message("Can't open %s\n", fname);
		bell();
		return;
	    }
	if (getw(fp)!=MAGIC)
	    {   message("Not a saved position\n");
		bell();
		return;
	    }
	fread(board, sizeof(POSITION), 1, fp);
	*colour = getw(fp);
	fclose(fp);
}

bell()
{
	putchar('\07');
	fflush(stdout);
}

message(f, a1, a2, a3, a4, a5)
{
	wclear(wmessages);
	wprintw(wmessages, f, a1, a2, a3, a4, a5);
	wrefresh(wmessages);
}

initwindows()
{
	wboard =	newwin(9, 30, 2, (COLS-30)/2);
	wmoves =	newwin(2, 30, 12, (COLS-30)/2);
	wmessages =	newwin(1, COLS, 0, 0);
	wtrace =	newwin(5, COLS-8, 18, 8);
	wremarks =	newwin(2, COLS, 16, 0);
}

redraw()
{
	clear();
	refresh();
	touchwin(wboard);
	touchwin(wmessages);
	touchwin(wtrace);
	touchwin(wremarks);
	touchwin(wmoves);
	wrefresh(wboard);
	wrefresh(wmessages);
	wrefresh(wtrace);
	wrefresh(wremarks);
	wrefresh(wmoves);
}
