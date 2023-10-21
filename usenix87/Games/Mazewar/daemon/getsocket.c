#ifndef lint
static char rcsid[] = "$Header: getsocket.c,v 1.1 84/08/25 17:04:51 lai Exp $";
#endif

#include <sys/types.h>

#ifdef PLUS5
#include "../berk/btypes.h"
#include "../berk/uio.h"
#include "../berk/socket.h"
#include "../berk/in.h"
#else
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

/*
 * Set up, initialize and return a socket ready for dgram communication
 */
getsocket(port)
	int port;
{
	struct sadr_in sin;
	int s;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		syserr("can't set up AF_INET, SOCK_DGRAM socket");
		exit(1);
	}

	bzero(&sin, sizeof sin);

	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = INADDR_ANY;

	if (bind(s, (char *)&sin, sizeof sin) < 0) {
		syserr("can't bind AF_INET, port=%d, INADDR_ANY to socket=%d",
			port, s);
		exit(1);
	}

	return (s);
}
