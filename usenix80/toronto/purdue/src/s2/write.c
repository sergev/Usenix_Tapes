#

/*
 *	write - send message to another terminal
 *
 *	converted from assembly 77/10/27.
 *		- HDS
 *
 *	Minor cosmetic and operational fixes
 *		- TGI	[ 12/30 08:05 ]
 */

#define PERM_BIT	02
#define	SIGINT	2
#define	SIGQIT	3
#define	RAW	040

char	dev[]	"/dev/tty?";
char	stbuf[64];
char name[] "/dev/tty?";
int mflag 0;

main(argc, argv)
char **argv;
{
	char chr, *nam, buf[8], who[16];
	int fd, fnd;
	register a, i;

	for (i = 0; i < 8; i++)
		buf[i] = ' ';
	if (argc < 2 || argc > 3) {
		write(2, "usage:  write <user-name> [tty]\n", 32);
		exit();
	}
	fstat(1, stbuf);
	name[8] = ttyn(1);
	if ((stbuf[4] & PERM_BIT) == 0) {
		mflag=1;
		if(chmod(name,0602)){
			write(2,"can't change permission\n",24);
			exit(1);
		}
	}
	nam = argv[1];
	for (i = 0; argv[1][i] && i < 8; i++)
		buf[i] = argv[1][i];
	if ((fd = open("/etc/utmp", 0)) < 0) {
		write(2, "can't open /etc/utmp\n", 21);
		exit();
	}
	if (argc < 3) {
		while ((a = read(fd, who, 16)) > 0) {
			fnd = 1;
			for (i = 0; i < 8 && who[i] != ' '; i++)
				fnd = fnd & (buf[i] == who[i]);
			if (fnd)
				sendto(who[8]);
		}
	} else {
		while ((a = read(fd, who, 16)) > 0) {
			if (who[8] == *argv[2]) {
				fnd = 1;
				for (i = 0; i < 8 && who[i] != ' '; i++)
					fnd = fnd & (buf[i] == who[i]);
				if (fnd)
					sendto(who[8]);
				else
					break;
			}
		}
	}
	while (*nam)
		write(2, nam++, 1);
	write(2, " not logged in", 14);
	if (argc > 2) {
		write(2, " at tty", 7);
		write(2, argv[2], 1);
	}
	write(2, "\n", 1);
}

sendto(tty)
char tty;
{
	register col;
	int i, a, wh, flag, fd, save[3], tbuf[3], t[2], *p, v;
	char x, st;

	dev[8] = tty;
	stat(dev, stbuf);
	if (getuid() != 0)
		if ((stbuf[4] & PERM_BIT) == 0) {
			write(2, "Permission denied\n", 18);
			exit();
		}
	if ((fd = open(dev, 1)) == -1) {
		write(2, "can't open tty", 14);
		write(2, &tty, 1);
		write(2, "\n", 1);
		exit();
	}
	signal(SIGINT, 1);
	signal(SIGQIT, 1);
	gtty(0, save);
	gtty(0, tbuf);
	tbuf[2] =| RAW;
	stty(0, tbuf);
	if ((st = ttyn(0)) == 'x')
		if ((st = ttyn(2)) == 'x')
			write(2, "Can't get tty number\n", 21);
	getpw(getuid(), stbuf);
	time(t);
	p = localtime(t);

	write(fd, "\07", 1);
	v = p[2] / 10 + '0';
	write(fd, &v, 1);
	v = p[2] % 10 + '0';
	write(fd, &v, 1);
	write(fd, ":", 1);
	v = p[1] / 10 + '0';
	write(fd, &v, 1);
	v = p[1] % 10 + '0';
	write(fd, &v, 1);
	write(fd, ":", 1);

	write(fd, " Message from ", 14);
	for (i = 0; stbuf[i] != ':'; i++)
		write(fd, &stbuf[i], 1);
	write(fd, " at tty", 7);
	write(fd, &st, 1);
	write(fd, "\n", 1);
	write(1, "\07", 1);
	col = 0;
	while (read(0, &x, 1) == 1 && x != '\04')
		if (x == '!' && col == 0) {
			stty(0, save);
			if (fork() == 0) {
				signal(SIGINT, 0);
				signal(SIGQIT, 0);
				execl("/bin/sh", "sh", "-t", 0);
				write(2, "can't execute /bin/sh\n", 22);
				exit();
			}
			wait();
			stty(0, tbuf);
			write(1, "!\n", 2);
		} else {
			write(fd, &x, 1);
			col++;
			if (x == '\n' || x == '\r')
				col = 0;
		}
	write(fd, "EOT\n", 4);
		stty(0, save);
		if(mflag)
			chmod(name,0600);
		exit(1);
	exit();
}
