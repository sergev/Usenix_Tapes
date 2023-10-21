#ifndef lint
static char rcsid[] = "$Header: verify.c,v 1.1 84/08/25 17:04:58 lai Exp $";
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

#include <stdio.h>
#ifdef PLUS5
#include "../berk/netdb.h"
#else
#include <netdb.h>
#endif

verify(addr)
	register struct sadr_in *addr;
{
	extern char *inet_ntoa(), *gethname();
	extern char *curtime();
	char *hostaddr;
	char *hostname;

	if (addr == NULL)
		return (0);
	else {
		hostaddr = inet_ntoa(addr->sin_addr.s_addr);
		hostname = gethname(addr->sin_addr.s_addr);

		syslog("%s \"%s\" (%s) port #%d on %s\n",
			"Request:",
			hostname,
			hostaddr ? hostaddr : "???",
			ntohs(addr->sin_port),
			curtime());

		if (addr->sin_family != AF_INET)
			return (0);
		else
			return (1);
	}
}

char *
gethname(hostaddr)
	long hostaddr;
{
	struct hostent *hp;
	static char bad[] = "???";

	hp = gethostbyaddr(&hostaddr, sizeof(hostaddr), AF_INET);
	if (hp == NULL) {
		syserr("gethostbyaddr: 0x%x", hostaddr);
		return(bad);
	}
	
	return(hp->h_name);
}
