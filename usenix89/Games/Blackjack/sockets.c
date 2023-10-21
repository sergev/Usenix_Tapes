#include "defs.h"

char *getenv();

/*
 * Make connection to host running the blackjack dealer server,
 * return fd of new socket.
 */
int make_con(servhost)
char *servhost;				/* name of host running server */
{
	int s;
	struct hostent *host;
	struct servent *dealer;
	struct sockaddr_in sockaddr;

	if ((host = gethostbyname(servhost)) == NULL)  {
		perror("gethostbyname");
		exit(1);
	}
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)  {
		perror("socket");
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
	if (connect(s, (char *) &sockaddr, sizeof(sockaddr)) < 0)  {
		perror("connect to dealer");
		exit(1);
	}
	return(s);
}

/*
 * Send host your name and machine (name@machine)
 */
void send_name(s)
int s;					/* socket to talk to host on */
{
	char *name, host[SLEN], buf[SLEN];

	if ((name = getenv("BJ")) == NULL)
		if ((name = getenv("NAME")) == NULL)
			name = getenv("USER");
	(void) gethostname(host, SLEN);
	(void) sprintf(buf, "%s@%s", name, host);
	sockwrite(s, buf);
}
