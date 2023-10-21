/*
 */
#ifndef lint
char sccsid[] = "@(#) client.c 1.2 85/10/19";
#endif

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<netdb.h>

usage(message)
char *message;
{
	fprintf (stderr, "client:  %s.\n", message);
	fprintf (stderr, "Usage:  client [ -p port ] host command.\n");
	exit(1);
}

main(argc, argv)
int	argc;
char 	**argv;
{
	int 			sock, c;
	register FILE 		*sock_r, *sock_w;
	char 			*host, *service = "server";
	struct sockaddr_in 	sin;
	struct hostent 		*host_p;
	struct servent 		*serv_p;

	if (argc < 3)
		usage("too few arguments");
	while (*++argv && **argv == '-') {
		switch (argv[0][1]) {
		case 'p':
			if ((service = *++argv) == NULL)
				usage("no value for -p option");
			break;
		default:
			usage("unknown option");
			break;
		}
	}
	if ((host = *argv) == NULL)
		usage("not enough arguments");
	host_p = gethostbyname(host);
	if (host_p == NULL) {
		fprintf(stderr, "client: %s: host unknown\n", host);
		exit(1);
	}
	host = host_p->h_name;
	sock = socket(host_p->h_addrtype, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("client: socket");
		exit(2);
	}
	sin.sin_family = host_p->h_addrtype;
	if (bind(sock, &sin, sizeof(sin)) < 0) {
		perror("client: bind");
		exit(3);
	}
	bcopy(host_p->h_addr, &sin.sin_addr, host_p->h_length);
	if ((sin.sin_port = atoi(service)) == 0) {
		serv_p = getservbyname(service, "tcp");
		if (serv_p == NULL) {
			fprintf(stderr,
				"client: %s/tcp: service unknown\n", service);
			exit(4);
		}
		sin.sin_port = serv_p->s_port;
	}
#ifdef DEBUG
	fprintf(stderr, "Connecting to %s...\n", host);
#endif
	if (connect(sock, &sin, sizeof(sin)) < 0) {
		perror("client: connect");
		exit(5);
	}
	sock_r = fdopen(sock, "r");
	sock_w = fdopen(sock, "w");
	if (sock_r == NULL || sock_w == NULL) {
		perror("client: fdopen");
		close(sock);
		exit(1);
	}
	for (fputs(*++argv, sock_w); *++argv != NULL; fputs(*argv, sock_w))
		putc(' ', sock_w);
	fputs("\r\n", sock_w);
	fflush(sock_w);
	if (ferror(sock_w)) {
		perror("client: write");
		exit(1);
	}
	while ((c = getc(sock_r)) != EOF)
		putchar(c);
	exit(0);
}
