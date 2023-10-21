#ifndef lint
static char rcsid[] = "$Header: init.c,v 1.1 84/08/25 17:04:52 lai Exp $";
#endif

#ifdef PLUS5
#include <sys/types.h>	/*RMM*/
#include <fcntl.h>	/*RMM*/
#endif
#include <sys/param.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <signal.h>

#include "../h/defs.h"
#include "../h/struct.h"
#include "../h/extern.h"

/*ARGSUSED*/
init(argc, argv)
	int argc;
	char **argv;
{	
	extern int ghostbuster();
	program = *argv;

	for(argc--, argv++; argc > 0; argc--, argv++) {
		if (**argv == '-') {
			(*argv)++;
			switch (**argv) {
#ifdef DEBUG
			case 'd':
				debug++;
				break;
#endif
			case 'u':
			default:
				printf("usage: %s [ -d ]\n", program);
				exit(0);
			}
		}
	}

#ifdef DEBUG
	if (!debug)
#endif
		background();

	if (initfds() < 0) {
		fputs("Cannot initialize daemon file descriptors.\n", stderr);
		exit(1);
	}

#ifdef DEBUG
	if (!debug)
#endif
		voidtty();

	ear = getsocket(SERV_PORT);

	initsigs();

	login(program);

	initstructs();

	ualarm(RMMGHOSTCHECK,ghostbuster);
}

/* 
 * Background the server
 */
background()
{
	switch(fork()) {
		case 0:
			break;
		case -1:
			perror("fork");
			exit(1);
		default:
			exit(0);
	}
}

/*
 * Initialize new file descriptors / pointers.
 */
initfds()
{
	extern FILE *errfp;
	register int fd;
	long clock;

	(void) fclose(stdin);
	(void) fclose(stdout);

	for (fd = 0; fd < NOFILE; fd++)
		if (fd != 2)
			(void) close(fd);

	fd = open(LOGFILE, O_WRONLY | O_CREAT, 0644);
	if (fd < 0) {
		fputs("Can't open ", stderr);
		perror(LOGFILE);
		return (-1);
	}

	if (fd != 0) {
		close(0);
		if (dup(fd) != 0) {
			fputs("Can't dup2 on ", stderr);
			perror(LOGFILE);
			return (-1);
		}

		(void) close(fd);
	}

	errfp = fdopen(0, "w");
	if (errfp == NULL) {
		fprintf(stderr, "Can't open %s: No more file pointers\n",
			LOGFILE);
		return (-1);
	}
	if (time(&clock) >= 0)
		fprintf(errfp, "---- Daemon started at %s", ctime(&clock));

	(void) fclose(stderr);

	return (0);
}


/*
 * Void all tty associations.
 */
voidtty()
{
	register int tty;

	tty = open("/dev/tty", O_RDWR);
	if (tty >= 0) {
	/*	(void) ioctl(tty, TIOCNOTTY, (char *)0); 	~RMM*/
		(void) close(tty);
	}
}

initsigs()
{
	extern int nice_exit();
	extern int ghostbuster();
	register int sig;

#ifdef DEBUG
	if (!debug)
#endif
		for (sig = 1; sig < NSIG; sig++)
			(void) signal(sig, SIG_IGN);

	(void) signal(SIGHUP, nice_exit);
	(void) signal(SIGINT, nice_exit);
}


initstructs()
{
	register int i;

	/*
	 * initially, all slots are free
	 */
	for (i = 0; i < MAXPLAYER; i++) {
		players[i].u_flag = 0;
	}
}

login(daemon)
	char *daemon;
{

	syslog("%s(%d): starting up at %s\n", daemon, getpid(), curtime());
}
