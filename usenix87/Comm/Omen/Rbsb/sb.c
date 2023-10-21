#define VERSION "sb 2.26 01-26-86"
#define PUBDIR "/usr/spool/uucppublic"

/*% cc -K -O -DUSG sb.c -o sb

 * sb.c By Chuck Forsberg
 *
 *  USG UNIX (3.0) ioctl conventions courtesy  Jeff Martin
 * 	cc -O -DV7  sb.c -o sb		Unix version 7, 2.8 - 4.3 BSD
 *	cc -O -DUSG sb.c -o sb		USG (3.0) Unix
 *  ******* Some systems (Venix, Coherent, Regulus) do not *******
 *  ******* support tty raw mode read(2) identically to    *******
 *  ******* Unix. ONEREAD must be defined to force one     *******
 *  ******* character reads for these systems.		   *******
 *
 * A small program for Unix to send 1 or more files YMODEM Batch protocol.
 *  Supports the CRC option or regular checksum.
 *  Exit status is 0 if all transfers completed successfully,
 *  1 for 1 or more unreadable files or'd with 200 for incomplete file xfer.
 *
 *  Sb accepts -k option for 1kb record length.
 *
 *  Sb uses buffered i/o to reduce the CPU time compared to UMODEM.
 */

#define LOGFILE "/tmp/sblog"

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>

#define PATHLEN 256
#define OK 0
#define FALSE 0
#define TRUE 1
#define ERROR (-1)

#define HOWMANY 2
#include "rbsb.c"	/* most of the system dependent stuff here */

FILE *in;

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
#define WANTG 0107	/* Send G not NAK to get nonstop batch xmsn */
#define TIMEOUT (-2)
#define RETRYMAX 10
#define SECSIZ 128	/* cp/m's Magic Number record size */
#define KSIZE 1024

char Lastrx;
char Crcflg;
int Wcsmask=0377;
int Verbose=0;
int Modem=0;		/* MODEM - don't send pathnames */
int Restricted=0;	/* restricted; no /.. or ../ in filenames */
int Quiet=0;		/* overrides logic that would otherwise set verbose */
int Fullname=0;		/* transmit full pathname */
int Unlinkafter=0;	/* Unlink file after it is sent */
int Dottoslash=0;	/* Change foo.bar.baz to foo/bar/baz */
int firstsec;
int errcnt=0;		/* number of files unreadable */
int blklen=SECSIZ;	/* length of transmitted records */
int Optiong;		/* Let it rip no wait for sector ACK's */
int Noeofseen;
int Totsecs;		/* total number of sectors this file */
char txbuf[KSIZE];
int Filcnt=0;		/* count of number of files opened */

jmp_buf tohere;		/* For the interrupt on RX timeout */

unsigned updcrc();
char *substr(), *getenv();

/* called by signal interrupt or terminate to clean things up */
bibi(n)
{
	canit(); fflush(stdout); mode(0);
	fprintf(stderr, "sb: caught signal %d; exiting\n", n);
	if (n == SIGQUIT)
		abort();
	exit(128+n);
}

#ifdef REGULUS
sendline(c)
{
	static char d[2];

	d[0] = c&Wcsmask;
	write(1, d, 1);
}
#else
#define sendline(c) putchar(c & Wcsmask)
#endif

main(argc, argv)
char *argv[];
{
	register char *cp;
	register npats;
	int agcnt; char **agcv;
	char **patts;
	int exitcode;
#ifndef REGULUS
	static char xXbuf[BUFSIZ];
#endif

	if ((cp=getenv("SHELL")) && (substr(cp, "rsh") || substr(cp, "rsh")))
		Restricted=TRUE;

	npats=0;
	if (argc<2)
		usage();
#ifndef REGULUS
	setbuf(stdout, xXbuf);		
#endif
	while (--argc) {
		cp = *++argv;
		if (*cp++ == '-' && *cp) {
			while ( *cp) {
				switch(*cp++) {
				case '1':
					iofd = 1; break;
				case '7':
					Wcsmask=0177; break;
				case 'd':
					++Dottoslash;
					/* **** FALL THROUGH TO **** */
				case 'f':
					Fullname=TRUE; break;
				case 'k':
					blklen=KSIZE; break;
				case 'q':
					Quiet=TRUE; Verbose=0; break;
				case 'u':
					++Unlinkafter; break;
				case 'v':
					++Verbose; break;
				case 'X':
					++Modem; break;
				default:
					usage();
				}
			}
		}
		else if ( !npats && argc>0) {
			if (argv[0][0]) {
				npats=argc;
				patts=argv;
				if ( !strcmp(*patts, "-"))
					iofd = 1;
			}
		}
	}
	if (npats < 1) 
		usage();
	if (Verbose) {
		if (freopen(LOGFILE, "a", stderr)==NULL) {
			printf("Can't open log file %s\n",LOGFILE);
			exit(0200);
		}
		setbuf(stderr, NULL);
	}
	if (fromcu() && !Quiet) {
		if (Verbose == 0)
			Verbose = 2;
	}
	if (Verbose != 1) {
		fprintf(stderr, "sb: %d file%s requested:\r\n",
		 npats, npats>1?"s":"");
		for ( agcnt=npats, agcv=patts; --agcnt>=0; ) {
			fprintf(stderr, "%s ", *agcv++);
		}
	}
	if ( !Modem)
		printf("\r\n\bSending in Batch Mode\r\n");
	fflush(stdout);

	mode(1);
	if (signal(SIGINT, bibi) == SIG_IGN) {
		signal(SIGINT, SIG_IGN); signal(SIGKILL, SIG_IGN);
	}
	else {
		signal(SIGINT, bibi); signal(SIGKILL, bibi);
		signal(SIGQUIT, bibi);
	}

	if (wcsend(npats, patts)==ERROR) {
		exitcode=0200;
		canit();
	}
	fflush(stdout);
	mode(0);
	exit((errcnt != 0) | exitcode);
}

wcsend(argc, argp)
char *argp[];
{
	register n;

	Crcflg=FALSE;
	firstsec=TRUE;
	for (n=0; n<argc; ++n) {
		Totsecs = 0;
		if (wcs(argp[n])==ERROR)
			goto fubar;
	}
	Totsecs = 0;
	if (Filcnt==0) {	/* bitch if we couldn't open ANY files */
		fprintf(stderr,"\r\nCan't open any requested files.\n");
		return ERROR;
	}
	else if (wctxpn("")==ERROR)
		goto fubar;
	return OK;
fubar:
	canit(); return ERROR;
}

wcs(oname)
char *oname;
{
	register c;
	register char *p;
	struct stat f;
	char name[PATHLEN];

	strcpy(name, oname);

	if (Restricted) {
		/* restrict pathnames to current tree or uucppublic */
		if ( substr(name, "../")
		 || (name[0]== '/' && strncmp(name, PUBDIR, strlen(PUBDIR))) ) {
			canit();
			fprintf(stderr,"\r\nsb:\tSecurity Violation\r\n");
			return ERROR;
		}
	}

	if ( !strcmp(oname, "-")) {
		sprintf(name, "s%d.sb", getpid());
		in = stdin;
	}
	else if ((in=fopen(oname, "r"))==NULL) {
		++errcnt;
		return OK;	/* pass over it, there may be others */
	}
	++Noeofseen;
	/* Check for directory or block special files */
#ifndef REGULUS		/* This doesn't seem to work on Regulus */
	fstat(fileno(in), &f);
	c = f.st_mode & S_IFMT;
	if (c == S_IFDIR || c == S_IFBLK) {
		fclose(in);
		return OK;
	}
#endif
	++Filcnt;
	if (wctxpn(name)== ERROR)
		return ERROR;
	if (wctx()==ERROR)
		return ERROR;
	if (Unlinkafter)
		unlink(oname);
	return 0;
}

/*
 * generate and transmit pathname block consisting of
 *  pathname (null terminated),
 *  file length, mode time and file mode in octal
 *  as provided by the Unix fstat call.
 *  N.B.: modifies the passed name, may extend it!
 */
wctxpn(name)
char *name;
{
	register firstch;
	register char *p, *q;
	char name2[PATHLEN];
	struct stat f;

	if (Modem)
		return OK;
	logent("\r\nAwaiting pathname nak for %s\r\n", *name?name:"<END>");
	if (getnak())
		return ERROR;

	q = (char *) 0;
	if (Dottoslash) {		/* change . to . */
		for (p=name; *p; ++p) {
			if (*p == '/')
				q = p;
			else if (*p == '.')
				*(q=p) = '/';
		}
		if (q && strlen(++q) > 8) {	/* If name>8 chars */
			q += 8;			/*   make it .ext */
			strcpy(name2, q);	/* save excess of name */
			*q = '.';
			strcpy(++q, name2);	/* add it back */
		}
	}

	for (p=name, q=txbuf ; *p; )
		if ((*q++ = *p++) == '/' && !Fullname)
			q = txbuf;
	*q++ = 0;
	p=q;
	while (q < (txbuf + KSIZE))
		*q++ = 0;
	if (in == stdin)
		strcpy(p, "1999999999");
#ifndef REGULUS		/* This doesn't seem to work on Regulus */
	else if (*name && fstat(fileno(in), &f)!= -1)
		sprintf(p, "%lu %lo %o", f.st_size, f.st_mtime, f.st_mode);
#endif
	/* force 1k blocks if name won't fit in 128 byte block */
	if (txbuf[125])
		blklen=KSIZE;
	else {		/* A little goodie for IMP/KMD */
		txbuf[127] = f.st_size >>7;
		txbuf[126] = f.st_size >>15;
	}
	if (wcputsec(txbuf, 0, SECSIZ)==ERROR)
		return ERROR;
	return OK;
}

getnak()
{
	register firstch;

	Lastrx = 0;
	for (;;) {
		switch (firstch = readock(800,2)) {
		case TIMEOUT:
			logent("Timeout on pathname\n");
			return TRUE;
		case WANTG:
			mode(2);	/* Set cbreak, XON/XOFF, etc. */
			Optiong = TRUE;
			blklen=KSIZE;
		case WANTCRC:
			Crcflg = TRUE;
		case NAK:
			return FALSE;
		case CAN:
			if (Lastrx == CAN)
				return TRUE;
		default:
			break;
		}
		Lastrx = firstch;
	}
}


wctx()
{
	register int sectnum, attempts, firstch;

	firstsec=TRUE;

	while ((firstch=readock(400, 2))!=NAK && firstch != WANTCRC
	  && firstch != WANTG && firstch!=TIMEOUT && firstch!=CAN)
		;
	if (firstch==CAN) {
		logent("Receiver CANcelled\n");
		return ERROR;
	}
	if (firstch==WANTCRC)
		Crcflg=TRUE;
	if (firstch==WANTG)
		Crcflg=TRUE;
	sectnum=1;
	while (filbuf(txbuf, blklen)) {
		if (wcputsec(txbuf, sectnum, blklen)==ERROR) {
			return ERROR;
		} else
			sectnum++;
	}
	if (Verbose>1)
		fprintf(stderr, " Closing ");
	fclose(in);
	attempts=0;
	do {
		logent(" EOT ");
		purgeline();
		sendline(EOT);
		fflush(stdout);
		++attempts;
	}
		while ((firstch=(readock(100, 1)) != ACK) && attempts < RETRYMAX);
	if (attempts == RETRYMAX) {
		logent("No ACK on EOT\n");
		return ERROR;
	}
	else
		return OK;
}

wcputsec(buf, sectnum, cseclen)
char *buf;
int sectnum;
int cseclen;	/* data length of this sector to send */
{
	register checksum, wcj;
	register char *cp;
	unsigned oldcrc;
	int firstch;
	int attempts;

	firstch=0;	/* part of logic to detect CAN CAN */

	if (Verbose>1)
		fprintf(stderr, "\rSector %3d %2dk ", Totsecs, Totsecs/8 );
	for (attempts=0; attempts <= RETRYMAX; attempts++) {
		Lastrx= firstch;
		sendline(cseclen==KSIZE?STX:SOH);
		sendline(sectnum);
		sendline(-sectnum -1);
		oldcrc=checksum=0;
		for (wcj=cseclen,cp=buf; --wcj>=0; ) {
			sendline(*cp);
			oldcrc=updcrc(*cp, oldcrc);
			checksum += *cp++;
		}
		if (Crcflg) {
			oldcrc=updcrc(0,updcrc(0,oldcrc));
			sendline((int)oldcrc>>8);
			sendline((int)oldcrc);
		}
		else
			sendline(checksum);

		if (Optiong) {
			firstsec = FALSE; return OK;
		}
		firstch = readock(400, (Noeofseen&&sectnum) ? 2:1);
gotnak:
		switch (firstch) {
		case CAN:
			if(Lastrx == CAN) {
cancan:
				logent("Cancelled\n");  return ERROR;
			}
			break;
		case TIMEOUT:
			logent("Timeout on sector ACK\n"); continue;
		case WANTCRC:
			if (firstsec)
				Crcflg = TRUE;
		case NAK:
			logent("NAK on sector\n"); continue;
		case ACK: 
			firstsec=FALSE;
			Totsecs += (cseclen>>7);
			return OK;
		case ERROR:
			logent("Got burst for sector ACK\n"); break;
		default:
			logent("Got %02x for sector ACK\n", firstch); break;
		}
		for (;;) {
			Lastrx = firstch;
			if ((firstch = readock(400, 2)) == TIMEOUT)
				break;
			if (firstch == NAK || firstch == WANTCRC)
				goto gotnak;
			if (firstch == CAN && Lastrx == CAN)
				goto cancan;
		}
	}
	logent("Retry Count Exceeded\n");
	return ERROR;
}


/* fill buf with count chars padding with ^Z for CPM */
filbuf(buf, count)
register char *buf;
{
	register c, m;
	static lfseen=0;

	m=count;
	while ((c=getc(in))!=EOF) {
		*buf++ =c;
		if (--m == 0)
			break;
	}
	if (m==count)
		return (Noeofseen=0);
	else
		while (--m>=0)
			*buf++ = CPMEOF;
	return count;
}


alrm()
{
	longjmp(tohere, -1);
}


/*
 * readock(timeout, count) reads character(s) from file descriptor 0
 *  (1 <= count <= 3)
 * it attempts to read count characters. If it gets more than one,
 * it is an error unless all are CAN
 * (otherwise, only normal response is ACK, CAN, or C)
 *  Only looks for one if Optiong, which signifies cbreak, not raw input
 *
 * timeout is in tenths of seconds
 */
readock(timeout, count)
{
	register int c;
	static char byt[5];

	if (Optiong)
		count = 1;	/* Special hack for cbreak */

	fflush(stdout);
	if (setjmp(tohere)) {
		logent("TIMEOUT\n");
		return TIMEOUT;
	}
	c = timeout/10;
	if (c<2)
		c=2;
	if (Verbose>3) {
		fprintf(stderr, "Timeout=%d Calling alarm(%d) ", timeout, c);
		byt[1] = 0;
	}
	signal(SIGALRM, alrm); alarm(c);
#ifdef ONEREAD
	c=read(iofd, byt, 1);		/* regulus raw read is unique */
#else
	c=read(iofd, byt, count);
#endif
	alarm(0);
	if (Verbose>5)
		fprintf(stderr, "ret cnt=%d %x %x\n", c, byt[0], byt[1]);
	if (c<1)
		return TIMEOUT;
	if (c==1)
		return (byt[0]&0377);
	else
		while (c)
			if (byt[--c] != CAN)
				return ERROR;
	return CAN;
}

purgeline()
{
#ifdef USG
	ioctl(iofd, TCFLSH, 0);
#else
	lseek(iofd, 0L, 2);
#endif
}

/* update CRC */
unsigned updcrc(c, crc)
register c;
register unsigned crc;
{
	register count;

	for (count=8; --count>=0;) {
		if (crc & 0x8000) {
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

/* send 10 CAN's to try to get the other end to shut up */
canit()
{
	register n;
	for (n=10; --n>=0; )
		sendline(CAN);
}

#ifdef REGULUS
/*
 * copies count bytes from s to d
 * (No structure assignment in Regulus C compiler)
 */
movmem(s, d, count)
register char *s, *d;
register count;
{
	while (--count >= 0)
		*d++ = *s++;
}
#endif

/*VARARGS1*/
logent(a, b, c)
char *a, *b, *c;
{
	if(Verbose)
		fprintf(stderr, a, b, c);
}

/*
 * return 1 iff stdout and stderr are different devices
 *  indicating this program operating with a modem on a
 *  different line
 */
fromcu()
{
	struct stat a, b;
	fstat(1, &a); fstat(2, &b);
	return (a.st_rdev != b.st_rdev);
}

/*
 * substr(string, token) searches for token in string s
 * returns pointer to token within string if found, NULL otherwise
 */
char *
substr(s, t)
register char *s,*t;
{
	register char *ss,*tt;
	/* search for first char of token */
	for (ss=s; *s; s++)
		if (*s == *t)
			/* compare token with substring */
			for (ss=s,tt=t; ;) {
				if (*tt == 0)
					return s;
				if (*ss++ != *tt++)
					break;
			}
	return NULL;
}

usage()
{
	fprintf(stderr,"%s by Chuck Forsberg\n", VERSION);
	fprintf(stderr,"Usage: sb [-17dfkquvX] [-] file ...\n");
	fprintf(stderr,"	1 Use stdout for all modem i/o\n");
	fprintf(stderr,"	d Change '.' to '/' in pathnames\n");
	fprintf(stderr,"	f Send full pathname\n");
	fprintf(stderr,"	k Send 1024 byte packets\n");
	fprintf(stderr,"	q Quiet (no progress reports)\n");
	fprintf(stderr,"	u Unlink file after transmission\n");
	fprintf(stderr,"	v Verbose - give more information\n");
	fprintf(stderr,"	X XMODEM protocol - send no pathnames\n");
	fprintf(stderr,"- as pathname sends standard input, filename=sPID.sb, requires -1 flag\n");
	exit(1);
}
