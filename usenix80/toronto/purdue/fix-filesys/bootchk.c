#

/*
 * bootstrap file system check and mount
 *
 * bootchk [-r] [-q] [-s] [-h] filesystem [mountsystem mountdirectory]
 *
 * Mike Tilson, Feb. 1977       (Toronto)
 *
 *
 * Filesystem is ichecked and dchecked, and then mounted (if a mount
 * device and mount directory are specified.  typically, the filesystem
 * will be the raw device and the mountsystem will be the block
 * device.) "-r" and "-q" and "-s" are passed along to icheck and dcheck.
 * "-h" flag causes bootchk to hang (inf loop) on unrecoverable problem
 * on a non root filesys. "-h" is always implied for root filesystem.
 * If "-h" is not specified, and unrecoverable filesystem error occurs,
 * the filesystem will not be mounted.
 *
 * -r == repair errors
 * -q == quiet, report only if problems
 * -s == Force freelist update, even if no errors
 * -h == Hang in infinite loop on unrecoverable filesystem problem
 * Various minor mods, G. Goble, Purdue U., April 79.
 *
 */

#define ALLSWELL 0
#define MINORPROBLEM 01
#define PROBLEM 02
#define DISASTER 04
#define ERROR (-1)

#define INTR 2

#define ICHECK "/bin/icheck"
#define DCHECK "/bin/dcheck"
#define MOUNTPROG "/etc/mount"
int rflg;
int sflg;
int hflg;       /* hang on unrecoverable fs error */
int qflg;

done() {
	printf("Boot check interrupted -- It's your funeral\n");
	exit(1);
}

usage() {
	printf("Usage:  bootchk [-r] [-q] [-s] [-h] filesystem [mountsystem mountdirec]\n");
	exit(1);
}

main(argc, argv)
	char **argv;
{
	register int status;

	signal(INTR, done);
	if(--argc==0)
		usage();
	argv++;
	while(argv[0][0]=='-') {
		switch(argv[0][1]){
		case 'h':
			hflg++;
			break;
		case 'r':
			rflg++;
			break;
		case 's':
			sflg++;
			break;
		case 'q':
			qflg++;
			break;
		default:
			usage();
		}
		--argc;
		argv++;
	}
	if(argc!=1 && argc!=3)
		usage();
	status = check(argv[0]);
	if (argc == 1 && sflg && status==0)
		reboot();
	if(status) {
		printf("Please save listing\n");
		if(argc==1) {
			if(status==MINORPROBLEM) {
				if(rflg)
					reboot();
				/* else do nothing for minor problem on root */
			}
			else
				bootabort();
		}
		else {
			if(status!=MINORPROBLEM) {
				printf("%s:  not mounted\n", argv[1]);
				if (hflg)
					bootabort();
				return(1);
			}
		}
	}
	if(argc==3) {
		printf("mounting %s on %s\n", argv[1], argv[2]);
		mount(argv[1],argv[2]);
	}
	return(0);
}

check(file)
	char *file;
{
	register int status;
	register char **v;
	char *av[10];

	v = av;
	*v++ = ICHECK;
	if(rflg)
		*v++ = "-r";
	if(sflg)
		*v++ = "-s";
	if(qflg)
		*v++ = "-q";
	*v++ = file;
	*v++ = 0;
	status = callsys(av);
	if (status & (DISASTER|PROBLEM))
		return(status);
	av[0] = DCHECK;
	status =| callsys(av);
	return(status);
}

mount(dev, dir)
	char *dev, *dir;
{
	char *av[10];
	register char **v;

	v = av;
	*v++ = MOUNTPROG;
	*v++ = dev;
	*v++ = dir;
	*v++ = 0;
	return(callsys(av));
}
callsys(v)
	char *v[];
{
	register int t;
	int status;
	register char *f;

	f = v[0];

	if ((t=fork())==0) {
		execv(f, v);
		printf("Can't exec %s\n", f);
		exit(DISASTER);
	} else
		if (t == -1) {
			printf("Can't fork\n");
			return(DISASTER);
		}
	while(t!=wait(&status))
		;
	if ((t=(status&0377)) != 0) {
		if (t!=INTR)		/* interrupt */
			printf("Fatal error in %s\n", f);
		return(ERROR);
	}
	return((status>>8) & 0377);
}

reboot() {
	printf("ROOT file system patched--please reboot system\n");
	idle();
}

bootabort() {
	printf("Automatic filesystem recovery not possible - get help\n");
	idle();
}

idle() {
	putchar(014);
	for(;;)
		sleep(10);
}
