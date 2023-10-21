/*
 * manage the board state
 */

#include <stdio.h>
#include <strings.h>
#include <sys/types.h>

#include "nchess.h"

#define	TABSIZE		8

BOOL GameOver = FALSE;			/* game is over */
int MyColor = EITHERCOLOR;		/* my color */
int PeerColor = EITHERCOLOR;		/* opponent's color */
int Turn;				/* whose turn it is */
int InitialTurn;			/* whose turn it was initially */

BOOL KingMoved = FALSE,			/* has the king moved ? */
    KingRookMoved = FALSE,		/* has the king's rook moved ? */
    QueenRookMoved = FALSE;		/* has the queen's rook moved ? */

struct moveListNode {			/* move record list node */
    struct moveListNode * prev;		/* previous move */
    struct moveListNode * next;		/* next move */
    Move move;				/* the mechanics of the move */
    int tookX;				/* x coordinate of captured piece */
    int tookY;				/* y coordinate of captured piece */
    PieceType tookPiece;		/* captured piece type */
    BOOL movedQueenRook;		/* did this move the queen rook for the first time? */
    BOOL movedKingRook;			/* did this move the king rook for the first time? */
    BOOL movedKing;			/* did this move the king for the first time? */
};

#define MOVELISTNODE	struct moveListNode

MOVELISTNODE 
    * firstMove = (MOVELISTNODE *) 0,	/* head of the move list */
    * lastMove = (MOVELISTNODE *) 0,	/* tail of the move list */
    ** nextLink = &firstMove;		/* next link */

PieceType SourceTypes[8] = {
    ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK
};

Square InitialBoard[8][8] = {
    {ROOK,BLACK}, {KNIGHT,BLACK}, {BISHOP,BLACK}, {QUEEN,BLACK}, {KING,BLACK}, {BISHOP,BLACK}, {KNIGHT,BLACK}, {ROOK,BLACK},
    {PAWN,BLACK}, {PAWN,BLACK}, {PAWN,BLACK}, {PAWN,BLACK}, {PAWN,BLACK}, {PAWN,BLACK}, {PAWN,BLACK}, {PAWN,BLACK},
    {NULLPC}, {NULLPC}, {NULLPC}, {NULLPC}, {NULLPC}, {NULLPC}, {NULLPC}, {NULLPC},
    {NULLPC}, {NULLPC}, {NULLPC}, {NULLPC}, {NULLPC}, {NULLPC}, {NULLPC}, {NULLPC},
    {NULLPC}, {NULLPC}, {NULLPC}, {NULLPC}, {NULLPC}, {NULLPC}, {NULLPC}, {NULLPC},
    {NULLPC}, {NULLPC}, {NULLPC}, {NULLPC}, {NULLPC}, {NULLPC}, {NULLPC}, {NULLPC},
    {PAWN,WHITE}, {PAWN,WHITE}, {PAWN,WHITE}, {PAWN,WHITE}, {PAWN,WHITE}, {PAWN,WHITE}, {PAWN,WHITE}, {PAWN,WHITE},
    {ROOK,WHITE}, {KNIGHT,WHITE}, {BISHOP,WHITE}, {QUEEN,WHITE}, {KING,WHITE}, {BISHOP,WHITE}, {KNIGHT,WHITE}, {ROOK,WHITE},
};

Square MainBoard[8][8];

/*
 * are there any pieces between the start and the destination on 
 * board 'board' ?
 * (the start and destination must be on the same diagonal, the 
 * same rank, or the same file.)
 */
BOOL
interveningPieces(board, from, to)
    Square board[][8];
    BoardCoordinate * from, * to;
{
    register int x, y, xmax, ymax, xdelt, ydelt;
    int xmin, ymin;

    /*
     * if there are no intervening squares
     */
    if (abs(from->x - to->x) <= 1 && abs(from->y - to->y) <= 1) {
	return(FALSE);
    }
    /*
     * build the intervening path information 
     */
    if (from->x < to->x) {
	xmin = from->x + 1;
	xmax = to->x - 1;
	xdelt = 1;
    } else if (from->x > to->x) {
	xmin = to->x + 1;
	xmax = from->x - 1;
	xdelt = -1;
    } else {
	xmin = xmax = from->x;
	xdelt = 0;
    }
    if (from->y < to->y) {
	ymin = from->y + 1;
	ymax = to->y - 1;
	ydelt = 1;
    } else if (from->y > to->y) {
	ymin = to->y + 1;
	ymax = from->y - 1;
	ydelt = -1;
    } else {
	ymin = ymax = from->y;
	ydelt = 0;
    }
    x = from->x + xdelt ; y = from->y + ydelt ;
    while (x >= xmin && x <= xmax && y >= ymin && y <= ymax) {
	if (board[y][x].type != NULLPC) 
	    return(TRUE);
	x += xdelt ; y += ydelt ;
    }
    return(FALSE);
}

/*
 * set up a new board 
 */
void
initBoard(board) 
    Square board[][8];
{
    register int i, j;
    
    for (i = 0 ; i < 8 ; i++) {
	for (j = 0 ; j < 8 ; j++) {
	    board[i][j].type = InitialBoard[i][j].type;
	    board[i][j].color = InitialBoard[i][j].color;
	}
    }
}

/*
 * set up the visible playing surface at the beginning of the game
 */
void
InitBoard()
{
    InitialTurn = Turn = WHITE;
    initBoard(MainBoard);
}

/*
 * describe the state of the board square at x, y
 */
Square *
GetSquare(x, y)
{
    return (&MainBoard[y][x]);
}

/*
 * describe the state of the source board square at x, y
 */
Square *
GetSrcSquare(x, y)
{
    static Square square;

    square.color = (y <= 3 ? BLACK : WHITE);
    if (y == 1 || y == 6)
	square.type = PAWN;
    else if (y > 1 && y < 6)
	square.type = NULLPC;
    else 
	square.type = SourceTypes[x];
    return (&square);
}

/*
 * does the user have a piece at bp?
 */
BOOL
IsOurPieceAt(bp)
    BoardCoordinate * bp;
{
    return (MainBoard[bp->y][bp->x].color == MyColor
    && MainBoard[bp->y][bp->x].type != NULLPC);
}

/*
 * is this a setup source square?
 */
BOOL 
IsSrcPieceAt(bp)
    BoardCoordinate * bp;
{
    return(bp->y < 2 || bp->y > 5);
}

void
DoSetupChange(setup)
    SetupChange * setup;
{
    InitialBoard[setup->y][setup->x].type = MainBoard[setup->y][setup->x].type = setup->type;
    InitialBoard[setup->y][setup->x].color = MainBoard[setup->y][setup->x].color = setup->color;
    DrawSquare(setup->x, setup->y, &MainBoard[setup->y][setup->x]);
}

/*
 * is the square at 'x','y' on the board 'board' threatened by any 
 * pieces of the opposite color of 'myColor'?
 */
BOOL
threatened(board, xt, yt, myColor)
    Square board[][8];
    int xt, yt, myColor;
{
    register int x, y;
    BoardCoordinate from, to;

    to.x = xt; to.y = yt;
    for (x = 0 ; x < 8 ; x++) {
	for (y = 0 ; y < 8 ; y++) {
	    if (board[y][x].type == NULLPC || board[y][x].color == myColor)
		continue;
	    from.x = x; from.y = y;
	    switch (board[y][x].type) {
	    case KNIGHT:
		if (xt == x - 2 && (yt == y + 1 || yt == y - 1)
		|| xt == x - 1 && (yt == y + 2 || yt == y - 2)
		|| xt == x + 1 && (yt == y + 2 || yt == y - 2)
		|| xt == x + 2 && (yt == y + 1 || yt == y - 1))
		    return(TRUE);
		break;
	    case ROOK:
		if ((x == xt || y == yt) && ! interveningPieces(board, &from, &to))
		    return(TRUE);
		break;
	    case BISHOP:
		if (abs(x - xt) == abs(y - yt) && ! interveningPieces(board, &from, &to))
		    return(TRUE);
		break;
	    case QUEEN:
		if ((abs(x - xt) == abs(y - yt) || x == xt || y == yt)
		&& ! interveningPieces(board, &from, &to))
		    return(TRUE);
		break;
	    case PAWN:
		if ((xt == x - 1 || xt == x + 1)
		&& (myColor == WHITE && yt == y + 1 
		    || myColor == BLACK && yt == y - 1)
		)
		    return(TRUE);
		break;
	    case KING:
		if (abs(x - xt) <= 1 && abs(y - yt) <= 1)
		    return(TRUE);
		break;
	    }
	}
    }
    return(FALSE);
}

/*
 * is this a legal move?
 */
BOOL
isMoveLegal(board, color, from, to)
    Square board[][8];
    int color;
    BoardCoordinate * from, * to;
{
    Square oldS1, oldS2, oldS3, * s1, * s2, * s3 = (Square *) 0;
    BoardCoordinate bc;
    BOOL ok;

    s1 = &board[from->y][from->x];
    s2 = &board[to->y][to->x];
    /* illegal: if there is a piece of my color on the destination */
    if (s2->type != NULLPC && s2->color == color)
	return(FALSE);
    switch (s1->type) {
    case KNIGHT:
	/* illegal: if this is not a probable knight move */
	if (to->x == from->x || to->x > from->x + 2 || to->x < from->x - 2
	|| to->x == from->x - 2 && to->y != from->y + 1 && to->y != from->y - 1
	|| to->x == from->x - 1 && to->y != from->y + 2 && to->y != from->y - 2
	|| to->x == from->x + 1 && to->y != from->y + 2 && to->y != from->y - 2
	|| to->x == from->x + 2 && to->y != from->y + 1 && to->y != from->y - 1)
	    return(FALSE);
	break;
    case ROOK:
	/* 
	 * illegal: if this is not a probable rook move 
	 * or there is a piece between start and destination
	 */
	if (from->x != to->x && from->y != to->y
	|| interveningPieces(board, from, to))
	    return(FALSE);
	break;
    case BISHOP:
	/* 
	 * illegal: if this is not a probable bishop move 
	 * or there is a piece between start and destination
	 */
	if (abs(from->x - to->x) != abs(from->y - to->y)
	|| interveningPieces(board, from, to))
	    return(FALSE);
	break;
    case QUEEN:
	/* 
	 * illegal: if this is not a probable queen move 
	 * or there is a piece between start and destination
	 */
	if (abs(from->x - to->x) != abs(from->y - to->y)
	&& from->x != to->x && from->y != to->y
	|| interveningPieces(board, from, to)) 
	    return(FALSE);
	break;
    case PAWN:
	if (color == WHITE) {
	    /* 
	     * potentially legal: moves forward onto an empty square 
	     */
	    if (to->x == from->x && s2->type == NULLPC
	    && (to->y == from->y - 1 
		|| to->y == 4 && from->y == 6 && ! interveningPieces(board, from, to)))
		break;
	    /* 
	     * potentially legal: diagonal captures (including en passant)
	     */
	    if (to->y == from->y - 1 && (to->x == from->x + 1 || to->x == from->x - 1)) {
		if (s2->type != NULLPC)
		    break;
		else if (to->y == 2
		&& lastMove != (MOVELISTNODE *) 0
		&& lastMove->move.x1 == to->x 
		&& lastMove->move.x2 == to->x 
		&& lastMove->move.y1 == 1
		&& lastMove->move.y2 == 3 
		&& board[3][to->x].type == PAWN) {
		    s3 = &board[3][to->x];
		    break;
		}
	    }
	} else {
	    /* 
	     * potentially legal: moves forward onto an empty square 
	     */
	    if (to->x == from->x && s2->type == NULLPC
	    && (to->y == from->y + 1 
		|| to->y == 3 && from->y == 1 && ! interveningPieces(board, from, to)))
		break;
	    /* 
	     * potentially legal: diagonal captures (including en passant) 
	     */
	    if (to->y == from->y + 1 && (to->x == from->x + 1 || to->x == from->x - 1)) {
		if (s2->type != NULLPC)
		    break;
		else if (to->y == 5
		&& lastMove != (MOVELISTNODE *) 0
		&& lastMove->move.x1 == to->x 
		&& lastMove->move.x2 == to->x 
		&& lastMove->move.y1 == 6
		&& lastMove->move.y2 == 4
		&& board[4][to->x].type == PAWN) {
		    s3 = &board[4][to->x];
		    break;
		}
	    }
	}
	/* illegal: any other pawn moves */
	return(FALSE);
    case KING:
	/*
	 * potentially legal: single space moves
	 */
	if (abs(to->x - from->x) <= 1 && abs(to->y - from->y) <= 1)
	    break;
	/*
	 * potentially legal: castling moves with unmoved king and rook
	 * with no pieces between king and rook
	 *
	 * note: need to verify that the king and rook are still on their
	 * original squares in addition to not having moved, since a setup
	 * that finds them there assumes they were always there, but does
	 * not void the moved flags if they are not initially there (to 
	 * avoid having to store that state information in the save file).
	 * this is compatible with the unix chess game's interpretation of
	 * setups.
	 */
	if ( ! KingMoved && s1->type == KING && s1->color == color
	&& ! threatened(board, from->x, from->y, color)) {
	    if (to->x == 6 && ! KingRookMoved 
	    && board[to->y][7].type == ROOK && board[to->y][7].color == color) {
		bc.x = 7;
		bc.y = from->y;
		if ( ! interveningPieces(board, from, &bc)
		&& ! threatened(board, 5, from->y, color))
		    break;
	    } else if (to->x == 2 && ! QueenRookMoved
	    && board[to->y][0].type == ROOK && board[to->y][0].color == color) {
		bc.x = 0;
		bc.y = from->y;
		if ( ! interveningPieces(board, from, &bc)
		&& ! threatened(board, 3, from->y, color))
		    break;
	    }
	}
	/* illegal: any other king move */
	return(FALSE);
    }
    /* 
     * single remaining constraint: the king must not be in check after 
     * the move 
     */
    /* temporarily stuff the move in the board state */
    oldS1.type = s1->type; oldS1.color = s1->color;
    s1->type = NULLPC;
    oldS2.type = s2->type; oldS2.color = s2->color;
    s2->type = oldS1.type; s2->color = oldS1.color;
    if (s3 != (Square *) 0) {
	oldS3.type = s3->type; oldS3.color = s3->color;
	s3->type = NULLPC;
    }
    ok = ! inCheck(board, color);
    /* back out the move */
    s1->type = oldS1.type; s1->color = oldS1.color;
    s2->type = oldS2.type; s2->color = oldS2.color;
    if (s3 != (Square *) 0) {
	s3->type = oldS3.type; s3->color = oldS3.color;
    }
    return(ok);
}

/*
 * is this move (of ours) legal?
 */
BOOL
IsMoveLegal(from, to)
    BoardCoordinate * from, * to;
{
    return (isMoveLegal(MainBoard, MyColor, from, to));
}

/*
 * perform a move (either ours or the opponent's)
 * the move is assumed to be legal; hence, checks for certain move types
 * are minimal.
 */
void 
DoMove(move, drawIt)
    register Move * move;
    BOOL drawIt;
{
    register Square * from, * to;
    register MOVELISTNODE * mlnp;

    from = &MainBoard[move->y1][move->x1];
    to = &MainBoard[move->y2][move->x2];
    /* 
     * create the new move list node 
     */
    if ((mlnp = (MOVELISTNODE *) malloc(sizeof(MOVELISTNODE))) == (MOVELISTNODE *) 0) {
	fprintf(stderr, "can't create move list node\n");
	exit(1);
    }
    * nextLink = mlnp;			/* stuff the forward link */
    mlnp->prev = lastMove;		/* and the backward link */
    mlnp->next = (MOVELISTNODE *) 0;	/* set up the next forward link */
    nextLink = &(mlnp->next);
    lastMove = mlnp;			/* and the new tail pointer */
    mlnp->move.x1 = move->x1;		/* store the basic move mechanics */
    mlnp->move.y1 = move->y1;
    mlnp->move.x2 = move->x2;
    mlnp->move.y2 = move->y2;
    mlnp->move.newPieceType = (int) (mlnp->tookPiece = NULLPC);
    mlnp->movedQueenRook = mlnp->movedKingRook = mlnp->movedKing = FALSE;
    /* if this is an en passant capture */
    if (from->type == PAWN && to->type == NULLPC && move->x1 != move->x2) {
	AddVictim(PAWN, MainBoard[move->y1][move->x2].color, drawIt);
	mlnp->tookPiece = PAWN;
	mlnp->tookX = move->x2;
	mlnp->tookY = move->y1;
	MainBoard[move->y1][move->x2].type = NULLPC;
	if (drawIt) 
	    DrawSquare(move->x2, move->y1, &MainBoard[move->y1][move->x2]);
    /* else if it is a normal capture */
    } else if (to->type != NULLPC) {
	AddVictim(to->type, to->color, drawIt);
	mlnp->tookPiece = to->type;
	mlnp->tookX = move->x2;
	mlnp->tookY = move->y2;
    }
    /* move the piece from the origin square to the dest. square */
    to->type = from->type;
    to->color = from->color;
    /* put a null piece in the origin square */
    from->type = NULLPC;
    /* re-draw the origin and destination squares */
    if (drawIt) {
	DrawSquare(move->x1, move->y1, from);
	DrawSquare(move->x2, move->y2, to);
    }
    switch(to->type) {
    case KING:
	/* if this is my king, void the castling option */
	if (to->color == MyColor && ! KingMoved) 
	    KingMoved = mlnp->movedKing = TRUE;
	/* if this is a king-side castling move, move the king's rook */
	if (move->x1 == 4 && move->x2 == 6) {
	    from = &MainBoard[move->y2][7];
	    to = &MainBoard[move->y2][5];
	    to->type = ROOK;
	    to->color = from->color;
	    from->type = NULLPC;
	    if (drawIt) {
		DrawSquare(7, move->y1, from);
		DrawSquare(5, move->y1, to);
	    }
	/* else if this is a queen-side castling move, move the queen's rook */
	} else if (move->x1 == 4 && move->x2 == 2) {
	    from = &MainBoard[move->y2][0];
	    to = &MainBoard[move->y2][3];
	    to->type = ROOK;
	    to->color = from->color;
	    from->type = NULLPC;
	    if (drawIt) {
		DrawSquare(0, move->y1, from);
		DrawSquare(3, move->y2, to);
	    }
	}
	break;
    case PAWN:
	/* if this is a pawn reaching the 8th rank, polymorph it */
	if (move->y2 == 0 || move->y2 == 7) {
	    to->type = 
		(PieceType) (mlnp->move.newPieceType = move->newPieceType);
	    if (drawIt)
		DrawSquare(move->x2, move->y2, to);
	}
	break;
    case ROOK:
	/* check for first rook moves (to void the castling option) */
	if (to->color == MyColor) {
	    if (MyColor == BLACK) {
		if (move->x1 == 0 && move->y1 == 0 && ! QueenRookMoved) 
		    QueenRookMoved = mlnp->movedQueenRook = TRUE;
		else if (move->x1 == 7 && move->y1 == 0 && ! KingRookMoved)
		    KingRookMoved = mlnp->movedKingRook = TRUE;
	    } else {
		if (move->x1 == 0 && move->y1 == 7 && ! QueenRookMoved) 
		    QueenRookMoved = mlnp->movedQueenRook = TRUE;
		else if (move->x1 == 7 && move->y1 == 7 && ! KingRookMoved)
		    KingRookMoved = mlnp->movedKingRook = TRUE;
	    }
	} 
	break;
    }
}

/*
 * add a resignation as the last move in the move list.
 * this is marked as a move with a negative y destination coordinate
 * the color of the resigning player is stored in the x destination
 * coordinate.
 *
 * note: the last play pointer is not updated, so that the "Last Play"
 * button will show the move that preceded the resignation.
 */
void 
DoResignation(color)
    int color;
{
    register MOVELISTNODE * mlnp;

    /* 
     * create the new move list node 
     */
    if ((mlnp = (MOVELISTNODE *) malloc(sizeof(MOVELISTNODE))) == (MOVELISTNODE *) 0) {
	fprintf(stderr, "can't create move list node\n");
	exit(1);
    }
    * nextLink = mlnp;			/* stuff the forward link */
    mlnp->prev = lastMove;		/* and the backward link */
    mlnp->next = (MOVELISTNODE *) 0;	/* set up the next forward link */
    mlnp->move.x2 = color;
    mlnp->move.y2 = -1;
    GameOver = TRUE;
}

/*
 * undo the last move (either ours or the opponent's).
 */
void 
UnDoMove()
{
    register Square * from, * to;
    register Move * move;
    register MOVELISTNODE * mlnp;

    if ((mlnp = lastMove) == (MOVELISTNODE *) 0) {
	Message("You are already undone.");
	return;
    }
    move = &(mlnp->move);
    if (move->y2 < 0) {
	fputs("internal botch: tried to undo a resignation\n", stderr);
	return;
    }
    from = &MainBoard[move->y1][move->x1];
    to = &MainBoard[move->y2][move->x2];
    /* move the piece from the destination square to the origin square */
    from->type = to->type;
    from->color = to->color;
    to->type = NULLPC;
    /* if this move was a pawn reaching the eighth rank, turn it back into a lowly pawn */
    if (move->newPieceType != (int) NULLPC) 
	from->type = PAWN;
    /* re-draw the origin and destination squares */
    DrawSquare(move->x1, move->y1, from);
    DrawSquare(move->x2, move->y2, to);
    /* if this was a capture, re-draw the captured piece */
    if (mlnp->tookPiece != NULLPC) {
	DeleteVictim(
	    MainBoard[mlnp->tookY][mlnp->tookX].type = mlnp->tookPiece,
	    MainBoard[mlnp->tookY][mlnp->tookX].color = (from->color == BLACK ? WHITE : BLACK));
	DrawSquare(mlnp->tookX, mlnp->tookY, &MainBoard[mlnp->tookY][mlnp->tookX]);
    }
    /* if this move affected the ability to castle, restore it */
    if (mlnp->movedKing) 
	KingMoved = FALSE;
    if (mlnp->movedKingRook)
	KingRookMoved = FALSE;
    if (mlnp->movedQueenRook)
	QueenRookMoved = FALSE;
    switch(from->type) {
    case KING:
	/* if this was a king-side castling move, move the king's rook back */
	if (move->x1 == 4 && move->x2 == 6) {
	    from = &MainBoard[move->y2][7];
	    to = &MainBoard[move->y2][5];
	    from->type = ROOK;
	    from->color = to->color;
	    to->type = NULLPC;
	    DrawSquare(7, move->y1, from);
	    DrawSquare(5, move->y1, to);
	/* else if this is a queen-side castling move, move the queen's rook back */
	} else if (move->x1 == 4 && move->x2 == 2) {
	    from = &MainBoard[move->y2][0];
	    to = &MainBoard[move->y2][3];
	    from->type = ROOK;
	    from->color = to->color;
	    to->type = NULLPC;
	    DrawSquare(0, move->y1, from);
	    DrawSquare(3, move->y2, to);
	}
	break;
    }
    /* peel off the tail */
    if (mlnp->prev != (MOVELISTNODE *) 0) {
	lastMove = mlnp->prev;
	lastMove->next = (MOVELISTNODE *) 0;
	nextLink = &(lastMove->next);
    } else {
	nextLink = &firstMove;
	firstMove = lastMove = (MOVELISTNODE *) 0;
    }
    free((char *) mlnp);
}

/*
 * flash the last play by either me or my opponent
 * (NOTE: this sleeps for 1-2 seconds while showing the previous position - 
 * this should not be extended much longer, since this may overlap with
 * an RPC timeout if the opponent sends us his next move in the interim.)
 */
void
ShowLastPlay()
{
    register Move * move;
    register MOVELISTNODE * mlnp;
#define	FROM		0
#define	TO		1
    Square pre[4], post[4];
    BoardCoordinate pos[4];
    int lastSquareIndex = 1;
    register int i;

    /* ignore bogus last play requests */
    if ((mlnp = lastMove) == (MOVELISTNODE *) 0) {
	return;
    }
    move = &(mlnp->move);
    pos[FROM].x = move->x1; pos[FROM].y = move->y1;
    pos[TO].x = move->x2; pos[TO].y = move->y2;
    post[FROM].type = NULLPC;
    post[TO].type = MainBoard[pos[TO].y][pos[TO].x].type;
    post[TO].color = MainBoard[pos[TO].y][pos[TO].x].color;
    pre[FROM].type = post[TO].type;
    pre[FROM].color = post[TO].color;
    pre[TO].type = NULLPC;
    /* if this move was a pawn reaching the eighth rank */
    if (move->newPieceType != (int) NULLPC) 
	pre[FROM].type = PAWN;
    /* if this was a capture */
    if (mlnp->tookPiece != NULLPC) {
	/* if this was an en passant capture */
	if (pos[TO].y != mlnp->tookY) {
	    pos[++lastSquareIndex].x = mlnp->tookX;
	    pos[lastSquareIndex].y = mlnp->tookY;
	    post[lastSquareIndex].type = NULLPC;
	}
	pre[lastSquareIndex].type = mlnp->tookPiece;
	pre[lastSquareIndex].color = (pre[FROM].color == BLACK ? WHITE : BLACK);
    } else if (post[TO].type == KING) {
	/* if this was a king-side castling move */
	if (pos[FROM].x == 4 && pos[TO].x == 6) {
	    ++lastSquareIndex;
	    pos[lastSquareIndex].x = 7;
	    pos[lastSquareIndex + 1].x = 5;
	    pos[lastSquareIndex].y = pos[lastSquareIndex + 1].y = pos[FROM].y;
	    pre[lastSquareIndex].type = post[lastSquareIndex + 1].type = ROOK;
	    pre[lastSquareIndex].color = post[lastSquareIndex + 1].color = pre[FROM].color;
	    post[lastSquareIndex].type = pre[lastSquareIndex + 1].type = NULLPC;
	    ++lastSquareIndex;
	/* else if this is a queen-side castling move */
	} else if (move->x1 == 4 && move->x2 == 2) {
	    ++lastSquareIndex;
	    pos[lastSquareIndex].x = 0;
	    pos[lastSquareIndex + 1].x = 3;
	    pos[lastSquareIndex].y = pos[lastSquareIndex + 1].y = pos[FROM].y;
	    pre[lastSquareIndex].type = post[lastSquareIndex + 1].type = ROOK;
	    pre[lastSquareIndex].color = post[lastSquareIndex + 1].color = pre[FROM].color;
	    post[lastSquareIndex].type = pre[lastSquareIndex + 1].type = NULLPC;
	    ++lastSquareIndex;
	}
    }
    /* re-draw the pre-move configuration */
    for ( i = 0 ; i <= lastSquareIndex ; i++ ) 
	DrawSquare(pos[i].x, pos[i].y, &pre[i]);
    sleep((unsigned) 2);
    /* re-draw the post-move configuration */
    for ( i = 0 ; i <= lastSquareIndex ; i++ ) 
	DrawSquare(pos[i].x, pos[i].y, &post[i]);
}

/*
 * have I moved yet? 
 */
BOOL IHaveMoved()
{
    return (firstMove != (MOVELISTNODE *) 0
    && (MyColor == WHITE || firstMove != lastMove));
}

PieceType
pawnMorphs[] = { 
    QUEEN,			/* pawn -> queen */
    QUEEN,			/* knight -> queen */
    KNIGHT,			/* bishop -> knight */
    BISHOP,			/* rook -> bishop */
    ROOK			/* queen -> rook */
};

/*
 * select the next possible pawn promotion option 
 */
int
PromotePawn(blocp)
    BoardCoordinate * blocp;
{
    register Square * sp;

    sp = &MainBoard[blocp->y][blocp->x];
    lastMove->move.newPieceType = (int) (sp->type = pawnMorphs[(int) sp->type]);
    DrawSquare(blocp->x, blocp->y, sp);
    return((int) sp->type);
}

/*
 * is the 'color' king in check?
 */
BOOL 
inCheck(board, color)
    Square board[][8];
    int color;
{
    register int x, y;

    /* search for the king */
    for (x = 0 ; x < 8 ; x++) 
	for (y = 0 ; y < 8 ; y++) 
	    if (board[y][x].type == KING && board[y][x].color == color) 
		return (threatened(board, x, y, color));
    return(FALSE);
}

/*
 * am I in check?
 */
BOOL
InCheck() 
{
    return(inCheck(MainBoard, MyColor));
}

/* 
 * piece type characters
 */
char 
pieceChars[] = { 'P', 'N', 'B', 'R', 'Q', 'K' };

/*
 * normal board file descriptions
 */
char *
fileStrings[] = { "QR", "QN", "QB", "Q", "K", "KB", "KN", "KR" };

/*
 * write a string describing the board location 
 */
void
locString(cp, color, x, y)
    char * cp;
    int color, x, y;
{
    if (color == WHITE) 
	sprintf(cp, "%s%d", fileStrings[x], 8 - y);
    else 
	sprintf(cp, "%s%d", fileStrings[x], 1 + y);
}

/*
 * alias positions for alternative interpretations of move descriptions
 */
typedef struct {
    BoardCoordinate pos;	/* piece position */
    BOOL valid;			/* "currently under consideration" flag */
} Alias;

/*
 * mark aliases as valid which lie on a particular file
 * return the number qualified.
 * the file to use is always the first the first entry in the aliases array.
 */
void
markSpecificFile(aliases, aliasCount)
    Alias aliases[];
    int aliasCount;
{
    register int i, file;

    file = aliases[0].pos.x;
    for (i = 1 ; i < aliasCount ; i++) 
	aliases[i].valid = (aliases[i].pos.x == file);
}

/*
 * mark aliases as valid which lie on a generic file (b, n, or r)
 * or a specific file (q, k).
 * the file to use is always the first the first entry in the aliases array.
 */
void
markGenericFile(aliases, aliasCount)
    Alias aliases[];
    int aliasCount;
{
    register int i, file;

    file = aliases[0].pos.x;
    if (file == 3 || file == 4)
	markSpecificFile(aliases, aliasCount);
    else 
	for (i = 1 ; i < aliasCount ; i++)
	    aliases[i].valid = 
		(aliases[i].pos.x == file || aliases[i].pos.x == 7 - file);
}

/*
 * mark all aliases as valid
 */
void 
markAllAsValid(aliases, aliasCount)
    Alias aliases[];
    int aliasCount;
{
    register int i;

    for (i = 0 ; i < aliasCount ; i++) 
	aliases[i].valid = TRUE;
}

#define	NULLSIDE	0
#define	KINGSIDE	1
#define	QUEENSIDE	2

/*
 * determine if an alias list can be disambiguated by a kingside vs.
 * queenside distinction; if so, return KINGSIDE or QUEENSIDE and
 * limit the alias list to the first entry - otherwise, return
 * NULLSIDE.
 * Note: only bishops, knights, and rooks can be distinguished this
 * way - it would be too wierd to allow queens specified as "QQ".
 */
int
markBySide(aliases, aliasCount, pieceType, color)
    Alias aliases[];
    int aliasCount, color;
    PieceType pieceType;
{
    /* can only disambiguate two pieces this way */
    if (aliasCount > 2)
	return(NULLSIDE);
    if (pieceType == BISHOP 
	&& ((aliases[0].pos.x + aliases[0].pos.y) & 0x01) 
	!= ((aliases[1].pos.x + aliases[1].pos.y) & 0x01) 
    || (pieceType == ROOK || pieceType == KNIGHT)
	&& (aliases[0].pos.x != aliases[1].pos.x)) 
    {
	aliases[0].valid = TRUE;
	aliases[1].valid = FALSE;
	switch(pieceType) {
	case BISHOP:
	    if (((aliases[0].pos.x + aliases[0].pos.y) & 0x01) != 0 && color == BLACK
	    || ((aliases[0].pos.x + aliases[0].pos.y) & 0x01) == 0 && color == WHITE)
		return(KINGSIDE);
	    else
		return(QUEENSIDE);
	default:
	    return(aliases[0].pos.x < aliases[1].pos.x ? QUEENSIDE : KINGSIDE);
	}
    }
    return(NULLSIDE);
}

/*
 * return the number of possible interpretations
 * of the srcs[] array combined with the dests[] array.
 * entries that are under consideration have their valid flag set.
 */
int
findPossibles(board, color, srcs, srcCount, dests, destCount)
    Square board[][8];
    Alias srcs[], dests[];
    int color, srcCount, destCount;
{
    register int i, j;
    int possibles = 0;

    for (i = 0 ; i < srcCount ; i++) 
	for (j = 0 ; j < destCount ; j++) 
	    if (srcs[i].valid && dests[j].valid 
	    && isMoveLegal(board, color, &srcs[i].pos, &dests[j].pos))
		possibles++;
    return(possibles);
}

/*
 * print the minimal normal notation for a capture (except en passant).
 * return the number of characters printed.
 *
 * note that we can find up to a maximum of ten pieces of the same type
 * in a "real" game where all eight pawns have been promoted to the
 * same piece type.
 */
int
printMinCapture(tfile, board, color, move, loc1, loc2, from, to)
    FILE * tfile;
    Square board[][8];
    register Square * from, * to;
    Move * move;
    char * loc1, * loc2;
    int color;
{
    register int srcCount = 1, destCount = 1, i, j;
    Alias srcs[10], dests[10];
    int possibles, moveCount;

    /* disambiguation state flags */
    BOOL 
    daSrcSingle = FALSE,	/* fully spec. src.; e.g., r(kr4)xp */
    daSrcBrdSide = NULLSIDE,  	/* side of the board/bishop color; e.g. krxp */
    daSrcSpecificFile = FALSE,	/* specific file; e.g. krpxr */
    daSrcGenericFile = FALSE,	/* generic file; e.g. rpxr */
    daDestSingle = FALSE,	/* fully spec. dest.; e.g., rxp(kr4) */
    daDestBrdSide = NULLSIDE,  	/* side of the board/bishop color; e.g. krxp */
    daDestSpecificFile = FALSE,  	/* specific file; e.g. rxkrp */
    daDestGenericFile = FALSE;	/* generic file; e.g. qxrp */

    /*
     * install the actual move as the first entries 
     */
    srcs[0].pos.x = move->x1; srcs[0].pos.y = move->y1; srcs[0].valid = TRUE;
    dests[0].pos.x = move->x2; dests[0].pos.y = move->y2; dests[0].valid = TRUE;
    /*
     * find the other pieces of the same color and type 
     * as the piece being moved,
     * along with the other pieces of the same type and color as the
     * piece being captured.
     */
    for (i = 0 ; i < 8 ; i++) {
	for (j = 0 ; j < 8 ; j++) {
	    if (board[j][i].type == from->type
	    && board[j][i].color == from->color
	    && ! (i == move->x1 && j == move->y1)) {
		srcs[srcCount].valid = FALSE;
		srcs[srcCount].pos.x = i;
		srcs[srcCount++].pos.y = j;
	    }
	    if (board[j][i].type == to->type
	    && board[j][i].color == to->color
	    && ! (i == move->x2 && j == move->y2)) {
		dests[destCount].valid = FALSE;
		dests[destCount].pos.x = i;
		dests[destCount++].pos.y = j;
	    }
	}
    }
    /*
     * reduce this to only the pieces that figure in ambiguous
     * captures.
     */
    possibles = 0;
    for (i = 0 ; i < srcCount ; i++) {
	for (j = 0 ; j < destCount ; j++) {
	    if (isMoveLegal(board, color, &srcs[i].pos, &dests[j].pos)) {
		possibles++;
		srcs[i].valid = dests[j].valid = TRUE;
	    }
	}
    }
    /* condense the piece lists */
    for (i = j = 0 ; i < srcCount ; i++) {
	if (srcs[i].valid) {
	    if (i > j) {
		srcs[j].pos.x = srcs[i].pos.x;
		srcs[j].pos.y = srcs[i].pos.y;
		srcs[j].valid = srcs[i].valid;
	    }
	    j++;
	}
    }
    srcCount = j;
    for (i = j = 0 ; i < destCount ; i++) {
	if (dests[i].valid) {
	    if (i > j) {
		dests[j].pos.x = dests[i].pos.x;
		dests[j].pos.y = dests[i].pos.y;
		dests[j].valid = dests[i].valid;
	    }
	    j++;
	}
    }
    destCount = j;
    /* if there are ambiguous source pieces */
    if (possibles > 1 && srcCount > 1) {
	/*
	 * if only the source is ambiguous 
	 * or if the source is a pawn
	 * or if the destination is not an ambiguous pawn
	 * try to disambiguate the source alone
	 */
	if (destCount == 1 || from->type == PAWN || to->type != PAWN) {
	    /* if the source is a pawn */
	    if (from->type == PAWN) {
		/* try specifying the generic file source pawn; e.g., RPxP. */
		markGenericFile(srcs, srcCount);
		daSrcGenericFile = TRUE;
		possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
		/* try specifying the specific file source pawn; e.g., KRPxP.
		 * (note - if there is only one destination, 
		 * this will always succeed) */
		if (possibles > 1) {
		    markSpecificFile(srcs, srcCount);
		    daSrcSpecificFile = TRUE;
		    possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
		}
		/* if the above didn't work, back out the restrictions */
		if (possibles > 1) {
		    daSrcSpecificFile = daSrcGenericFile = FALSE;
		    markAllAsValid(srcs, srcCount);
		}
	    } 
	    /* else the source is a piece */
	    else {
		/* try using kingside or queenside prefixes.  */
		if ((daSrcBrdSide = markBySide(srcs, srcCount, from->type, color)) != NULLSIDE) {
		    possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
		}
		/* if there is only one destination,
		 * punt and fully specify the source square.  */
		if (possibles > 1 && destCount == 1) {
		    daSrcSingle = TRUE;
		    srcCount = 1;
		    if ((possibles = findPossibles(board, color, srcs, srcCount, dests, destCount)) != 1)
		    {
			fprintf(stderr, "transcript error: assertion #1 failed\n");
			possibles = 1;
		    }
		}
		/* if the above didn't work, back out the restrictions */
		if (possibles > 1) {
		    daSrcBrdSide = FALSE;
		    markAllAsValid(srcs, srcCount);
		}
	    }
	}
	/* if unsolved at this point, 
	 * there must be ambiguous destination pieces;
	 * otherwise, the previous clause would have solved it.  */
	if (possibles > 1 && destCount == 1) {
	    fprintf(stderr, "transcript error: assertion #2 failed\n");
	    fprintf(tfile, "%cx%c", 
		pieceChars[(int) from->type],
		pieceChars[(int) to->type]);
	    return(3);
	}
	/*
	 * if the destination is a pawn 
	 */
	if (possibles > 1 && to->type == PAWN) {
	    /* try specifying the generic file destination pawn; e.g., PxRP. */
	    markGenericFile(dests, destCount);
	    daDestGenericFile = TRUE;
	    possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
	    /* try specifying the specific file destination pawn; e.g., PxKRP */
	    if (possibles > 1) {
		markSpecificFile(dests, destCount);
		daDestSpecificFile = TRUE;
		possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
	    }
	    /* if the source is a pawn */
	    if (possibles > 1 && from->type == PAWN) {
		/* back the destination spec. down to a generic file
		 * and specify the source generic file.  */
		markAllAsValid(dests, destCount);
		daDestGenericFile = daDestSpecificFile = FALSE;
		markGenericFile(dests, destCount);
		daDestGenericFile = TRUE;
		markGenericFile(srcs, srcCount); 
		daSrcGenericFile = TRUE;
		possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
		/*
		 * specify the specific source file.
		 * for example, consider the 
		 * case of two bishop pawns and two rook pawns 
		 * attacking two knight pawns and the queen and king
		 * pawns - - we will eventually wind up here
		 * with something like: "KBPxNP" (since "KBPxP" and
		 * "PxKNP" are both ambiguous.
		 * we may STILL have an ambiguity if there is more than 
		 * one pawn on the file.
		 */
		if (possibles > 1) {
		    markSpecificFile(srcs, srcCount);
		    daSrcSpecificFile = TRUE;
		    possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
		}
		/* fully specify the source square and back out all 
		 * destination specs.  */
		if (possibles > 1) {
		    markAllAsValid(dests, destCount);
		    daDestGenericFile = daDestSpecificFile = FALSE;
		    daSrcSingle = TRUE;
		    srcCount = 1;
		    possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
		}
		/* fully specify the source square and the generic
		 * destination file (will always be enough) */
		if (possibles > 1) {
		    markGenericFile(dests, destCount);
		    daDestGenericFile = TRUE;
		    possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
		}
		if (possibles > 1)
		    fprintf(stderr, "transcript error: assertion #3 failed\n");
		
	    }
	    /* else the source is a piece */
	    else if (possibles > 1) {
		/* back out the pawn file specs.
		 * try using kingside or queenside prefixes.  */
		markAllAsValid(dests, destCount);
		daDestGenericFile = daDestSpecificFile = FALSE;
		if ((daSrcBrdSide = markBySide(srcs, srcCount, from->type, color)) != NULLSIDE) 
		    possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
		/* try fully specifying the source square */
		if (possibles > 1) {
		    daSrcSingle = TRUE;
		    srcCount = 1;
		    possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
		}
		/* specify only the generic dest. pawn file (will always be 
		 * enough; for example, consider two bishops of 
		 * the same color attacking two pawns) */
		if (possibles > 1) {
		    markGenericFile(dests, destCount);
		    daDestGenericFile = TRUE;
		    possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
		}
		if (possibles > 1)
		    fprintf(stderr, "transcript error: assertion #4 failed\n");
	    }
	}
	/* else the destination is a piece */
	else if (possibles > 1) {
	    /* if the source is a pawn, we know from the above that 
	    * specifying the source pawn file alone doesn't work.  */
	    if (from->type == PAWN) {
		/* try using kingside or queenside prefixes.  */
		if ((daDestBrdSide = markBySide(dests, destCount, to->type, color == WHITE ? BLACK : WHITE)) != NULLSIDE) {
		    possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
		    /* try specifying the generic file src. pawn; e.g., RPxKB. */
		    if (possibles > 1) {
			markGenericFile(srcs, srcCount);
			daSrcGenericFile = TRUE;
			possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
		    }
		    /* try specifying the specific file source pawn; e.g., KRPxKB */
		    if (possibles > 1) {
			markSpecificFile(srcs, srcCount);
			daSrcSpecificFile = TRUE;
			possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
		    }
		}
		if (possibles > 1) {
		    /* back out any pawn limitations.
		     * try fully specifying the destination square.  */
		    daSrcSpecificFile = daSrcGenericFile = FALSE;
		    daDestSingle = TRUE;
		    destCount = 1;
		    possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
		}
		if (possibles > 1) {
		    /* specify the generic source file pawn.
		     * will always work, since at most one pawn can capture
		     * a specific square along a particular file.  */
		    daSrcGenericFile = TRUE;
		    markGenericFile(srcs, srcCount);
		    possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
		}
		if (possibles > 1)
		    fprintf(stderr, "transcript error: assertion #5 failed\n");
	    } 
	    /* else both source and destination are pieces.  we know from 
	     * above that there are multiple destinations and that specifying 
	     * source kingside/queenside alone won't work.  */
	    else {
		/* try using dest. kingside or queenside prefixes.  */
		if ((daDestBrdSide = markBySide(dests, destCount, to->type, color == WHITE ? BLACK : WHITE)) != NULLSIDE) 
		    possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
		/* try using src. kingside or queenside prefixes.  */
		if (possibles > 1 
		&& (daDestBrdSide = markBySide(srcs, srcCount, from->type, color)) != NULLSIDE) 
		    possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
		if (possibles > 1) {
		    /* back out dest. k/q-side and fully specify the source. */
		    daDestBrdSide = NULLSIDE;
		    markAllAsValid(dests, destCount);
		    daSrcSingle = TRUE;
		    srcCount = 1;
		    possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
		}
		if (possibles > 1) {
		    /* punt: fully specify the destination.  */
		    daDestSingle = TRUE;
		    /* no need to check the assertion that this is it,
		     * as it is really obvious. */
		}
	    }
	}
    } 
    /*
     * else only the destination is ambiguous
     */
    else if (possibles > 1) {
	if (to->type == PAWN) {
	    /* try generic file spec. */
	    markGenericFile(dests, destCount);
	    daDestGenericFile = TRUE;
	    possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
	    /* try specific file spec. */
	    if (possibles > 1) {
		markSpecificFile(dests, destCount);
		daDestSpecificFile = TRUE;
		possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
	    }
	    /* try fully specifying the destination square */
	    if (possibles > 1) {
		daDestSingle = TRUE;
		destCount = 1;
		possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
	    }
	    if (possibles > 1)
		fprintf(stderr, "transcript error: assertion #6 failed\n");
	} else {
	    /* try kingside/queenside */
	    if ((daDestBrdSide = markBySide(dests, destCount, to->type, color == WHITE ? BLACK : WHITE)) != NULLSIDE) 
		possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
	    /* fully specify the destination square */
	    if (possibles > 1) {
		daDestSingle = TRUE;
		destCount = 1;
		possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
	    }
	    if (possibles > 1)
		fprintf(stderr, "transcript error: assertion #7 failed\n");
	}
    }
    /* now print the capture */
    /* if we must fully specify the source */
    if (daSrcSingle) {
	fprintf(tfile, "%c(%s)", pieceChars[(int) from->type], loc1);
	moveCount = 3 + strlen(loc1);
    } else {
	moveCount = 0;
	loc1 = fileStrings[move->x1];
	/* if we must specify queenside source */
	if (daSrcBrdSide == QUEENSIDE) {
	    fputc('Q', tfile);
	    moveCount = 1;
	/* else if we must specify kingside source */
	} else if (daSrcBrdSide == KINGSIDE) {
	    fputc('K', tfile);
	    moveCount = 1;
	/* else if we must specify the specific file */
	} else if (daSrcSpecificFile) {
	    fprintf(tfile, "%s", loc1);
	    moveCount = strlen(loc1);
	/* else if we must specify the generic file */
	} else if (daSrcGenericFile) {
	    fprintf(tfile, "%s", strlen(loc1) == 2 ? loc1 + 1 : loc1);
	    moveCount = 1;
	}
	fputc(pieceChars[(int) from->type], tfile);
	moveCount++;
    }
    fputc('x', tfile);
    moveCount++;
    /* if we must fully specify the destination */
    if (daDestSingle) {
	fprintf(tfile, "%c(%s)", pieceChars[(int) to->type], loc2);
	moveCount += 3 + strlen(loc2);
    } else {
	loc2 = fileStrings[move->x2];
	/* if we must specify queenside dest. */
	if (daDestBrdSide == QUEENSIDE) {
	    fputc('Q', tfile);
	    moveCount++ ;
	/* else if we must specify kingside dest. */
	} else if (daDestBrdSide == KINGSIDE) {
	    fputc('K', tfile);
	    moveCount++ ;
	/* else if we must specify the specific file */
	} else if (daDestSpecificFile) {
	    fprintf(tfile, "%s", loc2);
	    moveCount += strlen(loc2);
	/* else if we must specify the generic file */
	} else if (daDestGenericFile) {
	    fprintf(tfile, "%s", strlen(loc2) == 2 ? loc2 + 1 : loc2);
	    moveCount++;
	}
	fputc(pieceChars[(int) to->type], tfile);
	moveCount++;
    }
    return(moveCount);
}

/*
 * print the minimal normal notation for a non-special move.
 * return the number of characters printed.
 *
 * note that we can find up to a maximum of ten pieces of the same type
 * in a real game where all eight pawns have been promoted to the
 * same piece type.
 */
int
printMinMove(tfile, board, color, move, loc1, loc2, from)
    FILE * tfile;
    Square board[][8];
    register Square * from;
    Move * move;
    char * loc1, * loc2;
    int color;
{
    register int i, j;
    Alias srcs[10], dests[2];
    int srcCount = 1, destCount = 1, possibles, moveCount;
    /* disambiguation state flags */
    BOOL 
    daSingleDest = FALSE,	/* fully spec. dest.; e.g., r - kr4 */
    daBoardSide = NULLSIDE,  	/* specific side of the board/bishop color; 
				 * e.g. kr-r4 */
    daSingleSrc = FALSE;	/* fully spec. src.; e.g. r(kr4) - r4 */

    /*
     * install the actual move as the first entries 
     */
    srcs[0].pos.x = move->x1; srcs[0].pos.y = move->y1; srcs[0].valid = TRUE;
    dests[0].pos.x = move->x2; dests[0].pos.y = move->y2; dests[0].valid = TRUE;
    /*
     * find the other pieces of the same color and type 
     * as the piece being moved.
     */
    for (i = 0 ; i < 8 ; i++) {
	for (j = 0 ; j < 8 ; j++) {
	    if (board[j][i].type == from->type
	    && board[j][i].color == from->color
	    && ! (i == move->x1 && j == move->y1)) {
		srcs[srcCount].valid = FALSE;
		srcs[srcCount].pos.x = i;
		srcs[srcCount++].pos.y = j;
	    }
	}
    }
    /*
     * if the destination is a rook, knight, or bishop file
     * and there is no piece on the mirrored square,
     * double the possible destinations (e.g., "n3" means "kn3" or "qn3")
     */
    if (move->x2 < 3 || move->x2 > 4
    && board[move->y2][7-move->x2].type == NULLPC) {
	dests[1].pos.x = 7 - move->x2;
	dests[1].pos.y = move->y2;
	destCount = 2;
    }
    /*
     * reduce this to only the srcs and dests that figure in ambiguous
     * moves.
     */
    possibles = 0;
    for (i = 0 ; i < srcCount ; i++) {
	for (j = 0 ; j < destCount ; j++) {
	    if (isMoveLegal(board, color, &srcs[i].pos, &dests[j].pos)) {
		possibles++;
		srcs[i].valid = dests[j].valid = TRUE;
	    }
	}
    }
    /* condense the src. piece list */
    for (i = j = 0 ; i < srcCount ; i++) {
	if (srcs[i].valid) {
	    if (i > j) {
		srcs[j].pos.x = srcs[i].pos.x;
		srcs[j].pos.y = srcs[i].pos.y;
		srcs[j].valid = srcs[i].valid;
	    }
	    j++;
	}
    }
    srcCount = j;
    /* condense the destination list */
    for (i = j = 0 ; i < destCount ; i++) {
	if (dests[i].valid) {
	    if (i > j) {
		dests[j].pos.x = dests[i].pos.x;
		dests[j].pos.y = dests[i].pos.y;
		dests[j].valid = dests[i].valid;
	    }
	    j++;
	}
    }
    destCount = j;
    if (possibles > 1) {
	/* try reducing to only one destination */
	if (destCount == 2) {
	    daSingleDest = TRUE;
	    dests[1].valid = FALSE;
	    possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
	    if (possibles > 1) {
		dests[1].valid = TRUE;
		daSingleDest = FALSE;
	    }
	}
	/* try specifying kingside/queenside */
	if (possibles > 1 
	&& (daBoardSide = markBySide(srcs, srcCount, from->type, color)) 
	    != NULLSIDE) 
	    possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
	/* try restricting to only one destination */
	if (possibles > 1 && destCount == 2) {
	    daSingleDest = TRUE;
	    dests[1].valid = FALSE;
	    possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
	    if (possibles > 1) {
		dests[1].valid = TRUE;
		daSingleDest = FALSE;
	    }
	}
	/* try fully specifying the source */
	if (possibles > 1) {
	    daSingleSrc = TRUE;
	    srcCount = 1;
	    if (destCount > 1) {
		possibles = findPossibles(board, color, srcs, srcCount, dests, destCount);
		if (possibles > 1) {
		    dests[1].valid = FALSE;
		    daSingleDest = TRUE;
		}
	    }
	}
    } 
    /* now print the move */
    /* if we must fully specify the source */
    if (daSingleSrc) {
	fprintf(tfile, "%c(%s)", pieceChars[(int) from->type], loc1);
	moveCount = 3 + strlen(loc1);
    } else {
	moveCount = 0;
	/* if we must specify queenside source */
	if (daBoardSide == QUEENSIDE) {
	    fputc('Q', tfile);
	    moveCount = 1;
	/* else if we must specify kingside source */
	} else if (daBoardSide == KINGSIDE) {
	    fputc('K', tfile);
	    moveCount = 1;
	} 
	fputc(pieceChars[(int) from->type], tfile);
	moveCount++;
    }
    fputc('-', tfile);
    moveCount++;
    /* if we must fully specify the destination */
    if (daSingleDest) {
	fprintf(tfile, "%s", loc2);
	moveCount += strlen(loc2);
    /* else print only the generic rank and file */
    } else {
	fprintf(tfile, "%s", strlen(loc2) == 2 ? loc2 : loc2 + 1);
	moveCount += 2;
    }
    return(moveCount);
}

/*
 * write a transcript file
 */
void 
WriteTranscript(filename, trType)
    char * filename;
    int trType;
{
    register Square * from, * to;
    register Move * move;
    FILE * tfile;
    MOVELISTNODE * mlnp;
    int moveSize, color = WHITE, moveNum = 1;
    Square board[8][8]; 
    char loc1[5], loc2[5];

    if ((tfile = fopen(filename, "w")) == (FILE *) 0) {
	Message("Can't open transcript file!");
	return;
    } else {
	Message("Writing transcript...");
    }
    /* create a new initial board layout */
    initBoard(board);
    fprintf(tfile, "\n\t%s", PlayerName[WHITE]);
    moveSize = strlen(PlayerName[WHITE]);
    if (moveSize < 2 * TABSIZE) {
	fputc('\t', tfile);
	if (moveSize < TABSIZE) 
	    fputc('\t', tfile);
    }
    fprintf(tfile, "%s\n\n", PlayerName[BLACK]);
    for (color = WHITE , mlnp = firstMove , moveSize = 0 ; 
    mlnp != (MOVELISTNODE *) 0 ; 
    mlnp = mlnp->next, color = (color == BLACK ? WHITE : BLACK)) 
    {
	move = &mlnp->move;
	/* if this is a resignation */
	if (move->y2 < 0) {
	    fputs(move->x2 == WHITE ? 
		"\n\tresigns\n" : 
		"\n\t\t\tresigns\n", tfile);
	    break;
	}
	if (color == WHITE) {
	    fprintf(tfile, "%d.\t", moveNum++);
	} else {
	    fputc('\t', tfile);
	    if (moveSize < TABSIZE) 
		fputc('\t', tfile);
	}
	/* if we are using algebraic notation */
	if (trType == TR_ALGEBRAIC) {
	    moveSize = 4;
	    fprintf(tfile, "%c%d%c%d",
		'a' + move->x1, 8 - move->y1,
		'a' + move->x2, 8 - move->y2);
	/* else normal notation */
	} else {
	    from = &board[move->y1][move->x1];
	    to = &board[move->y2][move->x2];
	    locString(loc1, color, move->x1, move->y1);
	    locString(loc2, color, move->x2, move->y2);
	    /* if this was an en passant capture */
	    if (from->type == PAWN && to->type == NULLPC && move->x1 != move->x2) {
		switch (trType) {
		case TR_FORMAL_NORMAL:
		    fprintf(tfile, "p/%sxp/%s", loc1, loc2);
		    moveSize = 5 + strlen(loc1) + strlen(loc2);
		    break;
		case TR_MIN_NORMAL:
		    fprintf(tfile, "PxP(ep)");
		    moveSize = 7;
		    break;
		}
		board[move->y1][move->x2].type = NULLPC;
	    /* else if this was a normal capture */
	    } else if (to->type != NULLPC) {
		switch (trType) {
		case TR_FORMAL_NORMAL:
		    fprintf(tfile, "%c/%sx%c/%s", 
			pieceChars[(int) from->type], loc1, 
			pieceChars[(int) to->type], loc2);
		    moveSize = 5 + strlen(loc1) + strlen(loc2);
		    break;
		case TR_MIN_NORMAL:
		    moveSize = printMinCapture(tfile, board, color, move, loc1, loc2, from, to);
		    break;
		}
	    /* else if this was a king move */
	    } else if (from->type == KING) {
		/* 
		 * if this was a king-side castling move
		 */
		if (move->x1 == 4 && move->x2 == 6) {
		    /* move the rook */
		    board[move->y2][5].type = ROOK;
		    board[move->y2][5].color = board[move->y2][7].color;
		    board[move->y2][7].type = NULLPC;
		    fprintf(tfile, "O-O");
		    moveSize = 3;
		/* 
		 * else if this was a queen-side castling move
		 */
		} else if (move->x1 == 4 && move->x2 == 2) {
		    /* move the rook */
		    board[move->y2][3].type = ROOK;
		    board[move->y2][3].color = board[move->y2][0].color;
		    board[move->y2][0].type = NULLPC;
		    fprintf(tfile, "O-O-O");
		    moveSize = 5;
		/* else this was a normal king move */
		} else {
		    switch (trType) {
		    case TR_FORMAL_NORMAL:
			fprintf(tfile, "k/%s-%s", loc1, loc2);
			moveSize = 3 + strlen(loc1) + strlen(loc2);
			break;
		    case TR_MIN_NORMAL:
			fprintf(tfile, "K-%s", 
			    strlen(loc2) == 2 ? loc2 : loc2 + 1);
			moveSize = 4;
			break;
		    }
		}
	    /* else this was a normal piece move */
	    } else {
		switch (trType) {
		case TR_FORMAL_NORMAL:
		    fprintf(tfile, "%c/%s-%s", 
			pieceChars[(int) from->type], loc1, loc2);
		    moveSize = 3 + strlen(loc1) + strlen(loc2);
		    break;
		case TR_MIN_NORMAL:
		    moveSize = printMinMove(tfile, board, color, move, loc1, loc2, from);
		    break;
		}
	    }
	}
	/* move the piece from the origin square to the dest. square */
	to->type = from->type;
	to->color = from->color;
	/* put a null piece in the origin square */
	from->type = NULLPC;
	/* 
	 * if this was a pawn reaching the 8th rank
	 */
	if (to->type == PAWN 
	&& (move->y2 == 0 || move->y2 == 7)) {
	    to->type = (PieceType) move->newPieceType;
	    fprintf(tfile, "(%c)", pieceChars[move->newPieceType]);
	    moveSize += 3;
	}
	/* if this put the opposite side in check */
	if (inCheck(board, color == WHITE ? BLACK : WHITE)) {
	    switch(trType) {
	    case TR_FORMAL_NORMAL:
		fputc('+', tfile);
		moveSize++;
		break;
	    case TR_MIN_NORMAL:
		fputs("ch", tfile);
		moveSize += 2;
		break;
	    }
	}
	if (color == BLACK)
	    fputc('\n', tfile);
    }
    WhoseMoveMessage("Transcript written");
    fclose(tfile);
}

/*
 * save game file format:
 *
 * int MyColor			color of player saving game
 * int InitialTurn		whose turn it was initially
 * int IsMachine[2]		whether each color is a machine
 * Square board[8][8]		initial board state
 * if (IsMachine[0])
 *	int chessSaveFileSize	# of bytes for embedded chess.out
 * 	char[...]		chess.out
 * endif
 * if (IsMachine[1])
 *	int chessSaveFileSize	# of bytes for embedded chess.out
 * 	char[...]		chess.out
 * endif
 * Move[...]			list of moves
 */

/*
 * save the game 
 */
void 
SaveGame(filename)
    char * filename;
{
    FILE * gameFile, * chessOutFile;
    register MOVELISTNODE * mlnp;
    register int i, j;
    int chessOutSize;

    if ((gameFile = fopen(filename, "w")) == (FILE *) 0) {
	Message("Can't open save file!");
	return;
    } else {
	Message("Saving game...");
    }
    if (fwrite((caddr_t) &MyColor, sizeof(MyColor), 1, gameFile) != 1
    || fwrite((caddr_t) &InitialTurn, sizeof(InitialTurn), 1, gameFile) != 1
    || fwrite((caddr_t) IsMachine, sizeof(IsMachine), 1, gameFile) != 1
    || fwrite((caddr_t) InitialBoard, sizeof(InitialBoard), 1, gameFile) != 1)
    {
	Message("Write error - save aborted");
	return;
    }
    for (i = 0 ; i < 2 ; i++) {
	if (IsMachine[i]) {
	    chessOutSize = MachineSave(i);
	    if ((chessOutFile = fopen("/tmp/chess.out", "r")) == (FILE *) 0) {
		Message("Can't open chess.out!");
		return;
	    }
	    if (fwrite((caddr_t) &chessOutSize, sizeof(chessOutSize), 1, gameFile) != 1) {
		Message("Write error - save aborted");
		return;
	    }
	    while ((j = fgetc(chessOutFile)) != EOF) {
		fputc(j, gameFile);
		chessOutSize--;
	    }
	    fclose(chessOutFile);
	    unlink("/tmp/chess.out");
	    if (chessOutSize != 0) {
		Message("Write error - save aborted");
		return;
	    }
	}
    }
    /*
     * save everything except trailing resignations
     */
    for (mlnp = firstMove ; 
    mlnp != (MOVELISTNODE *) 0 && mlnp->move.x2 >= 0 ; 
    mlnp = mlnp->next) {
	if (fwrite((caddr_t) &mlnp->move, sizeof(mlnp->move), 1, gameFile) != 1)
	{
	    Message("Write error - save aborted");
	    return;
	}
    }
    fclose(gameFile);
    WhoseMoveMessage("Game saved");
}

/*
 * note: my color has been previously read
 */
void
RestoreGame()
{
    Move move;
    SetupChange setup;
    register int i, j;
    BOOL wasMachine[2];
    FILE * chessOutFile;
    int chessOutSize;

    if (fread((caddr_t) &InitialTurn, sizeof(InitialTurn), 1, RestoreFile) != 1
    || fread((caddr_t) wasMachine, sizeof(wasMachine), 1, RestoreFile) != 1) {
	Message("Read error - restore aborted");
	goto abortRestore;
    }
    if (wasMachine[0] != IsMachine[0] || wasMachine[1] != IsMachine[1]) {
	Message("Restore file type mismatch - restore aborted");
	goto abortRestore;
    }
    Turn = InitialTurn;
    for (i = 0 ; i < 8 ; i++) {
	for (j = 0 ; j < 8 ; j++) {
	    if (fread((caddr_t) &InitialBoard[i][j].type, 
		sizeof(InitialBoard[i][j].type), 
		1, RestoreFile) != 1
	    || fread((caddr_t) &InitialBoard[i][j].color, 
		sizeof(InitialBoard[i][j].color), 
		1, RestoreFile) != 1)
	    {
		Message("Read error - restore truncated");
		goto abortRestore;
	    } else {
		setup.type = MainBoard[i][j].type = InitialBoard[i][j].type;
		setup.color = MainBoard[i][j].color = InitialBoard[i][j].color;
		setup.x = j;
		setup.y = i;
		SendSetupChange(&setup, PeerColor);
	    }
	}
    }
    for (i = 0 ; i < 2 ; i++) {
	if (wasMachine[i]) {
	    if ((chessOutFile = fopen("/tmp/chess.out", "w")) == (FILE *) 0) {
		Message("Can't create /tmp/chess.out - restore aborted");
		goto abortRestore;
	    }
	    if (fread((caddr_t) &chessOutSize, sizeof(chessOutSize), 1, RestoreFile) != 1) {
		Message("Read error - restore aborted");
		fclose(chessOutFile);
		goto abortRestore;
	    }
	    while (chessOutSize--) {
		if ((j = fgetc(RestoreFile)) == EOF) {
		    Message("Read error - restore aborted");
		    fclose(chessOutFile);
		    goto abortRestore;
		} 
		fputc(j, chessOutFile);
	    }
	    fclose(chessOutFile);
	    MachineRestore(i);
	    sleep(4);
	    unlink("/tmp/chess.out");
	}
    }
    while (fread((caddr_t) &move, sizeof(move), 1, RestoreFile)) {
	DoMove(&move, FALSE);
	SendRestoreMove(&move, PeerColor);
	Turn = OTHERCOLOR(Turn);
    }
    /*
     * if this was a machine vs. machine game, resubmit the last move
     * to the program whose turn it was.
     */
    if (wasMachine[BLACK] && wasMachine[WHITE]) {
	SendMachineMove(&lastMove->move, Turn);
    /*
     * else if the peer is not a machine, turn him loose
     */
    } else if ( ! IsMachine[PeerColor]) {
	SendEndRestore();
    }
abortRestore:
    fclose(RestoreFile);
    WhoseMoveMessage((char *) 0);
}

