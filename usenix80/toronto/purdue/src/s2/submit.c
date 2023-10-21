#

/*
 *	submit nn < file
 *
 *	submit reads a file from standard input, and
 *	inserts it into the appropriate directory providing:
 *	    1)	it hasn't already been submitted,
 *	    2)	it is not past the due-date.
 *
 *	The due-date file has the following format:
 *
 *		1)  A two-digit assignment number, between
 *			00 and 99 (inclusive).
 *		2)  A blank (space) or a tab (ctrl-I).
 *		3)  The last date during which the assignment
 *			is to be accepted.  The assignment
 *			will be accepted up to the midnight
 *			of that date.
 *			The form is MMDDYY immediately
 *			followed by a new-line.
 *
 *	- TGI	[ 10/27 14:00 ]
 */

#define NBYTES	512

char	uname[]	"/usr2/mowle/assgn00/xxxxxxxx";
char	dname[]	"/usr2/mowle/due-dates";

struct {
	char	num1, num0;	/* 2-digit assignment number */
	char	tab;
	char	mon1, mon0;	/* 2-digit month */
	char	day1, day0;	/* 2-digit day */
	char	yr1, yr0;	/* 2-digit year */
	char	nl;		/* new-line */
} dstruct;

struct {
	char	minor;		/* +0: minor device of i-node */
	char	major;		/* +1: major device */
	int	inumber;	/* +2 */
	int	flags;		/* +4: see below */
	char	nlinks;		/* +6: number of links to file */
	char	uid1;		/* +7: high byte of user ID of owner */
	char	uid0;		/* +8: low  byte of user ID of owner */
	char	size0;		/* +9: high byte of 24-bit size */
	int	size1;		/* +10: low word of 24-bit size */
	int	addr[8];	/* +12: block numbers or device number */
	int	actime[2];	/* +28: time of last access */
	int	modtime[2];	/* +32: time of last modification */
};

char	buf[NBYTES];

main(argc, argv)
char **argv;
{
	register n, *p;
	int tvec[2];
	int assgnum;
	int yy, mm, dd;
	int fd;

	if (argc != 2) {
		write(2, "usage: ", 7);
		while (**argv)
			write(2, (*argv)++, 1);
		write(2, " num < file\n", 12);
		exit();
	}
	argv++;
	n = 0;
	while ('0' <= **argv && **argv <= '9')
		n = n * 10 + *(*argv)++ - '0';
	if (**argv || n < 0 || n > 99) {
		write(2, "bad assignment number\n", 22);
		exit();
	}
	assgnum = n;
	uname[17] = n / 10 + '0';
	uname[18] = n % 10 + '0';
	getpw(getuid(), buf);
	for (n = 0; n < 8; n++) {
		if (buf[n] == ':')
			break;
		uname[20 + n] = buf[n];
	}
	uname[20 + n] = 0;
	if (stat(uname, buf) != -1 && (buf->size0 || buf->size1)) {
		write(2, "assignment already submitted\n", 29);
		exit();
	}
	if ((fd = open(dname, 0)) < 0) {
		write(2, "can't find due date for assignment\n", 35);
		exit();
	}
	do {
		if (read(fd, &dstruct, sizeof dstruct) !=
		    sizeof dstruct) {
			write(2, "no due date for assignment\n", 27);
			exit();
		}
		n = (dstruct.num1 - '0') * 10 + dstruct.num0 - '0';
	} while (n != assgnum);
	close(fd);
	time(tvec);
	p = localtime(tvec);
	p[4]++;
	mm = (dstruct.mon1 - '0') * 10 + dstruct.mon0 - '0';
	dd = (dstruct.day1 - '0') * 10 + dstruct.day0 - '0';
	yy = (dstruct.yr1  - '0') * 10 + dstruct.yr0  - '0';
	if (p[5] > yy || p[4] > mm || p[3] > dd) {
		write(2, "due date was ", 13);
		write(2, &dstruct.mon1, 1);
		write(2, &dstruct.mon0, 1);
		write(2, "/", 1);
		write(2, &dstruct.day1, 1);
		write(2, &dstruct.day0, 1);
		write(2, "/", 1);
		write(2, &dstruct.yr1, 1);
		write(2, &dstruct.yr0, 1);
		write(2, " -- assignment not entered\n", 27);
		exit();
	}
	if ((n = read(0, buf, NBYTES)) <= 0) {
		write(2,"empty assignment file -- nothing submitted\n",
		    43);
		exit();
	}
	if ((fd = creat(uname, 0600)) < 0) {
		write(2, "can't create assignment file\n", 29);
		exit();
	}
	do {
		write(fd, buf, n);
	} while ((n = read(0, buf, NBYTES)) > 0);
	write(2, "assignment submitted\n", 21);
}
