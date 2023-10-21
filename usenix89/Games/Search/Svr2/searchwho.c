#ifndef lint
static char rcsid[] = "$Header: searchwho.c,v 2.2 85/08/15 13:37:49 matt Exp $";
#endif
/*
 *
 * searchwho
 *
 * multi-player and multi-system search and destroy.
 *
 * Original by Dave Pare	1983
 * Ported & improved
 *      by Matt Crawford	1985
 *
 * who is playing search
 *
 * Copyright (c) 1983
 *
 * $Log:	searchwho.c,v $
 * Revision 2.2  85/08/15  13:37:49  matt
 * Limit dtabsiz to an int's worth of bits.
 * 
 * Revision 2.1  85/04/10  17:31:55  matt
 * Major de-linting and minor restructuring.
 * 
 * Revision 1.3  85/02/10  02:06:33  matt
 * Allow a port to be specified, as for search itself.
 * 
 * Revision 1.2  85/02/09  23:50:40  matt
 * Eliminated the dependence on the value of the mask after
 * select() times out.  Use the return value to distinguish
 * a timeout!!
 * 
 */

#include <stdio.h>
#include <sys/param.h>		/* includes <sys/types.h> and <signal.h> */
#include <sys/stat.h>
#include <sys/file.h>
#include <sgtty.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "defines.h"
#include "structs.h"


extern	int errno;
int	dtabsiz;			/* how many fd's here? */
int	sock;				/* socket to daemon */

main(argc, argv)
int argc;
char *argv[];
{
	void reset();
	register int    cc;	/* returned from all sorts of calls */
	register int    i;	/* general purpose register */
	register int    sockmask;
	register char   buf[4096];/* misc buffer for i/o */
	int     mask;		/* masks used in select() calls */
	struct timeval  timeout;/* time outs for select calls */
	int     save_mask;	/* don't calculate mask each time */
	int     first_time;	/* for printing header */
	int     junk;
	struct sockaddr loc_addr;/* local socket address */

	dtabsiz = getdtablesize();
 /* 
  * set all the signals
  */
	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGTERM, SIG_IGN);
	(void) signal(SIGQUIT, reset);
	(void) signal(SIGHUP, reset);
	(void) signal(SIGPIPE, reset);
	(void) signal(SIGALRM, reset);
	(void) signal(SIGTSTP, SIG_IGN);
	(void) signal(SIGSTOP, SIG_IGN);
	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("search, socket");
		exit(1);
	}
 /* 
  * connect to the local search daemon thru the lock
  * file (usually /tmp/slock for historical reasons)
  */
	if (connect(sock, &loc_addr, sizeof(loc_addr))) {
		printf("no local search daemon running\n");
		exit(1);
	}
 /* 
  * daemon knows that a 1 byte message 'w' is from "searchwho".
  */
	if (write(sock, "w", 1) != 1) {
		perror("write");
		printf("searchwho: bad connect on socket\n");
		exit(1);
	}
 /* 
  * set up all the stuff for the select loop
  */
	sockmask = 1 << sock;
	save_mask = sockmask;
	timeout.tv_usec = 0L;
	timeout.tv_sec = 3L;
 /* 
  * just in case nobody's there
  */
	alarm(20);
	first_time = 1;
	for (;;) {
		mask = save_mask;
		i = select(dtabsiz, &mask, 0, 0, &timeout);
		if (i < 0) {
			if (errno = EINTR)
				continue;
			perror("select");
		}
	/* 
	 * nope - no data waiting.  select() timed out.
	 */
		if (!i) {
			printf("no data yet, we'll wait a bit longer...\n");
			fflush(stdout);
			continue;
		}
		if (!(sockmask & mask))
			continue;
	/* 
	 * input waiting from the daemon - read it in and
	 * write it to the screen (stdout).
	 */
		alarm (3);
		if (fgets(buf, 80, &_iob[sock]) != NULL) {
		/* 
		 * when you get zero characters from a socket that
		 * had input waiting, it means that the socket was
		 * closed down (from the other side)
		 */
			if (first_time) {
				if (strncmp(buf, "nobody", 6) != 0) {
					printf("    username   points \n");
					first_time = 0;
				} else
					puts(buf);
			} else
				puts(buf);
			fflush(stdout);
		} else
			reset(0);
		alarm(0);
	}
}

/*
 * reset disconnects us from any sockets we've connected to,
 * and shuts everything down nicely.
 */
void reset(signal)
int signal;
{
	char	buf[1024];
	struct	timeval delay;
	int	mask;
	int	i;

	/*
	 * disconnect from our socket (getting rid of the
	 * gunk possibly leftover)
	 */
	mask = 1 << sock;
	delay.tv_sec = 0L;
	delay.tv_usec = 500L;
	shutdown(sock, 1);
	i = select(dtabsiz, &mask, 0, 0, &delay);
	if (i > 0) {
		i = read(sock, buf, sizeof(buf));
		write(1, buf, i);
	}
	shutdown(sock, 2);
	(void) close(sock);
	putchar('\n');
	exit(0);
}
