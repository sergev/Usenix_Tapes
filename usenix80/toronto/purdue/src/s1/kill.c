#

/*
 * kill -- kill (send signal) to process
 *
 * syntax:
 *
 *	kill [-sig] pid ...
 *
 *	sig is kill (9) by default.
 *	pid = 0 causes signal to be sent to all
 *		processes with same controlling tty.
 */

#include "/usr/sys/param.h"
#define ERROR	-1

int	signo	9;	/* signal - default is kill */
int	pid;		/* pid to whom signal is to be dispatched */
int	sigerr;		/* bad signal conversion */
int	piderr;		/* bad pid conversion */

main(argc, argv)
char **argv;
{
	extern fout;

	fout = 2;
	if (argc < 2) {
		printf("usage: %s [-signo] pid ...\n", *argv);
		flush();
		exit(ERROR);
	}
	while (--argc) {
		if (**++argv == '-') {
			++*argv;
			signo = 0;
			while ('0' <= **argv && **argv <= '9')
				signo = signo * 10 + *(*argv)++ - '0';
			sigerr = **argv? 1 : 0;
		} else {
			pid = 0;
			while ('0' <= **argv && **argv <= '9')
				pid = pid * 10 + *(*argv)++ - '0';
			piderr = **argv? 1 : 0;
			if (piderr || sigerr) {
				printf("kill: bad digit in %s\n",
				    sigerr? "signal" : "pid");
				continue;
			}
			if (signo <= 0 || signo >= NSIG) {
				printf("kill: bad signal: %d\n",signo);
				sigerr++;
			}
			if (kill(pid, signo) < 0)
				printf("kill: %l not found\n",pid);
		}
	}
	flush();
}
