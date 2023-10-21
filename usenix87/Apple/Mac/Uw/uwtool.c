#

/*
 *	uwtool
 *
 * Copyright 1985 by John D. Bruner.  All rights reserved.  Permission to
 * copy this program is given provided that the copy is not sold and that
 * this copyright notice is included.
 *
 * Compile: cc -o uwtool -O uwtool.c
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/uio.h>
#include <signal.h>
#include <stdio.h>

#define	NFDS	20			/* max number of file descriptors */

typedef int fildes_t;

extern char *strncpy(), *strncat();
extern char *getenv();

main(argc, argv)
char **argv;
{
	register char *cp;
	register int pid;
	register fildes_t sd;
	auto fildes_t fd;
	char *portal, *shell;
	char tty[32], pty[32], ptysufx[2];
	int lmode;
	struct sgttyb sg;
	struct tchars tc;
	struct ltchars ltc;
	struct iovec iov;
	struct msghdr msg;
	static char ptyidx[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	struct sockaddr sa;

	/*
	 * Close all file descriptors except 0, 1, and 2.
	 */

	for (fd=3; fd < NFDS; fd++)
		(void)close(fd);

	/*
	 * Get the terminal configuration for this tty.
	 */
	(void)ioctl(0, (int)TIOCGETP, (char *)&sg);
	(void)ioctl(0, (int)TIOCGETC, (char *)&tc);
	(void)ioctl(0, (int)TIOCGLTC, (char *)&ltc);
	(void)ioctl(0, (int)TIOCLGET, (char *)&lmode);

	/*
	 * Create a UNIX-domain socket.
	 */

	if (!(portal=getenv("UWIN"))) {
		fprintf(stderr,
		    "You must run %s under the window manager\n", *argv);
		exit(1);
	}

	if ((sd=socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}
	sa.sa_family = AF_UNIX;
	(void)strncpy(sa.sa_data, portal, sizeof sa.sa_data-1);
	sa.sa_data[sizeof sa.sa_data-1] = '\0';


	/*
	 * Obtain a pseudo-tty.  This code isn't very smart about finding one.
	 */

	ptysufx[1] = '\0';
	for (cp=ptyidx; *cp; cp++) {
		ptysufx[0] = *cp;
		(void)strncpy(pty, "/dev/ptyp", sizeof pty-1);
		(void)strncat(pty, ptysufx, sizeof pty-1);
		if ((fd = open(pty, 2)) >= 0)
			break;
	}
	if (fd < 0) {
		fprintf(stderr, "Can't obtain a pseudo-tty for a new window\n");
		exit(1);
	}
	(void)strncpy(tty, "/dev/ttyp", sizeof tty-1);
	(void)strncat(tty, ptysufx, sizeof tty-1);


	/* 
	 * Fork a child process using this pseudo-tty.  Initialize the
	 * terminal modes on the pseudo-tty to match those of the parent
	 * tty.
	 */

	while ((pid=fork()) < 0)
		sleep(5);
	if (!pid) {
		setuid(getuid());  /* in case it is setuid-root by mistake */
		(void)signal(SIGTSTP, SIG_IGN);
		(void)ioctl(open("/dev/tty", 2), (int)TIOCNOTTY, (char *)0);
		if ((fd=open(tty, 2)) < 0)
			_exit(1);
		(void)dup2(fd, 0);
		(void)dup2(0, 1);
		(void)dup2(0, 2);
		for (fd=3; fd < NFDS; fd++)
			(void)close(fd);
		(void)ioctl(0, (int)TIOCSETN, (char *)&sg);
		(void)ioctl(0, (int)TIOCSETC, (char *)&tc);
		(void)ioctl(0, (int)TIOCSLTC, (char *)&ltc);
		(void)ioctl(0, (int)TIOCLSET, (char *)&lmode);
		if (argc == 1) {
			if (!(shell=getenv("SHELL")))
				shell = "/bin/sh";
			execl(shell, shell, (char *)0);
			perror(shell);
		} else {
			execvp(argv[1], argv+1);
			perror(argv[1]);
		}
		_exit(1);
	}


	/*
	 * Pass the file descriptor to the window server and exit.
	 */

	iov.iov_base = pty;
	iov.iov_len = strlen(pty);
	msg.msg_name = (caddr_t)&sa;
	msg.msg_namelen = sizeof sa.sa_family + strlen(sa.sa_data);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_accrights = (caddr_t)&fd;
	msg.msg_accrightslen = sizeof fd;
	if (sendmsg(sd, &msg, 0) < 0) {
		perror("sendmsg");
		exit(1);
	}

	exit(0);
}
