#ifndef lint
char rcsid[] = "$Header: calldaemon.c,v 1.1 84/08/25 17:11:10 chris Exp $";
#endif

#include <stdio.h>
#include <sys/param.h>
#ifdef PLUS5
#include <signal.h>
#include <sys/types.h>
#include "../berk/time.h"
#include "../berk/btypes.h"
#include "../berk/uio.h"
#include "../berk/socket.h"
#include "../berk/in.h"
#include "../berk/netdb.h"
#else
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include <sys/time.h>
#include <errno.h>

#include "../h/defs.h"
#include "../h/struct.h"
#include "../h/extern.h"

struct sadr_in earsin;				/*XXX*/

extern int errno;

#define bit(i)			(1 << (i))
#define BLOCK			((struct timeval *)0)
#define MASK(sig)		(1 << ((sig) - 1))
#define SIGHOLD(signo) 		(sigblock(MASK(signo)))
#define SIGRELSE(signo) 	(sigsetmask(sigblock(0) &~ MASK(signo)))

/*ARGSUSED*/
initearsin(host, port)
	char *host;
	int port;
{
	struct hostent *hp;

	/* Get host entry */
	if ((hp = gethostbyname(host)) == NULL) {
		perror(host);
		exit(1);
	}	

	bzero((caddr_t)&earsin, sizeof (earsin));

	bcopy(hp->h_addr, (caddr_t)&earsin.sin_addr, hp->h_length);

	earsin.sin_family = /*hp->h_addrtype*/AF_INET;
	earsin.sin_port = htons(port);
#ifdef DEBUG
	if (debug)
	    debugs("Got ear address\n");
#endif
}

/*
 * send all of the user structs to user at this slot
 */
calldaemon(player, host)
	struct user *player;
	char *host;
{
	struct packet p;
	struct timeval timeout;
	int readmask = (1 << ear);

	bzero((caddr_t) &p, sizeof(struct packet));

#ifdef	NOASGNSTRUCT
	bcopy(player, &p.p_data.pu_user, sizeof(struct user));
#else
	p.p_data.pu_user = *player;
#endif
	p.p_flag |= P_USER;
	p.p_slot = -1;					/*XXX*/

	/*
	 * Send the daemon our initial user struct
	 */
	if (sendto(ear, (char *) &p, sizeof(p), 0,
		(struct sadr *) &earsin, sizeof(struct sadr_in)) < 0) {
		perror("sendto");
		exit(1);
	}

	timeout.tv_sec = CD_TIMEOUT;
	timeout.tv_usec = 0;

	(void) SIGHOLD(SIGALRM);
	if (select(NOFILE, &readmask, (int *)0, (int *)0, &timeout) < 1) {
		printf("\"%s\": timed out.\n", host);
		quit(0);
	}
	(void) SIGRELSE(SIGALRM);

	/*
	 * The daemon now sends us back OUR user struct, replete with
	 * our slot number
	 */
	if (read(ear, (char *) &p, sizeof(p)) != sizeof(p)) {
		perror("read: our struct from daemon");
		exit(1);
	}

	me = &players[p.p_data.pu_user.u_slot];

	if (p.p_data.pu_user.u_flag == U_BADVERSION) {
		printf("You're playing an old version, bitch to installer!\n");
		quit(1);
	}

	/*
	 * Now the daemon sends us the array of all of the players'
	 * user structs
	 */
	if (read(ear, (char *) players, sizeof(players)) != sizeof(players)) {
		perror("read: array of structs from daemon");
		exit(1);
	}
}
