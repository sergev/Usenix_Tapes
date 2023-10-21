/* safe - Mike Blackwell - Aug 1986 */
/* safe.c - safeguard a program: kill it after `n` seconds if its still alive */

#include <signal.h>
#include <stdio.h>

/* delay constants */
#define MINDELAY	10
#define MAXDELAY	60*60
#define DEFDELAY	60
#define DLYMSG		"safe: <delay> must be between %d and %d seconds\n"
#define WARNING		"\007\007safe: killing process %d in %d seconds\n"
#define BELL		"\007\007"
#define USAGE		"Usage:\n\tsafe -d <delay> -b -w <warning>\n"

int pid;

main(argc,argv)
int argc;
char *argv[];
{
	int done(),maim();
	unsigned int delay = DEFDELAY;
	unsigned int warndelay = 0;
	int opt;
	extern char *optarg;
	extern int optind;
	short int bell = 0;

	while ((opt = getopt(argc,argv,"d:bw:")) != EOF) {
		switch (opt) {
		case 'd':
			sscanf(optarg,"%d",&delay);
			if (delay < MINDELAY || delay > MAXDELAY) {
				fprintf(stderr,DLYMSG,MINDELAY,MAXDELAY);
				exit(-1);
			}
			break;
		case 'b':
			bell = 1;
			break;
		case 'w':
			sscanf(optarg,"%d",&warndelay);
			break;
		case '?':
			fprintf(stderr,USAGE);
			exit(-1);
			break;
		}
	}

	if (argv[optind] == NULL) {
		fprintf(stderr,"safe: no program to execute\n");
		exit(1);
	}

	if (warndelay >= delay) {
		fprintf(stderr,
	  "safe: delay less than warning time, no warning will be given\n");
		warndelay = 0;
	}

	if (delay > warndelay) {
		delay -= warndelay;
	}

	pid = fork();
	if (pid == -1) {
		fprintf(stderr,"safe: Can't fork\n");
		perror("safe");
		exit(-1);
	}
	if (pid == 0) {		/* child process */
		if (execvp(argv[optind],&argv[optind]) == -1) {
			perror("safe");
			exit(-1);
		}
	} else {		/* parent - monitor and kill */
		signal(SIGCLD,done);
		signal(SIGINT,maim);
		sleep(delay);
		if (warndelay != 0) {
			if (bell)
				fprintf(stderr,BELL);
			else
				fprintf(stderr,WARNING,pid,warndelay);
			sleep(warndelay);
		}
		maim();
		exit(1);
	}
}

maim()
{
	signal(SIGCLD,SIG_IGN);
	kill(pid,SIGKILL);
	fprintf(stderr,"safe: Process %d killed \n",pid);
	exit(1);
}

done()
{
	fprintf(stderr,"safe: Process terminated\n");
	exit(0);
}
