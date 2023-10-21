/*
 * network chess header
 */

#define BOOL 			int
#ifndef TRUE
#define TRUE			1
#endif
#ifndef FALSE
#define FALSE			0
#endif

#define	SERVERPROGNUM	((u_long) 0x31233216)	/* RPC daemon program number */
#define VERSNUM			((u_long) 1)	/* RPC version number */

/*
 * daemon procedure numbers
 */
#define	REQPROCNUM		((u_long) 1)	/* game request */
#define	ACKPROCNUM		((u_long) 2)	/* game acknowledge */
#define	CANCELPROCNUM		((u_long) 3)	/* cancel a game request */

/*
 * peer procedure numbers 
 */
#define	ACCEPTPROCNUM		((u_long) 1)	/* game accepted */
#define	MOVEPROCNUM		((u_long) 2)	/* chess move */
#define	COLORFAILPROCNUM	((u_long) 3)	/* color arbitration failure */
#define	UNDOPROCNUM		((u_long) 4)	/* undo request */
#define	RESIGNPROCNUM		((u_long) 5)	/* resignation */
#define	UNDOACKPROCNUM		((u_long) 6)	/* undo request response */
#define	MSGPROCNUM		((u_long) 7)	/* one-liner antagonism */
#define	RESTOREMOVEPROCNUM	((u_long) 8)	/* move during game restoration */
#define	ENDRESTOREPROCNUM	((u_long) 9)	/* restoration/setup complete */
#define	SETUPPROCNUM		((u_long) 10)	/* board setup change */
#define	GOODBYEPROCNUM		((u_long) 11)	/* player killed his tool */

/*
 * color arbitration values - the values are important, as the 
 * game can proceed when the sum of the two players' colors is 
 * WANTSWHITE+WANTSBLACK; also, the values are used as indices 
 * into arrays.
 */
#define	WANTSBLACK		0		/* user wants black */
#define	BLACK			WANTSBLACK	/* user has black */
#define	WANTSWHITE		1		/* user wants white */
#define	WHITE			WANTSWHITE	/* user has white */
#define	EITHERCOLOR		2		/* user doesn't care */

#define	OTHERCOLOR(a)	((a) == WHITE ? BLACK : WHITE)
/*
 * rendezvous information 
 */
typedef struct {
    unsigned long progNum;			/* RPC program number of requester */
    int color;					/* requested color */
    int resumeGame;				/* boolean: wants to resume a game */
    char hostName[256];				/* host name of requester */
    char userName[256];				/* name of requesting user */
} GameRequest;

/*
 * piece move information 
 */
typedef struct {
    int x1;					/* origin square */
    int y1;
    int x2; 					/* destination square */
    int y2;
    int newPieceType;				/* new piece type for 8th rank pawns */
} Move;

/*
 * board square dimensions
 */
#define	SQUARE_WIDTH	64
#define	SQUARE_HEIGHT	SQUARE_WIDTH

/*
 * board coordinates
 */
typedef struct {
    int x;
    int y;
} BoardCoordinate;

/*
 * piece types
 */
typedef enum {
    PAWN = 0,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    NULLPC,
} PieceType;

/*
 * square state
 */
typedef struct {
    PieceType type;
    int color;
} Square;

/*
 * setup change information 
 */
typedef struct {
    int x;
    int y;
    PieceType type;
    int color;
} SetupChange;

/*
 * mouse (in the board subwindow) activity states
 */
typedef enum {
    IDLE,		/* nada */
    MOVING_PIECE,	/* animating a piece with the left button down */
    PROMOTING_PAWN,	/* selecting a piece type for a pawn promotion */
    CONFIRMING,		/* borrowed for confirmation of something */
    LOCKED,		/* locked (ignored until unlocked) */
    SETUP,		/* setting up a board */
} MouseState;

/*
 * transcript types
 */
#define	TR_MIN_TYPE		0
#define	TR_FORMAL_NORMAL	0	/* P/K2-K4, R/KB3xP/KB5, etc. */
#define	TR_MIN_NORMAL		1	/* P-K4, RxP, etc. */
#define	TR_ALGEBRAIC		2	/* D2D4, etc. */
#define	TR_MAX_TYPE		2

#include "decls.h"

