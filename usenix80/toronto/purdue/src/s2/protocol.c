/*
 *      protocol -- run shell from terminal, save input and output in file
 */

int protofd;                            /* Protocol file descriptor */
int to_sh[2], from_sh[2];               /* Pipes */
int in 0;                               /* Input fd */
int out 1;                              /* Output fd */
char buf[512];                          /* I/O buffer */
int pflg 1;                             /* Pause flag */

main(argc, argv)
char **argv;
{
	int resume(), exit();
	register len, pid;

	if (argc < 2){
		write(2, "Syntax is \"% protocol <file>\"\n", 30);
		exit();
	}

	if ((protofd = creat(argv[1], 0644)) < 0){
		write(2, "Can't create protocol file\n", 27);
		exit();
	}

	if (pipe(to_sh) || pipe(from_sh)){
		write(2, "Can't create pipe\n", 18);
		exit();
	}

	signal(19, resume);             /* Synchronizing section */

	if ((pid=fork()) < 0){
		write(2, "Couldn't fork\n", 14);
		exit();
	} else if (!pid){
		close(from_sh[0]);
		close(to_sh[1]);
		close(0);
		close(1);
		close(2);
		dup(to_sh[0]);
		dup(dup(from_sh[1]));
		if (pflg) pause();              /* Wait until parent ready */
		execl("/bin/sh", "sh", 0);
		exit();
	}

	signal(2,1);
	signal(3,1);
	signal(13,exit);                /* Exit if write on broken pipe */
	signal(19,0);                   /* Not really important */

	switch(fork()){
	case -1:write(2, "Second fork failed\n", 19);
		kill(pid,9);
		exit();

	case 0: close(from_sh[0]);      /* Child will do sending */
		close(from_sh[1]);
		close(to_sh[0]);
		out = to_sh[1];
		break;

	default:close(from_sh[1]);      /* Parent will receive */
		close(to_sh[0]);
		close(to_sh[1]);
		in = from_sh[0];
		kill(pid,19);           /* Set up now, so signal first kid */
		break;
	}

	while((len=read(in, buf, 512)) > 0){
		write(protofd, buf, len);
		write(out, buf, len);
	}
}


resume(){
	pflg = 0;
}
