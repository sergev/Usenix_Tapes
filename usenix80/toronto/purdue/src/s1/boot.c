#
/*
 * Boot - boot up system
 *
 * boot [-s]
 *
 * if -s is present, a "sync" will not be performed
 */


int bootstrap[] {
	000403,		/*  br 10 in loc 0 */
	000000,
	000000,		/* trap to 4 causes start at 0 abs */
	000340,		/* spl 7 (new ps) in loc 6 */
	0105737,	/* tstb @#176700 (wait rp04 rdy) */
	0176700,
	0100375,	/* bpl .-4 (rp04 busy) */
	000005,		/* reset */
	012706,		/* mov #2,sp  M9301-YC aborts if sp > 0136000 */
	000002,
	000137,		/* jmp @#173000 */
	0173000,		/* hardware boot program */
	0000000
	};

int crash[] {
	000000,		/* this addr is fetched when cpu halts */
	000000,
	000010,		/* trap to 4 causes start at 10 abs */
	000340,		/* spl 7 (new ps) in loc 6 */
	0105737,	/* tstb @#176700 (wait rp04 rdy) */
	0176700,
	0100375,	/* bpl .-4 (rp04 busy) */
	000005,		/* reset */
	005000,		/* clr r0 */
	005037,		/* clr ps */
	0177776,
	000137,		/* jmp PS */
	0177776
	};

main(argc,argv)
int argc; char *argv[];

{
	int fd; int *oddaddr;
	int pid;
	int fd2;
	char *bootp;

	bootp = &bootstrap;
	if((getuid()) && (ttyn(0) != '8')) {
		printf("Boot: not super-user\n");
		exit(1);
	}
	nice(-128);	/* raise priority */
	pid = fork();	/* fork off job to lock out cpu */
	if(pid == 0) {
		nice(-110);
		while(1);
	}
	nice(-128);	
	if((fd=open("/dev/kmem",2)) < 0) {
		printf("Can't open /dev/kmem for write\n");
		kill(pid,9);
		exit(1);
	}
	seek(fd, 0, 0);
	if(argc == 1) {

	/*
	 * Create a junk file "/down" on the root to force the
	 * super-block modified flag to be set so it will be
	 * written out (with the time) on the following sync().
	 * /down also indicates the system went down gracefully.
	 */

		if((fd2=creat("/down",0644)) < 0) 
			printf("Can't create /down\n");
		write(fd2,0,512);	/* 1 block of garbage */
		close(fd2);
		delay();
		sync();
		delay();
		sync();
	}
	delay();	/* no clock to sleep on */

	/*
	 * Temp kludge until better shutdown is written.
	 * If program name is not boot, then just crash system.
	 */

	if (*argv[0] != 'b')
		bootp = &crash;

/* Copy boot program into locations 0 - 12 ABS */
	write(fd, bootp, sizeof bootstrap); 
/* Enter bootstrap program via a Trap 4 (odd address) */
	oddaddr = 1;
	*oddaddr = 1;
	printf("Something went wrong - boot failed\n");
	exit();
}

delay()
{
	int i,j;
	i = j = 0;
	while(--i)
		j = j * j * j * j * j; /* burn up cpu time */
}
