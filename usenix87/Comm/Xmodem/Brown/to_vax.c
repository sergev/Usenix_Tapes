#
# include <file.h>
# include <curses.h>
# include <signal.h>

# define	ST_BLK	33
# define	READY	34
# define	OKAY	35
# define	NOT_OK	36
# define	CANCEL	37
# define	CR	13
# define	ctoi(c)	(c - '0' > 9 ? c - '0' - 7 : c - '0')

char filename[64];
int old;
main(argc,argv)
int argc;
char *argv[]; {

	int fd;
	register i;
	register char *ptr, *op;
	char buf[512], seq[4];
	char *index();
	int xsum, lim, old_seq, pid;

	if (!((argc == 2) || (argc == 3))) {
		printf("ILLEGAL NUMBER OF ARGUMENTS.\n");
		printf("SYNOPSIS: to_vax [-t] filename\n");
		exit(-1);
	}

	if (argc == 3) {
		if (strcmp(argv[1],"-t")) {
			printf("bad argument `%s'\n",argv[1]);
			printf("SYNOPSIS: to_vax [-t] filename\n");
			exit(-1);
		}
	}

	strcpy(filename,argv[argc-1]);
	if ((fd = open(filename,O_WRONLY|O_CREAT|O_TRUNC,0644)) < 0) {
		perror(filename);
		exit(-1);
	}

	initscr();
	crmode();		/* set terminal to unbuffered I/O */

	printf("\nPress Alt-F7 to transmit file to VAX.\n");


/*
**	Ignore interrupts from the user.
**	If the user could delete this program the terminal
**	would be left in a undiserable state of mind.
*/
	sigsetmask( -1 );    /* ignore all signals */

	if ((pid = fork()) == 0) {
		sleep(100);
		RESET(fd);
		kill(0, SIGKILL); 	/* Can't wait forever for a READY reply */
	}

	ptr = buf;
	while (1) {  			/* wait for ready character */
		read(0,ptr,1);
		if (*ptr == READY) break;
	}
	kill(pid, SIGKILL);
	wait(0);
	read(0,ptr,1);			/* get the CR from the READY */
	WCHAR(OKAY); WCHAR(CR);

	for(;;) {
input:
		if ((pid = fork()) == 0) {
			i = 0;
			while ( i++ < 10 ) {
				sleep(10);
				WCHAR(NOT_OK); WCHAR(CR);
			}
			sleep(10);
			RESET(fd);
			kill(0, SIGKILL);  	/* kill all processes */
		}
		READ(buf);			/* read data block */
		kill( pid, SIGKILL );  		/* kill child */
		wait(0);

		/* check for 90 character block */
		ptr = index(buf,ST_BLK);
		if (strlen(ptr) != 90) {
			WCHAR(NOT_OK); WCHAR(CR);
			goto input;
		}

		xsum = 0;
		while ( ptr < &buf[86] )  /* compute check sum */
			xsum = (xsum ^ *ptr++) & 0177;
		if ( xsum != atoi(ptr) ) { 	/* check sum not ok */
			WCHAR(NOT_OK); WCHAR(CR);
			goto input;
		}

		ptr = index(buf,ST_BLK) + 1;
		strncpy(seq,ptr,3);
		switch( (lim = atoi(seq)) - old_seq ) {
			case 0:			/* reply garbled, resend reply */
				WCHAR(OKAY); WSTR(seq); WCHAR(OKAY); WCHAR(CR);
				goto input;
			case 1: case -999:      /* sequence number ok */
				old_seq = lim;
				break;
			default:             /* fatal loss of synchronization */
				WCHAR(CANCEL); WCHAR(CANCEL); WCHAR(CANCEL); WCHAR(CR);
				RESET(fd);
				exit(-1);
		}
		
		ptr += 3;
		if (!(strncmp(ptr,"XX",2) && strncmp(ptr,"EE",2))) break;

		if (strncmp(ptr,"HH",2)) {  	/* decode data block */
			lim = ctoi(*ptr) * 10 + ctoi(*(ptr+1));
			op = buf;
			for ( i = 2; i < lim + 2; i += 2 )
				*op++ = ctoi( *(ptr+i) ) * 16 + ctoi( *(ptr+i+1) );
			write(fd, buf, lim/2); 	/* write data block to file */
		}

		WCHAR(OKAY); WSTR(seq); WCHAR(OKAY); WCHAR(CR);
	} /* end while */

	WCHAR(OKAY); WSTR(seq); WCHAR(OKAY); WCHAR(CR);
	nocrmode();
	endwin();
	close(fd);
	if (argc == 3) {
		printf("Deleting carriage returns\n");
		DELCR(filename);
	}
} /* end main */

WCHAR(c)
char c; {   /* write a character to standard output */

	write(1, &c, 1);
}

WSTR(s)
char *s; {   /* write a string to standard output */
	
	write(1, s, strlen(s));
}

READ(s) 
char *s; {
	register char *ptr;

	ptr = s;
	while ( read(0, ptr, 1) > 0 ) {   	/* read characters until CR */
		if (*ptr++ == CR) break;
	}
	*ptr = '\0';
}

RESET(fd) 
int fd; {			/* put terminal back into start up state */

	nocrmode();
	endwin();
	close(fd);
	unlink(filename);
}
