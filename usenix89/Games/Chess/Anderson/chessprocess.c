/*
 * manage one or two chess game processes
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <stdio.h>
#include <strings.h>
#include <ctype.h>

#include "nchess.h"

extern char * gets();

int ChessPid[2],
    ChessProcessFDs[2];
FILE * ReadChessProcess[2], * WriteChessProcess[2];
BOOL SigChildInterceptOn = FALSE;
BOOL MachineDebug = FALSE;
char * colorStrings[] = { "black", "white" };

/*
 * intercept SIGCHLD
 *
 * print a message for the first chess process that dies and abort the 
 * game.  (note: any change in child status is interpreted as death).
 */
handleSigChild()
{
    int pid;
    union wait status;

    while(1) {
	pid = wait3(&status, WNOHANG, (struct rusage *) 0);
	if (pid <= 0)
	    return;
	if ( ! GameOver && (pid == ChessPid[BLACK] || pid == ChessPid[WHITE])) {
	    Message (pid == ChessPid[WHITE] ?
		"White's process died" :
		"Black's process died" );
	    KillMouseActivity();
	    GameOver = TRUE;
	    Mouse = LOCKED;
	}
    }
}

/*
 * have a chess process make the first move.
 *
 * note: with the wonderful unix chess program, "first" also implies
 * playing the white pieces.  nonetheless, color is an argument to this
 * function in case we ever find out how to get around this problem.
 */
void
MachineFirst(color)
    int color;
{
    fputs("first\n", WriteChessProcess[color]);
    fflush(WriteChessProcess[color]);
    if (MachineDebug)
	fprintf(stderr, "sent to %s: first\n", colorStrings[color]);
}

/*
 * open a pipe to a chess game process
 *
 * startIt indicates whether the process should be sent the "first"
 * command.
 */
void
InitChessProcess(cp, color, startIt)
    char ** cp;
    int color;
    BOOL startIt;
{
    int sv[2];
    char c[128];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) {
	fputs("socketpair failed\n", stderr);
	exit(1);
    }
    if ((ChessPid[color] = fork()) == 0) {
	dup2(sv[0], 0);
	dup2(sv[0], 1); 
	chdir("/tmp");		/* so that "chess.out" can always be written */
	execvp(cp[0], cp);
	puts("exec");
	fflush(stdout);
	_exit(1);
    }
/*    dup2(sv[1], 0);
    dup2(sv[1], 1); */
    /* want to intercept SIGCHLD */
    if ( ! SigChildInterceptOn) {
	SigChildInterceptOn = TRUE;
	signal(SIGCHLD, handleSigChild);
    }
    ChessProcessFDs[color] = (1 << sv[1]);
    ReadChessProcess[color] = fdopen(sv[1], "r");
    WriteChessProcess[color] = fdopen(sv[1], "w");
    setbuf(ReadChessProcess[color], (char *) 0);
    /* 
     * eat the "Chess" prompt 
     * note: if the child's exec failed,
     * catch the "exec" prompt and give up
     */
    fgets(c, sizeof(c), ReadChessProcess[color]);
    if (strcmp(c, "exec\n") == 0) {
	KillChessProcesses();
	fprintf(stderr, "exec of %s failed\n", cp[0]);
	exit(1);
    }
    /* set algebraic notation mode */
    fputs("alg\n", WriteChessProcess[color]);
    fflush(WriteChessProcess[color]);
    if (MachineDebug)
	fprintf(stderr, "sent to %s: alg\n", colorStrings[color]);
    if (startIt)
	MachineFirst(color);
}
 

/*
 * reap one or more chess process
 */
void
ReapChessProcesses()
{
    union wait status;

    while (wait3(&status, WNOHANG, (struct rusage *) 0) >= 0)
	;
}

/*
 * kill any and all chess processes
 */
void
KillChessProcesses()
{
    signal(SIGCHLD, SIG_IGN);
    if (ChessPid[BLACK])
	kill(ChessPid[BLACK], SIGKILL);
    if (ChessPid[WHITE])
	kill(ChessPid[WHITE], SIGKILL);
    ReapChessProcesses();
}

/*
 * get a move from a chess process 
 * 
 * return 1 if a move was successfully obtained, 0 if not.
 */
int
GetMachineMove(move, color)
    Move * move;
    int color;
{
    char c[256], c2[256];
    char file1, file2;
    int rank1, rank2;

    if (fgets(c, sizeof(c), ReadChessProcess[color]) == (char *) 0) 
	return(0);
    if (MachineDebug)
	fprintf(stderr, "rec'd from %s: %s", colorStrings[color], c);
    /* look for special announcements */
    if (strcmp(c, "Forced mate\n") == 0) {
	Message(color == WHITE ? 
	    "White announces forced mate" :
	    "Black announces forced mate");
	return(0);
    } 
    if (strcmp(c, "Resign\n") == 0) {
	Message(color == WHITE ? "White resigns" : "Black resigns");
	DoResignation(color);
	Mouse = LOCKED;
	return(0);
    } 
    if (strcmp(c, "Illegal move\n") == 0) {
	Message("Internal botch - illegal move detected");
	return(0);
    } 
    if (strncmp(c, "done", 4) == 0) {
	return(0);
    }
    if (strncmp(c, "White wins", 10) == 0
    || strncmp(c, "Black wins", 10) == 0) {
	c[10] = '\0';
	Message(c);
	GameOver = TRUE;
	Mouse = LOCKED;
	return(0);
    }
    /* e.g., 1. d2d4 */
    if (sscanf(c, "%*d. %c%d%c%d", &file1, &rank1, &file2, &rank2) == 4
    /* e.g., 1. ... d2d4 */
    || sscanf(c, "%*d. ... %c%d%c%d", &file1, &rank1, &file2, &rank2) == 4) {
	move->x1 = file1 - (isupper(file1) ? 'A' : 'a');
	move->y1 = 8 - rank1;
	move->x2 = file2 - (isupper(file2) ? 'A' : 'a');
	move->y2 = 8 - rank2;
	/* TBD: possible non-queen pawn promotion 
	 * (the existing chess program doesn't implement this) */
	move->newPieceType = (int) QUEEN;
	return(1);
    } else {
	strcpy(c2, color == WHITE ? "White announces: " : "Black announces: ");
	Message(strncat(c2, c, sizeof(c2) - strlen(c2) - 2));
	if (strncmp(c, "Draw", 4) == 0) {
	    GameOver = TRUE;
	    Mouse = LOCKED;
	}
	return(0);
    }
}

/*
 * send a move to a chess process
 *
 * note: if the process responds with a move too quickly, we won't 
 * get another select() trigger to get its move.  thus, we need to
 * check for the availability of a move when the echo is received.
 */
void
SendMachineMove(move, color)
    Move * move;
    int color;
{
    char c[128];

    if (move->x1 != move->x2
    && GetSquare(move->x1, move->y1)->type == PAWN
    && GetSquare(move->x2, move->y2)->type == NULLPC) {
	/* re-encode en passant captures as horizontal moves */
	fprintf(WriteChessProcess[color], "%c%d%c%d\n", 
	    move->x1 + 'a', 8 - move->y1, 
	    move->x2 + 'a', 8 - move->y1);
	if (MachineDebug)
	    fprintf(stderr, "sent move to %s: %c%d%c%d\n", 
		colorStrings[color],
		move->x1 + 'a', 8 - move->y1, 
		move->x2 + 'a', 8 - move->y1);
    } else {
	fprintf(WriteChessProcess[color], "%c%d%c%d\n", 
	    move->x1 + 'a', 8 - move->y1, 
	    move->x2 + 'a', 8 - move->y2);
	if (MachineDebug)
	    fprintf(stderr, "sent move to %s: %c%d%c%d\n", 
		colorStrings[color],
		move->x1 + 'a', 8 - move->y1, 
		move->x2 + 'a', 8 - move->y2);
    }
    fflush(WriteChessProcess[color]);
    fgets(c, sizeof(c), ReadChessProcess[color]); /* eat the move echo */
    if (MachineDebug)
	fprintf(stderr, "rec'd from %s: %s", colorStrings[color], c);
}

/*
 * undo the last machine move and our move 
 */
void
MachineUndo(color)
{
    fputs("remove\n", WriteChessProcess[color]);
    fflush(WriteChessProcess[color]);
    if (MachineDebug)
	fprintf(stderr, "sent to %s: remove\n", colorStrings[color]);
    UnDoMove();
    UnDoMove();
}

/*
 * set up the board 
 * (as noted above: with the existing unix chess program, there is no 
 * apparent way (short of deciphering the chess.out format, which is 
 * totally un-fun) to inform the chess program whose turn it is and 
 * which color it is supposed to play - it assumes that white always 
 * is the first to move.  
 */

char whitePieceChars[] = { 'p', 'n', 'b', 'r', 'q', 'k', ' ' };
char blackPieceChars[] = { 'P', 'N', 'B', 'R', 'Q', 'K', ' ' };

/*
 * set up a board state against the machine.
 * returns TRUE if successful, FALSE otherwise
 */
BOOL
MachineSetup(color)
    int color;
{
    register int x, y;
    Square * sqp;
    char c[128];

    fputs("setup\n", WriteChessProcess[color]);
    if (MachineDebug)
	fprintf(stderr, "sent to %s:\nsetup\n", colorStrings[color]);
    for (y = 0 ; y < 8 ; y++) {
	for (x = 0 ; x < 8 ; x++) {
	    sqp = GetSquare(x, y);
	    putc(sqp->color == WHITE ? 
		whitePieceChars[(int) sqp->type] :
		blackPieceChars[(int) sqp->type], 
		WriteChessProcess[color]);
	    if (MachineDebug)
		fputc(sqp->color == WHITE ? 
		    whitePieceChars[(int) sqp->type] :
		    blackPieceChars[(int) sqp->type], 
		    stderr);
	}
	putc('\n', WriteChessProcess[color]);
	if (MachineDebug)
	    fputc('\n', stderr);
    }
    fflush(WriteChessProcess[color]);
    fgets(c, sizeof(c), ReadChessProcess[color]);
    if (MachineDebug)
	fprintf(stderr, "rec'd from %s: %s", colorStrings[color], c);
    return(strcmp(c, "Setup successful\n") == 0);
}

/*
 * have a chess process save the game as "/tmp/chess.out".
 * return the size of chess.out.
 */
int
MachineSave(color)
    int color;
{
    FILE * saveFile;
    struct stat saveFileStatus;
    int saveFileSize;
    int retryCount = 0;

    fputs("save\n", WriteChessProcess[color]);
    fflush(WriteChessProcess[color]);
    if (MachineDebug)
	fprintf(stderr, "sent to %s: save\n", colorStrings[color]);
    sleep((unsigned) 1);
    /* wait for the chess process to create chess.out */
    do {
	if ((saveFile = fopen("/tmp/chess.out", "r")) == (FILE *) 0) 
	    sleep((unsigned) 2); 
    } while(saveFile == (FILE *) 0 && ++retryCount < 10);
    if (saveFile == (FILE *) 0) {
	Message("Can't open chess.out!");
	return(-1);
    }
    saveFileStatus.st_size = -1;
    /* wait until chess.out stops growing */
    do {
	sleep((unsigned) 1);
	saveFileSize = saveFileStatus.st_size;
	fstat(fileno(saveFile), &saveFileStatus);
    } while (saveFileSize != saveFileStatus.st_size);
    fclose(saveFile);
    return(saveFileSize);
}

/*
 * restore the game 
 */
void
MachineRestore(color)
    int color;
{
    fputs("restore\n", WriteChessProcess[color]);
    fflush(WriteChessProcess[color]);
    if (MachineDebug)
	fprintf(stderr, "sent to %s: restore\n", colorStrings[color]);
}

