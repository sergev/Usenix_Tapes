
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>

static char	*ProgName;

/*
 * ypreset --
 *
 * Every once in a while the Sun 2.3 yp server gets hung, and all attempts to
 * do yp lookups hang indefinitely.  Clients complain "yp: server not
 * responding for domain DOMAIN; still trying".  So far, the only way I have
 * found to "reset" the yp server is to "rlogin" to it from some client
 * (or host) that does not use the afflicted file server for yellow pages
 * service.  The server resets as soon as the rlogin connection is
 * established.  Therefore this program takes a numeric internet spec as
 * an argument (so that no yp lookups need to be done), establishes a tcp
 * connection to the rlogin port on the server, then closes the connection.
 * I know this behavior is wierd, but it works.
 *
 * Written by Eric Negaard <negaard@spam.istc.sri.com>, Apr 30, 1987.
 * I hearby give permission for anyone to do anything that they want to with
 * this code. . .
 */

main (argc, argv)
    int				 argc;
    char			*argv[];
{
    int				 fd;
    static struct in_addr	 defaddr;
    static struct sockaddr_in	 sin;

    if ((ProgName = rindex (*argv, '/')) != (char *)0)	{
	ProgName += 1;
    } else	{
	ProgName = *argv;
    }

    if (argc < 2)	{
	printf ("Usage: %s host-addr\n", ProgName);
	exit (1);
    }
 
    /*
     * Must build the hostent structure by ourselves.  Using gethostbyname
     * or gethostbyaddr would cause a yellow pages lookup, and thus would hang
     * the program.
     */
    defaddr.s_addr = inet_addr (argv[1]);
    if ((int)defaddr.s_addr == -1)	{
	fprintf (stderr, "%s: `%s': Invalid address\n", ProgName, argv[1]);
	exit (1);
    }

    /* Open the channel to the remote host */
    printf ("[%s]\n", argv[1]);
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)	{
	perror ("socket");
	exit (1);
    }

    /* Bind the address to the socket */
    bzero ((char *)&sin, sizeof (sin));
    if (bind (fd, (struct sockaddr *)&sin, sizeof (sin)) < 0)	{
	perror ("bind");
	exit (1);
    }

    /*
     * Must use the constant IPPORT_LOGINSERVER defined in <netinet/in.h>
     * because using getservbyname ("login", "tcp") would cause a yellow
     * pages lookup, and we would be stuck.
     */
    sin.sin_family = AF_INET;
    sin.sin_port = htonl (IPPORT_LOGINSERVER);
    bcopy ((char *)&defaddr, (char *)&sin.sin_addr, sizeof (defaddr));

    /* Connect to the remote login server */
    if (connect (fd, (struct sockaddr *)&sin, sizeof (sin)) < 0)	{
	perror ("connect");
	exit (1);
    }
    
    /* Now close it and exit, the server should now be "unstuck" */
    (void)shutdown (fd, 2);
    (void)close (fd);
    exit (0);
}
