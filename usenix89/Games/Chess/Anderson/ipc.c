/*
 * manage communication with the peer chess tool, if one exists.
 * some communication functions with a background chess process are
 * relayed by functions in this file.
 */

#include <stdio.h>
#include <rpc/rpc.h>
#include <strings.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <pwd.h>
#include <signal.h>

#include "nchess.h"

unsigned long MyProgNum = (u_long) 0;	/* my program number */
unsigned long PeerProgNum;		/* program number of peer */
char * PeerHostName;			/* host name of peer */
char * PeerUserName;			/* user name of peer */
BOOL peerDead = FALSE;			/* peer connection lost or chess program died */

BOOL UndoWanted = FALSE;		/* undo was requested locally */
BOOL RestoringGame = FALSE;		/* restoring a saved game via the peer */
SVCXPRT * Xprt;				/* RPC service transport handle */

/* 
 * get a transient program number 
 */
unsigned long
getTransient(vers, sockp)
    unsigned long vers; 
    int * sockp;
{
    unsigned long progNum = 0x52000000L;
    int s, len;
    struct sockaddr_in addr;

    /* create a socket */
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	fprintf(stderr, "can't create a socket\n");
	exit(1);
    }
    * sockp = s;
    addr.sin_addr.s_addr = 0;
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    len = sizeof(addr);
    bind(s, (struct sockaddr *) &addr, len);
    if (getsockname(s, (caddr_t) &addr, &len) < 0) {
	fprintf(stderr, "getsockname failed\n");
	exit(1);
    }
    /* now find and reserve an available transient program number */
    while ( ! pmap_set(progNum++, vers, IPPROTO_UDP, addr.sin_port))
	continue;
    return(progNum - 1);
}

/*
 * timed out the peer
 */
void 
peerDied()
{
    Message("Lost connection to your opponent");
    peerDead = GameOver = TRUE;
    Mouse = LOCKED;
}

/*
 * RPC procedure dispatch routine
 */
void
dispatch(request, xprt)
    struct svc_req * request;
    SVCXPRT * xprt;
{
    Move move;
    SetupChange setup;
    GameRequest gr;
    char ans[80];
    int isUndoOK;

    switch (request->rq_proc) {
    /* 
     * are you there? 
     */
    case NULLPROC:
	if ( ! svc_sendreply(xprt, xdr_void, (caddr_t) 0)) {
	    fprintf(stderr, "can't reply to remote peer\n");
	    exit(1);
	}
	break;
    /*  
     * game invitation accepted 
     */
    case ACCEPTPROCNUM:
	if ( ! svc_getargs(xprt, XdrGameReq, &gr)) {
	    svcerr_decode(xprt);
	    exit(1);
	}
	if ( ! svc_sendreply(xprt, xdr_void, (caddr_t) 0)) {
	    fprintf(stderr, "can't reply to remote peer\n");
	    exit(1);
	}
	PeerProgNum = gr.progNum;
	PeerColor = gr.color;
	if (MyColor + PeerColor != WANTSWHITE + WANTSBLACK) {
	    switch(MyColor) {
	    /* I don't care - pick a color based on what the peer wants */
	    case EITHERCOLOR:
		switch(PeerColor) {
		case WANTSBLACK:
		    MyColor = WANTSWHITE;
		    break;
		case WANTSWHITE:
		    MyColor = WANTSBLACK;
		    break;
		default:
		    fprintf(stderr, "color arbitration botch\n");
		    exit(1);
		}
		break;
	    case WANTSBLACK:
		if (PeerColor == WANTSBLACK) {
		    printf("Your opponent insists on playing black - is that OK? ");
		    scanf("%s", ans);
		    if (ans[0] == 'y' || ans[0] == 'Y') 
			MyColor = WANTSWHITE;
		    else {
			fprintf(stderr, "couldn't agree on colors - bye\n");
			(void) callrpc(PeerHostName, PeerProgNum, VERSNUM, COLORFAILPROCNUM,
			    xdr_void, (caddr_t) 0,
			    xdr_void, (caddr_t) 0);
			exit(1);
		    }
		}
		break;
	    case WANTSWHITE:
		if (PeerColor == WANTSWHITE) {
		    printf("Your opponent insists on playing white - is that OK? ");
		    scanf("%s", ans);
		    if (ans[0] == 'y' || ans[0] == 'Y') 
			MyColor = WANTSBLACK;
		    else {
			fprintf(stderr, "couldn't agree on colors - bye\n");
			(void) callrpc(PeerHostName, PeerProgNum, VERSNUM, COLORFAILPROCNUM,
			    xdr_void, (caddr_t) 0,
			    xdr_void, (caddr_t) 0);
			exit(1);
		    }
		}
		break;
	    }
	}
	break;
    /* 
     * the other player didn't ameliorate our insistence on color 
     */
    case COLORFAILPROCNUM:
	fprintf(stderr, "couldn't agree on colors - bye\n");
	(void) svc_sendreply(xprt, xdr_void, (caddr_t) 0);
	exit(1);
    /* 
     * receive a move from the other player 
     */
    case MOVEPROCNUM:
	if ( ! svc_getargs(xprt, XdrMove, &move)) {
	    fprintf(stderr, "can't get RPC args\n");
	    exit(1);
	}
	if ( ! svc_sendreply(xprt, xdr_void, (caddr_t) 0)) {
	    peerDied();
	} else {
	    DoMove(&move, TRUE); 
	    Turn = MyColor;
	    Flashing = TRUE;
	    if (Mouse != CONFIRMING)
		WhoseMoveMessage((char *) 0);
	}
	break;
    /* 
     * receive a setup change from the other player 
     */
    case SETUPPROCNUM:
	if ( ! svc_getargs(xprt, XdrSetup, &setup)) {
	    fprintf(stderr, "can't get RPC args\n");
	    exit(1);
	}
	if ( ! svc_sendreply(xprt, xdr_void, (caddr_t) 0)) {
	    peerDied();
	} else {
	    DoSetupChange(&setup);
	}
	break;
    /* 
     * receive a restoration move from the other player's process
     */
    case RESTOREMOVEPROCNUM:
	if ( ! svc_getargs(xprt, XdrMove, &move)) {
	    fprintf(stderr, "can't get RPC args\n");
	    exit(1);
	}
	if ( ! svc_sendreply(xprt, xdr_void, (caddr_t) 0)) {
	    peerDied();
	} else {
	    DoMove(&move, TRUE); 
	    Turn = OTHERCOLOR(Turn);
	}
	break;
    /*
     * restoration complete 
     */
    case ENDRESTOREPROCNUM:
	if ( ! svc_getargs(xprt, xdr_int, &Turn)) {
	    fprintf(stderr, "can't get RPC args\n");
	    exit(1);
	}
	if ( ! svc_sendreply(xprt, xdr_void, (caddr_t) 0)) {
	    peerDied();
	} else {
	    WhoseMoveMessage((char *) 0);
	    RestoringGame = FALSE;
	    Mouse = IDLE;	/* unlock the mouse */
	    Flashing = (Turn == MyColor);
	}
	break;
    /* 
     * other player wants to undo his previous move 
     */
    case UNDOPROCNUM:
	if ( ! svc_sendreply(xprt, xdr_void, (caddr_t) 0)) {
	    peerDied();
	} else {
	    RequestUndo();
	    Flashing = TRUE;
	}
	break;
    /*
     * the other player blew away his tool.
     */
    case GOODBYEPROCNUM:
	svc_sendreply(xprt, xdr_void, (caddr_t) 0);
	KillMouseActivity();
	peerDead = GameOver = TRUE;
	Mouse = LOCKED;
	Message("Your opponent killed his tool process");
	break;
    /*
     * other player is acknowledging our request to undo.
     */
    case UNDOACKPROCNUM:
	if ( ! svc_getargs(xprt, xdr_int, &isUndoOK)) {
	    fprintf(stderr, "can't get RPC args\n");
	    exit(1);
	}
	if ( ! svc_sendreply(xprt, xdr_void, (caddr_t) 0)) {
	    peerDied();
	} else {
	    if (isUndoOK) {
		UnDoMove();
		if (Turn == MyColor)
		    UnDoMove();
		else
		    Turn = MyColor;
		WhoseMoveMessage("Undo accepted");
	    } else {
		WhoseMoveMessage("Undo refused");
	    }
	    UndoWanted = FALSE;
	    Mouse = IDLE;
	    Flashing = TRUE;
	}
	break;
    /*
     * other player resigned 
     */
    case RESIGNPROCNUM:
	if ( ! svc_sendreply(xprt, xdr_void, (caddr_t) 0))
	    peerDead = TRUE;
	Message(PeerColor == BLACK ? "Black resigns" : "White resigns");
	KillMouseActivity();
	Mouse = LOCKED;		/* game is over */
	DoResignation(OTHERCOLOR(MyColor));
	Flashing = TRUE;
	break;
    case MSGPROCNUM:
	if ( ! svc_getargs(xprt, XdrString, ans)) {
	    fprintf(stderr, "can't get RPC args\n");
	    exit(1);
	}
	if ( ! svc_sendreply(xprt, xdr_void, (caddr_t) 0)) {
	    peerDied();
	} else {
	    RecvTalkMsg(ans);
	    Flashing = TRUE;
	}
	break;
    }
}


/*
 * outstanding invitation - shared by InitRPC() and quitEarly().
 */
GameRequest gr;

/*
 * cancel our invitation with the remote daemon if we receive a signal
 * before the invitee responds.
 */
quitEarly()
{
    callrpc(PeerHostName, SERVERPROGNUM, VERSNUM, CANCELPROCNUM, 
	XdrGameReq, (caddr_t) &gr, 
	xdr_void, (caddr_t) 0);
    exit(1);
}

/*
 * initialize the RPC connection, using the user's "user@host" arg.
 */
void
InitRPC(cp, progName)
    char * cp, * progName;
{
    GameRequest gr2;
    int sock, rfds; 
    char localHostName[256], ans[80];
    BOOL resumeGame = FALSE;

    /*
     * decipher the remote host and user strings 
     */
    if (cp == (char *) 0 || (PeerHostName = index(cp, '@')) == (char *) 0) {
	fprintf(stderr, "%s: illegal/non-existent user@host arg\n", progName);
	exit(1);
    }
    *PeerHostName++ = '\0';
    PeerUserName = cp;
    /* 
     * determine the local host and user names 
     */
    if (gethostname(localHostName, sizeof(localHostName)) != 0) {
	fprintf(stderr, "%s: can't determine local host name\n", progName);
	exit(1);
    }
    /*
     * generate the transient RPC program number used by this player.
     */
    MyProgNum = getTransient(VERSNUM, &sock);
    if ((Xprt = svcudp_create(sock)) == (SVCXPRT *) 0) {
	fprintf(stderr, "svcudp_create failed\n");
	exit(1);
    }
    /*
     * register the peer-to-peer RPC dispatch procedure
     */
    (void) svc_register(Xprt, MyProgNum, VERSNUM, dispatch, 0L);
    /*
     * check for an invitation registered with the local chess
     * daemon, indicating that we are responding to an invitation.
     */
    (void) strcpy(gr.userName, PeerUserName);
    (void) strcpy(gr.hostName, PeerHostName);
    if (callrpc(localHostName, SERVERPROGNUM, VERSNUM, ACKPROCNUM,
	XdrGameReq, (caddr_t) &gr, 
	XdrGameReq, (caddr_t) &gr2) != 0) 
    {
	fprintf(stderr, "error: callrpc of local daemon (%s)\n", localHostName);
	exit(1);
    }
    PeerProgNum = gr2.progNum;
    PeerColor = gr2.color;
    resumeGame = gr2.resumeGame;
    do {
	/* 
	 * if no invitation was registered or the remote program 
	 * is no longer running,
	 * 	 then register an invitation with the remote daemon.
	 */
	if (PeerProgNum == 0L
	|| callrpc(PeerHostName, PeerProgNum, VERSNUM, NULLPROC, 
	    xdr_void, (caddr_t) 0, 
	    xdr_void, (caddr_t) 0) != 0) 
	{
	    gr.progNum = MyProgNum;
	    gr.color = MyColor;
	    gr.resumeGame = (RestoreFile != (FILE *) 0 || SetupMode);
	    (void) strcpy(gr.userName, UserPWEntry->pw_name);
	    (void) strcpy(gr.hostName, localHostName);
	    signal(SIGINT, quitEarly);
	    signal(SIGQUIT, quitEarly);
	    signal(SIGHUP, quitEarly);
	    if (callrpc(PeerHostName, SERVERPROGNUM, VERSNUM, REQPROCNUM, 
		XdrGameReq, (caddr_t) &gr, 
		xdr_void, (caddr_t) 0) != 0) 
	    {
		fprintf(stderr, "error: callrpc of remote daemon\n");
		exit(1);
	    }
	    printf("waiting for %s to respond...\n", PeerUserName);
	    /* 
	     * wait for the remote peer to send us his program number and color 
	     */
	    do {
		rfds = svc_fds;
		switch(select(32, &rfds, (int *) 0, (int *) 0, (struct timeval *) 0)) {
		case -1:
		    if (errno == EINTR)
			continue;
		    fprintf(stderr, "RPC select botch\n");
		    exit(1);
		case 0:
		    break;
		default:
		    svc_getreq(rfds);
		    break;
		}
	    } while(PeerProgNum == 0L);
	    signal(SIGINT, SIG_DFL);
	    signal(SIGQUIT, SIG_DFL);
	    signal(SIGHUP, SIG_DFL);
	}
	/*
	 * else accept the invitation, sending the remote program 
	 * our program number and color.
	 */
	else {
	    if (resumeGame) {
		MyColor = EITHERCOLOR;
		RestoringGame = TRUE;
		SetupMode = FALSE;
	    }
	    /* try to pre-arrange colors */
	    if (MyColor + PeerColor != WANTSWHITE + WANTSBLACK) {
		switch(MyColor) {
		/* I don't care - pick a color based on what the peer wants */
		case EITHERCOLOR:
		    switch(PeerColor) {
		    case EITHERCOLOR:
			MyColor = (random() & 0x01) ? WANTSBLACK : WANTSWHITE;
			break;
		    case WANTSBLACK:
			MyColor = WANTSWHITE;
			break;
		    case WANTSWHITE:
			MyColor = WANTSBLACK;
			break;
		    }
		    break;
		case WANTSBLACK:
		    if (PeerColor == WANTSBLACK) {
			printf("%s also wants to play black - is that OK? ", PeerUserName);
			scanf("%s", ans);
			if (ans[0] == 'y' || ans[0] == 'Y') 
			    MyColor = WANTSWHITE;
		    }
		    break;
		case WANTSWHITE:
		    if (PeerColor == WANTSWHITE) {
			printf("%s also wants to play white - is that OK? ", PeerUserName);
			scanf("%s", ans);
			if (ans[0] == 'y' || ans[0] == 'Y') 
			    MyColor = WANTSBLACK;
		    }
		    break;
		}
	    }
	    gr.progNum = MyProgNum;
	    gr.color = MyColor;
	    if (callrpc(PeerHostName, PeerProgNum, VERSNUM, ACCEPTPROCNUM, 
		XdrGameReq, (caddr_t) &gr, 
		xdr_void, (caddr_t) 0) != 0)
	    {
		PeerProgNum = 0L;
	    }
	}
    } while(PeerProgNum == 0L);
    PeerColor = OTHERCOLOR(MyColor);
    PlayerName[MyColor] = UserPWEntry->pw_name;
    PlayerName[PeerColor] = PeerUserName;
    /*
     * if the other player is shipping us the game state, lock the
     * mouse in the meantime.
     */
    if (RestoringGame)
	Mouse = LOCKED;
}

/*
 * send a move to the 'color' player
 */
BOOL
SendMove(move, color)
    Move * move;
    int color;
{
    if ( ! peerDead) {
	if (IsMachine[color]) {
	    SendMachineMove(move, color);
	} else if (callrpc(PeerHostName, PeerProgNum, VERSNUM, MOVEPROCNUM,
	    XdrMove, (caddr_t) move, 
	    xdr_void, (caddr_t) 0) != 0) 
	{
	    peerDied();
	}
    }
    return( ! peerDead);
}

void
SendRestoreMove(move, color)
    Move * move;
    int color;
{
    if ( ! IsMachine[color] 
    && ! peerDead
    && callrpc(PeerHostName, PeerProgNum, VERSNUM, RESTOREMOVEPROCNUM,
	XdrMove, (caddr_t) move, 
	xdr_void, (caddr_t) 0) != 0)
    {
	peerDied();
    }
}

void
SendEndRestore()
{
    if ( ! peerDead
    && callrpc(PeerHostName, PeerProgNum, VERSNUM, ENDRESTOREPROCNUM,
	xdr_int, (caddr_t) &Turn, 
	xdr_void, (caddr_t) 0) != 0)
    {
	peerDied();
    }
}

void
SendUndoRequest(color)
    int color;
{
    if ( ! peerDead) {
	if (IsMachine[color]) {
	    if (Turn != MyColor) {
		Message("Will undo after machine moves...");
	    } else {
		MachineUndo(color);
		UndoWanted = FALSE;
		Mouse = IDLE;
	    }
	} else {
	    Message("Sending undo request to your opponent (please wait)...");
	    if (callrpc(PeerHostName, PeerProgNum, VERSNUM, UNDOPROCNUM,
		xdr_void, (caddr_t) 0, 
		xdr_void, (caddr_t) 0) != 0)
	    {
		peerDied();
	    }
	}
    }
}

void
SendResignation(color)
    int color;
{
    if ( ! peerDead) {
	if (IsMachine[color]) {
	    KillChessProcesses();
	} else if (callrpc(PeerHostName, PeerProgNum, VERSNUM, RESIGNPROCNUM,
	    xdr_void, (caddr_t) 0, 
	    xdr_void, (caddr_t) 0) != 0)
	{
	    peerDied();
	}
    }
}

void
SendTalkMsg(cp)
    char * cp;
{
    if ( ! peerDead
    && callrpc(PeerHostName, PeerProgNum, VERSNUM, MSGPROCNUM,
	XdrString, (caddr_t) cp, 
	xdr_void, (caddr_t) 0) != 0)
    {
	peerDied();
    }
}

void
SendSetupChange(setup, color)
    SetupChange * setup;
    int color;
{
    if ( ! IsMachine[color] 
    && ! peerDead
    && callrpc(PeerHostName, PeerProgNum, VERSNUM, SETUPPROCNUM,
	XdrSetup, (caddr_t) setup, 
	xdr_void, (caddr_t) 0) != 0)
    {
	peerDied();
    }
}

void
SendUndoAcknowledgement(isUndoOK)
    int isUndoOK;
{
    if (callrpc(PeerHostName, PeerProgNum, VERSNUM, UNDOACKPROCNUM,
	xdr_int, (caddr_t) &isUndoOK, 
	xdr_void, (caddr_t) 0) != 0)
    {
	peerDied();
    } else if (isUndoOK) {
	UnDoMove();
	if (Turn == MyColor) 
	    Turn = OTHERCOLOR(MyColor);
	else 
	    UnDoMove();
	Message(MyColor == WHITE ?
	    "Waiting for black to re-play..." :
	    "Waiting for white to re-play...");
    } else 
	WhoseMoveMessage((char *) 0);
}

void
SendGoodbye()
{
    if ( ! IsMachine[PeerColor] && ! peerDead) {
	callrpc(PeerHostName, PeerProgNum, VERSNUM, GOODBYEPROCNUM,
	    xdr_void, (caddr_t) 0,
	    xdr_void, (caddr_t) 0);
    }
    if (MyProgNum != 0) {
	/* unregister the RPC transport handle */
	xprt_unregister(Xprt);
	/* release my program number for re-use */
	pmap_unset(MyProgNum, VERSNUM);
    }
}
