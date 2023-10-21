#define VERSION "2.17 12-07-85"
#define PUBDIR "/usr/spool/uucppublic"

/*% cc -DUSG -DNFGVMIN -O -K % -o rb
 *
 * rb.c By Chuck Forsberg
 *
 *  USG UNIX (3.0) ioctl conventions courtesy  Jeff Martin
 * 	cc -O -DV7  rb.c -o rb		Unix V7, BSD 2.8 - 4.2
 *	cc -O -DUSG rb.c -o rb		USG (3.0) Unix
 *	cc -o rb.c			Regulus
 *		(Don't try this on Unix, you'll clobber the source!)
 *  Unix is a trademark of Western Electric Company
 *
 *  Regulus conventions 1-10-83 CAF
 *
 *  Some systems (Venix, Coherent, Regulus) do not support tty raw mode
 *  read(2) the same way as Unix. ONEREAD must be defined to force one
 *  character reads for these systems. Added 7-01-84 CAF
 *
 *  Alarm signal handling changed to work with 4.2 BSD 7-15-84 CAF 
 *
 *  NFGVMIN Added 1-13-85 CAF for PC-AT Xenix systems where c_cc[VMIN]
 *  doesn't seem to work (even though it compiles without error!).
 *
 * A program for Unix which can receive
 *  files from computers running YAM or MODEM.
 *  rb uses Unix System III buffered input to reduce CPU time.
 *  If no filename is given, YAM batch mode is assumed.
 *
 * Iff the program is invoked by rbCOMMAND, output is piped to 
 * "COMMAND filename"
 *
 *  Supports the CRC option or regular checksum.
 *  Received pathnames containing no lowercase letters will be changed to lower
 *  case unless -u option is given.
 *
 *  Unless the -b (binary) option is given, \r is discarded and
 *  ^Z (which is also discarded) acts as end of file.
 *
 *  Any slashes in the pathname are changed to underscore.
 *  If the raw pathname ends in .MSG or .TXT, any existing file will
 *  be appended to rather than replaced. Trailing periods are eliminated.
 *
 *  If the raw pathname ends in any of the extensions in Extensions,
 *   or .?Q* (squeezed file), or if the first sector contains binary-like
 *   data (parity bits or characters in the range 0 to 6 before ^Z is seen),
 *   or if the transmitted file mode has the 0100000 but set,
 *   that file will be received in binary mode anyway.
 *
 *
 * A log of activities is appended to LOGFILE with the -v option
 * If stdout and stderr refer to different devices, progress is displayed to
 * stderr.
 *
 * rb is derived from yam2.c and sb.c
 */
#define LOGFILE "/tmp/rblog"

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
FILE *popen();

#define OK 0
#define FALSE 0
#define TRUE 1
#define ERROR (-1)

#define HOWMANY 133
#include "rbsb.c"	/* most of the system dependent stuff here */

char *substr();
FILE *fout;

char *Extensions[] = {
".A",
".ARC",
".CCC",
".CL",
".CMD",
".COM",
".CRL",
".DAT",
".DIR",
".EXE",
".O",
".OBJ",
".OVL",
".PAG",
".REL",
".SAV",
".SUB",
".SWP",
".SYS",
".TAR",
".UTL",
".a",
".o",
".tar",
""
};

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
#define PATHLEN 257	/* ready for 4.2 bsd ? */
#define KSIZE 1024	/* record size with k option */
#define UNIXFILE 0x8000	/* happens to the the S_IFREG file mask bit for stat */

int Lastrx;
int Crcflg;
int Firstsec;
int Eofseen;		/* indicates cpm eof (^Z) has been received */
int totblocks;		/* total number of blocks received */
int errors;
int Restricted=0;	/* restricted; no /.. or ../ in filenames */

#define DEFBYTL 2000000000L	/* default rx file size */
long Bytesleft;		/* number of bytes of incoming file left */
long Modtime;		/* Unix style mod time for incoming file */
short Filemode;		/* Unix style mode for incoming file */
char Pathname[PATHLEN];
char *Progname;		/* the name by which we were called */

int Batch=0;
int Wcsmask=0377;
int Topipe=0;
int MakeLCPathname=TRUE;	/* make received pathname lower case */
int Verbose=0;
int Quiet=0;		/* overrides logic that would otherwise set verbose */
int Rxbinary=FALSE;	/* receive all files in bin mode */
int Thisbinary;		/* current file is to be received in bin mode */
int Blklen;		/* record length of received packets */
char secbuf[KSIZE];
char linbuf[KSIZE];
int Lleft=0;		/* number of characters in linbuf */

jmp_buf tohere;		/* For the interrupt on RX timeout */

unsigned short updcrc();

alrm()
{
	longjmp(tohere, -1);
}

/* called by signal interrupt or terminate to clean things up */
bibi(n)
{
	canit(); mode(0);
	fprintf(stderr, "sb: caught signal %d; exiting", n);
	exit(128+n);
}

main(argc, argv)
char *argv[];
{
	register char *cp;
	register npats;
	char *virgin, **patts;
	char *getenv();
	int exitcode;

	setbuf(stderr, NULL);
	if ((cp=getenv("SHELL")) && (substr(cp, "rsh") || substr(cp, "rsh")))
		Restricted=TRUE;

	chkinvok(virgin=argv[0]);	/* if called as [-]rbCOMMAND set flag */
	npats = 0;
	while (--argc) {
		cp = *++argv;
		if (*cp == '-') {
			while( *++cp) {
				switch(*cp) {
				case '1':
					iofd = 1; break;
				case '7':
					Wcsmask = 0177;
				case 'b':
					Rxbinary=TRUE; break;
				case 'k':
				case 'c':
					Crcflg=TRUE; break;
				case 'q':
					Quiet=TRUE; Verbose=0; break;
				case 'u':
					MakeLCPathname=FALSE; break;
				case 'v':
					++Verbose; break;
				default:
					usage();
				}
			}
		}
		else if ( !npats && argc>0) {
			if (argv[0][0]) {
				npats=argc;
				patts=argv;
			}
		}
	}
	if (npats > 1)
		usage();
	if (Verbose) {
		if (freopen(LOGFILE, "a", stderr)==NULL) {
			printf("Can't open log file %s\n",LOGFILE);
			exit(0200);
		}
		setbuf(stderr, NULL);
		fprintf(stderr, "argv[0]=%s Progname=%s\n", virgin, Progname);
	}
	if (fromcu() && !Quiet) {
		if (Verbose == 0)
			Verbose = 2;
	}
	mode(1);
	if (signal(SIGINT, bibi) == SIG_IGN) {
		signal(SIGINT, SIG_IGN); signal(SIGKILL, SIG_IGN);
	}
	else {
		signal(SIGINT, bibi); signal(SIGKILL, bibi);
	}
	if (wcreceive(npats, patts)==ERROR) {
		exitcode=0200;
		canit();
	}
	mode(0);
	if (exitcode != 0)	/* bellow again with all thy might. */
		canit();
#ifdef REGULUS
	else
		printf("\6\6\6\6\6\n");	/* Regulus doesn't wait ... */
#endif
	exit(exitcode);
}


usage()
{
	fprintf(stderr,"%s %s by Chuck Forsberg\n", Progname, VERSION);
	fprintf(stderr,"Usage:	rb [-17buv]\n\tor rb [-1bcuv] file\n");
	exit(1);
}

wcreceive(argc, argp)
char **argp;
{
	if (Batch || argc==0) {
		Crcflg=(Wcsmask==0377);
		fprintf(stderr, "rb: ready ");
		for (;;) {
			totblocks=0;
			if (wcrxpn(secbuf)== ERROR)
				goto fubar;
			if (secbuf[0]==0)
				return OK;
			if (procheader(secbuf) == ERROR)
				goto fubar;
			if (wcrx()==ERROR)
				goto fubar;
		}
	} else {
		totblocks=0; Bytesleft = DEFBYTL; Filemode = 0; Modtime = 0L;

		strcpy(Pathname, *argp);
		checkpath(Pathname);
		fprintf(stderr, "\nrb: ready to receive %s ", Pathname);
		if ((fout=fopen(Pathname, "w")) == NULL)
			return ERROR;
		if (wcrx()==ERROR)
			goto fubar;
	}
	return OK;
fubar:
	canit();
	if (Topipe && fout) {
		pclose(fout);  return ERROR;
	}
	if (fout)
		fclose(fout);
	if (Restricted) {
		unlink(Pathname);
		fprintf(stderr, "\r\nrb: %s removed.\r\n", Pathname);
	}
	return ERROR;
}


/*
 * Fetch a pathname from the other end as a C ctyle ASCIZ string.
 * Length is indeterminate as long as less than Blklen
 * a null string represents no more files
 */
wcrxpn(rpn)
char *rpn;	/* receive a pathname */
{
	register c;

#ifdef NFGVMIN
	readline(1);
#else
	purgeline();
#endif

et_tu:
	Firstsec=TRUE;
	sendline(Crcflg?WANTCRC:NAK);
	while ((c = wcgetsec(rpn, 100)) != 0) {
		log( "Pathname fetch returned %d\n", c);
		if (c == WCEOT) {
			sendline(ACK); readline(1); goto et_tu;
		}
		return ERROR;
	}
	sendline(ACK);
	return OK;
}

/*
 * Adapted from CMODEM13.C, written by
 * Jack M. Wierda and Roderick W. Hart
 */

wcrx()
{
	register int sectnum, sectcurr;
	register char sendchar;
	register char *p;
	int cblklen;			/* bytes to dump this block */
	time_t timep[2];

	Firstsec=TRUE;sectnum=0; Eofseen=FALSE;
	sendchar=Crcflg?WANTCRC:NAK;

	for (;;) {
		sendline(sendchar);	/* send it now, we're ready! */
		sectcurr=wcgetsec(secbuf, (sectnum&0177)?50:130);
		report(sectcurr);
		if (sectcurr==(sectnum+1 &Wcsmask)) {

			if (sectnum==0 && !Thisbinary)
				for (p=secbuf,sectcurr=Blklen;
				  *p != 032 && --sectcurr>=0; ++p)
					if (*p < 07 || (*p & 0200)) {
						Thisbinary++;
						if (Verbose)
							fprintf(stderr, "Changed to BIN\n");
						break;
					}
			sectnum++;
			cblklen = Bytesleft>Blklen ? Blklen:Bytesleft;
			if (putsec(secbuf, cblklen)==ERROR)
				return ERROR;
			if ((Bytesleft-=cblklen) < 0)
				Bytesleft = 0;
			sendchar=ACK;
		}
		else if (sectcurr==(sectnum&Wcsmask)) {
			log( "Received dup Sector\n");
			sendchar=ACK;
		}
		else if (sectcurr==WCEOT) {
			if (Topipe) {
				if (pclose(fout)) {
					canit(); return ERROR;
				}
				sendline(ACK); return OK;
			}
			if (fclose(fout)==ERROR) {
				canit();
				fprintf(stderr, "file close ERROR\n");
				return ERROR;
			}
			if (Modtime) {
				timep[0] = time(NULL);
				timep[1] = Modtime;
				utime(Pathname, timep);
			}
			if (Filemode)
				chmod(Pathname, (07777 & Filemode));
			sendline(ACK);
			return OK;
		}
		else if (sectcurr==ERROR)
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

wcgetsec(rxbuf, maxtime)
char *rxbuf;
int maxtime;
{
	register checksum, wcj, firstch;
	register unsigned short oldcrc;
	register char *p;
	int sectcurr;

	for (Lastrx=errors=0; errors<RETRYMAX; errors++) {

		if ((firstch=readline(maxtime))==STX) {
			Blklen=KSIZE; goto get2;
		}
		if (firstch==SOH) {
			Blklen=SECSIZ;
get2:
			sectcurr=readline(1);
			if ((sectcurr+(oldcrc=readline(1)))==Wcsmask) {
				oldcrc=checksum=0;
				for (p=rxbuf,wcj=Blklen; --wcj>=0; ) {
					if ((firstch=readline(1)) < 0)
						goto bilge;
					oldcrc=updcrc(firstch, oldcrc);
					checksum += (*p++ = firstch);
				}
				if ((firstch=readline(1)) < 0)
					goto bilge;
				if (Crcflg) {
					oldcrc=updcrc(firstch, oldcrc);
					if ((firstch=readline(1)) < 0)
						goto bilge;
					oldcrc=updcrc(firstch, oldcrc);
					if (oldcrc)
						log("CRC=0%o\n", oldcrc);
					else {
						Firstsec=FALSE;
						return sectcurr;
					}
				}
				else if (((checksum-firstch)&Wcsmask)==0) {
					Firstsec=FALSE;
					return sectcurr;
				}
				else
					log( "Checksum Error\n");
			}
			else
				log("Sector number garbled 0%o 0%o\n",
				 sectcurr, oldcrc);
		}
		/* make sure eot really is eot and not just mixmash */
#ifdef NFGVMIN
		else if (firstch==EOT && readline(1)==TIMEOUT)
			return WCEOT;
#else
		else if (firstch==EOT && Lleft==0)
			return WCEOT;
#endif
		else if (firstch==CAN) {
			if (Lastrx==CAN) {
				log( "Sender CANcelled\n");
				return ERROR;
			} else {
				Lastrx=CAN;
				continue;
			}
		}
		else if (firstch==TIMEOUT) {
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
		if (Firstsec)
			sendline(Crcflg?WANTCRC:NAK);
		else {
			maxtime=40; sendline(NAK);
		}
	}
	/* try to stop the bubble machine. */
	canit();
	return ERROR;
}

/*
 * This version of readline is reasoably well suited for
 * reading many characters.
 *  (except, currently, for the Regulus version!)
 *
 * timeout is in tenths of seconds
 */
readline(timeout)
int timeout;
{
	register n;
	static char *cdq;	/* pointer for removing chars from linbuf */

	if (--Lleft >= 0) {
		if (Verbose > 8) {
			fprintf(stderr, "%02x ", *cdq&0377);
		}
		return (*cdq++ & Wcsmask);
	}
	n = timeout/10;
	if (n < 2)
		n = 3;
	if (Verbose > 3)
		fprintf(stderr, "Calling read: n=%d ", n);
	if (setjmp(tohere)) {
#ifdef TIOCFLUSH
/*		ioctl(iofd, TIOCFLUSH, 0); */
#endif
		Lleft = 0;
		if (Verbose>1)
			fprintf(stderr, "Readline:TIMEOUT\n");
		return TIMEOUT;
	}
	signal(SIGALRM, alrm); alarm(n);
#ifdef ONEREAD
	/* Sorry, Regulus and some others don't work right in raw mode! */
	Lleft=read(iofd, cdq=linbuf, 1);
#else
	Lleft=read(iofd, cdq=linbuf, KSIZE);
#endif
	alarm(0);
	if (Verbose > 3) {
		fprintf(stderr, "Read returned %d bytes\n", Lleft);
	}
	if (Lleft < 1)
		return TIMEOUT;
	--Lleft;
	if (Verbose > 8) {
		fprintf(stderr, "%02x ", *cdq&0377);
	}
	return (*cdq++ & Wcsmask);
}




purgeline()
{
	Lleft = 0;
#ifdef USG
	ioctl(iofd, TCFLSH, 0);
#else
	lseek(iofd, 0L, 2);
#endif
}

/* update CRC */
unsigned short
updcrc(c, crc)
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

/*
 * process incoming header
 */
procheader(name)
char *name;
{
	register char *openmode, *p, **pp;

	/* set default parameters */
	openmode = "w"; Thisbinary=Rxbinary;
	Bytesleft = DEFBYTL; Filemode = 0; Modtime = 0L;

	p = name + 1 + strlen(name);
	if (*p) {	/* file coming from Unix or DOS system */
		sscanf(p, "%ld%lo%o", &Bytesleft, &Modtime, &Filemode);
		if (Filemode & UNIXFILE)
			++Thisbinary;
		if (Verbose) {
			fprintf(stderr,  "Incoming: %s %ld %lo %o\n",
			  name, Bytesleft, Modtime, Filemode);
		}
	}
	else {		/* File coming from CP/M system */
		for (p=name; *p; ++p)		/* change / to _ */
			if ( *p == '/')
				*p = '_';

		if ( *--p == '.')		/* zap trailing period */
			*p = 0;
	}

	/* scan for extensions that signify a binary file */
	if (p=substr(name, "."))
		for (pp=Extensions; **pp; ++pp)
			if (strcmp(p, *pp)==0) {
				Thisbinary=TRUE; break;
			}

	/* scan for files which should be appended */
	if ( !Thisbinary
	  && (substr(name, ".TXT")
	  || substr(name, ".txt")
	  || substr(name, ".MSG")))
		openmode = "a";
	if (MakeLCPathname && !IsAnyLower(name))
		uncaps(name);
	if (Topipe) {
		sprintf(Pathname, "%s %s", Progname+2, name);
		if (Verbose)
			fprintf(stderr,  "Topipe: %s %s\n",
			  Pathname, Thisbinary?"BIN":"ASCII");
		if ((fout=popen(Pathname, "w")) == NULL)
			return ERROR;
	} else {
		strcpy(Pathname, name);
		if (Verbose) {
			fprintf(stderr,  "Receiving %s %s %s\n",
			  name, Thisbinary?"BIN":"ASCII", openmode);
		}
		checkpath(name);
		if ((fout=fopen(name, openmode)) == NULL)
			return ERROR;
	}
	return OK;
}

/* make string s lower case */
uncaps(s)
register char *s;
{
	for ( ; *s; ++s)
		if (isupper(*s))
			*s = tolower(*s);
}


/*
 * IsAnyLower returns TRUE if string s has lower case letters.
 */
IsAnyLower(s)
register char *s;
{
	for ( ; *s; ++s)
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
	if (Thisbinary)
	{
		for (p=buf; --n>=0; )
			putc( *p++, fout);
	}
	else {
		if (Eofseen)
			return OK;
		for (p=buf; --n>=0; ++p ) {
			if ( *p == '\r')
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
	char d;

	d = c;
	if (Verbose>2)
		fprintf(stderr, "Sendline: %x\n", c);
	write(1, &d, 1);
	Lleft=0;	/* Do read next time ... */
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

/*VARARGS1*/
log(s,p,u)
char *s, *p, *u;
{
	if (!Verbose)
		return;
	fprintf(stderr, "error %d: ", errors);
	fprintf(stderr, s, p, u);
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

report(sct)
int sct;
{
	if (Verbose>1)
		fprintf(stderr,"%03d%c",sct,sct%10? ' ' : '\r');
}

/*
 * if called as [-][dir/../]vrbCOMMAND set Verbose to 1
 * if called as [-][dir/../]rbCOMMAND set the pipe flag
 */
chkinvok(s)
char *s;
{
	register char *p;

	p = s;
	while (*p == '-')
		s = ++p;
	while (*p)
		if (*p++ == '/')
			s = p;
	if (*s == 'v') {
		Verbose=1; ++s;
	}
	Progname = s;
	if (s[2] && s[0]=='r' && s[1]=='b')
		Topipe=TRUE;
}

checkpath(name)
char *name;
{
	if (Restricted) {
		if (fopen(name, "r") != NULL) {
			canit();
			fprintf(stderr, "\r\nrb: %s exists\n", name);
			bibi();
		}
		/* restrict pathnames to current tree or uucppublic */
		if ( substr(name, "../")
		 || (name[0]== '/' && strncmp(name, PUBDIR, strlen(PUBDIR))) ) {
			canit();
			fprintf(stderr,"\r\nrb:\tSecurity Violation\r\n");
			bibi();
		}
	}
}

