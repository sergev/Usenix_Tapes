
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


/* Handy shorthands. */
#define SOCK(Socket)	(struct sockaddr *)&(Socket)
#define DIE(s)		perror(s), exit(1)

/* Get the default host to connect to; you may want to change this. */
#define DEFAULT		(gethostname(Name, sizeof Name) < 0 ? "sri-nic" : Name)


/* Global variables. */
char			*Who = "";
char			*Where;
char			 Name[32];
struct sockaddr_in	 Socket;
struct hostent		*Host;
struct servent		*Service;

/* Linked in later. */
extern char		*index();


main(ac, av)
    int			 ac;
    char		*av[];
{
    register FILE	*Stream;
    register char	*p;
    register int	 c;
    register int	 s;

    /* Parse JCL. */
    Where = DEFAULT;
    switch (ac)
    {
	default:
Bad:
	    fprintf(stderr, "usage:\t%s [-h host] user\nor\t%s [user][@host]\n",
			    av[0], av[0]);
	    exit(1);
	case 1:
	    /* default to "@DEFAULT-HOST" */
	    break;
	case 2:
	    /* "user@host" "@host" "user" */
	    Who = av[1];
	    if (p = index(av[1], '@'))
	    {
		*p++ = '\0';
		Where = p;
	    }
	    break;
	case 4:
	    if (strcmp(av[1], "-h"))
		goto Bad;
	    /* "-h host user" */
	    Where = av[2];
	    Who = av[3];
    }

    /* Try to get host and the service. */
    Host = gethostbyname(Where);
    if (Host == NULL)
    {
	fprintf(stderr, "whois: %s: host unknown\n", Where);
	exit(1);
    }
    if ((Service = getservbyname("whois", "tcp")) == NULL)
	DIE("whois (whois/tcp unknown service)");

    /* Creation and bondage (kinky sex?). */
    if ((s = socket(Host->h_addrtype, SOCK_STREAM, 0)) < 0)
	DIE("whois (socket)");
    Socket.sin_family = Host->h_addrtype;
    if (bind(s, SOCK(Socket), sizeof Socket) < 0)
	DIE("whois (bind)");

    /* Connect to the server at the right port. */
    bcopy(Host->h_addr, (char *)&Socket.sin_addr, Host->h_length);
    Socket.sin_port = Service->s_port;
    if (connect(s, SOCK(Socket), sizeof Socket) < 0)
	DIE("whois (connect)");

    /* Send out the request. */
    if ((Stream = fdopen(s, "w")) == NULL)
	DIE("whois (fdopen to service)");
    fprintf(Stream, "%s\r\n", Who);
    (void)fflush(Stream);

    /* Get the response. */
    if ((Stream = fdopen(s, "r")) == NULL)
	DIE("whois (fdopen from service)");
    while ((c = getc(Stream)) != EOF)
	(void)putchar(c);
    (void)fclose(Stream);

    exit(0);
}
