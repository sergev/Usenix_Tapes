

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

char p[3] ;				/* board pieces: p[0], p[X], p[O] */
int listcount;				/* most recently allocated list */
int mistakes ;				/* number of mistakes player's made */
int updown, leftright ;			/* used in scanboard only - see that */
int computer ;				/* flag for computer against itself */
int turns ;				/* number of turns */
int firstmove;				/* TRUE if player first, else FALSE */
int lookahead;				/* depth of search for comp's move */
int autolook;				/* lookahead for player when computer
					   takes over player's position */

