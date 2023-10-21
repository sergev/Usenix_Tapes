#ifndef lint
static char rcsid[] = "$Header: getsocket.c,v 1.1 84/08/25 17:11:14 chris Exp $";
#endif

#include <sys/param.h>

#ifdef PLUS5
#include <sys/types.h>
#include "../berk/btypes.h"
#include "../berk/uio.h"
#include "../berk/socket.h"
#include "../berk/in.h"
#else
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include "../h/defs.h"
#include "../h/struct.h"
#include "../h/extern.h"

/*
 * Set up, initialize and return a socket ready for dgram communication
 */
getsocket()
{
	struct sadr_in sin;
	int s;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		(void) syserr("can't set up AF_INET, SOCK_DGRAM socket");
		exit(1);
	}

	bzero((caddr_t)&sin, sizeof sin);

	sin.sin_family = AF_INET;
	sin.sin_port = 0;			/* give my any socket */
	sin.sin_addr.s_addr = INADDR_ANY;

	if (bind(s, (struct sadr *)&sin, sizeof sin) < 0) {
		perror("bind");
		exit(1);
	}

#ifdef DEBUG
	if (debug)
	    debugs("Got socket\n");
#endif
	return (s);
}
