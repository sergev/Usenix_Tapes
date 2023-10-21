
/* Othello - by Jonathan Dubman - Hard Hat Area - work in progress */

/* Version 1.0   - 9/29/86  - grabs as many pieces as it can, plays by rules */
/* Version 1.1   - 10/2/86  - includes log */
/* Version 1.1.1 - 10/2/86  - includes suggest */
/* Version 1.1.2 - 10/6/86  - looks ahead one move and goes for corners -bugs*/
/* Version 1.1.3 - 10/13/86 - cleaner version that actually looks ahead */
/* Version 1.1.4 - 10/17/86 - attempt at looking ahead N moves */
/* Version 1.1.5 - 10/31/86 - sort of works when looking ahead N moves */
/* Version 1.1.6 - 11/7/86  - debugged version - there were lots of 'em! */
/* Version 1.1.7 - 11/10/86 - forced passing implemented */
/* Version 1.1.8 - 11/22/86 - significant amount of documentation added 

/* This program plays the game of Othello: player against computer, or
 * computer against itself.  It uses a recursive search to determine the best
 * possible move.  The rules of Othello are summarized in the instructions
 * at the end of this file.  The basic strategy is to take as many pieces as
 * possible, subject to the exceptions that corners are very good, edges are
 * somewhat good, and any squares adjacent to corners are bad.  The program is
 * intended to be portable and flexible, though I haven't tried to compile it
 * on a non-unix machine, and changing the board size might create problems.
 *
 * The main parts of the program are: List Handler, Board Handler, Scanner,
 * Evaluation Functions, and User Interface.  I will describe all of these
 * in brief.  I am not going to bother including the actual parameters of
 * each function.  Only the most important functions are described.  Detailed
 * descriptions are given in the function definitions.
 *
 *
 * LIST HANDLER
 *
 * Othello is a game that deals with groups of pieces, all of which have
 * coordinates.  A group of coordinates is called a list.  Many routines must
 * deal with groups of coordinates; for example, the routines that flip pieces
 * after a move, form a list of pieces taken if a player makes a certain move,
 * etc.  The lists are stored on a large stack of coordinates.  There is also
 * another array, list, that keeps track of the length and starting position
 * of each list.
 *
 * initlists()		Erase all lists and start fresh.  listcount keeps
 *			track of how many lists are in memory.
 * list_x(), list_y()	Get coordinates out out of a certain list.
 * add_list()		Add an entirely new list to memory.
 * zap_list()		Delete an entire list from memory.
 * consolidate_lists()	Combine several lists into one list.
 * add_element()	Add a pair of coordinates to a given list.
 *
 *
 * BOARD HANDLER
 *
 * There is a type BOARD which is a grid of pieces - either empty, X, O, or
 * border.  Lots of functions pass around boards.  The evaluation functions
 * create plenty of new, temporary boards with which they perform experiments.
 *
 * erase_board()	Clear board to all empty, with border beyond the edges.
 * initboard()		Eraseboard and place first four pieces.
 * copyboard()		Copy the contents of one board to another.
 * printboard()		Show a text display of the contents of a board,along
 *			with who's winning, the coordinate grid, and an
 *			optional message string, i.e. "O just took 4 pieces."
 * count()		Count how many pieces a certain player has.
 *
 *
 * SCANNER
 *
 * The most important part of the game is the intelligence part.  How does the
 * computer decide how to make a move?  All the routines are equally valid for
 * both players.  The computer can make a suggestion to the player without much
 * extra work.  The key function is scanboard, and the key idea is that it can
 * be made to work with any evaluation function by C's pointer-to-function
 * ability.
 *
 * validmove()		Determine whether a given move is valid for a given
 *			player.
 * formfliplist()	Form a coordinate list of pieces that would be taken
 *			by a given player moving at a given place.  Pieces
 *			can be taken in any combination of all 8 directions.
 * scanboard()		Given a player, board, pointer to evaluation function,
 *			and depth of search, maximize that function over all
 *			X,Y combinations on the board.
 *
 *
 * EVALUATION FUNCTIONS
 *
 * Each evaluation function takes a board, a specific position, a player, and
 * a lookahead value, and must return an integer corresponding to how good the
 * move is.
 *
 * mostpieces()		Just grab as many pieces as you can.
 * mostcorners()	Take into account the fact that corners are good, etc.
 * lookone()		Look ahead one move.  It is kept as a demonstration.
 * look()		The recursive lookahead function, that gets very
 *			slow with lookahead greater than three or four.
 *
 *
 * USER INTERFACE
 *
 * The user interface is all that need be changed to change the program to
 * graphics, etc.  Main is a little messy, but more flexible that way.
 *
 * main()		Main is just a front end to the whole works.
 *			Set skill level, first player, etc., set up the
 *			game, then alternate players' moves, keeping track of
 *			passing if necessary and stopping at the end of the
 *			game.
 * getmove()		Get a pair of coordinates from the user.  Getmove
 *			also supports numerous niceties like help, suggestion,
 *			etc.  It keeps trying until it gets a valid move.
 * printboard()		Obviously needs to be changed for a graphics version.
 *
 *
 * MISCELLANEOUS
 *
 * There is a game log and a lot of other functions, but they are not too
 * important to the understanding of the overall working of the game.
 * 
 *
 * USER MODIFICATIONS
 *
 * Aside from fixing bugs, converting the whole thing to graphics, etc., the
 * most obvious extension to the program would be to improve the evaluation
 * functions.  The evaluation functions must return an integer corresponding
 * to how good the move is.  A high number is a good move.  Negative values
 * ARE permitted.  The function should return INVALID if that move is not
 * possible.
 *
 * ---------------------------------------------------------------------------
 * This program is freely redistributable subject to the restriction that it
 * shall not be sold for profit and my name shall not be removed from it.
 *
 * I would greatly appreciate comments, bugs, etc.
 * Until December 17th, send mail to ucbvax!buddy!c9c-bi
 * After then, I don't know what my account will be.  Try ucbvax!cory!dubman
 */

#include <stdio.h>
#include <strings.h>

#define VERSION "1.1.8"

/* Define the best strategy for playing the game : an evaluation function */

#define beststrategy	look

#define FALSE	0
#define TRUE	1

#define debug	FALSE		/* whether or not to print diagnostics */

#define EMPTY	0		/* an empty space on the board */
#define X	1		/* X piece in board array */
#define O	2		/* O piece, not zero */
#define BORDER	3		/* Put all along the edge of the board */

#define NOWHERE -1		/* Coordinate of an invalid move */
#define INVALID	10000		/* Evaluation of an invalid move - arbitary */
				/* number, but no confuse with REAL eval # */
#define PASS -1			/* Parameter for gamelog when player passes */

#define MAXLIST		100	/* maximum number of lists in memory */
#define STACKSIZE	1000	/* size of coordinate stack */

/*  The board must be square and should have an even dimension.
 *  printboard() and getmove() will have to be modified if BOARDSIZE >=10.
 */

#define BOARDSIZE	8	/* 8x8 playing board */

#define NUMSQUARES	BOARDSIZE * BOARDSIZE

/*  Structure for storing a playing board.  Access as board.loc[x][y] where
 *  x is across, y is down, origin is upper left.
 *  Each element in the array is a piece - either EMPTY, X, or O.
 *  Note: Subscript of 9 means 0..8 are valid indexes!  Not 1..9 or 0..9!
 *  Note: On the edge of the playing board, i.e. the 0th column and 9th column
 *  and 0th row and 9th row, we put BORDER pieces instead of EMPTY, to simplify
 *  range checking in some functions.
 */

struct BOARD {
    int loc[BOARDSIZE+2][BOARDSIZE+2];
} gameboard;

/*  Structure for storing a pair of coordinates */

struct COORDINATES {
    int xc;
    int yc;
    int eval;		/* NOTE: Some routines use this, other's don't. */
};

/*  Structure for storing an entire list of coordinates, such as the list of
 *  coordinates the computer would take if it placed a piece at (x1, y1).
 *  Note that start indexes, and not points to, a coordinate structure.
 *  There are no actual addresses being stored, just indexes to the stack
 *  array.  There may be many lists in memory.
 */

struct LIST {
    int start;				/* index to first pair in list */
    int size;				/* number of coordinates in list */
} list[MAXLIST];

/*  The stack is where all the actual coordinates are stored.  list[]
 *  refers to addresses on the stack.  Each stack element is of type
 *  COORDINATES.  It is a stack because allocations and deallocations of
 *  lists must be made in LIFO order, even though one can access any of the
 *  lists at any time.
 */

struct COORDINATES stack[STACKSIZE];

/*  The gamelog keeps track of each turn: location of the players' move, and
 *  number of pieces they had at that time.  Associated functions have the name
 *  'log' imbedded in them.  Note the number of turns must be no more than the
 *  number of squares.
 */

struct GAMELOG {
    char xloc;
    char yloc;
    char X_count;
    char O_count;
    char take;
    char pass;
} gamelog[NUMSQUARES];

/*  Evaluation functions */

int mostpieces();
int mostcorners();
int lookone();
int look();

/*  Other functions */

struct COORDINATES getmove();
struct COORDINATES scanboard();

/*  Miscellaneous global varibles */

char p[3] = { '-', 'X', 'O' };		/* board pieces: p[0], p[X], p[O] */
int listcount;				/* most recently allocated list */
int mistakes = 0;			/* number of mistakes player's made */
int updown = 0, leftright = 0;		/* used in scanboard only - see that */
int computer = FALSE;			/* flag for computer against itself */
int turns = 0;				/* number of turns */
int firstmove;				/* TRUE if player first, else FALSE */
int lookahead;				/* depth of search for comp's move */
int autolook;				/* lookahead for player when computer
					   takes over player's position */

main()
{
    struct COORDINATES playermove, computermove;

    char choice, firstchoice, garbage;	/* character inputs */
    char string[80];			/* for passing msg to printboard */
    int cx, cy;				/* coordinates */
    int xpass, opass;			/* both true -> end of game */

    init_gamelog();

    printf("OTHELLO v%s\nby Jonathan Dubman\n", VERSION);
    printf("Do you want instructions? (Y/N) [No] ");
    choice = getchar();

    if (choice == 'y' || choice == 'Y') {
	instructions();
    }
    /* skip past all the junk until the newline */
    if (choice != '\n') while ((garbage = getchar()) != '\n');

    printf("Do you want to go first (Y/N) [Yes] ");
    firstchoice = getchar();
    if (firstchoice != '\n') while ((garbage = getchar()) != '\n');

    printf("Skill level 0 (easy,fast) to 9 (hard,slow) [0] ");
    choice = getchar();
    if (choice != '\n') while ((garbage = getchar()) != '\n');
    lookahead = 0;
    if (choice > '0' && choice <= '9') {
	lookahead = choice - '0';
    }

    printf("\n");
    initlists();
    initboard(&gameboard);
    printboard(&gameboard, "Beginning new game");

    if (firstchoice == 'N' || firstchoice == 'n') {
	firstmove = FALSE;
	goto compmove;
    } else {
	firstmove = TRUE;
    }

    /* Main turn loop */

    while (TRUE) {
	playerturn:
	playermove = scanboard(&gameboard, X, mostpieces, 0);
	cx = playermove.xc;
	cy = playermove.yc;
	if (validmove(&gameboard, cx, cy, X) == FALSE) {
	    printf("Player is forced to pass.\n");
	    update_gamelog(X, PASS);
	    sprintf(string, "X is forced to pass.");
	    xpass = TRUE;
	} else {
	    if (computer == FALSE) {
	        playermove = getmove(&gameboard);
	        if (computer == TRUE) goto playerturn;
	        cx = playermove.xc;
	        cy = playermove.yc;
	    } else {
		playermove = scanboard(&gameboard, X, beststrategy, autolook);
		cx = playermove.xc;
		cy = playermove.yc;
	        printf("\nNonplayer moves at ");
	        printcoordinates(cx, cy);
	        printf("\n");
	    }
	    formfliplist(&gameboard, cx, cy, X);
	    gameboard.loc[cx][cy] = X;
	    do_flip(&gameboard, listcount);
	    zap_list();
	    update_gamelog(X, cx, cy);
	    if (gamelog[turns].take == 1) {
	        sprintf(string, "X just took 1 piece");
	    } else {
	        sprintf(string, "X just took %d pieces", gamelog[turns].take);
	    }
	}
        printboard(&gameboard, string);
	
	compmove:
	computermove = scanboard(&gameboard, O, beststrategy, lookahead);
	cx = computermove.xc;
	cy = computermove.yc;
	if (validmove(&gameboard, cx, cy, O) == FALSE) {
	    printf("Computer is forced to pass.\n\n");
	    update_gamelog(O, PASS);
	    sprintf(string, "O was forced to pass");
	    opass = TRUE;
	} else {
	    printf("\nComputer moves at ");
	    printcoordinates(cx, cy);
	    printf("\n");
	    formfliplist(&gameboard, cx, cy, O);
	    gameboard.loc[cx][cy] = O;
            do_flip(&gameboard, listcount);
	    zap_list();
	    update_gamelog(O, cx, cy);
	    if (gamelog[turns].take == 1) {
	        sprintf(string, "O just took 1 piece");
	    } else {
	        sprintf(string, "O just took %d pieces", gamelog[turns].take);
	    }
	}
        printboard(&gameboard, string);
	if (xpass || opass) break;
    }
    printf("\nThe Game is Over.\n\n");
    printf("FINAL TALLY\n");
    printf("You:  %d\n", cx = count(&gameboard, X));
    printf("Me:   %d\n", cy = count(&gameboard, O));
    printf("\n");
    if (cx > cy) {
	printf("You win.\n");
    } else if (cy > cx) {
	printf("I win.\n");
	printf("One more in the bag for Artificial Intelligence.\n");
    } else {
	printf("A tie.\n");
    }
}

/*  LIST HANDLER ROUTINES */

/*  Erase all lists and start fresh.  The first list is a null list that merely
 *  specifies the starting index for the next list.  Obviously, the size
 *  of the null list is zero, and since it occupies list[1], the next list
 *  (the first real one), goes in list[1].
 */

initlists()
{
    list[0].start = 0;
    list[0].size = 0;
    listcount = 0;
}

/*  This is useful only as a debugging tool.  It prints data on every list
 *  in memory including the null list at the beginning, but does not print
 *  the coordinate lists themselves.
 */

printlists()
{
    int loop;

    printf("current is list number %d\n", listcount);
    printf("list_num   size     start\n");
    for (loop = 0; loop <= listcount; loop++) {
	printf("%5d     %5d     %5d\n",
		loop, list[loop].size, list[loop].start);
    }
}

/*  Another debugging tool.  This one prints out all the coordinates of a
 *  list, given the list number.
 */

list_number(listnum)
int listnum;
{
    int loop;

    printf("List number %d\n", listnum);
    if (list_length(listnum) == 0)
	printf("is empty.\n");
    else
	for (loop = 0; loop < list_length(listnum); loop++) {
	    printf("x = %d   y = %d\n",
		    list_x(listnum, loop), list_y(listnum, loop));
	}
}

/* Return the x value in the index'th element of list listnum */

list_x(listnum, index)
int listnum, index;
{
    return(stack[list[listnum].start + index].xc);
}

/* Return the y value in the index'th element of list listnum */

list_y(listnum, index)
int listnum, index;
{
    return(stack[list[listnum].start + index].yc);
}

/*  This list handling routine should be called whenever a new list is to be
 *  added.  Use zap_list() to delete it later.  Add info to the list with
 *  add_element().  add_list returns the list number that was just added.
 */

add_list()
{
    listcount++;
    list[listcount].start = list[listcount - 1].start +
			    list[listcount - 1].size;
    list[listcount].size = 0;
    if (listcount < 0 || listcount > 10) {
	printf("Something strange!  There are %d lists.\n", listcount);
    }
    return(listcount);
}

/*  This function consoliates the last N lists in memory as one large list.
 *  It returns the size of the new list.
 */

consolidate_lists(how_many)
int how_many;
{
    int size_so_far = 0;
    int loop;

    if (how_many > listcount || how_many < 1) return(0);

    for (loop = 1; loop < how_many; loop++) {
	size_so_far += list_length(listcount);
	zap_list();
    }
    list[listcount].size += size_so_far;
    return(list[listcount].size);
}

/*  Erase the most recently allocated list.  It is made its own function
 *  for readability and to make code independent of the implementation of
 *  the coordinate lists.
 */

zap_list()
{
    if (listcount == 0) {
	printf("Error in program - trying to zap a nonexistent flip list!\n");
    } else {
        listcount--;
    }
}

/*  Given a list number, return its length (number of elements) */

list_length(listnum)
int listnum;
{
    return(list[listnum].size);
}

/*  This list handling routine should be called whenever a new element is
 *  to be added to the most recently allocated list.
 */

add_element(x, y)
int x, y;
{
    struct COORDINATES *elmptr;		/* points to next element on stack */

    elmptr = &stack[list[listcount].start + list[listcount].size];
    elmptr->xc = x;
    elmptr->yc = y;
    list[listcount].size++;
}

/*  BOARD HANDLER ROUTINES */

/*  Clear the specified board to all EMPTY in the middle
 *  and BORDER on the edge
 */

erase_board(board)
struct BOARD *board;
{
    int x, y;

    for (y = 1; y <= BOARDSIZE; y++) {
	for (x = 1; x <= BOARDSIZE; x++) {
	    board->loc[x][y] = EMPTY;
	}
    }

    for (x = 0; x <= BOARDSIZE+1, x++;) {
	board->loc[0][x] = BORDER;
	board->loc[x][0] = BORDER;
	board->loc[BOARDSIZE][x] = BORDER;
	board->loc[x][BOARDSIZE] = BORDER;
    }
}

/*  Clear the named board and place the first four pieces in the center.
 *  It is generalized for any board size, as long as it is even.
 */

initboard(board)
struct BOARD *board;
{
    int x1, x2;

    x1 = BOARDSIZE / 2;
    x2 = BOARDSIZE / 2 + 1;

    erase_board(&gameboard);
    board->loc[x1][x1] = O;
    board->loc[x2][x1] = X;
    board->loc[x1][x2] = X;
    board->loc[x2][x2] = O;
}

/*  Copy the entire board structure from one board array to another */

copyboard(source, destination)
struct BOARD *source, *destination;
{
    int x, y;

    for (y = 0; y <= BOARDSIZE + 1; y++) {
	for (x = 0; x <= BOARDSIZE + 1; x++) {
	    destination->loc[x][y] = source->loc[x][y];
	}
    }
}

/*  Called by printboard to print "  1 2 3 4 5 6 7 8\n" up to BOARDSIZE */

shownumbers()
{
    int loop;

    printf("  ");
    for (loop = 1; loop <= BOARDSIZE; loop++) {
	printf("%d ", loop);
    }
}

/*  Show all the pieces on the given board with the coordinate grid:
 *  numbers 1-8 going left to right and letters A-H going top to bottom.
 *  Also tell who is ahead and by how many pieces.
 *  Added bonus:  Print the given string on the second line.
 */

printboard(board, string)
struct BOARD *board;
char *string;
{
    int x, y;
    int xcount, ocount;

    shownumbers();
    xcount = count(board, X);
    ocount = count(board, O);
    printf("    You: %2d  Me: %2d    ", xcount, ocount);
    if (xcount > ocount) {
	if (xcount - ocount == 1) {
	    printf("You are ahead by 1 piece.\n");
	} else {
	    printf("You are ahead by %d pieces.\n", xcount - ocount);
	}
    } else if (ocount > xcount) {
	if (ocount - xcount == 1) {
	    printf("I am ahead by 1 piece.\n");
	} else {
	    printf("I am ahead by %d pieces.\n", ocount - xcount);
	}
    } else {
	printf("We are tied.\n");
    }

    for (y = 1; y <= BOARDSIZE; y++) {
	printf("%c ", 'A' + y - 1);
	for (x = 1; x <= BOARDSIZE; x++) {
	    printf("%c ", p[board->loc[x][y]]);
	}
	printf("%c", 'A' + y - 1);
	if (y == 2) {
	    printf("     %s", string);
	}
	printf("\n");
    }
    shownumbers();
    printf("\n");
}

/*  Given a certain board and player, return the number of that player's
 *  pieces on the board.
 */

count(board, pl)
struct BOARD *board;
int pl;
{
    int x, y, number = 0;

    for (y = 1; y <= 8; y++) {
        for (x = 1; x <= 8; x++) {
	    if (board->loc[x][y] == pl) number++;
	}
    }
    return(number);
}

/*  Given a certain player, return the value of the other player */

other(player)
int player;
{
    switch (player) {
	case X:    return(O);
	case O:    return(X);
	default:   return(EMPTY);
    }
}

/*  Print the pair of coordinates for use in "Computer Moves at ##" */

printcoordinates(x, y)
int x, y;
{
    putchar('A' + y - 1);
    putchar('0' + x);
}

/*  See if the given player can move at the given location:  Form a flip
 *  list and test its length, then delete the flip list.
 *  Return TRUE if valid move, FALSE if invalid.  Obviously, it's also invalid
 *  if it ain't on the board!
 */

validmove(board, x, y, player)
struct BOARD *board;
int x, y, player;
{
    int length;

    if (x < 1 || y < 1 || x > BOARDSIZE || y > BOARDSIZE) {
	return(FALSE);
    }

    formfliplist(board, x, y, player);
    length = list_length(listcount);
    zap_list();
    if (length == 0) {
	return(FALSE);
    } else {
	return(TRUE);
    }
}

/*  Get a line of input from the user, terminated by a RETURN */

getline(string)
char *string;
{
    int index = 0;
    char c;

    while ((c = getchar()) != '\n') {
	string[index++] = c;
    }
    string[index] = '\0';
}

/*  Get an X-Y coordinate from the user, return in the COORDINATE structure.
 *  It would have been implemented as a return value, but you can't return
 *  two variables at a time.
 */

struct COORDINATES getmove(board)
struct BOARD *board;
{
    struct COORDINATES coordinates;	/* return coordinates */
    struct COORDINATES suggestion;	/* for suggesting a move */
    char xch, ych;			/* characters for input */
    int xc = -1, yc = -1;		/* actual coordinates of input */
    char string[80];			/* input string */

    while (validmove(board, xc, yc, X) == FALSE) {
	/* Specify move as A1..H8 */
	printf("Move: ");
	getline(string);

	/* put checking for extra commands here, in an else-if structure */

	if (strcmp(string, "help") == 0 || strcmp(string, "?") == 0) {
	    help();
	} else if (strcmp(string, "instructions") == 0) {
	    instructions();
	} else if (strcmp(string, "auto") == 0) {
	    computer = TRUE;
	    coordinates.xc = NOWHERE;
	    coordinates.yc = NOWHERE;
	    do {
		printf("Enter skill level of computer's opponent: ");
		scanf("%d", &autolook);
		if (autolook < 0 || autolook > 9) {
		    printf("Valid values are between 0 and 9.\n");
		}
	    } while (autolook < 0 || autolook > 9);
	    return(coordinates);
	} else if (strcmp(string, "log") == 0) {
	    print_gamelog();
	} else if (strcmp(string, "suggest") == 0) {
	    printf("I suggest moving at ");
	    suggestion = scanboard(board, X, beststrategy, lookahead);
	    printcoordinates(suggestion.xc, suggestion.yc);
	    printf("\n");
	} else if (strcmp(string, "printboard") == 0) {
	    printboard(board, "Reprint of Board");
	} else if (strcmp(string, "quit") == 0 || strcmp(string, "stop") == 0) {
	    printf("Bye!\n");
	    exit(0);
	} else {
	    ych = string[0];
	    if ((ych >= 'A') && (ych <= 'Z')) {
	        yc = ych - 'A';
	    } else {
	        yc = ych - 'a';
	    }
            yc++;
	    xch = string[1];
	    xc = xch - '0';

            if (validmove(board, xc, yc, X) == FALSE) {

	        /* Every fifth mistake - give him help */
	        if (mistakes % 5 != 0) {
	            printf("Illegal move\n");
	        } else {
		    printf("Illegal move - type help for command list\n");
	        }
	        mistakes++;
            }
	}
    }
    coordinates.xc = xc;
    coordinates.yc = yc;
    return(coordinates);
}

/*  Given a certain board and a list of coordinates, flip all the pieces at
 *  each of those coordinates:  O changes to X, X changes to O.
 */

do_flip(board, listnum)
struct BOARD *board;
int listnum;
{
    int loop, x, y;

    for (loop = 0; loop < list_length(listnum); loop++) {
	x = list_x(listnum, loop);
	y = list_y(listnum, loop);
	switch (board->loc[x][y]) {
	    case X: 
		    board->loc[x][y] = O;
		    break;
	    case O: 
		    board->loc[x][y] = X;
		    break;
	    default:
		    break;
	}
    }
}

/*  Given a certain board, a certain X-Y location, and a certain player,
 *  X or O, form a list of coordinates corresponding to the pieces that
 *  would be flipped if that player were to move at that place.
 *  It is guaranteed to add one and only one list to the stack.
 *
 *  Of course - don't even bother looking at a square that is not empty.
 *
 *  Return the number of pieces taken if player performed that move.
 *
 *  Algorithm:  Search in directions (-1,-1), (0,-1), (1,-1) ... (1,1)
 *  If there is an opponent's piece there, add it to the list and start
 *  "walking" in that direction.  If we get to the end of the board or to
 *  an EMPTY square, throw away the list and continue searching in another
 *  direction.  If we get to another of the player's pieces, keep the list
 *  and continue searching in another direction.
 */

formfliplist(board, x, y, player)
struct BOARD *board;
int x, y;
int player;
{
    int round_x, round_y;		/* scan in all directions */
    int walk_x, walk_y;			/* walk forward in each direction */
    int numlists;			/* number of the new list */
    int opponent;			/* the other player */
    int old;

    add_list();

    if (board->loc[x][y] != EMPTY) return(0);

    old = listcount;
    numlists = 1;
    opponent = other(player);
    for (round_y = -1; round_y <= 1; round_y++) {
	for (round_x = -1; round_x <= 1; round_x++) {

	    /* do not try to walk in the "nowhere" direction */
	    if (round_x == 0 && round_y == 0) continue;

	    /* take one step */
	    walk_x = x + round_x;
	    walk_y = y + round_y;

	    /* keep walking as long as we see opponents */
	    if (board->loc[walk_x][walk_y] == opponent) {
		add_list();
		numlists++;
	        while (board->loc[walk_x][walk_y] == opponent &&
		       board->loc[walk_x][walk_y] != BORDER)
		{
		    add_element(walk_x, walk_y);
		    walk_x += round_x;
		    walk_y += round_y;
		}
		/*  OK - We've come to end of opponents' string of pieces.
		 *  If there is one of your pieces at the end, keep it,
		 *  otherwise throw it away.
		 */
		if (board->loc[walk_x][walk_y] != player) {
		    zap_list();
		    numlists--;
		}
	    } /* end of walk in a certain direction */
	}
    } /* end of directional scan */
    /* put all the lists together as one */
    consolidate_lists(numlists);
    if (listcount != old) {
	printf("error! we've added a list!");
	printf("old=%d  new=%d\n", old, listcount);
    }
    return(list_length(listcount));
} /* end of formfliplist() */

/*  rerandomize - change the values leftright and updown - called by scanboard
 */

rerandomize(times)
int times;
{
    int loop;

    for (loop = 0; loop < times; loop++) {
        leftright++;
        if (leftright == 2) {
	    leftright = 0;
	    updown++;
	    if (updown == 2) {
	        updown = 0;
	    }
        }
    }
}

/*  EVALUTATION mostpieces - Conforms to the standard for evaluation functions.
 *  This function says the evaluation of a certain move x, y on a given board
 *  by a given player is equal to the number of pieces he takes.  Other
 *  evaluation functions should base their evaluations on this scale.
 *  0 is worthless, 5 is a pretty good move, 10 is excellent, neg = REAL bad
 *  Return INVALID if the move is invalid.
 */

mostpieces(board, x, y, player, countdown)
struct BOARD *board;
int x, y, player, countdown;
{
    int number;

    formfliplist(board, x, y, player);
    number = list_length(listcount);
    zap_list();
    if (number == 0) {
	return(INVALID);
    } else {
        return(number);
    }
}

/*  EVALUATION mostcorners - same as mostpieces but it tries to get corners
 *  and edges.  Corners are worth 10 points and edges are worth 5.
 */

mostcorners(board, x, y, player, countdown)
struct BOARD *board;
int x, y, player, countdown;
{
    int evaluation;

    evaluation = mostpieces(board, x, y, player, 0);
    if (evaluation != INVALID) {
	evaluation += bonusvalue(x, y);
    }
    return(evaluation);
}

/*  EVALUATION lookone - looks ahead one move.  Calculates the number of
 *  pieces that it is going to take.  If valid, internally form the board
 *  that results.  Call scanboard with mostcorners as the evaluation
 *  function and then subtract the other player's best move's evaluation from
 *  yours.  Return that number.
 */

lookone(board, x, y, player, countdown)
struct BOARD *board;
int x, y, player, countdown;
{
    int eval_you, eval_him;
    struct BOARD tempboard;
    struct COORDINATES coordinates;

    eval_you = mostcorners(board, x, y, player, 0);
    if (eval_you == INVALID) return(INVALID);

    /* Make a temporary board and move the player */
    copyboard(board, &tempboard);
    formfliplist(&tempboard, x, y, player);
    do_flip(&tempboard, listcount);
    zap_list();
    tempboard.loc[x][y] = player;

    /* Find value opponents' best move and make it */
    coordinates = scanboard(&tempboard, other(player), mostcorners, 1);
    formfliplist(&tempboard, coordinates.xc, coordinates.yc, other(player));
    eval_him = list_length(listcount) + 
	       bonusvalue(coordinates.xc, coordinates.yc);
    zap_list();
    return(eval_you - eval_him);
}

/*  Called only by look - prints a bunch of spaces to make the game tree
 *  readable. 
 */

tabover(countdown)
int countdown;
{
    int loop;
    for (loop = 0; loop < lookahead - countdown; loop++) {
	printf("        ");
    }
}

/*  EVALUATION look - looks ahead countdown moves.  Returns the best move.
 *  The idea is an extension of the above routine, lookone.  It uses a depth-
 *  first maximization of move value for the game-tree formed by all possible
 *  moves and responses for each player.  This "game-tree" is not an actual
 *  tree, as it never needs to be stored.  Look calls scanboard to maximize
 *  the move value for each possible move, which in turn calls look.  Look
 *  also has to handle the base-case, which is to evaluate using mostcorners.
 */

look(board, x, y, player, countdown)
struct BOARD *board;
int x, y, player, countdown;
{
    int eval_you, eval_him;
    int loop;
    struct BOARD tempboard;
    struct COORDINATES coordinates;

    /* base case: no lookahead */

    if (debug) {
        tabover(countdown);
        printf("d=%d pl=%c loc=", countdown, p[player]);
        printcoordinates(x,y);
        printf("\n");
    }

    if (countdown == 0) {
	int worth;
	worth = mostcorners(board, x, y, player, countdown);
	if (debug) {
	    tabover(countdown);
	    printf("worth = %d\n", worth);
	}	
	return(worth);
    }

    /* lookahead: pretend player goes at this x,y, form the resultant board,
     * and call scanboard with the other player and the temporary board, with
     * a depth of countdown-1.
     */

    copyboard(board, &tempboard);
    formfliplist(&tempboard, x, y, player);
    if (list_length(listcount) == 0) {
	zap_list();
	return(INVALID);
    }
    do_flip(&tempboard, listcount);
    eval_you = list_length(listcount) + bonusvalue(x, y);
    zap_list();
    tempboard.loc[x][y] = player;

    /* We've formed the new board.  The other player gets a 'chance' to play
     * now and make his supposed best move.  He's also going to look ahead,
     * but we'll have to make him look ahead one move less.
     */

    coordinates = scanboard(&tempboard, other(player), look, countdown-1);
    eval_him = coordinates.eval;
    if (debug) {
        tabover(countdown);
        printf("mval=%d bresp=%d\n\n", eval_you, eval_him);
    }
    return(eval_you - eval_him);
}

/*  corner - returns TRUE if (x,y) is a corner, FALSE otherwise
 */

corner(x, y)
int x, y;
{
    return((x==1 || x==8) && (y==1 || y==8));
}

/*  edge - returns TRUE if (x,y) is on an edge but NOT a corner, else FALSE
 */

edge(x, y)
int x, y;
{
    return((!corner(x, y)) && (x == 1 || x == 8) && (y == 1 || y == 8));
}

/*  nextcorner- returns TRUE if (x,y) is within one square of a corner, else
 *  FALSE
 */

nextcorner(x, y)
int x, y;
{
    int xsearch, ysearch;

    for (xsearch = -1; xsearch <= 1; xsearch++) {
	for (ysearch = -1; ysearch <= 1; ysearch++) {
	    if (corner(x + xsearch, y + ysearch)) return(TRUE);
	}
    }
    return(FALSE);
}

/*  bonusvalue - return 0 if the piece is in the middle, 5 if on edge, 10 if
 *  on corner or -8 if it is next to a corner.
 */

bonusvalue(x, y)
int x, y;
{
    if (corner(x, y)) return(10);
    if (nextcorner(x, y)) return(-8);
    if (edge(x, y)) return(5);
    return(0);
}

/*  scanboard - given a certain board and player, and a POINTER TO A FUNCTION
 *  that returns the evaluation of a given move at location (x,y)...
 *
 *  Find the best move possible for that player by returning coordinates.
 *  Return coordinates if found ANY move, invalid coordinates if found none.
 *  Also return, in the coordinate structure, the evaluation of that move,
 *  which is not necessarily the number of pieces taken.
 *
 *  This function is guaranteed to add no lists to the flip stack.
 *
 *  Algorithm: For each square, form flip list.  If not the best move so far,
 *  get rid of that flip list.  If best move so far, delete old best move, if
 *  there was one, and replace it with this one.
 *
 *  Great pains are taken to decrease predictability by varying the search
 *  order of locations.  All the loop_init and xupper stuff relates to that.
 */

struct COORDINATES scanboard(board, player, evaluate, depth)
struct BOARD *board;
int player;
int (*evaluate)();
int depth;
{
    struct COORDINATES coordinates;	/* return value */
    int scan_x, scan_y;			/* scan every x, y coordinate */
    int xc, yc;				/* best coordinates so far */
    int max = -9999;			/* best evaluation so far */
    int evaluation;			/* value of certain move */

    /* Extra stuff added to ensure randomness in left-right, up-down
     * searching.  updown and leftright determine which are to be used:
     * count from 1 to 8 or from 8 down to 1.
     */

    static int loop_init[] = {1, 8};
    static int loop_final[] = {8, 1};
    static int loop_inc[] = {1, -1};

    int xlower, xupper, xinc;
    int ylower, yupper, yinc;

    xlower = loop_init[leftright];
    xupper = loop_final[leftright];
    xinc = loop_inc[leftright];

    ylower = loop_init[updown];
    yupper = loop_final[updown];
    yinc = loop_inc[updown];

    xc = NOWHERE;			/* an invalid move to begin with */
    yc = NOWHERE;

    for (scan_x = xlower ; scan_x != xupper + xinc; scan_x += xinc) {
	for (scan_y = ylower; scan_y != yupper + yinc ; scan_y += yinc) {
	    if (board->loc[scan_x][scan_y] != EMPTY) continue;

	    /* Evaluate the move at scan_x, scan_y using the function pointed
	     * to by evaluate.  Get the evaluation number in the integer
	     * evaluation.  It is up to the evaluation function to return
	     * the constant INVALID if the move is invalid */

	    rerandomize(board->loc[scan_x][scan_y]);
	    if (validmove(board, scan_x, scan_y, player)) {
	        evaluation = (*evaluate)(board, scan_x, scan_y, player, depth);
                if (evaluation > max && evaluation != INVALID) {
		    xc = scan_x;
		    yc = scan_y;
		    max = evaluation;
		}
	    }
	}
    }
    coordinates.xc = xc;
    coordinates.yc = yc;
    coordinates.eval = max;
    return(coordinates);
}

/*  Initialize the game log */
init_gamelog()
{
    gamelog[0].X_count = 2;
    gamelog[0].O_count = 2;
}

/*  Update the game log, given the coordinates of the most recent move
 *  If pass, xc=PASS and yc is undefined. (Only first two parameters passed)
 */

update_gamelog(player, xc, yc)
int player, xc, yc;
{
    turns++;
    if (xc == PASS) {
	gamelog[turns].pass = TRUE;
	gamelog[turns].take = 0;
    } else {
	gamelog[turns].pass = FALSE;
        gamelog[turns].xloc = xc;
        gamelog[turns].yloc = yc;
        gamelog[turns].X_count = count(&gameboard, X);
        gamelog[turns].O_count = count(&gameboard, O);
        if (player == X) {
	gamelog[turns].take = gamelog[turns-1].O_count - count(&gameboard, O);
        } else {
	gamelog[turns].take = gamelog[turns-1].X_count - count(&gameboard, X);
        }
    }
} 

/*  Print out the gamelog */

print_gamelog()
{
    int loop, player;
    int xcount, ocount;
    int totaltaken[2];    		/* how many each has taken so far */

    if (firstmove == TRUE) {
	player = X;
    } else {
	player = O;
    }

    totaltaken[X] = 0;
    totaltaken[O] = 0;

    for (loop = 1; loop <= turns; loop++) {
	printf("%2d: ", loop);
	if (gamelog[loop].pass == TRUE) {
	    printf("%c forced to pass...", p[player]);
	} else {
	    printf("%c at ", p[player]);
	    printcoordinates(gamelog[loop].xloc, gamelog[loop].yloc);
	    xcount = gamelog[loop].X_count;
	    ocount = gamelog[loop].O_count;

	    /* print how many pieces player just took */
	    printf(" taking %2d  ", gamelog[loop].take);
	    totaltaken[player] += gamelog[loop].take;

	    /* print 'ahead by' message */
	    if (xcount > ocount) {
	        printf("X ahead by %2d", xcount - ocount);
	    } else if (ocount > xcount) {
	        printf("O ahead by %2d", ocount - xcount);
	    } else {
	        printf("Tie so far     ");
	    }
	    printf("  Total taken = %2d", totaltaken[player]);
	}

	printf("\n");
	player = other(player);
    }
}

/*  Print the instructions */

instructions()
{
printf("\
Welcome to Othello!  The object of the game is to finish with more pieces\n\
that your opponent.  You will play X and I will play O.  We take turns\n\
placing pieces on an 8x8 grid.  You MUST take opponents' pieces on every\n\
turn.  To take pieces, place an X such that is surrounds a line or lines of\n\
O's.  Then all the O's are flipped to X's, i.e. \"X O O O X\" becomes\n\
\"X X X X X\".  Note that by clever placement you can take more than one\n\
line of O's at a time!  There are many extra features in the game; type help\n\
for more information.  Also: If a player has no move, he is forced to pass.\
Example first moves: D3, C4, F5, E6.\n\
");
}

/*  Print help */

help()
{
printf("\
The program is expecting a pair of coordinates in the form\n\
A1, B6, F2, etc.  You are player X.  You may move only on empty\n\
squares such that you surround a line of O's in any direction.\n\
\n\
help, ?             list these commands\n\
quit, stop          quit the game\n\
instructions        see the instructions again\n\
printboard          see the board again\n\
suggest             let the computer make a suggestion\n\
auto                let the computer finish the game\n\
log                 print a log of all the moves so far, with other info\n\
stats               print numerous statistics on the game so far\n\
");
}

/* END OF OTHELLO BY JONATHAN DUBMAN - APPROXIMATELY 1300 LINES, 38K TEXT */
