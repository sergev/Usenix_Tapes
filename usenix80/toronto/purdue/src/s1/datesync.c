/*
 *	datesync -- synchronize date from other net machine
 */


char junk[36];			/* Dummy buffer for "fstat" */
long tvec;			/* Time vector */
int pipefd[2];			/* File descriptors for pipes */
int pidx;

main(argc, argv)
char **argv;
{
	int bombed();

	/* "datesync" must be called with one argument.
	 * If that argument is "-d", it will output the time
	 * in 4-byte raw format on the standard output and
	 * then exit.  This format takes care of the distant
	 * end of the link.  If the argument is not "-d",
	 * datesync assumes that it is the name of a connected
	 * machine and uses that name to call the other machine.
	 */

	if (argc != 2){
		write(open("/dev/ttyD",2), "ERROR\n", 6);
		write(2, "Format is \"datesync host\"\n", 26);
		exit();
	}

	if (cmpar(argv[1], "-d")){
		time(&tvec);
		write(1, &tvec, 4);
		exit();
	}


	/* Set up pipe and check if super-user. */

	if (pipe(pipefd)){
		write(2, "Can't create pipe\n", 18);
		exit();
	}

	if (getuid()){
		write(2, "Not super-user\n", 15);
		exit();
	}


	/* Fork off "csh" to get time from machine specified
	 * by argument 1 to destination machine.  The remote
	 * "datesync" process will then write the time back.
	 */

	switch(pidx = fork()) {

	case -1:write(2, "Can't fork -- try again\n", 24);
		exit();

	case  0:close(pipefd[0]);
		close(1);
		dup(pipefd[1]);
		execl("/bin/csh", "csh", argv[1], "/usr/bin/datesync",
			"-d", 0);
		execl("/usr/bin/csh", "csh", argv[1], "/usr/bin/datesync",
			"-d", 0);
		exit();

	}

	signal(14, bombed);
	close(pipefd[1]);
	alarm(30);
	wait();			/* Await other's termination */
	alarm(0);


	/* Now read other machine's time from pipe and set
	 * system time on this machine.  Then, call up "date"
	 * to print it in a understandable fashion.
	 */

	if (read(pipefd[0], &tvec, 4) < 4){
		write(2, "Pipe read failed\n", 17);
		exit();
	}

	stime(&tvec);
	execl("/bin/date", "date", 0);
	execl("/usr/bin/date", "date", 0);
	exit();
}

cmpar(p, q)
char *p, *q;
{
	while(*p) if (*p++ != *q++) return(0);
	return(*q == 0);
}

bombed()
{

	kill(pidx, 9);
	wait();
	printf("Can't do datesync -- network hung\n");
	exit();
}
