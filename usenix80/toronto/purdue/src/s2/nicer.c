/*
 * nicer - do a nice on a running job
 *
 * nicer [ -n ] pid1 [ pid2 pid3 ... pidn ]
 *
 * The priority bias word (u_nice) of process pid will be
 * set to n.  The priority is the same as in the "nice"
 * command. Negative priorities (nicer --20) may be used
 * for more favorable scheduling.  If -n is omitted, 0
 * is used; this effectively clears the effect of a previous
 * nice or nicer function on the process.  
 *
 *
 * G. Goble 4/1/77
 */

main(argc,argv)
int argc;
char *argv[];
{
	int pri;
	int pid;

	if(argc == 1)
		usage();
	if(argc == 2)  {
		if(argv[1][0] == '-')
			usage();
	}
	while(--argc) {
		argv++;
		if(**argv == '-') {
			pri = atoi(++argv[0]);
			continue;
		}
		enxp(atoi(*argv), pri);
	}
}

enxp(pid, pri)
{
	if(nicer(pid, pri))
		printf("nicer: %l not found\n", pid);
}

usage()
{
	printf("usage: nicer [ -n ] pid1 [ pid2 pid3 ... pidn ]\n");
}
