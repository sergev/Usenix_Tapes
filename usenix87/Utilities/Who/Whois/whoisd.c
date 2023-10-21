/*
**  A "Whois" server, TCP Port 43 (RFC 812)
**
**  Copyright, 1985, Richard E. $alz.  Permission is granted to the public
**  to copy and use this software without charge, provided that this notice
**  and any statement of authorship are reproduced on all copies.  The author
**  makes no warranty expressed or implied, nor assumes any liability or
**  responsibility for the use of this software.
*/

#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>


/* Handy shorthands. */
#define SIZE		256			/* This is reasonable	*/
#define SOCK(Socket)	(struct sockaddr *)&(Socket)	/*  For lint	*/
#define DIE(s)		perror(s), exit(1);	/* Say bye-bye		*/


/* Global variables. */
struct hostent		*Host;
struct servent		*Service;
struct sockaddr_in	 Socket = { AF_INET };
int			 Size;
char			 Buff[SIZE];
char			 Command[SIZE];
char			 LOSE_MSG[] = "Sorry; can't\n";


/* Linked in later. */
extern int		 errno;
extern FILE		*popen();
extern char		*strcpy();
extern char		*strcat();


Wait()
{
    union wait	Status;

    while (wait3(&Status, WNOHANG, (struct rusage *)NULL) > 0)
	;
}


main()
{
    register FILE		*Stream;
    register int		 In;
    register int		 Client;
    register int		 i;

    /* Who are we? */
    if (getuid())
	DIE("whoisd (not super user)");

    /* Where are we? */
    if (gethostname(Buff, sizeof Buff) < 0)
	DIE("whoisd (gethostname)");
    if ((Host = gethostbyname(Buff)) == NULL)
	DIE("whoisd (gethostbyname)");

    /* What are we trying to do? */
    if ((Service = getservbyname("whois", "tcp")) == NULL)
	DIE("whoisd (tcp/whois unknown service)");

#ifndef	DEBUG
    if (fork())
	exit(0);

    /* Open up the console for error logging, but ignore signals
       that come from it. */
    (void)freopen("/dev/console", "w", stderr);
    if ((i = open("/dev/tty", 2)) >= 0)
    {
	(void)ioctl(i, TIOCNOTTY, 0);
	(void)close(i);
    }
    (void)close(0);
    (void)close(1);
#endif	DEBUG

    /* Creation and bondage (my, how kinky). */
    Socket.sin_family = Host->h_addrtype;
    Socket.sin_port = Service->s_port;
    bcopy(Host->h_addr, (char *)&Socket.sin_addr, Host->h_length);
    if ((In = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	DIE("whoisd (socket)");
    if (bind(In, SOCK(Socket), sizeof Socket) < 0)
	DIE("whoisd (bind)");

    if (listen(In, 5) < 0)
	DIE("whoisd (listen)");
    (void)signal(SIGCHLD, Wait);

    /* Service loop. */
    for ( ; ; )
    {
	/* Wait for something to come in. */
	Size = sizeof Socket;
	if ((Client = accept(In, SOCK(Socket), &Size)) < 0)
	{
	    if (errno != EINTR)
		perror("whoisd (accept)");
	    (void)sleep(1);
	    continue;
	}

	/* Spawn off a kid to handle it.  Should check for fork failure... */
	if (fork())
	{
	    (void)close(Client);
	    continue;
	}

	/* Shut down stuff the kid doesn't need, read the request. */
	(void)signal(SIGCHLD, SIG_IGN);
	if ((i = read(Client, Buff, sizeof Buff)) < 0)
	    DIE("whoisd (read)");

	/* Cons up the command line. */
	(void)strcpy(Command, "/usr/ucb/finger");
	if (i)
	{
	    /* Strip off the \r\n, append the argument. */
	    Buff[i - 2] = '\0';
	    (void)strcat(Command, " ");
	    (void)strcat(Command, Buff);
	}

	if ((Stream = popen(Command, "r")) == NULL)
	{
	    perror("whoisd (popen)");
	    (void)write(Client, LOSE_MSG, sizeof LOSE_MSG - 1);
	}
	else
	{
	    register FILE	*Reply;

	    Reply = fdopen(Client, "w");
	    while (fgets(Buff, sizeof Buff, Stream))
		fprintf(Reply, "%s", Buff);
	    (void)fflush(Reply);
	    if (pclose(Stream) == EOF)
		perror("whoisd (fclose)");
	}

	if (close(Client) < 0)
	    DIE("whoisd (close)");
	exit(0);
    }
}
