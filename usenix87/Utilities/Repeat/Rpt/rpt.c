/*
 * rpt - repeat command much like csh but works on both sh and csh
 *
 * By Bradley Smith , Sep 1984
 * {ihnp4,uiucdcs,cepu}!bradley!brad
 * Bradley University
 * Text Processing
 * Peoria, IL  61625
 */
#include <stdio.h>
#include <signal.h>

int	pid;
main(argc, argv)
char *argv[];
int argc;
{
	register int i, j;
	char *c;
	int status[2], inpfd, swi, ret;
	int onintr();


	swi = 0;
	if(argv[1][0] == '-') {	/* input file */
		c = argv[1];
		c++;
		swi = 1;
		argv++;
		argc--;
	}
		
	if((swi== 0) && (argc < 3)) {
		fprintf(stderr,"Usage: %s [-inputfile] count command\n",
			argv[0]);
		exit(4);
	}
	signal(SIGINT, onintr);
	signal(SIGQUIT, onintr);
	signal(SIGKILL, onintr);
	++argv;
	--argc;
	i = atoi(*argv);
	argv++;
	--argc;
	for(j = 0; j < i; j++) {
		pid = fork();
		if(pid == 0) {	/* child process */
			signal(SIGINT,SIG_DFL);
			signal(SIGQUIT,SIG_DFL);
			signal(SIGKILL,SIG_DFL);
			if(swi==1)  {	/* check on input */
				if((inpfd=open(c,0))== (-1))
					fprintf(stderr,"Can't open '%s'\n", c);
				else {
					close(0);
					if((ret=dup(inpfd))) {
 					  fprintf(stderr,"Error %d on dup\n",
						ret);
					}
				}
			}
			run( argv, argc);
			_exit(2);
		}
		while((wait(status)) != pid)
			;
	}
	exit(0);
}
run(argv, argc)
char **argv;
int argc;
{
	char **cp;
	char cmd[5120],*cx;

	cp = argv;
	cx = cmd;

	while(argc > 0) {
		sprintf(cx,"%s ", *cp);
		cx = strlen(cmd) + cmd;
		cp++;
		--argc;
	}
	execl("/bin/sh","sh","-c",cmd,0);
		
}
onintr()
{
	sleep(2);
	fprintf(stderr,"\nInterrupt...\n");
	exit(1);
}
