#define VERSION "sb 1.01 12-21-81"

/*
 * sb.c By Chuck Forsberg
 *
 * A small program for Unix which can send 1 or more
 * files in Batch mode to computers running YAM. (Use "rb" in yam.)
 * Supports the CRC option or regular checksum.
 * There are no messages of any sort (except incorrect usage).
 * sb is derived from yam2.c
 * Uses buffered i/o to reduce the CPU time compared to UMODEM.
 *  USG UNIX (3.0) ioctl conventions courtesy  Jeff Martin
 * 	cc -O -DV7  sb.c -o sb		vanilla Unix version 7
 *	cc -O -DUSG sb.c -o sb		USG (3.0) Unix
 *  Unix is a trademark of Western Electric Company
 */

#include <stdio.h>
#include <signal.h>
#ifdef USG
#include <termio.h>
#include <sys/ioctl.h>
#else
#include <sgtty.h>
#endif

FILE *in;

#define OK 0
#define FALSE 0
#define TRUE 1
#define ERROR (-1)

/* Ward Christensen / CP/M parameters - Don't change these! */
#define ENQ 005
#define CAN ('X'&037)
#define XOFF ('s'&037)
#define XON ('q'&037)
#define SOH 1
#define EOT 4
#define ACK 6
#define NAK 025
#define CPMEOF 032
#define WANTCRC 0103	/* send C not NAK to get crc not checksum */
#define TIMEOUT (-2)
#define ERRORMAX 5
#define RETRYMAX 5
#define WCEOT (-10)
#define SECSIZ 128	/* cp/m's Magic Number record size */

char Lastrx;
char Crcflg;
char firstsec;
#ifdef USG
struct termio oldtty, tty;
#else
struct sgttyb oldtty, tty;
#endif

unsigned updcrc();

main(argc, argv)
char *argv[];
{
	register char *cp;
	register npats=0;
	char **patts;
	int exitcode;
	char xXbuf[BUFSIZ];

	if(argc<2)
		goto usage;
	setbuf(stdout, xXbuf);		
	while (--argc) {
		cp = *++argv;
		if(*cp == '-') {
			while( *++cp) {
				switch(*cp) {
				default:	
					goto usage;
				}
			}
		}
		else if( !npats && argc>0) {
			if(argv[0][0]) {
				npats=argc;
				patts=argv;
			}
		}
	}
	if(npats < 1) {
usage:
		fprintf(stderr,"%s by Chuck Forsberg\n", VERSION);
		fprintf(stderr,"Usage: sb file ...\n");
		exit(1);
	}
#ifdef USG
	(void) ioctl(0, TCGETA, &oldtty);
	tty = oldtty;
	tty.c_lflag &= ~(ECHO | ICANON | ISIG); /* No echo, crlf mapping, INTR
						or QUIT chars, no crlf delays,
						no erase/kill processing */
	tty.c_iflag = IGNBRK;	/* Ignore break, disable parity check, no
				stripping, no crnl mapping or ^S^Q */
	tty.c_oflag = 0;	/* Transparent output, no delays, no crlf
				mapping */
	tty.c_cflag &= ~PARENB;	/* Leave baud rate alone, disable parity
				generation and checking */
	tty.c_cflag |= CS8;	/* Set character size = 8 */
	tty.c_cc[VMIN] = 20;	/* Satisfy reads when this many chars in */
	tty.c_cc[VTIME] = 1;	/* ... or in this many tenths of seconds */
	(void) ioctl(0, TCSETA, &tty);
#else
	ioctl(1, TIOCEXCL, 0);
	ioctl(1, TIOCGETP, &oldtty);
	tty = oldtty;
	tty.sg_flags |= RAW;
	tty.sg_flags &= ~ECHO;
	ioctl(1, TIOCSETP, &tty);
#endif
	if(wcsend(npats, patts)==ERROR) {
		exitcode=128;
		sendline(CAN);
		sendline(CAN);
		sendline(CAN);
		sendline(CAN);
	}
	fflush(stdout);
#ifdef USG
	(void) ioctl(0, TCSBRK, 1);	/* Wait for output to drain */
	(void) ioctl(0, TCFLSH, 1);	/* Flush input queue */
	(void) ioctl(0, TCSETAW, &oldtty);	/* Restore original modes */
	(void) ioctl(0, TCXONC,1);	/* Restart output */
#else
	ioctl(1, TIOCSETP, &oldtty);
	ioctl(1, TIOCNXCL, 0);
#endif
	exit(exitcode);
}




wcsend(argc, argp)
char *argp[];
{
	register n;

	Crcflg=FALSE;
	firstsec=TRUE;
	for(n=0; n<argc; ++n)
		if(wcs(argp[n])==ERROR)
			goto fubar;
	if(wctxpn("")==ERROR)
		goto fubar;
	return OK;
fubar:
	fclose(in);
	sendline(CAN);sendline(CAN);sendline(CAN);
	return ERROR;
}

wcs(name)
char *name;
{
	if((in=fopen(name, "r"))==NULL)
		return ERROR;
	if(wctxpn(name)!= ERROR)
		return wctx();
	else {
		return ERROR;
	}
}


wctxpn(name)
char *name;
{
	register firstch;
	register char *p, *q;
	char lname[SECSIZ];

	if((firstch=readline(400))==TIMEOUT)
		return ERROR;
	if(firstch==WANTCRC)
		Crcflg=TRUE;
	for(p=name, q=lname ; *p; )
		if((*q++ = *p++) == '/')
			q = lname;
	while(q < lname + SECSIZ)
		*q++ = 0;
	if(wcputsec(lname, 0)==ERROR)
		return ERROR;
	return OK;
}

wctx()
{
	int sectnum, attempts, firstch;
	char txbuf[SECSIZ];

	firstsec=TRUE;

	while((firstch=readline(400))!=NAK && firstch != WANTCRC
	  && firstch!=TIMEOUT && firstch!=CAN)
		;
	if(firstch==CAN)
		return ERROR;
	if(firstch==WANTCRC)
		Crcflg=TRUE;
	sectnum=1;
	while(filbuf(txbuf, SECSIZ)) {
		if(wcputsec(txbuf, sectnum)==ERROR) {
			return ERROR;
		} else
			sectnum++;
	}
	fclose(in);
	attempts=0;
	do {
		sendline(EOT);
		purgeline();
		attempts++;
	}
		while((firstch=(readline(100)) != ACK) && attempts < RETRYMAX);
	if(attempts == RETRYMAX)
		return ERROR;
	else
		return OK;
}

wcputsec(txbuf, sectnum)
char *txbuf;
{
	register checksum, wcj;
	register char *cp;
	unsigned oldcrc;
	int firstch;
	int attempts;

	firstch=0;	/* part of logic to detect CAN CAN */

	for(attempts=0; attempts <= RETRYMAX; attempts++) {
		Lastrx= firstch;
		sendline(SOH);
		sendline(sectnum);
		sendline(-sectnum-1);
		oldcrc=checksum=0;
		for(wcj=SECSIZ,cp=txbuf; --wcj>=0; ) {
			sendline(*cp);
			oldcrc=updcrc(*cp, oldcrc);
			checksum += *cp++;
		}
		if(Crcflg) {
			oldcrc=updcrc(0,updcrc(0,oldcrc));
			sendline(oldcrc>>8);sendline(oldcrc);
		}
		else
			sendline(checksum);
		purgeline();

		firstch=readline(100);
		if(firstch==CAN && Lastrx==CAN)
			return ERROR;

		else if(firstch==ACK) {
			firstsec=FALSE;
			return OK;
		}
		else if(firstch==TIMEOUT)
			;
		else {
			if(firstsec && firstch==WANTCRC)
				Crcflg=TRUE;
			for(;;) {
				Lastrx=firstch;

				if((firstch=readline(1))==TIMEOUT)
					break;
				if(firstch==CAN && Lastrx==CAN)
					return ERROR;
			}
		}
	}
	return ERROR;

}


/* fill buf with count chars padding with ^Z for CPM */
filbuf(buf, count)
char *buf;
{
	register c, m;
	m=count;
	while((c=getc(in))!=EOF) {
		*buf++ =c;
		if(--m == 0)
			break;
	}
	if(m==count)
		return 0;
	else
		while(--m>=0)
			*buf++ = CPMEOF;
	return count;
}

sendline(c)
{
	putchar(c);
}

alrm()
{
/* does nothing; actual effect is to give an error return on read */
}


/*
 * This version of readline is not so well suited for
 * reading many characters.  Fortunately, it doesn't
 * have to.
 * timeout is in tenths of seconds
 */
readline(timeout)
{
	char byt;
	register int c;
	fflush(stdout);
	signal(SIGALRM, alrm);
	c = timeout/10;
	if(c==0)
		++c;
	alarm(c);
	if(read(1, &byt, 1)<1)
		return TIMEOUT;
	alarm(0);

	return byt&0377;
}

purgeline()
{
	lseek(1, 0L, 2);
}

/* update CRC */
unsigned updcrc(c, crc)
register c;
register unsigned crc;
{
	register count;

	for(count=8; --count>=0;) {
		if(crc & 0x8000) {
			crc <<= 1;
			crc += (((c<<=1) & 0400)  !=  0);
			crc ^= 0x1021;
		}
		else {
			crc <<= 1;
			crc += (((c<<=1) & 0400)  !=  0);
		}
	}
	return crc;	
}
