/*
 * chess invitation rendezvous server
 *
 * there are three entry points: 
 * 	- request a game 
 *	- reply to a request
 *	- cancel a request
 */

#include <stdio.h>
#include <rpc/rpc.h>
#include <sys/time.h>
#include <strings.h>

#include "nchess.h"

char LocalHostName[256];		/* local host name string */

#define	MAXINVITES	32
typedef struct {			/* registered invitations with call-back information */
    GameRequest gr;			/* call-back information */
    BOOL active;			/* is this slot active? */
    unsigned long time;			/* time invitation was registered */
} Invitation;

Invitation Invitations[MAXINVITES];

/*
 * register a game invitation and print a message on the console.
 */
void
gameRequest(gameReq)
    GameRequest * gameReq;
{
    struct timeval tv;
    struct tm * timep;
    register int i, oldSlot, oldTime;
    register Invitation * invp;
    int avail = -1;
    FILE * console;


    (void) gettimeofday(&tv, (struct timezone *) 0);
    timep = localtime((long *) &tv.tv_sec);
    oldTime = tv.tv_sec;
    /* find an empty invitation slot, keeping track of the oldest active one */
    for (i = 0 ; i < MAXINVITES ; i++) {
	invp = &Invitations[i];
	if ( ! invp->active) {
	    avail = i;
	    break;
	} else if (invp->time < oldTime) {
	    oldSlot = i;
	    oldTime = invp->time;
	}
    }
    /* if no slots are empty, re-use the oldest one */
    if (avail < 0) 
	avail = oldSlot;
    /* fill out the invitation slot */
    invp = &Invitations[avail];
    invp->active = TRUE;
    invp->time = tv.tv_sec;
    invp->gr.progNum = gameReq->progNum;
    invp->gr.color = gameReq->color;
    invp->gr.resumeGame = gameReq->resumeGame;
    strncpy(invp->gr.hostName, gameReq->hostName, sizeof(gameReq->hostName));
    strncpy(invp->gr.userName, gameReq->userName, sizeof(gameReq->userName));
    /* 
     * print the invitation message on the local console 
     */
    if ((console = fopen("/dev/console", "w")) != (FILE *) 0) {
	fprintf(console, "\n\007\007\007Message from Chess Daemon@%s at %d:%02d ...\n", 
	    LocalHostName, timep->tm_hour, timep->tm_min);
	if (gameReq->resumeGame) 
	    fprintf(console, "%s wants to resume a game of chess.\n", 
		gameReq->userName);
	else
	    fprintf(console, "%s wants to play a game of chess.\n", 
		gameReq->userName);
	fprintf(console, "reply with: nchess %s@%s\n", 
	    gameReq->userName, gameReq->hostName);
	fflush (console);
	fclose(console);
    }
}

/*
 * attempt to respond to a game invitation 
 */
GameRequest *
gameAcknowledge(gameAck)
    GameRequest * gameAck;
{
    static GameRequest gr;
    register int i, newestTime = 0, newestIndex = -1;
    
    /* 
     * look for the most recently registered invitation
     * if one exists, return the prog. number and the color,
     *		then remove the entry.
     * else return an entry with a program number of zero.
     */
    gr.progNum = 0;
    for (i = 0 ; i < MAXINVITES && Invitations[i].active ; i++) {
	if (strcmp(Invitations[i].gr.hostName, gameAck->hostName) == 0
	&& strcmp(Invitations[i].gr.userName, gameAck->userName) == 0
	&& Invitations[i].time > newestTime) {
	    newestIndex = i;
	    newestTime = Invitations[i].time;
	}
    }
    if (newestIndex >= 0) {
	gr.progNum = Invitations[newestIndex].gr.progNum;
	gr.color = Invitations[newestIndex].gr.color;
	gr.resumeGame = Invitations[newestIndex].gr.resumeGame;
	Invitations[newestIndex].active = FALSE;
    }
    return(&gr);
}

/*
 * cancel a game invitation 
 */
void
cancelRequest(gr)
    GameRequest * gr;
{
    register int i;
    
    for (i = 0 ; i < MAXINVITES && Invitations[i].active ; i++) {
	if (strcmp(Invitations[i].gr.hostName, gr->hostName) == 0
	&& strcmp(Invitations[i].gr.userName, gr->userName) == 0
	&& Invitations[i].gr.progNum == gr->progNum) {
	    Invitations[i].active = FALSE;
	    break;
	}
    }
}

/*ARGSUSED*/
main(argc, argv)
    int argc;
    char ** argv;
{
    if (gethostname(LocalHostName, sizeof(LocalHostName)) != 0) {
	fprintf(stderr, "%s: can't determine the local host name\n", argv[0]);
	exit(1);
    }
    pmap_unset(SERVERPROGNUM, VERSNUM);
    /* register the entry points */
    registerrpc(SERVERPROGNUM, VERSNUM, REQPROCNUM, 
	gameRequest, XdrGameReq, xdr_void);
    registerrpc(SERVERPROGNUM, VERSNUM, ACKPROCNUM, 
	gameAcknowledge, XdrGameReq, XdrGameReq);
    registerrpc(SERVERPROGNUM, VERSNUM, CANCELPROCNUM, 
	cancelRequest, XdrGameReq, xdr_void);
    svc_run();
    fprintf(stderr, "%s: exiting - svc_run returned\n", argv[0]);
    exit(1);
}

