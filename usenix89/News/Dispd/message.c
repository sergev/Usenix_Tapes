#include "dispd.h"
#include <pwd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
main(argc,argv)
char **argv;
{
	struct passwd *pw;
	int s,n;
	struct sockaddr_in sin;
	struct hostent *hp;
	struct servent *sp;
	char text[200], host[10], mess[200];
	char *ttyname();
	(void) gethostname(host, 10); /* error in  4.2 llib-lc here ??? */
	n = 1;
	if ((argc > 1) && (argv[1][0] == '-')) {
		n = 3;
		if ((argc < 3) || (argv[1][1] != 'u')) {
			fprintf(stderr,"usage -u %s username [message]\n", argv[0]);
			exit(1);
		}
		if ((pw = getpwnam(argv[2])) == 0) {
			fprintf(stderr, "Unknown user\n");
			exit(1);
		}
	}
	else if ((pw = getpwuid(getuid())) == 0) exit(1);
	
	for ( ; n < argc; n++) {
		(void) strcat(mess, argv[n]);
		(void) strcat(mess, " ");
	}
	(void) sprintf(text,"%s %s %s",pw->pw_name,host,mess);
	if ((hp = gethostbyname(HOST)) == 0) {
		perror("cant find host");
		exit(1);
	}
	if ((sp = getservbyname("display", "udp")) == 0) {
		perror("can't find service");
		exit(1);
	}
	if ((s = socket(AF_INET, SOCK_DGRAM,0)) == -1) {
		perror("socket");
		exit(1);
		}
	sin.sin_family = AF_INET;
	sin.sin_port = sp->s_port;
	bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
	if (sendto(s, text, 100, 0, (struct sockaddr *)&sin, sizeof(sin)) ==  -1) {
		perror("sendto");
		exit(1);
	}
	
}

