#include <sys/ioctl.h>
#include <fcntl.h>
#include <termio.h>

main(argc, argv)
char **argv; {
	struct termio tbuf;
	int fd;
	char ttypath[80];
	
	if (argc != 2 || (argv[1][0] != '-' && argv[1][0] != '+')) {
		write(2, "usage: dtr [+-]tty\n", 19);
		exit(1);
	}
	strcpy(ttypath, "/dev/");
	strcat(ttypath, &argv[1][1]);
	if ((fd = open(ttypath, O_RDONLY|O_NDELAY)) < 0) {
		write(2, "can't open ", 11);
		write(2, &argv[1][1], strlen(&argv[1][1]));
		write(2, "\n", 1);
		exit(2);
	}
	ioctl(fd, TCGETA, &tbuf);
	tbuf.c_cflag &= ~CBAUD;
	tbuf.c_cflag |= (argv[1][0] == '-'? B0: B300);
	ioctl(fd, TCSETA, &tbuf);
	close(fd);
	exit(0);
}
