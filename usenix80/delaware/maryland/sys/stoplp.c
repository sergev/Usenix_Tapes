#define STOPSIG 15
#define LOCK    "/usr/lpd/lock"
#define READ    0

main() {
register int lock_fd;
int pid;

if ((lock_fd = open(LOCK, READ)) < 0) {
	write(2, "Printer daemon not active.\n", 27);
	exit(1);
	}

if (read(lock_fd, &pid, 2) < 2) {
	write(2, "Error in lock file.\n", 20);
	exit(1);
	}

if (kill(pid, STOPSIG) < 0) {
	write(2, "Can't interrupt daemon.\n", 24);
	exit(1);
	}
}
