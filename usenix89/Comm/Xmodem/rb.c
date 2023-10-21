#define VERSION "rb 0.05 7-19-82"

/*
 * rb.c By Chuck Forsberg
 *
 * A program for Unix which can receive
 *  files from computers running YAM or MODEM.
 *  If no filename is given, YAM batch mode is assumed.
 *
 *  Supports the CRC option or regular checksum.
 *  Received pathnames containing no lowercase letters will be changed to lower
 *  case unless -u option is given.
 *
 *  Unless the -b (binary) option is given, \r is discarded and
 *  ^Z (which is also discarded) acts as end of file.
 *
 *  Any slashes in the pathname are changed to underscore.
 *  If the raw pathname ends in .MSG, .TXT or .DOC, any existing file will
 *  be appended to rather than replaced. Trailing periods are eliminated.
 *
 *  If the raw pathname ends in
 *   .COM .CMD .DAT .O .REL .PAG .CRL .OBJ .SAV
 *   or .?Q* (squeezed file), or if the parity bit of the first or second bytes
 *   of the file is set,
 *   that file will be received in binary mode.
 *
 *
 * a log of activities is appended to "rblog"
 *
 * rb is derived from yam2.c and sb.c
 * rb should use Unix System III buffered input to reduce CPU time.
 *  USG UNIX (3.0) ioctl conventions courtesy  Jeff Martin
 * 	cc -O -DV7  rb.c -o rb		vanilla Unix version 7
 *	cc -O -DUSG rb.c -o rb		USG (3.0) Unix
 *  Unix is a trademark of Western Electric Company
 */

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#ifdef USG
#include <termio.h>
#include <sys/ioctl.h>
#else
#include <sgtty.h>
#endif

char *substr();
FILE *fout;
FILE *logfile;

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
#define STX 2
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
#define KSIZE 1024	/* record size with k option */

int Lastrx;
int Crcflg;
int Firstsec;
int Eofseen;		/* indicates cpm eof (^Z) has been received */
int totblocks;		/* total number of blocks received */
int errors;

int Batch;
int MakeLCPathname=TRUE;	/* make received pathname lower case */
int Verbose=FALSE;
int Rxbinary=FALSE;	/* receive all files in bin mode */
int Thisbinary;		/* current file is to be received in bin mode */
int blklen;		/* record length of received packets */
char secbuf[KSIZE];
char linbuf[KSIZE];
int lleft=0;		/* number of characters in linbuf */
int llorig;
char *cdq;		/* pointer for removing chars from linbuf */


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

	while (--argc) {
		cp = *++argv;
		if(*cp == '-') {
			while( *++cp) {
				switch(*cp) {
				case 'b':
					Rxbinary=TRUE; break;
				case 'k':
				case 'c':
					Crcflg=TRUE; break;
				case 'u':
					MakeLCPathname=FALSE; break;
				case 'v':
					Verbose=TRUE; break;
				default:
					usage();
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
	logfile=fopen("rblog", "a");
	if(wcreceive(npats, patts)==ERROR) {
		exitcode=0200;
		sendline(CAN);
		sendline(CAN);
		sendline(CAN);
		sendline(CAN);
	}
	fclose(logfile);
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

usage()
{
	fprintf(stderr,"%s by Chuck Forsberg\n", VERSION);
	fprintf(stderr,"Usage: rb [-bcuv]\n");
	exit(1);
}

wcreceive(argc, argp)
char **argp;
{
	if(Batch || argc==0) {
		Crcflg=TRUE; fprintf(stderr, "Receiving in batch mode ");
		for(;;) {
			totblocks=0;
			if(wcrxpn(secbuf)== ERROR)
				goto fubar;
			if(secbuf[0]==0)
				return OK;
			if(wcrx(secbuf)==ERROR)
				goto fubar;
		}
	}
	else
		for(; --argc>=0;) {
			totblocks=0;
			if(wcrx(*argp++)==ERROR)
				goto fubar;
		}
	return OK;
fubar:
	sendline(CAN);sendline(CAN);sendline(CAN);
	if(fout)
		fclose(fout);
	return ERROR;
}


/*
 * Fetch a pathname from the other end as a C ctyle ASCIZ string.
 * Length is indeterminate as long as less than blklen
 * a null string represents no more files
 */
wcrxpn(rpn)
char *rpn;	/* receive a pathname */
{
	purgeline();
	Firstsec=TRUE;
	sendline(Crcflg?WANTCRC:NAK);
	if(wcgetsec(rpn, 100) != 0) {
		log( "Pathname fetch failed\n");
		return ERROR;
	}
	sendline(ACK);
	return OK;
}

/*
 * Adapted from CMODEM13.C, written by
 * Jack M. Wierda and Roderick W. Hart
 */

wcrx(name)
char *name;
{
	register int sectnum, sectcurr;
	register char sendchar;
	register char *filemode, *p;

	for(p=name; *p; ++p)		/* change / to _ */
		if( *p == '/')
			*p = '_';
	if ( *--p == '.')		/* zap trailing period */
		*p = 0;
	filemode = "w";
	Thisbinary=Rxbinary;
	if (substr(name, ".COM") || substr(name, ".CRL")
	  || substr(name, ".REL") || substr(name, ".PAG")
	  || substr(name, ".CMD")
	  || substr(name, ".SAV")
	  || substr(name, ".O")
	  || substr(name, ".DAT")
	  || substr(name, ".OBJ")
	  || ((p=substr(name, ".")) && p[2] == 'Q' )
	  || ((p=substr(name, ".")) && p[2] == 'q' ))
		Thisbinary = TRUE;
	if(substr(name, ".TXT")
	  || substr(name, ".MSG")
	  || substr(name, ".DOC"))
		filemode = "a";
	if(MakeLCPathname && !IsAnyLower(name))
		uncaps(name);
	fprintf(logfile,  "Receiving %s %s %s\n",
	  name, Thisbinary?"BIN":"", filemode);
	if ((fout=fopen(name, filemode)) == NULL)
		return ERROR;
	Firstsec=TRUE;sectnum=0; Eofseen=FALSE;
	sendchar=Crcflg?WANTCRC:NAK;

	for(;;) {
/*
		purgeline();
*/
		sendline(sendchar);	/* send it now, we're ready! */
		sectcurr=wcgetsec(secbuf, (sectnum&0177)?50:130);
		if(sectcurr==(sectnum+1 &0377)) {

			if(sectnum==0 && ((secbuf[0]&0200) | (secbuf[1]&0200)))
				Thisbinary++;
			sectnum++;
			if(putsec(secbuf, blklen)==ERROR)
				return ERROR;
			sendchar=ACK;
		}
		else if(sectcurr==(sectnum&0377)) {
			log( "Received dup Sector\n");
			sendchar=ACK;
		}
		else if(sectcurr==WCEOT) {
			sendline(ACK);
			/* don't pad the file any more than it already is */
			if(fclose(fout)==ERROR)
				return ERROR;
			return OK;
		}
		else if(sectcurr==ERROR)
			return ERROR;
		else {
			log( "Sync Error\n");
			return ERROR;
		}
	}
}

/*
 * wcgetsec fetches a Ward Christensen type sector.
 * Returns sector number encountered or ERROR if valid sector not received,
 * or CAN CAN received
 * or WCEOT if eot sector
 * time is timeout for first char, set to 4 seconds thereafter
 ***************** NO ACK IS SENT IF SECTOR IS RECEIVED OK **************
 *    (Caller must do that when he is good and ready to get next sector)
 */

wcgetsec(rxbuf, time)
char *rxbuf;
int time;
{
	register checksum, wcj, firstch;
	register unsigned oldcrc;
	register char *p;
	int sectcurr;

	for(Lastrx=errors=0; errors<RETRYMAX; errors++) {

		if ((firstch=readline(time))==STX) {
			blklen=KSIZE; goto get2;
		}
		if (firstch==SOH) {
			blklen=SECSIZ;
get2:
			sectcurr=readline(1);
			if((sectcurr+readline(1))==255) {
				oldcrc=checksum=0;
				for(p=rxbuf,wcj=blklen; --wcj>=0; ) {
					if((firstch=readline(1)) < 0)
						goto bilge;
					oldcrc=updcrc(firstch, oldcrc);
					checksum += (*p++ = firstch);
				}
				if((firstch=readline(1)) < 0)
					goto bilge;
				if(Crcflg) {
					oldcrc=updcrc(firstch, oldcrc);
					if((firstch=readline(1)) < 0)
						goto bilge;
					oldcrc=updcrc(firstch, oldcrc);
					if(oldcrc)
						log("CRC=0%o\n", oldcrc);
					else {
						Firstsec=FALSE;
						return sectcurr;
					}
				}
				else if(((checksum-firstch)&0377)==0) {
					Firstsec=FALSE;
					return sectcurr;
				}
				else
					log( "Checksum Error\n");
			}
			else
				log( "Sector number garbled\n");
		}
		/* make sure eot really is eot and not just mixmash */
		else if(firstch==EOT && readline(1)==TIMEOUT)
			return WCEOT;
		else if(firstch==CAN) {
			if(Lastrx==CAN) {
				log( "Sender CANcelled\n");
				return ERROR;
			} else {
				Lastrx=CAN;
				continue;
			}
		}
		else if(firstch==TIMEOUT) {
			if (Firstsec)
				goto humbug;
bilge:
			log( "Timeout\n");
		}
		else
			log( "Got 0%o sector header\n", firstch);

humbug:
		Lastrx=0;
		while(readline(1)!=TIMEOUT)
			;
		if(Firstsec)
			sendline(Crcflg?WANTCRC:NAK);
		else {
			time=40; sendline(NAK);
		}
	}
	/* try to stop the bubble machine. */
	sendline(CAN); sendline(CAN); sendline(CAN);
	return ERROR;
}

/*
 * This version of readline is not so well suited for
 * reading many characters.
 *
 * timeout is in tenths of seconds
 */
readline(timeout)
int timeout;
{
	if(--lleft>=0)
		return (*cdq++ & 0377);
	settimeout(timeout);
	if((llorig=lleft=read(1, cdq=linbuf, KSIZE))<1)
		return TIMEOUT;
	alarm(0);
	--lleft;
	return (*cdq++ & 0377);
}


alrm()
{
/* does nothing; actual effect is to give an error return on read */
}

/*
 * timeout is in tenths of seconds
 */
settimeout(timeout)
{
	register int c;

	signal(SIGALRM, alrm);
	if((c = timeout/10)<2)
		c=2;
	alarm(c);
}
purgeline()
{
	lleft=0;
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

/* make string s lower case */
uncaps(s)
register char *s;
{
	for( ; *s; ++s)
		if(isupper(*s))
			*s = tolower(*s);
}


/*
 * IsAnyLower returns TRUE if string s has lower case letters.
 */
IsAnyLower(s)
register char *s;
{
	for( ; *s; ++s)
		if (islower(*s))
			return TRUE;
	return FALSE;
}
/*
 * putsec writes the n characters of buf to receive file fout.
 *  If not in binary mode, carriage returns, and all characters
 *  starting with CPMEOF are discarded.
 */
putsec(buf, n)
char *buf;
register n;
{
	register char *p;

	++totblocks;
	if(Thisbinary)
	{
		for (p=buf; --n>=0; )
			putc( *p++, fout);
	}
	else {
		if(Eofseen)
			return OK;
		for (p=buf; --n>=0; ++p ) {
			if( *p == '\r')
				continue;
			if (*p == CPMEOF) {
				Eofseen=TRUE; return OK;
			}
			putc(*p ,fout);
		}
	}
	return OK;
}
sendline(c)
{
	putchar(c);
	fflush(stdout);
}


/*
 * substr(string, token) searches for token in string s
 * returns pointer to token within string if found, NULL otherwise
 */
char *substr(s, t)
register char *s,*t;
{
	register char *ss,*tt;
	/* search for first char of token */
	for(ss=s; *s; s++)
		if(*s == *t)
			/* compare token with substring */
			for(ss=s,tt=t; ;) {
				if(*tt == 0)
					return s;
				if(*ss++ != *tt++)
					break;
			}
	return NULL;
}
log(s,p)
char *s, *p;
{
	fprintf(logfile, "error %d: ", errors);
	fprintf(logfile, s,p);
}
