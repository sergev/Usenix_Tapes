char *lock "/case/lpd0/lock";

main(argc, argv)
char *argv[];
{
	int fd, pid;

	if (argc > 2) {
		printf("Usage: lpdkill\n");
		exit(8);
	}
	if (argc > 1)
		lock[9] = *argv[1];
	if ((fd = open(lock, 0)) < 0) {
		printf("Daemon not running\n");
		exit(8);
	}
	if (read(fd, &pid, 2) != 2 || pid == 0) {
		printf("Bad lock file\n");
		exit(8);
	}
	if (kill(pid, 9) >= 0) {
		printf("Killed\n");
		if (unlink(lock) < 0)
			printf("Could not unlink\n");
	} else {
		printf("Could not kill\n");
	}
}
