/*
**	receive filename
*/
#
# include <file.h>
# include <curses.h>
# include <signal.h>
# include <time.h>
# include <setjmp.h>

# define	SOH	0x1
# define	NAK	0x15
# define	ACK	0x6
# define	EOT	0x4
# define	CANCEL	0x18

char filename[64];
int retries;		/* retries counter */
int seq_num;		/* sequence number */
int caught();		/* interrupt routine, called if haven't received a
			   block in 1 second */
struct sigvec vec;
jmp_buf env;

main(argc,argv)
int argc;
char *argv[]; {

	int fd;
	char c;
	register int i;

	if (!((argc == 2) || (argc == 3))) {
		printf("ILLEGAL NUMBER OF ARGUMENTS.\n");
		printf("SYNOPSIS: to_vax_x [-t] [-m] filename\n");
		exit(-1);
	}

	if (argc == 3) {
		if (strcmp(argv[1],"-t") && strcmp(argv[1],"-m")) {
			printf("bad argument `%s'\n",argv[1]);
			printf("SYNOPSIS: to_vax_x [-t] [-m] filename\n");
			exit(-1);
		}
	}

	strcpy(filename,argv[argc-1]);
	if ((fd = open(filename,O_WRONLY|O_CREAT|O_TRUNC,0644)) < 0) {
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

	sigsetmask(-1 ^ (1 << (SIGALRM-1)));	/* enable alarm signal */

	c = NAK;
	retries = 0;
	if (setjmp(env)) {
	    write(1,&c,1);
	    if (retries++ > 10) {
		reset(1); 	     /* restore terminal to normal state */
		printf("Can not get reply from transmitter\n");
		exit(0); 
	    }
	}
	
	SET_TIMER(10);
	read(0,&c,1);
	DISABLE_TIMER();

	seq_num = 1; retries = 0;
	for(;;) {

		if (retries == 10) {
			c = CANCEL;
			write(1,&c,1);
			reset(1);
			printf("Terminated after 10 retries\r\n");
			exit(-1);
		}

		switch( c & 0x7f ) {
			case SOH:
				BLOCK(fd);
				break;
			case CANCEL:
				printf("Received CANCEL signal\r\n");
				reset(1);
				exit(-1);
			case EOT:
				c = ACK;
				write(1,&c,1);
				reset(0);
				goto END;
			default:
				sleep(2);	/* wait for a bit */
				noraw();	/* flush input buffer */
				raw();
				c = NAK;
				write(1,&c,1);
				retries++;
				break;
		}
		c = NAK;
		if (setjmp(env)) {
		    write(1,&c,1);
		    if (retries++ > 10) {
			reset(1); 	     /* restore terminal to normal state */
			printf("Can not get reply from transmitter\n");
			exit(0); 
		    }
		}
	
		SET_TIMER(10);
		read(0,&c,1);
	        DISABLE_TIMER();
		
	}
END:

	close(fd);
	if (argc == 3) {
		switch( argv[1][1] ) {
			case 't':
				printf("Deleting carriage returns\n");
				DELCR(filename);
				break;
			case 'm':
				printf("Changing cr's to lf's\n");
				CR2LF(filename);
		}
	}
}

BLOCK(fd) 
int fd; {
	
	register i;
	register char *p;
	char block[131];
	char c;
	int xsum;

	retries = 0;		/* start fresh */

	/* used to be 1 */
	SET_TIMER(10);
	if (setjmp(env)) {
	    noraw();
	    raw();
	    c = NAK;
	    write(1,&c,1);
	    retries++;
	    return;
	} else {
	    i = read(0,block,131);
	    while( i < 131 ) {
	    	i += read(0,&block[i],131-i);	/* data block */
	    }
	    DISABLE_TIMER();
	}
	
	if ( (block[0] & 0xff) != seq_num ) {
		i = (seq_num ? seq_num - 1 : 255);
		if ((block[0] & 0xff) != i ) {
			if ( (block[1] & 0xff) != (255 - seq_num) ) {
				i = 255 - i;
				if ((block[1] & 0xff) == i ) {
					c = NAK;
					write(1,&c,1);
					retries++;
					return;
				}
				c = CANCEL; 	/* loss of synchronization */
				write(1,&c,1);
				reset(1);
				exit(-1);
			}
		}
		retries++;
		goto leave;
	}

	if ( (block[1] & 0xff) != (255 - seq_num) ) {
		c = NAK;
		write(1,&c,1);
		retries++;
		return;
	}
	
	xsum = 0;
	for (i = 0, p = &block[2]; i < 128; i++) xsum += (*p++ & 0xff);
	xsum &= 0xff;
	if ( (block[130] & 0xff) != xsum ) {
		c = NAK;
		write(1,&c,1);
		retries++;
		return;
	}

	write(fd,&block[2],128);
	retries = 0;
	seq_num = ++seq_num % 256;
leave:
	c = ACK;
	write(1,&c,1);
}

reset(remove) 
int remove; {
	
	DISABLE_TIMER();
	sigsetmask(0);
	vec.sv_handler = SIG_DFL;
	vec.sv_mask = vec.sv_onstack = 0;
	sigvec(SIGALRM,&vec,(struct sigvec *)0);
	noraw(); echo();
	endwin();
	if (remove) unlink(filename);
}
	

caught() {
	longjmp(env,1);
}

SET_TIMER(secs) 
int secs; {
	struct itimerval val;

	val.it_value.tv_sec = secs;
	val.it_value.tv_usec = secs * 1000;
	timerclear(&val.it_interval);
	setitimer(ITIMER_REAL,&val,(struct itimerval *)0);
}

DISABLE_TIMER() {
	struct itimerval val;

	timerclear(&val.it_value);
	setitimer(ITIMER_REAL,&val,(struct itimerval *)0);
}
