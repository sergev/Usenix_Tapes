#
/* lpd.c - line printer daemon written in c
 *
 * May 28th, 1980 - Fred Blonder
 */

#define LP      "/dev/lp"
#define LOCK    "/usr/lpd/lock"
#define LPDIR   "/usr/lpd"
#define STOPSIG 15
#define DF      ('d' | ('f' << 8))
#define WRITE   1
#define READ    0
#define BUFSIZ  400
#define BLOCKSZ 512

char buf[BUFSIZ], data_buf[BLOCKSZ];

int lock_fd, pid, lp_fd, lpdir_fd, file_fd, proto_fd, top_of_form 1;

struct {
	int     d_inode;
	char    d_name[14];
	} dbuf;

struct {
	int     d_inode;
	int     d_i_name;
	};

main() {
register int x;
int (stop)();

signal(1, 1);
signal(2, 1);
signal(3, 1);
signal(STOPSIG, stop);

if ((stat(LOCK, buf) == 0) || ((lock_fd = creat(LOCK, 0444)) < 0))
	exit(0);

/* check for any spooled work */
if ((lpdir_fd = open(LPDIR, READ)) < 0)
	done();

do {
	if (read(lpdir_fd, &dbuf, 16) < 16)
		done();
	} while ((dbuf.d_inode == 0) || (dbuf.d_i_name != DF));

/* yes, there's something there */

if (fork() > 0)
	exit(0);

pid = getpid();
write(lock_fd, &pid, 2);

retry:

aclose();

while (((lp_fd = open(LP, WRITE)) < 0)
	|| (chdir(LPDIR) < 0)
	|| ((lpdir_fd = open(LPDIR, READ)) < 0)) {
		aclose();
		sleep(10);
		if (stat(LOCK, buf))
			done();
	}

loop:   /* look in directory for work */

seek(lpdir_fd, 0, 0);

do {
	if (read(lpdir_fd, &dbuf, 16) < 16)
		done();
	} while ((dbuf.d_inode == 0)
		|| (dbuf.d_i_name != DF)
		|| ((proto_fd = open(dbuf.d_name, READ)) < 0));

/* found a prototype file */

loop1:

for (;;) {

	if ((x = readnl(proto_fd, buf, BUFSIZ)) < 1) {
		eloop1:

		seek(proto_fd, 0, 0);

		while ((x = readnl(proto_fd, buf, BUFSIZ)) > 0)
			if (*buf == 'U') {
				buf[--x] = 0;
				unlink(&buf[1]);
				}

		close(proto_fd);
		unlink(dbuf.d_name);
		goto loop;
		}

	buf[--x] = 0;

	switch (*buf) {

		case 'F':
			/* don't do form feed first time through */
			if (top_of_form)
				top_of_form = 0;
			else
				write(lp_fd, "\014", 1);

		case 'B':
			if ((file_fd = open(&buf[1], READ)) < 0)
				break;

			/* copy */
			while ((x = read(file_fd, data_buf, BLOCKSZ)) > 0)
				write(lp_fd, data_buf, x);

			close(file_fd);
			break;

		case 'L':
		case 'U':
		default:
		;
		}
	}
}

stop() {

close(file_fd);
signal(STOPSIG, stop);
}

aclose() {
register int fd;

for (fd = 0; fd < 12; fd++)
	close(fd);
}

done() {

unlink(LOCK);
exit(0);
}
