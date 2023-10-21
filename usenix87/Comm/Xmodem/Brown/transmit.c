/*
**	transmit filename
*/
# include <curses.h>
# include <file.h>
# include <signal.h>
# include <time.h>

# define	SOH	0x1
# define	NAK	0x15
# define	ACK	0x6
# define	EOT	0x4
# define	CANCEL	0x18

int flags;
char filename[64];

main(argc,argv)
int argc;
char *argv[]; {

	int fd;
	register i;
	register char *p;
	char block[132];
	char c;
	int length, pid;
	int seq_num, xsum;
	int retries;
	struct sigvec vec;
	int caught();

	flags = argc;
	if (!((argc == 2) || (argc == 3))) {
		printf("ILLEGAL NUMBER OF ARGUMENTS.\n");
		printf("SYNOPSIS: to_pc_x [-t] [-m] filename\n");
		exit(-1);
	}

	if (argc == 2) strcpy(filename,argv[1]);  /* don't put in CR's */
	else { 
		switch (argv[1][1]) {
			case 't':
				printf("Adding carriage returns, wait .......\n");
				strcpy(filename,ADDCR(argv[2]));
				printf("DONE\n");
				break;
			case 'm':
				printf("Changing lf's to cr's, wait .......\n");
				strcpy(filename,LF2CR(argv[2]));
				printf("DONE\n");
				break;
			default:
				printf("bad argument `%s'\n",argv[1]);
				printf("SYNOPSIS: to_pc_x [-t] [-m] filename\n");
				exit(-1);
		}
	}

	if ((fd = open(filename,O_RDONLY,0644)) < 0) {
		perror(filename);
		exit(-1);
	}

	initscr();
	raw();			/* set terminal to 8-bit I/O */
	noecho();

/*
**	Ignore interrupts from the user.
**	If the user could delete this program the terminal
**	would be left in a undiserable state of mind.
*/
	vec.sv_handler = caught;
	vec.sv_mask = vec.sv_onstack = 0;
	sigvec(SIGALRM,&vec,(struct sigvec *)0);

	sigsetmask(-1 ^ (1 << (SIGALRM-1)));

	SET_TIMER();

	for(;;) {  		/* wait for NAK */
		read(0,&c,1);	/* if not received in 60 sec's, program terminates */
		if ((c&0x7f) == NAK) break;
	}

	DISABLE_TIMER();

	seq_num = 1;
	p = &block[3];
	while ( (length = read(fd,p,128)) > 0 ) {

		p = block;
		*p++ = SOH;
		*p++ = seq_num;
		*p++ = 255 - seq_num;

		p += length;
		for( i = length; i < 128; i++) *p++ = 0;
		xsum = 0;
		for (i = 3; i < 131; i++) xsum += block[i];
		*p = xsum & 0xff;

		retries = 0;
		do {
			if (retries) sleep(10);

			write(1,block,132);
			SET_TIMER();
			read(0,&c,1);
			DISABLE_TIMER();

			if ( (c&0x7f) == CANCEL ) {RESET(); exit(-1);}
			if (retries++ > 10 ) {
				c = CANCEL;
				write(1,&c,1);
				RESET(); exit(-1);
			}
		} while ( (c&0x7f) != ACK );

		seq_num = ++seq_num % 256;
		p = &block[3];
			
	}

	SET_TIMER();
	do {
		c = EOT;
		write(1,&c,1);
		read(0,&c,1);
	} while ( (c&0x7f) != ACK );
	DISABLE_TIMER();

	RESET();
}

RESET() {

	noraw();
	echo();
	endwin();
	if (flags == 3) unlink(filename);
} /* END */

caught() {
	RESET();
	kill(0,SIGKILL);
}

SET_TIMER() {
	struct itimerval val;

	val.it_value.tv_sec = 60;
	val.it_value.tv_usec = 60 * 1000;
	timerclear(&val.it_interval);
	setitimer(ITIMER_REAL,&val,(struct itimerval *)0);
}

DISABLE_TIMER() {
	struct itimerval val;

	timerclear(&val.it_value);
	setitimer(ITIMER_REAL,&val,(struct itimerval *)0);
}
