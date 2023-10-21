#
/*
 *      Setup terminal for logon
 *
 *
 * This program will fork off a copy of itself
 * (after performing the necessary checks) to
 * run "getty" and "login" to allow use of an
 * inactive terminal.  This is primarily intended
 * for use when UNIX is in single-user mode, but
 * can be used on any terminal which is not
 * logically turned "on".
 *
 * The entry for the terminal in /etc/ttys is
 * changed so that if normal multi-user mode is
 * started, the terminal will not be "on".
 *
 * When the shell started by the "login" terminates,
 * /etc/ttys will be returned to its state prior
 * to the execution of this program and a "kill -1 1"
 * will be performed to start up normal login.  This
 * will not occur if the switches are set to 0173030.
 */

#define	TTYFILE	"/etc/ttys"
#define GETTY   "/etc/getty"
#define SINGLE  0173030

char buf[4], status;
int fd;
int m4 -4;
char *tty "/dev/ttyx";
char arg[2];
char arg2[2];

main(argc,argv)
int argc;
char **argv;
{
	register fd2, i;
	int pid, pid2;

	if (getuid() != 0){
		printf("Not super-user.\n");
		exit();
	}

	if (argc < 2){
		printf("Syntax is %s <char>\n", argv[0]);
		exit();
	}

	if ((fd=open(TTYFILE,2)) < 0){
		printf("Can't open %s\n", TTYFILE);
		exit();
	}

	for(i=0; i < 20; i++) signal(i, 1);     /* Ignore all signals */
	tty[8] = argv[1][0];
	if ((fd2=open(tty, 2)) < 0){
		printf("Can't open %s\n", tty);
		exit();
	}
	close(fd2);

	setup(argv);                        /* Set terminal status */

	if (fork() > 0) exit();         /* Terminate parent process */

	if (pid=fork()){
		if (pid < 0) cleanup();
		while((pid = wait()) > 0) if (pid == pid2) break;
	} else {
		close(0);
		close(1);
		close(2);
		clrtty();               /* Change terminal of process */
		if (dup(dup(open(tty,2))) < 0) exit();
		if (argc == 2) execl(GETTY, "-", arg, 0);
			else {
				arg2[1] = 0;
				arg2[0] = argv[2][0];
				execl(GETTY, "-", arg, arg2, 0);
			}
		exit();
	}

	cleanup();                      /* Reset terminal to normal mode */
}


setup(argv)
char **argv;
{

	while(read(fd, buf, 4) == 4){
		if (buf[1] == argv[1][0]){
			status = buf[0];
			seek(fd, m4, 1);
			buf[0] = '0';
			write(fd, buf, 4);
			arg[0] = buf[2];
			arg[1] = 0;
			return;
		}
	}

	printf("%s not defined in %s\n", argv[1], TTYFILE);
	close(fd);
	exit();
}

cleanup(){

	seek(fd, m4, 1);                /* Restore terminal status */
	buf[0] = status;
	write(fd, buf, 4);
	close(fd);
	if (getcsw() != SINGLE) kill(1,1); /* Signal "init" of changes */
	exit();
}
