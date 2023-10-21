#include <stdio.h>
#include <signal.h>
/*
 * Loopback test.   This program forks a copy of rvi and connects it to
 * a local ``ed'' program.
 */

main(argc, argv)
int argc;
char **argv;
{
	int in[2], out[2];
	char ibuf[12], obuf[12];

	if (argv[1] == NULL)
		argv[1] = "";

	pipe(in);
	pipe(out);

	sprintf(ibuf, "%d", in[1]);
	sprintf(obuf, "%d", out[0]);
	signal(SIGPIPE, SIG_IGN);

	if (fork() == 0) {
		close(in[0]);
		close(out[1]);
		execlp("rvi", "rvi", obuf, ibuf, argv[1], "-l", NULL);
		execl("/u/aek/bin/rvi", "rvi", obuf, ibuf, argv[1], "-l", NULL);
		perror("exec rvi");
		_exit(1);
	}

	close(in[1]);
	close(out[0]);

	close(0);
	dup(in[0]);
	close(1);
	dup(out[1]);

	execlp("ed", "ed", NULL);
}
