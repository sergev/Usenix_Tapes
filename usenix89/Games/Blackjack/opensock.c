#include "defs.h"

extern int s;
extern struct sockaddr_in sockaddr;

/*
 * Open main socket and set s to it.
 */
void opensock()
{
	struct hostent *host;
#ifdef ROOTPRIV
	struct servent *dealer;
#endif
	char hostnm[SLEN];

	(void) gethostname(hostnm, SLEN);
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)  {
		perror("socket");
		exit(1);
	}
	if ((host = gethostbyname(hostnm)) == NULL)  {
		perror("gethostbyname");
		exit(1);
	}
	bzero((char *) &sockaddr, sizeof (sockaddr));
	bcopy(host->h_addr, (char *) &sockaddr.sin_addr, host->h_length);
	sockaddr.sin_family = AF_INET;
#ifdef ROOTPRIV
	if ((dealer = getservbyname(SERVICE, PROTO)) == NULL)  {
		fputs("blackjack: service not found\n", stderr);
		exit(1);
	}
	sockaddr.sin_port = htons(dealer->s_port);
#else
	sockaddr.sin_port = htons(PORT);
#endif
/*
 * Allow reuse of local addresses.  Speeds up reinvocation of dealer.
 */
	(void) setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) 0, 0);
	if (bind(s, (char *) &sockaddr, sizeof(sockaddr)) < 0)  {
		perror("bind");
		exit(1);
	}
	if ((listen(s, 5)) == -1)  {
		perror("listen");
		exit(1);
	}
}
