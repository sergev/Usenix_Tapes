#
# include <curses.h>
# include <signal.h>
# include <file.h>

# define	ST_BLK	33		/* ! */
# define	READY	34		/* " */
# define	OKAY	35		/* # */
# define	NOT_OK	36		/* $ */
# define	CANCEL	37		/* % */
# define	CR	13		/* \r */
# define	pad "    "

int seq_num, old, flags;
char buf[95], seq[4], filename[64];

main(argc,argv)
int argc;
char *argv[]; {

	int fd;
	register i;
	register char *ptr;
	char *ADDCR();
	char c, chunck[45];
	char *MK_SEQ(), *MK_START(), *itos();
	int lim, pid;
	int first = 1;

	flags = argc;
	if (!((argc == 2) || (argc == 3))) {
		printf("ILLEGAL NUMBER OF ARGUMENTS.\n");
		printf("SYNOPSIS: to_pc [-t] filename\n");
		exit(-1);
	}

	if (argc == 2) strcpy(filename,argv[1]);  /* don't put in CR's */
	else { 
		if (strcmp(argv[1],"-t")) {
			printf("bad argument `%s'\n",argv[1]);
			printf("SYNOPSIS: to_pc [-t] filename\n");
			exit(-1);
		}
		printf("Adding carriage returns, wait .......\n");
		strcpy(filename,ADDCR(argv[2]));
	}

	if ((fd = open(filename,O_RDONLY,0644)) < 0) {
		perror(filename);
		exit(-1);
	}

	printf("\nPress Alt-F6 to transmit file to PC.\n");

	initscr();
	crmode();	/* set terminal to unbuffered I/O */

/*
**	Ignore interrupts from the user.
**	If the user could delete this program the terminal
**	would be left in a undiserable state of mind.
*/
	sigsetmask( -1 );    /* ignore all signals */

	if ((pid = fork()) == 0) {   /* child sends ready prompt */
		sleep(8); i = 0;
		strcpy(buf,"\"\r");
		while (1) {
			write(1, buf, 2);
			sleep(2);
			if ( i++ == 30 ) break;  /* not receiving ready character */
		}
		RESET(); 	    /* restore terminal to normal state */
		kill(0,SIGKILL);    /* kill all processes */
	}

	for(;;) {  		/* wait for ready character */
		READ(buf);	/* if not received in 68 sec's, program terminates */
		if (buf[0] == OKAY) break;
	}

	kill(pid,SIGKILL);  /* received ok response, so kill child */
	wait(0);

	seq_num = 0; strcpy(chunck,argv[argc-1]); lim = strlen(chunck);
	do {			 /* make 90 character block */

		ptr = MK_START();			/* make start of block */
		if (first) {			/* header record */
			strcpy(ptr,"HH");
			strcat(ptr,chunck);
			ptr = lim + &buf[6];
			first = 0;
		} else {
			if (lim < 5) strcat(ptr,"0");
			strcat(ptr,itos( 2 * lim ));
			ptr = &buf[6];
			for (i = 0; i < lim; i++) {
				c = (chunck[i] & 0xf0) / 16 + '0';
				*ptr++ = c > '9' ? c + 7 : c;
				c = (chunck[i] & 0xf) + '0';
				*ptr++ = c > '9' ? c + 7 : c;
			}
		}

		strcat(ptr,MK_SEQ(MK_XSUM(ptr)));

		WRITE();
		WAIT(chunck);
		ERR_CHECK(chunck);

	} while ( (lim = read(fd,chunck,40)) > 0 );

	ptr = MK_START();
	strcat(ptr,"EE");
	ptr = &buf[6];
	strcat(ptr,MK_SEQ(MK_XSUM(ptr)));
	WRITE();
	READ(chunck);
	ERR_CHECK(chunck);

	RESET();

	close(fd);
}

WAIT(s)
char *s; {
	int pid;
	register i;

	if ((pid = fork()) == 0) {
		i = 1;
		while (i++ < 10) {
			WRITE();
		}
		sleep(10);
		LEAVE();
		kill(0,SIGKILL);    /* kill all processes */
	}

	READ(s);
	kill(pid,SIGKILL);          /* received response, kill child */
	wait(0);
}


MK_XSUM(s) 
char *s; {
	register xsum;
	register char *ptr;

	ptr = s;
	while ( ptr < &buf[86] ) *ptr++ = ' ';   /* pad buffer with blanks */
	xsum = 0; *ptr = '\0'; ptr = buf;
	while ( *ptr )  			 /* compute check sum */
		xsum = (xsum ^ *ptr++) & 0177;
	return(xsum);
}

char *MK_START() {
	register char *ptr;

	ptr = buf;
	*ptr = ST_BLK; *(ptr + 1) = '\0';
	seq_num = ++seq_num % 1000;
	strcpy(seq,MK_SEQ(seq_num));
	strcat(ptr,seq);
	return( &buf[4] );       /* return address to record type */
}

ERR_CHECK(resp)
char *resp; {
	char ok[6];
	int count = 1;

	while (*resp == NOT_OK) {
		if (count++ > 10) {     /* tried 10 times so exit */
			LEAVE(); exit(0);
		}
		WRITE();
		WAIT(resp);
	}

	strcpy(ok,"#"); strcat(ok,seq); strcat(ok,"#");
	count = 1;
	while (strncmp(ok,resp,5)) {
		/* used to be a literal control X */
		if (*resp == '\030') { /* user terminated transfer Alt-F6 */
			RESET(); exit(0);
		}
		if (*resp == CANCEL) { /* programs not in synch */
			RESET(); exit(0);
		}  
		if (count++ > 10) {    /* gave up, tried ten times */
			LEAVE(); exit(0);
		}
		WRITE();
		WAIT(resp);
	}
}


LEAVE() {
	register char *ptr;

	ptr = MK_START();
	strcat(ptr,"XX");
	ptr = &buf[6];
	strcat(ptr,MK_SEQ(MK_XSUM(ptr)));
	WRITE();
	RESET();
}


char *MK_SEQ(i)
int i; {
	char *itos();
	static char s[3];

	if ( i < 100 ) strcpy(s,"0");
	else s[0] = '\0';
	if ( i < 10 ) strcat(s,"0");
	strcat(s,itos(i));
	return( s );
}

READ(s)
register char *s; {
	
	char c;
	while (read( 0, s++, 1) > 0) {      /* read characters until a CR is read */
		c = *(s-1);
		if ((c == '\n') || (c == CR)) break;
	}
	*s = '\0';
}

WRITE() {

	buf[89] = CR; buf[90] = '\0';
	write( 1, buf, 90);
}

RESET() {

	if (flags == 3) unlink(filename);
	endwin();
}

char *itos(i1)
int	i1;
{
	register char	*a;
	register int	i;
	static char	buf[10];

	i = i1;
	if (i < 0)
		i = -i;

	a = &buf[sizeof buf - 1];
	*a-- = '\0';
	do
	{
		*a-- = i % 10 + '0';
		i /= 10;
	} while (i);
	if (i1 < 0)
		*a-- = '-';

	a++;
	return (a);
}
