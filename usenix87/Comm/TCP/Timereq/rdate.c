#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>      
#include <netdb.h>

#include <stdio.h>

#define BASE1970	2208988800L	/* difference between Unix time and net time */


main (argc,argv)
int argc;
char *argv[];
{
	int i;

	if (argc == 1) {
		printf("usage: rdate <host1> <host2> <host...>\n");
		exit(1);
	}

	for( i = 1; i < argc; i++ )
		RemoteData(argv[i]);
}

RemoteData(host)
char *host;
{
	struct	hostent *him;		/* host table entry */
	struct	servent *timeServ;	/* sevice file entry */
	struct	sockaddr_in sin;	/* socket address */
	int	fd;			/* network file descriptor */
	long	unixTime;		/* time in Unix format */
	u_char  netTime[4];		/* time in network format */
	int	i;			/* loop variable */
	char	*ctime();

	if ((him = gethostbyname(host)) == NULL) {
		fprintf(stderr, "rdate: Unknown host %s\n", host);
		return(-1);
	}

        if ((timeServ = getservbyname("time","tcp")) == NULL) {
                fprintf(stderr, "rdate: time/tcp: unknown service\n");
                return(-1);
        }

	if ((fd = socket(AF_INET, SOCK_STREAM, 0, 0)) < 0) {
		perror("rdate");
		return(-1);
	}

        sin.sin_family = him->h_addrtype;      
        bcopy(him->h_addr, (caddr_t)&sin.sin_addr, him->h_length);
        sin.sin_port = timeServ->s_port;

        if (connect(fd, (caddr_t)&sin, sizeof(sin), 0) < 0) {
		perror("rdate");
		close(fd);
		return(-1);
	}

	printf("[%s]\t", him->h_name);

	/* read in the response */
	for (i = 0; i < 4; i++)
		if (read(fd, &netTime[i], 1) != 1) {
			perror("rdate");
			close(fd);
			return(-1);
		}

	close(fd);
	unixTime = ntohl(* (long *) netTime) - BASE1970;
	printf("%s", ctime(&unixTime));
}
