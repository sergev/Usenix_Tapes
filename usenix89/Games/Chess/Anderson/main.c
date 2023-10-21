/*
 * main client part of network chess.
 */

#include <stdio.h>
#include <strings.h>
#include <errno.h>
#include <suntool/tool_hs.h>
#include <pwd.h>

#include "nchess.h"

int errno;				/* global error number */

char * TranscriptFileName = "nchess.transcript";
int TranscriptType = TR_MIN_NORMAL;
char * SaveFileName = "nchess.save";
char * PlayerName[2];
BOOL SetupMode = FALSE;
BOOL IsMachine[2] = { FALSE, FALSE };
FILE * RestoreFile;			/* restoration file */
struct passwd * UserPWEntry;

#define	MAXCHESSPROCARGS	16
char * ChessProcessArgs[2][MAXCHESSPROCARGS];

/*
 * walk away from the game in the middle on receipt of an appropriate
 * signal
 */
bobbyFischer()
{
    /* kill any and all chess processes */
    KillChessProcesses();
    /* send a goodbye to the peer */
    SendGoodbye();
    exit(1);
}

main(argc, argv)
    int argc;
    char ** argv;
{
    register int i, j;
    char * cp = (char *) 0, * iconDirectory = (char *) 0;
    int chessArgIndex[2];
    BOOL useRetained = TRUE;

    if ((UserPWEntry = getpwuid(getuid())) == (struct passwd *) 0) {
	fputs("Can't determine who you are\n", stderr);
	exit(1);
    }
    chessArgIndex[BLACK] = chessArgIndex[WHITE] = 0;
    /* randomize things a bit */
    (void) srandom((long) getpid());
    /* parse and strip out the tool-related arguments */
    ParseToolArgs(&argc, argv);
    /* scan the remaining argument list */
    for (i = 1 ; i < argc ; i++) {
	if (argv[i][0] == '-') {
	    switch (argv[i][1]) {
	    case 'c':
		useRetained = FALSE;	/* use a retained pixrect for the board */
		break;
	    case 'v':
		MachineDebug = TRUE;	/* turn on chess program i/f debugging */
		break;
	    case 'd':			/* custom piece icon directory name */
		if (argv[i][j=2] == '\0' && (j = 0 , ++i >= argc)) {
		    fprintf(stderr, "%s: bad piece icon directory name arg\n", argv[0]);
		    exit(1);
		} 
		iconDirectory = &argv[i][j];
		break;
	    case 'b':			/* wants to play the black pieces */
		MyColor = WANTSBLACK;
		break;
	    case 'w':			/* wants to play the white pieces */
		MyColor = WANTSWHITE;
		break;
	    case 'x':			/* specify transcript notation type */
		if (argv[i][j=2] == '\0' && (j = 0 , ++i >= argc)
		|| sscanf(&argv[i][j], "%d", &TranscriptType) != 1
		|| TranscriptType < TR_MIN_TYPE
		|| TranscriptType > TR_MAX_TYPE) {
		    fprintf(stderr, "%s: bad transcript mode arg\n", argv[0]);
		    exit(1);
		}
		break;
	    case 't':			/* specify transcript file name */
		if (argv[i][j=2] == '\0' && (j = 0 , ++i >= argc)) {
		    fprintf(stderr, "%s: bad transcript filename arg\n", argv[0]);
		    exit(1);
		} 
		TranscriptFileName = &argv[i][j];
		break;
	    case 's':			/* specify save file name */
		if (argv[i][j=2] == '\0' && (j = 0 , ++i >= argc)) {
		    fprintf(stderr, "%s: bad save filename arg\n", argv[0]);
		    exit(1);
		} 
		SaveFileName = &argv[i][j];
		break;
	    case 'r':
		if (argv[i][j=2] == '\0' && (j = 0 , ++i >= argc)
		|| ((RestoreFile = fopen(&argv[i][j], "r")) == (FILE *) 0)) {
		    fprintf(stderr, "%s: bad restore file arg\n", argv[0]);
		    exit(1);
		}
		if (fread((caddr_t) &MyColor, sizeof(MyColor), 1, RestoreFile) != 1) {
		    fputs("Bad save file\n", stderr);
		    exit(1);
		}
		break;
	    case 'e':
		SetupMode = TRUE;
		Mouse = SETUP;
		break;
	    case 'm':
		IsMachine[WHITE] = TRUE;
		if (argv[i][j=2] == '\0' && (j = 0 , ++i >= argc)) {
		    fprintf(stderr, "%s: bad chess program arg\n", argv[0]);
		    exit(1);
		}
		if (chessArgIndex[WHITE] >= MAXCHESSPROCARGS - 1) {
		    fprintf(stderr, "%s: too many chess program args\n", argv[0]);
		    exit(1);
		}
		ChessProcessArgs[WHITE][chessArgIndex[WHITE]++] = &argv[i][j];
		ChessProcessArgs[WHITE][chessArgIndex[WHITE]] = (char *) 0;
		break;
	    case 'n':
		IsMachine[BLACK] = TRUE;
		if (argv[i][j=2] == '\0' && (j = 0 , ++i >= argc)) {
		    fprintf(stderr, "%s: bad chess program arg\n", argv[0]);
		    exit(1);
		}
		if (chessArgIndex[BLACK] >= MAXCHESSPROCARGS - 1) {
		    fprintf(stderr, "%s: too many chess program args\n", argv[0]);
		    exit(1);
		}
		ChessProcessArgs[BLACK][chessArgIndex[BLACK]++] = &argv[i][j];
		ChessProcessArgs[BLACK][chessArgIndex[BLACK]] = (char *) 0;
		break;
	    }
	} else 
	    cp = argv[i];
    }
    if (SetupMode && RestoreFile != (FILE *) 0) {
	fprintf(stderr, "%s: can only specify one of -r and -s\n", argv[0]);
	exit(1);
    }
    /*
     * if white is a machine, start up the white chess process and
     * force us to play the black pieces (if we get to play at all)
     */
    if (IsMachine[WHITE]) {
	InitChessProcess(ChessProcessArgs[WHITE], WHITE, 
	    ! SetupMode && RestoreFile == (FILE *) 0);
	PlayerName[WHITE] = ChessProcessArgs[WHITE][0];
	MyColor = BLACK;
    }
    /*
     * if black is a machine, start up the black chess process and
     * force us to play the white pieces (if we get to play at all)
     */
    if (IsMachine[BLACK]) {
	InitChessProcess(ChessProcessArgs[BLACK], BLACK, FALSE);
	PlayerName[BLACK] = ChessProcessArgs[BLACK][0];
	MyColor = WHITE;
    }
    /*
     * if both players are human, initialize the peer-to-peer RPC 
     * "connection" 
     */
    if ( ! IsMachine[WHITE] && ! IsMachine[BLACK]) {
	InitRPC(cp, argv[0]);
    /*
     * else if both players are machines, lock the mouse to avoid
     * spectator interference (unless the spectator is going to set
     * up the cannon fodder first)
     */
    } else if (IsMachine[WHITE] && IsMachine[BLACK]) {
	if ( ! SetupMode)
	    Mouse = LOCKED;			/* lock out the mouse */
    /*
     * else the user is playing a machine
     */
    } else {
	PlayerName[MyColor] = UserPWEntry->pw_name;
	PeerColor = OTHERCOLOR(MyColor);
    }
    signal(SIGINT, bobbyFischer);
    signal(SIGQUIT, bobbyFischer);
    signal(SIGHUP, bobbyFischer);
    /* initialize the board state */
    InitBoard();
    /* 
     * if we want to restore the game and our opponent hasn't beaten
     * us to it, try to do it now 
     */
    if (RestoreFile != (FILE *) 0 && ! RestoringGame)
	RestoreGame();
    /* initialize and install the tool */
    InitTool(useRetained, iconDirectory);
    tool_install(NchessTool);
    /* now play the game */
    RunTool();
    /* post-game cleanup */
    KillChessProcesses();
    SendGoodbye();
}

