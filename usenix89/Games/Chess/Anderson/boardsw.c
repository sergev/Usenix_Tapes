/*
 * handle the board subwindow (as well as fielding RPC and chess game
 * process selects)
 */

#include <stdio.h>
#include <suntool/tool_hs.h>
#include <suntool/panel.h>
#include <suntool/gfxsw.h>
#include <suntool/icon_load.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <strings.h>

#include "nchess.h"

#define	MOVE_PR_WIDTH	(SQUARE_WIDTH * 2)
#define	MOVE_PR_HEIGHT	(SQUARE_HEIGHT * 2)
#define	MOVE_X_OFFSET	(MOVE_PR_WIDTH/2 - SQUARE_WIDTH/2)
#define	MOVE_Y_OFFSET	(MOVE_PR_HEIGHT/2 - SQUARE_HEIGHT/2)

extern int svc_fds;			/* RPC service file descriptor(s) */
int BoardSWMask;			/* board gfx sw file des. */
BOOL Flashing = FALSE;			/* tool icon is flashing */

enum {					/* confirmation state using mouse */
    CONFIRM_WANTED,
    CONFIRM_SETUP_END,
    CONFIRM_WHOSE_TURN,
    CONFIRM_UNDO,
    CONFIRM_RESIGNATION,
} confirmState;

MouseState Mouse = IDLE;

char * PieceIconFileNames[] = {
    "pawn.icon",
    "knight.icon",
    "bishop.icon",
    "rook.icon",
    "queen.icon",
    "king.icon",
};

char * PieceStencilFileNames[] = {
    "pawnStencil.icon",
    "knightStencil.icon",
    "bishopStencil.icon",
    "rookStencil.icon",
    "queenStencil.icon",
    "kingStencil.icon",
};

/*
 * piece icons and stencils
 */
unsigned short RookBlackImage[] = {
#include "Icons/rook.icon"
};
unsigned short RookWhiteImage[] = {
#include "Icons/rook.icon"
};
unsigned short RookStencilImage[] = {
#include "Icons/rookStencil.icon"
};
mpr_static(RookBlackPR, SQUARE_WIDTH, SQUARE_HEIGHT, 1, RookBlackImage);
mpr_static(RookWhitePR, SQUARE_WIDTH, SQUARE_HEIGHT, 1, RookWhiteImage);
mpr_static(RookStencilPR, SQUARE_WIDTH, SQUARE_HEIGHT, 1, RookStencilImage);

unsigned short KnightBlackImage[] = {
#include "Icons/knight.icon"
};
unsigned short KnightWhiteImage[] = {
#include "Icons/knight.icon"
};
unsigned short KnightStencilImage[] = {
#include "Icons/knightStencil.icon"
};
mpr_static(KnightBlackPR, SQUARE_WIDTH, SQUARE_HEIGHT, 1, KnightBlackImage);
mpr_static(KnightWhitePR, SQUARE_WIDTH, SQUARE_HEIGHT, 1, KnightWhiteImage);
mpr_static(KnightStencilPR, SQUARE_WIDTH, SQUARE_HEIGHT, 1, KnightStencilImage);

unsigned short BishopBlackImage[] = {
#include "Icons/bishop.icon"
};
unsigned short BishopWhiteImage[] = {
#include "Icons/bishop.icon"
};
unsigned short BishopStencilImage[] = {
#include "Icons/bishopStencil.icon"
};
mpr_static(BishopBlackPR, SQUARE_WIDTH, SQUARE_HEIGHT, 1, BishopBlackImage);
mpr_static(BishopWhitePR, SQUARE_WIDTH, SQUARE_HEIGHT, 1, BishopWhiteImage);
mpr_static(BishopStencilPR, SQUARE_WIDTH, SQUARE_HEIGHT, 1, BishopStencilImage);

unsigned short KingBlackImage[] = {
#include "Icons/king.icon"
};
unsigned short KingWhiteImage[] = {
#include "Icons/king.icon"
};
unsigned short KingStencilImage[] = {
#include "Icons/kingStencil.icon"
};
mpr_static(KingBlackPR, SQUARE_WIDTH, SQUARE_HEIGHT, 1, KingBlackImage);
mpr_static(KingWhitePR, SQUARE_WIDTH, SQUARE_HEIGHT, 1, KingWhiteImage);
mpr_static(KingStencilPR, SQUARE_WIDTH, SQUARE_HEIGHT, 1, KingStencilImage);

unsigned short QueenBlackImage[] = {
#include "Icons/queen.icon"
};
unsigned short QueenWhiteImage[] = {
#include "Icons/queen.icon"
};
unsigned short QueenStencilImage[] = {
#include "Icons/queenStencil.icon"
};
mpr_static(QueenBlackPR, SQUARE_WIDTH, SQUARE_HEIGHT, 1, QueenBlackImage);
mpr_static(QueenWhitePR, SQUARE_WIDTH, SQUARE_HEIGHT, 1, QueenWhiteImage);
mpr_static(QueenStencilPR, SQUARE_WIDTH, SQUARE_HEIGHT, 1, QueenStencilImage);

unsigned short PawnBlackImage[] = {
#include "Icons/pawn.icon"
};
unsigned short PawnWhiteImage[] = {
#include "Icons/pawn.icon"
};
unsigned short PawnStencilImage[] = {
#include "Icons/pawnStencil.icon"
};
mpr_static(PawnBlackPR, SQUARE_WIDTH, SQUARE_HEIGHT, 1, PawnBlackImage);
mpr_static(PawnWhitePR, SQUARE_WIDTH, SQUARE_HEIGHT, 1, PawnWhiteImage);
mpr_static(PawnStencilPR, SQUARE_WIDTH, SQUARE_HEIGHT, 1, PawnStencilImage);

struct pixrect * PieceStencils[6] = {
    &PawnStencilPR,
    &KnightStencilPR,
    &BishopStencilPR,
    &RookStencilPR,
    &QueenStencilPR,
    &KingStencilPR,
};

struct pixrect * PieceIcons[6][2] = {
    &PawnBlackPR,	&PawnWhitePR,
    &KnightBlackPR,	&KnightWhitePR,
    &BishopBlackPR,	&BishopWhitePR,
    &RookBlackPR,	&RookWhitePR,
    &QueenBlackPR,	&QueenWhitePR,
    &KingBlackPR,	&KingWhitePR,
};

/*
 * blank square pixrects
 */
unsigned short WhiteSquareImage[] = {
#include "Icons/whiteSquare.icon"
};
mpr_static(WhiteSquarePR, SQUARE_WIDTH, SQUARE_HEIGHT, 1, WhiteSquareImage);

unsigned short BlackSquareImage[] = {
#include "Icons/blackSquare.icon"
};
mpr_static(BlackSquarePR, SQUARE_WIDTH, SQUARE_HEIGHT, 1, BlackSquareImage);

/* board subwindow handles */
struct toolsw * BoardSW;
struct gfxsubwindow * Board;

/* pixrects used for piece animation */
struct pixrect * MoveFromPR, * MoveToPR;

/* pixrect used for victim drawing */
struct pixrect * VictimPR;

/*
 * board sigwinch handler 
 */
/*ARGSUSED*/
boardSigwinch(sw)
    caddr_t sw;
{
    gfxsw_interpretesigwinch(Board);
    gfxsw_handlesigwinch(Board);
    if (Board->gfx_flags & GFX_RESTART) {
	Board->gfx_flags &= ~ GFX_RESTART;
	DrawBoard();
    }
}

/*
 * map a mouse coordinate to a board coordinate
 */
void
mapMouseToBoard(mlocp, blocp)
    struct pr_pos * mlocp;
    BoardCoordinate * blocp;
{
    if (MyColor == WHITE) {
	blocp->x = mlocp->x / (SQUARE_WIDTH-1);
	blocp->y = mlocp->y / (SQUARE_HEIGHT-1);
    } else {
	blocp->x = (8 * (SQUARE_WIDTH-1) - mlocp->x)/(SQUARE_WIDTH-1);
	blocp->y = (8 * (SQUARE_HEIGHT-1) - mlocp->y)/(SQUARE_HEIGHT-1);
    }
}

/* 
 * map a board coordinate to a mouse coordinate
 */
void
mapBoardToMouse(blocp, mlocp)
    BoardCoordinate * blocp;
    struct pr_pos * mlocp;
{
    if (MyColor == WHITE) {
	mlocp->x = blocp->x * (SQUARE_WIDTH-1) - 1;
	mlocp->y = blocp->y * (SQUARE_HEIGHT-1) - 1;
    } else {
	mlocp->x = (7 - blocp->x) * (SQUARE_WIDTH-1) - 1;
	mlocp->y = (7 - blocp->y) * (SQUARE_WIDTH-1) - 1;
    }
}

/*
 * put a piece back where we got it (used to abort piece animation for 
 * various reasons)
 */
void
putPieceBack(from, stencil, icon)
    BoardCoordinate * from;
    struct pixrect * stencil, * icon;
{
    struct pr_pos loc;

    mapBoardToMouse(from, &loc);
    pw_stencil(Board->gfx_pixwin, 
	loc.x, loc.y,
	SQUARE_WIDTH, SQUARE_HEIGHT,
	PIX_SRC, stencil, 0, 0, icon, 0, 0);
}

/*
 * parameters which belong in boardSelected(), but which are shared
 * with RequestUndo() so that we can abort piece animation if the 
 * klutz at the other end panics (via an undo request or a resignation).  
 * all of them need to be static, anyway.
 */

BoardCoordinate from, to;
struct pixrect * pieceIcon, * pieceStencil;
struct pr_pos lastMouseLoc;

/*
 * abort any mouse activity - this really only means aborting piece 
 * animation.
 */
void
KillMouseActivity()
{
    switch (Mouse) {
    case PROMOTING_PAWN:
	UnDoMove();
	break;
    case MOVING_PIECE:
	/* repaint the background */
	pw_rop(Board->gfx_pixwin,
	    lastMouseLoc.x - MOVE_X_OFFSET,
	    lastMouseLoc.y - MOVE_Y_OFFSET,
	    MOVE_PR_WIDTH, MOVE_PR_HEIGHT,
	    PIX_SRC, MoveFromPR, 0, 0);
	putPieceBack(&from, pieceStencil, pieceIcon);
	break;
    }
}

/*
 * relay a request for an undo by the opponent.
 *
 * this has the side-effect of aborting any mouse activity that the 
 * user had going at the time.  however, setups will not be interrupted 
 * by undo requests since the (spurious) undo request will be prevented 
 * when the opponent's program detects that he hasn't moved yet.
 */
void
RequestUndo()
{
    Message("Your opponent wants to undo - left OK, other not OK");
    KillMouseActivity();
    Mouse = CONFIRMING;
    confirmState = CONFIRM_UNDO;
}

/*
 * confirm a wish to resign
 */
void
ConfirmResignation()
{

    Message("Sure you want to resign? - left yes, other no");
    Mouse = CONFIRMING;
    confirmState = CONFIRM_RESIGNATION;
}

/*
 * board select() handler 
 */
/*ARGSUSED*/
boardSelected(nullsw, ibits, obits, ebits, timer)
    caddr_t * nullsw;
    int * ibits, * obits, * ebits;
    struct timeval ** timer;
{
    static tickCount = 0;
    static Move move;
    static SetupChange setup;
    struct inputevent ie;
    struct pr_pos newMouseLoc;
    Square * sqp;
    long nbytes;
    BOOL clamped;
    int color, i;
    struct pixrect * pr;

    /*
     * 1-second ticker.  this cannot use SIGALRM, since the RPC
     * package already has dibs on that signal for implementing timeouts.
     * hence, it uses the subwindow select timeout mechanism.
     * the ticker is used to flash the window every 5 seconds if 
     * there is some event the user hasn't seen yet (opening the 
     * tool window turns the flasher off).
     *
     * note - timeouts render the file descriptor masks undefined, 
     * so we clean up and return w/o checking them.
     */
    if ((*timer)->tv_sec == 0L && (*timer)->tv_usec == 0L) {
	if (Flashing) {
	    if (wmgr_iswindowopen(NchessTool->tl_windowfd)) {
		tickCount = 0;
		Flashing = FALSE;
	    } else if (tickCount-- <= 0
	    && (pr = (struct pixrect *) tool_get_attribute(NchessTool, 
		WIN_ICON_IMAGE)) != (struct pixrect *) -1)
	    {
		tickCount = 5;
		pr_rop(pr, 0, 0, pr->pr_size.x, pr->pr_size.x, 
		    PIX_NOT(PIX_SRC), pr, 0, 0);
		tool_set_attributes(NchessTool, WIN_ICON_IMAGE, pr, 0);
		pr_rop(pr, 0, 0, pr->pr_size.x, pr->pr_size.x, 
		    PIX_NOT(PIX_SRC), pr, 0, 0);
		tool_set_attributes(NchessTool, WIN_ICON_IMAGE, pr, 0);
		tool_free_attribute(WIN_ICON_IMAGE, pr);
	    }
	}
	/* reset the timer and the file des. masks */
	(*timer)->tv_sec = 1L;
	(*timer)->tv_usec = 0L;
	* ibits = svc_fds | BoardSWMask 
	    | ChessProcessFDs[BLACK] | ChessProcessFDs[WHITE];
	* obits = * ebits = 0;
	return;
    }
    /* 
     * check for RPC service required 
     */
    if (*ibits & svc_fds) {
	svc_getreq(*ibits & svc_fds);
    } 
    /*
     * check for machine move received
     *
     * note: it is possible in some circumstances to receive a move
     * from a machine player after the game is over (for example, 
     * the opponent of the machine which is on move dies).
     * in that event, we simply throw the move away.
     */
    for (color = BLACK , i = 0  ; i < 2 ; color = WHITE , i++) {
	if ((*ibits & ChessProcessFDs[color]) 
	&& GetMachineMove(&move, color)
	&& ! GameOver) {
	    BOOL updateMsgWindow = TRUE;

	    Flashing = TRUE;
	    DoMove(&move, TRUE);
	    Turn = OTHERCOLOR(Turn);
	    /* 
	     * if we are trying to save a game of machine vs. machine,
	     * hold up sending the move until both machine states 
	     * have been saved.  when subsequently restoring the 
	     * game, this move will be resubmitted.
	     */
	    if (SaveWanted) {
		SaveGame(SaveFileName);
		SaveWanted = FALSE;
		updateMsgWindow = FALSE;
	    }
	    /* 
	     * if the player not moving is a machine, send the move 
	     */
	    if (IsMachine[OTHERCOLOR(color)]) {
		SendMove(&move, OTHERCOLOR(color));
	    /*
	     * else if the player not moving is a human that wants an
	     * undo, back it out
	     */
	    } else if (UndoWanted) {
		MachineUndo(color);
		UndoWanted = FALSE;
		Mouse = IDLE;
	    }
	    if (updateMsgWindow)
		WhoseMoveMessage((char *) 0);
	}
    }
    /*
     * check for board subwindow service required
     */
    if (*ibits & BoardSWMask) {
	if (input_readevent(BoardSW->ts_windowfd, &ie) == -1) {
	    perror("input failed");
	    abort();
	}
	switch (Mouse) {
	/*
	 * locked: ignore all mouse activity
	 */
	case LOCKED:
	    break;
	/* 
	 * we are using the mouse to confirm something 
	 */
	case CONFIRMING:
	    if (win_inputposevent(&ie) 
	    && ie.ie_code >= BUT_FIRST
	    && ie.ie_code <= BUT_LAST) {
		Mouse = IDLE;
		switch(confirmState) {
		case CONFIRM_RESIGNATION:
		    if (ie.ie_code == MS_LEFT) {
			SendResignation(PeerColor);
			Mouse = LOCKED;		/* game is over */
			DoResignation(MyColor);
			Message("Resigned");
		    } else {
			WhoseMoveMessage((char *) 0);
		    }
		    break;
		case CONFIRM_SETUP_END:
		    if (ie.ie_code == MS_LEFT) {
			/* 
			 * if either player is a machine, white always moves
			 * first following a setup (another brain-damaged
			 * attribute of the unix chess program).
			 */
			if (IsMachine[WHITE] || IsMachine[BLACK]) {
			    BOOL legalSetup = TRUE;

			    Turn = WHITE;
			    if (IsMachine[BLACK]) {
				legalSetup = MachineSetup(BLACK);
			    }
			    if (legalSetup && IsMachine[WHITE]) {
				if (legalSetup = MachineSetup(WHITE))
				    MachineFirst(WHITE);
			    }
			    if ( ! legalSetup) {
				Message("Illegal setup - try again");
				Mouse = SETUP;
			    } else {
				/*
				 * if both players are machines, the human part
				 * is over.
				 */
				if (IsMachine[BLACK] && IsMachine[WHITE]) 
				    Mouse = LOCKED;
				/*
				 * else we get to play
				 */
				else 
				    InitialTurn = WHITE;
				WhoseMoveMessage((char *) 0);
				SetupMode = FALSE;
			    }
			/*
			 * else the opponent is a human, and we can specify
			 * who moves first.
			 */
			} else {
			    Message("Left button to move first, other to move second");
			    Mouse = CONFIRMING;
			    confirmState = CONFIRM_WHOSE_TURN;
			    SetupMode = FALSE;
			}
		    } else {
			Message("Setup: left - source, middle - delete, right - end");
			Mouse = SETUP;
		    }
		    break;
		case CONFIRM_WHOSE_TURN:
		    Turn = InitialTurn = (ie.ie_code == MS_LEFT ? 
			MyColor : 
			OTHERCOLOR(MyColor));
		    WhoseMoveMessage((char *) 0);
		    SendEndRestore();
		    break;
		case CONFIRM_UNDO:
		    SendUndoAcknowledgement(ie.ie_code == MS_LEFT);
		    break;
		}
	    }
	    break;
	/*
	 * we are setting up an initial board layout
	 */
	case SETUP:
	    switch (ie.ie_code) {
	    /*
	     * generate and pick up a source piece
	     */
	    case MS_LEFT:
		if (win_inputposevent(&ie)) {
		    newMouseLoc.x = ie.ie_locx;
		    newMouseLoc.y = ie.ie_locy;
		    mapMouseToBoard(&newMouseLoc, &from);
		    /* if this a source square */
		    if (IsSrcPieceAt(&from)) {
			Mouse = MOVING_PIECE;
			sqp = GetSrcSquare(from.x, from.y);
			setup.type = sqp->type;
			setup.color = sqp->color;
			/*
			 * create the first background pixrect,
			 * centered on the selected board square
			 */
			mapBoardToMouse(&from, &lastMouseLoc);
			/* grab the currently displayed image */
			pw_read(MoveFromPR, 
			    0, 0, MOVE_PR_WIDTH, MOVE_PR_HEIGHT,
			    PIX_SRC,
			    Board->gfx_pixwin,
			    lastMouseLoc.x - MOVE_X_OFFSET,
			    lastMouseLoc.y - MOVE_Y_OFFSET);
			/* repaint the blank square */
			pr_rop(MoveFromPR,
			    MOVE_X_OFFSET,
			    MOVE_Y_OFFSET,
			    SQUARE_WIDTH, SQUARE_HEIGHT,
			    PIX_SRC,
			    ((from.x + from.y) & 0x01) ? 
				&BlackSquarePR :
				&WhiteSquarePR,
			    0, 0);
			/* 
			 * remember the pixrect used to paint the piece 
			 * being moved 
			 */
			pieceIcon = PieceIcons[(int) sqp->type][sqp->color];
			pieceStencil = PieceStencils[(int) sqp->type];
			/* 
			 * if there is a piece at the source square, repaint 
			 * the piece on the background pixrect 
			 */
			sqp = GetSquare(from.x, from.y);
			if (sqp->type != NULLPC) {
			    pr_stencil(MoveFromPR,
				MOVE_X_OFFSET,
				MOVE_Y_OFFSET,
				SQUARE_WIDTH, SQUARE_HEIGHT,
				PIX_SRC, 
				PieceStencils[(int)sqp->type], 0, 0, 
				PieceIcons[(int) sqp->type][sqp->color], 0, 0);
			}
		    }
		}
		break;
	    /*
	     * delete a piece 
	     */
	    case MS_MIDDLE:
		if (win_inputposevent(&ie)) {
		    newMouseLoc.x = ie.ie_locx;
		    newMouseLoc.y = ie.ie_locy;
		    mapMouseToBoard(&newMouseLoc, &from);
		    setup.x = from.x;
		    setup.y = from.y;
		    setup.type = NULLPC;
		    DoSetupChange(&setup);
		    SendSetupChange(&setup, PeerColor);
		}
		break;
	    /*
	     * exit setup
	     */
	    case MS_RIGHT:
		if (win_inputposevent(&ie)) {
		    Message("Sure you want to end setup? left - yes, other - no");
		    Mouse = CONFIRMING;
		    confirmState = CONFIRM_SETUP_END;
		}
		break;
	    }
	    break;
	/*
	 * we are promoting a pawn
	 */
	case PROMOTING_PAWN:
	    switch(ie.ie_code) {
	    /*
	     * select the next pawn morph
	     */
	    case MS_LEFT:
		if (win_inputposevent(&ie))
		    move.newPieceType = PromotePawn(&to);
		break;
	    /*
	     * go for the current morph
	     */
	    case MS_MIDDLE:
		if (win_inputposevent(&ie) && SendMove(&move, PeerColor)) {
		    Turn = OTHERCOLOR(Turn);
		    WhoseMoveMessage((char *) 0);
		    Mouse = IDLE;
		}
		break;
	    /*
	     * we exited the window - back out the pawn move 
	     */
	    case LOC_WINEXIT:
		UnDoMove();
		Mouse = IDLE;
		break;
	    }
	    break;
	/* 
	 * we aren't currently doing anything
	 */
	case IDLE:
	    switch(ie.ie_code) {
	    case MS_LEFT:
		/*
		 * if this a left button press 
		 * and it is our turn
		 * and we aren't waiting for an undo acknowledgement
		 */
		if (win_inputposevent(&ie) && Turn == MyColor && ! UndoWanted) {
		    newMouseLoc.x = ie.ie_locx;
		    newMouseLoc.y = ie.ie_locy;
		    mapMouseToBoard(&newMouseLoc, &from);
		    /* 
		     * if there is one of our pieces on this square 
		     */
		    if (IsOurPieceAt(&from)) {
			Mouse = MOVING_PIECE;
			sqp = GetSquare(from.x, from.y);
			/*
			 * create the first background pixrect,
			 * centered on the selected board square
			 */
			mapBoardToMouse(&from, &lastMouseLoc);
			/* grab the currently displayed image */
			pw_read(MoveFromPR, 
			    0, 0, MOVE_PR_WIDTH, MOVE_PR_HEIGHT,
			    PIX_SRC,
			    Board->gfx_pixwin,
			    lastMouseLoc.x - MOVE_X_OFFSET,
			    lastMouseLoc.y - MOVE_Y_OFFSET);
			/* repaint the blank square */
			pr_rop(MoveFromPR,
			    MOVE_X_OFFSET,
			    MOVE_Y_OFFSET,
			    SQUARE_WIDTH, SQUARE_HEIGHT,
			    PIX_SRC,
			    ((from.x + from.y) & 0x01) ? 
				&BlackSquarePR :
				&WhiteSquarePR,
			    0, 0);
			/* 
			 * remember the pixrect used to paint the piece 
			 * being moved 
			 */
			pieceIcon = PieceIcons[(int) sqp->type][sqp->color];
			pieceStencil = PieceStencils[(int) sqp->type];
			/*
			 * a bit un-structured, but we are forced to do this
			 * if we want the piece to "jump" to the cursor when
			 * the button is initially depressed ("forced" in the
			 * sense that we are inside a switch statement).
			 */
			goto moveIt;
		    }
		}
		break;
	    }
	    break;
	/*
	 * we are animating a piece 
	 */
	case MOVING_PIECE:
	    switch(ie.ie_code) {
	    case MS_LEFT:
		/*
		 * if we are putting down a piece
		 */
		if (win_inputnegevent(&ie)) {
		    BOOL legal;

		    Mouse = IDLE;
		    newMouseLoc.x = ie.ie_locx;
		    newMouseLoc.y = ie.ie_locy;
		    mapMouseToBoard(&newMouseLoc, &to);
		    legal = TRUE;
		    move.x1 = from.x; move.y1 = from.y;
		    move.x2 = to.x; move.y2 = to.y;
		    if (SetupMode) {
			/* repaint the background */
			pw_rop(Board->gfx_pixwin,
			    lastMouseLoc.x - MOVE_X_OFFSET,
			    lastMouseLoc.y - MOVE_Y_OFFSET,
			    MOVE_PR_WIDTH, MOVE_PR_HEIGHT,
			    PIX_SRC, MoveFromPR, 0, 0);
			/* put the piece down no matter what */
			setup.x = to.x;
			setup.y = to.y;
			DoSetupChange(&setup);
			SendSetupChange(&setup, PeerColor);
			Mouse = SETUP;
		    } else if ( ! (from.x == to.x && from.y == to.y) 
		    && Turn == MyColor 
		    && (legal = IsMoveLegal(&from, &to))) {
			/* if this is a pawn promotion */
			if (GetSquare(from.x, from.y)->type == PAWN
			&& (to.y == 0 || to.y == 7)) {
			    /* repaint the background */
			    pw_rop(Board->gfx_pixwin,
				lastMouseLoc.x - MOVE_X_OFFSET,
				lastMouseLoc.y - MOVE_Y_OFFSET,
				MOVE_PR_WIDTH, MOVE_PR_HEIGHT,
				PIX_SRC, MoveFromPR, 0, 0);
			    move.newPieceType = (int) QUEEN;
			    DoMove(&move, TRUE);
			    /* if we are playing the brain-damaged unix chess
			     * program, can only promote to queens */
			    if (IsMachine[OTHERCOLOR(MyColor)]) {
				Turn = OTHERCOLOR(Turn);
				SendMove(&move, PeerColor);
				WhoseMoveMessage((char *) 0);
			    /* else need to have the user select the promoted 
			     * piece type */
			    } else {
				Message("Left button: select piece type, middle: send move.");
				Mouse = PROMOTING_PAWN;
			    }
			/* else if we can send the move */
			} else if (SendMove(&move, PeerColor)) {
			    /* repaint the background */
			    pw_rop(Board->gfx_pixwin,
				lastMouseLoc.x - MOVE_X_OFFSET,
				lastMouseLoc.y - MOVE_Y_OFFSET,
				MOVE_PR_WIDTH, MOVE_PR_HEIGHT,
				PIX_SRC, MoveFromPR, 0, 0);
			    DoMove(&move, TRUE);
			    Turn = OTHERCOLOR(Turn);
			    WhoseMoveMessage((char *) 0);
			/* else the peer is dead */
			} else {
			    /* repaint the background */
			    pw_rop(Board->gfx_pixwin,
				lastMouseLoc.x - MOVE_X_OFFSET,
				lastMouseLoc.y - MOVE_Y_OFFSET,
				MOVE_PR_WIDTH, MOVE_PR_HEIGHT,
				PIX_SRC, MoveFromPR, 0, 0);
			    putPieceBack(&from, pieceStencil, pieceIcon);
			}
		    } else {
			/* repaint the background */
			pw_rop(Board->gfx_pixwin,
			    lastMouseLoc.x - MOVE_X_OFFSET,
			    lastMouseLoc.y - MOVE_Y_OFFSET,
			    MOVE_PR_WIDTH, MOVE_PR_HEIGHT,
			    PIX_SRC, MoveFromPR, 0, 0);
			putPieceBack(&from, pieceStencil, pieceIcon);
			if ( ! legal)
			    Message("Illegal move");
		    }
		}
		break;
	    /*
	     * exited the window - clean it up.
	     */
	    case LOC_WINEXIT:
		/* repaint the background */
		pw_rop(Board->gfx_pixwin,
		    lastMouseLoc.x - MOVE_X_OFFSET,
		    lastMouseLoc.y - MOVE_Y_OFFSET,
		    MOVE_PR_WIDTH, MOVE_PR_HEIGHT,
		    PIX_SRC, MoveFromPR, 0, 0);
		if (SetupMode) {
		    Mouse = SETUP;
		} else {
		    putPieceBack(&from, pieceStencil, pieceIcon);
		    Mouse = IDLE;
		}
		break;
	    /* 
	     * animate the piece
	     */
	    case LOC_MOVEWHILEBUTDOWN:
	moveIt:
		/* ignore old motion events */
		ioctl(Board->gfx_windowfd, (int) FIONREAD, (char *) &nbytes);
		if (nbytes/sizeof(struct inputevent) <= 3) {
		    do {
			newMouseLoc.x = ie.ie_locx - SQUARE_WIDTH/2;
			newMouseLoc.y = ie.ie_locy - SQUARE_HEIGHT/2;
			clamped = FALSE;
			/*
			 * clamp motion if necessary
			 */
			if (newMouseLoc.x - lastMouseLoc.x >= MOVE_X_OFFSET) {
			    newMouseLoc.x = lastMouseLoc.x + (MOVE_X_OFFSET-1);
			    clamped = TRUE;
			} else if (newMouseLoc.x - lastMouseLoc.x <= - MOVE_X_OFFSET) {
			    newMouseLoc.x = lastMouseLoc.x - (MOVE_X_OFFSET-1);
			    clamped = TRUE;
			} 
			if (newMouseLoc.y - lastMouseLoc.y >= MOVE_Y_OFFSET) {
			    newMouseLoc.y = lastMouseLoc.y + (MOVE_Y_OFFSET-1);
			    clamped = TRUE;
			} else if (newMouseLoc.y - lastMouseLoc.y <= - MOVE_Y_OFFSET) {
			    newMouseLoc.y = lastMouseLoc.y - (MOVE_Y_OFFSET-1);
			    clamped = TRUE;
			}
			/* grab the new area */
			pw_read(MoveToPR, 
			    0, 0, MOVE_PR_WIDTH, MOVE_PR_HEIGHT,
			    PIX_SRC, Board->gfx_pixwin,
			    newMouseLoc.x - MOVE_X_OFFSET,
			    newMouseLoc.y - MOVE_Y_OFFSET);
			/* paste the old background over the new area */
			pr_rop(MoveToPR,
			    lastMouseLoc.x - newMouseLoc.x,
			    lastMouseLoc.y - newMouseLoc.y,
			    MOVE_PR_WIDTH, MOVE_PR_HEIGHT,
			    PIX_SRC, MoveFromPR, 0, 0);
			/* save the new background */
			pr_rop(MoveFromPR, 0, 0, MOVE_PR_WIDTH, MOVE_PR_HEIGHT,
			    PIX_SRC, MoveToPR, 0, 0);
			/* paint the piece on the new area */
			pr_stencil(MoveToPR, 
			    MOVE_X_OFFSET, MOVE_Y_OFFSET,
			    SQUARE_WIDTH, SQUARE_HEIGHT,
			    PIX_SRC, pieceStencil, 0, 0, pieceIcon, 0, 0);
			/* now paint the new area on the screen */
			pw_rop(Board->gfx_pixwin,
			    newMouseLoc.x - MOVE_X_OFFSET,
			    newMouseLoc.y - MOVE_Y_OFFSET,
			    MOVE_PR_WIDTH, MOVE_PR_HEIGHT,
			    PIX_SRC, MoveToPR, 0, 0);
			lastMouseLoc.x = newMouseLoc.x;
			lastMouseLoc.y = newMouseLoc.y;
		    } while (clamped);
		}
		break;
	    }
	    break;
	}
    }
    * ibits = svc_fds | BoardSWMask | ChessProcessFDs[BLACK] | ChessProcessFDs[WHITE];
    * obits = * ebits = 0;
}

/*
 * initialize the board subwindow
 */
void
InitBoardSW(useRetained, iconDirectory)
    BOOL useRetained;			/* use a retained pixrect */
    char * iconDirectory;		/* custom piece icon directory */
{
    static struct timeval tickValue;
    struct inputmask mask;
    register unsigned int i;
    int height = (SQUARE_HEIGHT-1) * 8 + ((7 * SQUARE_HEIGHT)/4) + 10;

    /*
     * initialize the subwindow
     */
    if ((BoardSW = gfxsw_createtoolsubwindow(NchessTool, "",
	TOOL_SWEXTENDTOEDGE, 
	/* playing surface    +  victim area */
/*	(SQUARE_HEIGHT-1) * 8 + ((7 * SQUARE_HEIGHT)/4) + 10,  */
	height,
	NULL)) == NULL) 
    {
	fprintf(stderr, "Can't create board subwindow\n");
	exit(1);
    }
    Board = (struct gfxsubwindow *) BoardSW->ts_data;
    if (useRetained)
	gfxsw_getretained(Board); 
    BoardSW->ts_io.tio_handlesigwinch = boardSigwinch;
    BoardSW->ts_io.tio_selected = boardSelected;
    input_imnull(&mask);
    win_setinputcodebit(&mask, MS_LEFT);
    win_setinputcodebit(&mask, MS_MIDDLE);
    win_setinputcodebit(&mask, MS_RIGHT);
    win_setinputcodebit(&mask, LOC_MOVEWHILEBUTDOWN);
    win_setinputcodebit(&mask, LOC_WINEXIT);
    mask.im_flags |= IM_NEGEVENT;
    win_setinputmask(BoardSW->ts_windowfd, &mask, NULL, WIN_NULLLINK);
    /* 
     * add the RPC service and chess process file descriptor select
     * masks to this subwindow.
     */
    BoardSW->ts_io.tio_inputmask = svc_fds 
	| (BoardSWMask = 1 << BoardSW->ts_windowfd) 
	| ChessProcessFDs[BLACK] 
	| ChessProcessFDs[WHITE] ;
    /*
     * set the timeout to 1 second 
     */
    tickValue.tv_sec = 1L;
    tickValue.tv_usec = 0L;
    BoardSW->ts_io.tio_timer = &tickValue;
    /*
     * create the white pieces by inverting the black pieces, using custom
     * pieces where appropriate.
     */
    for ( i = 0 ; i < 6 ; i++ ) {
	if (iconDirectory != (char *) 0) {
	    FILE * iconFile, * stencilFile;
	    icon_header_object iconHeader, stencilHeader;
	    char fileName[512], errorMsg[IL_ERRORMSG_SIZE + 2];

	    strcpy(fileName, iconDirectory);
	    strcat(fileName, "/");
	    strcat(fileName, PieceIconFileNames[i]);
	    if ((iconFile = icon_open_header(fileName, errorMsg, 
		&iconHeader)) != (FILE *) 0) 
	    {
		if (iconHeader.width != SQUARE_WIDTH 
		|| iconHeader.height != SQUARE_HEIGHT
		|| iconHeader.depth != 1) {
		    fprintf(stderr, "warning: bogus icon (ignored): %s\n", fileName);
		} else {
		    strcpy(fileName, iconDirectory);
		    strcat(fileName, "/");
		    strcat(fileName, PieceStencilFileNames[i]);
		    if ((stencilFile = icon_open_header(fileName, errorMsg, 
			&stencilHeader)) != (FILE *) 0) 
		    {
			if (stencilHeader.width != SQUARE_WIDTH 
			|| stencilHeader.height != SQUARE_HEIGHT
			|| stencilHeader.depth != 1) {
			    fprintf(stderr, "warning: bogus icon (ignored): %s\n", fileName);
			} else {
			    icon_read_pr(iconFile, &iconHeader, PieceIcons[i][BLACK]);
			    icon_read_pr(stencilFile, &stencilHeader, PieceStencils[i]);
			}
			fclose(stencilFile);
		    }
		}
		fclose(iconFile);
	    }
	}
	pr_rop(PieceIcons[i][WHITE], 0, 0, SQUARE_WIDTH, SQUARE_HEIGHT,
	    PIX_NOT(PIX_SRC),
	    PieceIcons[i][BLACK], 0, 0);
    }
    /* 
     * create the pixrects used for piece animation and victim drawing
     */
    if ((MoveFromPR = mem_create(MOVE_PR_WIDTH, MOVE_PR_HEIGHT, 1)) == (struct pixrect *) 0
    || (MoveToPR = mem_create(MOVE_PR_WIDTH, MOVE_PR_HEIGHT, 1)) == (struct pixrect *) 0
    || (VictimPR = mem_create(SQUARE_WIDTH, SQUARE_HEIGHT, 1)) == (struct pixrect *) 0) {
	fprintf(stderr, "can't create the animation pixrects\n");
	exit(1);
    }
}

/*
 * draw a square, including the piece (if any)
 */
void
DrawSquare(x, y, sqp)
    int x, y;
    register Square * sqp;
{
    BoardCoordinate bloc;
    struct pr_pos mloc;

    bloc.x = x; bloc.y = y;
    mapBoardToMouse(&bloc, &mloc);
    /* paint the blank square */
    pw_rop(Board->gfx_pixwin, 
	mloc.x, mloc.y,
	SQUARE_WIDTH, SQUARE_HEIGHT,
	PIX_SRC, 
	(((x + y) & 0x01) ? &BlackSquarePR : &WhiteSquarePR), 
	0, 0);
    /* paint the piece, if there is one */
    if (sqp->type != NULLPC) {
	pw_stencil(Board->gfx_pixwin, mloc.x, mloc.y,
	    SQUARE_WIDTH, SQUARE_HEIGHT,
	    PIX_SRC,
	    PieceStencils[(int) sqp->type], 0, 0,
	    PieceIcons[(int) sqp->type][sqp->color], 0, 0);
    }
}

/*
 * draw the playing surface and victim area
 */
void
DrawBoard()
{
    register int x, y;
    register Square * sqp;

    /* clear the board area */
    pw_rop(Board->gfx_pixwin,
	0, 0, Board->gfx_rect.r_width, Board->gfx_rect.r_height,
	PIX_CLR, (struct pixrect *) 0, 0, 0);
    /* draw the playing area */
    for (x = 0 ; x < 8 ; x++) {
	for (y = 0 ; y < 8 ; y++) {
	    sqp = GetSquare(x, y);
	    DrawSquare(x, y, sqp);
	}
    }
    /* draw the victims */
    drawVictims();
}

/*
 * your basic victim 
 */
typedef struct {
    PieceType type;			/* victim type */
    BOOL active;			/* victim slot state */
    int count;				/* victim overflow count */
} Victim;

#define	NUM_VICTIM_SLOTS 8

Victim victims[2 /* pawns(1) vs. pieces(0) */][2 /* color */ ][NUM_VICTIM_SLOTS /* victim slot # */];

/*
 * map the victim specified by the triple 
 * [ isPawn { 0, 1 }, color { BLACK, WHITE }, slot { 0 .. NUM_VICTIM_SLOTS-1 } ]
 * to a mouse coordinate (relative to the board subwindow)
 */
void
mapVictimToMouse(isPawn, color, slot, mlocp)
    BOOL isPawn;
    int color, slot;
    struct pr_pos * mlocp;
{
    mlocp->x = slot * (3 * SQUARE_WIDTH/4) + color * (3 * SQUARE_WIDTH/8);
    mlocp->y = 8 * (SQUARE_HEIGHT-1) + isPawn * (SQUARE_HEIGHT/2) + color * (SQUARE_HEIGHT/4) + 5;
}

/* 
 * draw the victims
 */
drawVictims()
{
    register int i, j, k;
    register Victim * victim;
    struct pr_pos victimOrigin;

    for (i = 0 ; i < 2 ; i++) {
	for (j = 0 ; j < 2 ; j++) {
	    for (k = 0 ; k < NUM_VICTIM_SLOTS ; k++) {
		victim = &victims[i][j][k];
		if (victim->active) {
		    mapVictimToMouse(i, j, k, &victimOrigin);
		    pw_stencil(Board->gfx_pixwin, 
			victimOrigin.x, victimOrigin.y,
			SQUARE_WIDTH, SQUARE_HEIGHT,
			PIX_SRC,
			PieceStencils[(int) victim->type], 0, 0,
			PieceIcons[(int) victim->type][j], 0, 0);
		}
	    }
	}
    }
}

/* 
 * add a piece to the set of victims 
 */
void
AddVictim(type, color, drawIt) 
    PieceType type;
    int color;
    BOOL drawIt;
{
    register int i, j, k;
    int empty, lastMatch, extras;
    register Victim * victim;
    BOOL isPawn = (type == PAWN);
    struct pr_pos victimOrigin, othersOrigin;

    /* 
     * look for the first empty slot and the last slot which 
     * contains a piece of the same type
     */
    for (lastMatch = empty = -1 , i = 0 ; i < NUM_VICTIM_SLOTS ; i++) {
	victim = &victims[isPawn][color][i];
	if (empty < 0 && ! victim->active)
	    empty = i;
	if (victim->active && victim->type == type)
	    lastMatch = i;
    }
    /*
     * if there were no empty slots 
     */
    if (empty == -1) {
	/* 
	 * if there was one or more pieces of the same type, update
	 * the last instance's overflow count (includes coalescing
	 * all overflows of that type at the last instance)
	 */
	if (lastMatch >= 0) {
	    for (extras = i = 0 ; i < lastMatch ; i++) {
		victim = &victims[isPawn][color][i];
		if (victim->active && victim->type == type && victim->count > 1) {
		    extras += victim->count - 1;
		    victim->count = 1;
		}
	    }
	    victims[isPawn][color][lastMatch].count += extras;
	}
    /*
     * else install the victim in the empty slot 
     */
    } else {
	victim = &victims[isPawn][color][empty];
	victim->type = type;
	victim->active = TRUE;
	victim->count = 1;
	if (drawIt) {
	    /*
	     * re-draw all the pieces in the victim's slot 
	     */
	    mapVictimToMouse(isPawn, color, empty, &victimOrigin);
	    pr_rop(VictimPR, 0, 0, SQUARE_WIDTH, SQUARE_HEIGHT,
		PIX_CLR, (struct pixrect *) 0, 0, 0);
	    for (i = 0 ; i < 2 ; i++) {
		for (j = 0 ; j < 2 ; j++) {
		    for (k = 0 ; k < NUM_VICTIM_SLOTS ; k++) {
			victim = &victims[i][j][k];
			if (victim->active) {
			    mapVictimToMouse(i, j, k, &othersOrigin);
			    pr_stencil(VictimPR,
				othersOrigin.x - victimOrigin.x,
				othersOrigin.y - victimOrigin.y,
				SQUARE_WIDTH, SQUARE_HEIGHT,
				PIX_SRC,
				PieceStencils[(int) victim->type], 0, 0,
				PieceIcons[(int) victim->type][j], 0, 0);
			}
		    }
		}
	    }
	    /*
	     * now re-draw the slot 
	     */
	    pw_rop(Board->gfx_pixwin,
		victimOrigin.x, victimOrigin.y, 
		SQUARE_WIDTH, SQUARE_HEIGHT,
		PIX_SRC, VictimPR, 0, 0);
	}
    }
}

/*
 * reincarnate a victim (via an undo)
 */
void
DeleteVictim(type, color)
    PieceType type;
    int color;
{
    register int i, j, k;
    int lastMatch, extras;
    register Victim * victim;
    BOOL isPawn = (type == PAWN);
    struct pr_pos victimOrigin, othersOrigin;

    /* 
     * look for the last slot which contains a piece of this type
     */
    for (lastMatch = -1 , i = 0 ; i < NUM_VICTIM_SLOTS ; i++) {
	victim = &victims[isPawn][color][i];
	if (victim->active && victim->type == type)
	    lastMatch = i;
    }
    /*
     * if there were no matches, don't do anything
     */
    if (lastMatch == -1) {
	/* do nothing */
    /*
     * else if the last match slot contains overflows, simply 
     * decrement the overflow count
     */
    } else if ((victim = &victims[isPawn][color][lastMatch])->count > 1) {
	victim->count--;
    /*
     * else zero out the slot and re-draw it as empty
     */
    } else {
	victim->active = FALSE;
	/*
	 * re-draw all the remaining pieces in the victim's slot 
	 */
	mapVictimToMouse(isPawn, color, lastMatch, &victimOrigin);
	pr_rop(VictimPR, 0, 0, SQUARE_WIDTH, SQUARE_HEIGHT,
	    PIX_CLR, (struct pixrect *) 0, 0, 0);
	for (i = 0 ; i < 2 ; i++) {
	    for (j = 0 ; j < 2 ; j++) {
		for (k = 0 ; k < NUM_VICTIM_SLOTS ; k++) {
		    victim = &victims[i][j][k];
		    if (victim->active) {
			mapVictimToMouse(i, j, k, &othersOrigin);
			pr_stencil(VictimPR,
			    othersOrigin.x - victimOrigin.x,
			    othersOrigin.y - victimOrigin.y,
			    SQUARE_WIDTH, SQUARE_HEIGHT,
			    PIX_SRC,
			    PieceStencils[(int) victim->type], 0, 0,
			    PieceIcons[(int) victim->type][j], 0, 0);
		    }
		}
	    }
	}
	/*
	 * now re-draw the slot 
	 */
	pw_rop(Board->gfx_pixwin,
	    victimOrigin.x, victimOrigin.y, 
	    SQUARE_WIDTH, SQUARE_HEIGHT,
	    PIX_SRC, VictimPR, 0, 0);
    }
}

