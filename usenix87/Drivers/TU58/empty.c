#ifdef NEEDEMPTY
#ifdef VENIX

/*
 * VENIX empty call - check if tty has any characters in input queue
 */

empty(fd)
int fd;
{
	struct sgttyb sg;

	ioctl(fd, TIOCQCNT, &sg);
	return (sg.sg_ispeed == 0);
}

full(fd)
int fd;
{
	struct sgttyb sg;

	ioctl(fd, TIOCQCNT, &sg);
	return sg.sg_ispeed;
}

#else
#ifdef	BSD

/*
 * empty(fildes) - For BSD systems
 * Is the tty or pipe empty ? (Will a read() block)
 */

empty(fd)
int fd;
{
	long nchar;
	int f;

	f = ioctl(fd, FIONREAD, &nchar);
	if (f == -1)
		return -1;
	if (nchar)
		return 0;
	return 1;
}
#else
	/* Need to write empty() for USG or rewrite using MINTIME */
#endif
#endif
#endif
