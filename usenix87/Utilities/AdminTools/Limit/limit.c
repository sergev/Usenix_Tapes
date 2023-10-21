/*
 * USAGE -- limit file args
 *
 * SYNOPSIS
 *
 * file is a file to exec
 * args are the arguments for the exec'd program
 *
 * Limit constantly watches the load average and if the load average
 * gets very high will renice its process group to higher values.
 *
 * Note that this program is dependent on such Bezerkeley ideas
 * as "load average", and "renice".
 * 
 * AUTHOR
 *	David Herron
 *	University of Kentucky, Computer Science
 */


#include <stdio.h>
#include <nlist.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>

struct nlist avenrun[] = {
	{ "_avenrun" },
	{ (char *)0 }
};

extern char **environ;

int pid;	/* Process id of child being monitored */
int lastpr;


#define NAMELIST "/vmunix"
#define CHECK_TIME 10	/* Time between load average checks in seconds */

#define MUCHTOOHIGH	6.0
#define TOOHIGH		5.0
#define HIGH		4.0
#define MODERATE	3.0
#define LIGHT		2.0
#define NONEXISTANT	1.0
#define NOLOAD		0.0


main(argc,argv)
int argc;
char *argv[];
{
	void check();
	void leave();
	void set();
	extern char *rindex();
	double avg[3];
	char buf[256];
	char *s;

	if (argc < 2) {
		fprintf(stderr, "Usage: limit file [args]\n");
		exit(1);
	}
	loadav(avg);
	lastpr = 200;
	if((pid=fork()) == 0) {
		strcpy(buf, argv[1]);
		s = rindex(argv[1], '/');
		argv[1] = ++s;
		setuid(getuid());
		setgid(getgid());
		execve(buf, &argv[1], environ);
		sprintf(buf, "Unknown program: %s\n", argv[1]);
		perror(buf);
		exit(1);
	}
	setuid(0);
	setgid(0);
	signal(SIGCHLD, leave);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	check();
	wait(0);
}

void leave() { exit(0); }

void check()
{
	double avg[3];
	double t;

	signal(SIGALRM, check);
	loadav(avg);
	t = avg[1];
	if (t > MUCHTOOHIGH)
		set(17);
	else if(t > TOOHIGH)
		set(15);
	else if(t > HIGH) 
		set(13);
	else if(t > MODERATE) 
		set(10);
	else if(t > LIGHT) 
		set(5);
	else if (t > NONEXISTANT)
		set(0);
	else 
		set(-2);
	alarm(CHECK_TIME);
}

void set(prio)
int prio;
{
	static int pgrp = 0;

	if (prio == lastpr) 
		return;
	if (!pgrp) 
		pgrp = getpgrp(pid);
	setpriority(PRIO_PGRP, pgrp, prio);
	/* setpriority(PRIO_PROCESS, pid, prio); */
	lastpr = prio;
}

loadav(avg)
register double *avg;
{
	register int kmem;

	if ((kmem = open("/dev/kmem", 0)) < 0)
		goto bad;
	nlist(NAMELIST, avenrun);
	if (avenrun[0].n_type == 0) {
		close(kmem);
bad:
		avg[0] = 0.0;
		avg[1] = 0.0;
		avg[2] = 0.0;
		return;
	}

	lseek(kmem, (long) avenrun[0].n_value, 0);
	read(kmem, (char *)avg, 3*sizeof(double));
	close(kmem);
}
