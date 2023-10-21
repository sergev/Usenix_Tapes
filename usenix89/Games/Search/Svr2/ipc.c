#ifndef lint
static char rcsid[] = "$Header: ipc.c,v 2.1 85/04/10 17:31:13 matt Stab $";
#endif
/*
 *
 * search
 *
 * multi-player and multi-system search and destroy.
 *
 * Original by Dave Pare	1984
 * Ported & improved
 *      by Matt Crawford	1985
 *
 * routines to initialize and watch the communications sockets
 * for new players, "searchwho" and "sdata" requests.
 *
 * Copyright (c) 1984
 *
 * $Log:	ipc.c,v $
 * Revision 2.1  85/04/10  17:31:13  matt
 * Major de-linting and minor restructuring.
 * 
 * Revision 1.6  85/02/24  22:50:47  matt
 * Make the select() polls into real polls by setting the timeouts
 * to zero.  This cuts down on context switches and speeds the game
 * up IMMENSELY!
 * 
 * Revision 1.5  85/02/11  14:59:28  matt
 * Tell searchwho that game is in GUTS mode
 * 
 * Revision 1.4  85/02/09  23:48:52  matt
 * Eliminated the dependence on the value of the mask after
 * select() times out.  Use the return value to distinguish
 * a timeout!!
 * 
 * Revision 1.3  84/07/08  17:03:41  matt
 * Added Log
 * 
 * Revision 1.2  84/07/07  18:20:50  matt
 * Put new->p_speed into host byte order after reading from client
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "defines.h"
#include "structs.h"

static struct message message;
extern int sock;

/*
 * newplayer checks to see if there is a new player waiting
 * on one of the communication sockets.  If there is some input
 * waiting, newplayer() checks to see if it's a request from
 * "searchwho", or if it's an actual player requesting to
 * enter the game.
 *
 * newplayer() returns the new player, or a NULL if no new player
 * was found.
 */

t_player *newplayer(mask)
int mask;
{
	extern	void	core_dump();
	extern	char	*strcpy();
	extern	t_player	*startup();
	extern	t_player	player[NPLAYER];
	extern	int	errno,
			nobody,
			gutsflag;
	extern	struct	timeval	tim_inp;
	register	t_player	*p,	*np;
	t_file	*new;
	long	lbuf[1024];	/* Try to force alignment */
	char	*buf = (char *)lbuf;
	int	nfds,
		namelen,
		i;

	i = msgrcv(sock, &message, sizeof message, 1, mask ? 0 : IPC_NOWAIT);
#ifdef NOTDEF
	if ( i != -1 ) {	/* Special Request */
		switch ( *buf ) {
		case 'w':	/* "who's playing search?" */
			buf[0] = NULL;
			for (p=player; p < &player[NPLAYER]; p++) {
				if (p->status.alive == FALSE)
					continue;
				if (*buf == NULL)
					sprintf(buf, "%s  player   points\n",
					 gutsflag?"*game in GUTS mode*\n":"");
				sprintf(buf,"%s\n%-16s %-5d", buf,
						p->plname,p->points);
			}
			if (buf[0] == NULL) {
				(void)strcpy(buf, "nobody playing");
				nobody++;
			}
			write(newsock, buf, strlen(buf));
			errlog("searchwho request\n");
	        break;
		case '#':	/* "dump the player list" (top secret) */
			for (p=player; p < &player[NPLAYER]; p++)
				if (p->status.alive == TRUE)
					write(newsock, (char *)p,
						sizeof(t_player));
			errlog("sdata request\n");
	        break;
		default:	/* garbage! */
			write(newsock, "What?\n", 6);
			errlog("garbage request\n");
	        break;
		}
		shutdown(newsock, 2);
		close(newsock);
		return 0;
	} else
#endif NOTDEF
	if (i == -1) return 0;
	new = (t_file *)message.text;
	new->uid = message.ident;
	if ((np = startup(new)) == NULL) {
		errlog("startup returned NULL in newplayer()\n");
		return 0;
	}
	return np;
}


/*
 * initsocks initializes the two "new player" communications
 * sockets.  It returns 1 if everything is ok, and 0 if not.
 * Argument is passed in host byte order.
 */

int initsocks()
{
	int i;

	/*
	 * bind and listen at the local socket "/tmp/slock"
	 */
	if (access("/tmp/SEARCH", 0)) {
	    printf("File doesn't exists: /tmp/SEARCH\n");
	    return 0;
	}
	sock = getid();
	if (sock == -1) {
	    perror("initsockets: got none");
	    return 0;
	}
	return 1;
}

getid()
{
    int key = ftok("/tmp/SEARCH", 0);
    if (key == -1) return -1;
    return msgget(key, IPC_CREAT|IPC_EXCL|0777);
}

/*
 * read input from the socket
 *
 */

int pinput(p)
register t_player *p;
{
	extern	int	errlog();
	extern	void	done();
	extern	int	errno;
	register	int	i = 0;
	int	nfds;

	/*
	 * poll the socket for input
	 */
	nfds = msgrcv(sock, &message, sizeof message, p->uid, IPC_NOWAIT);
	if (nfds < 0) {
		return 0;
	}
	i = strlen(message.text);
	strncpy (p->inputq, message.text, QSIZE);

	/*
	 *  If player request to die -- clean up after him.
	 */
	if (i == 0) {
		errlog("PINPUT: Player died the hard way\n");
		done(p);
		return -1;
	}

	/*
	 *  Make an acknowledge on this message.
	 */
	message.mtype = p->uid + 2;
	message.text[0] = 1; message.text[1] = 0;
	if (msgsnd(sock, &message, 8+1, 0) == -1)
		errlog("PINPUT: Msgsnd returned -1.\n");

	p->ninput = i;
	p->pinputq = p->inputq;
	return i;
}
