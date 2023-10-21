#define VERSION "vvrb 1.01 12-18-85"

/*
 *
 * vvrb.c By Chuck Forsberg
 *	with code fragments stolen from VMS version of UMODEM.C
 *
 * A program for VMS which can receive
 *  files from computers running YAM or MODEM.
 *  If no filename is given, YMODEM (YAM batch) protocol is assumed.
 *
 *  Supports the CRC option or regular checksum.
 *  Received pathnames containing no lowercase letters will be changed
 *  to lower case unless -u option is given.
 *
 *  Unless the -b (binary) option is given, 
 *  ^Z (which is discarded) acts as end of file.
 *
 *  If the raw pathname ends in any of the extensions in Extensions,
 *   or .?Q* (squeezed file), or if the first sector contains binary-like
 *   data (parity bits or characters in the range 0 to 6 before ^Z is seen),
 *   or if the transmitted file mode has the 0100000 but set,
 *   that file will be received in binary mode anyway.
 *
 *
 * A log of activities is appended to LOGFILE with the -v option
 *
 * To compile on VMS:
 *		cc vvrb.c
 *		cc vvmodem.c
 *		link vvrb,vvmodem
 *	rb :== $disk$user2:[username.subdir]vvrb.exe
 *
 *	Manual page is "rb.1" in the Unix version rbsb.sh file
 */
#define LOGFILE "rblog"

#include <stdio.h>
#include <ctype.h>
#include "vmodem.h"

#ifdef vms
#include ssdef
#include tt2def
#include ttdef
#define SS_NORMAL SS$_NORMAL
#else
#define SS_NORMAL 0
#endif

/*  VMS structures  */
#ifdef vms
/*
 *	TT_INFO structures are used for passing information about
 *	the terminal.  Used in GTTY and STTY calls.
 */
struct	tt_info	ttys, ttysnew, ttystemp;
#endif

#define OK 0
#define FALSE 0
#define TRUE 1
#define ERROR (-1)

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
#define RETRYMAX 10
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

#define DEFBYTL 2000000000L	/* default rx file size */
long Bytesleft;		/* number of bytes of incoming file left */
long Modtime;		/* Unix style mod time for incoming file */
short Filemode;		/* Unix style mode for incoming file */
char Pathname[PATHLEN];

int Batch=0;
int Wcsmask=0377;
int MakeLCPathname=TRUE;	/* make received pathname lower case */
int Verbose=0;
int Quiet=0;		/* overrides logic that would otherwise set verbose */
int Rxbinary=FALSE;	/* receive all files in bin mode */
int Thisbinary;		/* current file is to be received in bin mode */
int Blklen;		/* record length of received packets */
char secbuf[KSIZE];
char linbuf[KSIZE];
int Lleft=0;		/* number of characters in linbuf */

unsigned short updcrc();

main(argc, argv)
char *argv[];
{
	register char *cp;
	register npats;
	char **patts;
	int exitcode;

	setbuf(stderr, NULL);
	npats = 0;
	while (--argc) {
		cp = *++argv;
		if (*cp == '-') {
			while( *++cp) {
				switch(*cp) {
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
			exit(SS_NORMAL);
		}
		setbuf(stderr, NULL);
	}
	setmodes();

	if (wcreceive(npats, patts)==ERROR) {
		exitcode=0200;
		canit();
	}
	restoremodes(FALSE);
	if (exitcode != 0)	/* bellow again with all thy might. */
		canit();
	exit(SS_NORMAL);
}


usage()
{
	fprintf(stderr,"rb %s by Chuck Forsberg\n", VERSION);
	fprintf(stderr,"Usage:	rb [-buv]\n\tor rb [-bcuv] file\n");
	exit(SS_NORMAL);
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
		if (fopen(*argp, "r") != NULL) {
			fprintf(stderr, "rb: %s exists\n", Pathname);
			goto fubar;
		}
		fprintf(stderr, "\nrb: ready to receive %s ", Pathname);
		if ((fout=fopen(Pathname, "w")) == NULL)
			return ERROR;
		if (wcrx()==ERROR)
			goto fubar;
	}
	return OK;
fubar:
	canit();
	if (fout)
		fclose(fout);
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
	long timep[2];

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
			if (fclose(fout)==ERROR) {
				canit();
				fprintf(stderr, "file close ERROR\n");
				return ERROR;
			}
			if (Modtime) {
				timep[0] = time(NULL);
				timep[1] = Modtime;
/*
				utime(Pathname, timep);
*/
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
			sectcurr=readline(5);
			if ((sectcurr+(oldcrc=readline(5)))==Wcsmask) {
				oldcrc=checksum=0;
				for (p=rxbuf, wcj=Blklen; wcj; wcj -= 128) {
					if (raw_read(128,p,7) == SS$_TIMEOUT) {
						firstch=TIMEOUT; goto bilge;
					}
					p += 128;
				}
				for (p=rxbuf, wcj=Blklen; --wcj>=0; ) {
					firstch = *p++ & Wcsmask;
					oldcrc=updcrc(firstch, oldcrc);
					checksum += (firstch);
				}
				if ((firstch=readline(5)) < 0)
					goto bilge;
				if (Crcflg) {
					oldcrc=updcrc(firstch, oldcrc);
					if ((firstch=readline(5)) < 0)
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
		else if (firstch==EOT && Lleft==0)
			return WCEOT;
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
		if (firstch != TIMEOUT)
			junkpacket();
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
	if (*p) {	/* file coming from Unix type system */
		sscanf(p, "%ld%lo%o", &Bytesleft, &Modtime, &Filemode);
		if (Filemode & UNIXFILE)
			++Thisbinary;
		if (Verbose) {
			fprintf(stderr,  "Incoming: %s %ld %lo %o\n",
			name, Bytesleft, Modtime, Filemode);
		}
	}
	else {		/* File coming from CP/M type system */
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
	strcpy(Pathname, name);
	if (Verbose) {
		fprintf(stderr,  "Receiving %s %s %s\n",
		name, Thisbinary?"BIN":"ASCII", openmode);
	}
	if ((fout=fopen(name, openmode)) == NULL)
		return ERROR;
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
/*
			if ( *p == '\r')
				continue;
*/
			if (*p == CPMEOF) {
				Eofseen=TRUE; return OK;
			}
			putc(*p ,fout);
		}
	}
	return OK;
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


report(sct)
int sct;
{
	if (Verbose>1)
		fprintf(stderr,"%03d%c",sct,sct%10? ' ' : '\r');
}

/* set tty modes for vvrb transfers */
setmodes()
{
/*  Device characteristics for VMS  */
#ifdef vms
	int	*iptr, parameters;

/*
 *	Get current terminal parameters.
 */
	if (gtty(&ttys) != SS$_NORMAL)
		error("SETMODES:  error return from GTTY (1)", FALSE);
	if (gtty(&ttysnew) != SS$_NORMAL)
		error("SETMODES:  error return from GTTY (2)", FALSE);

/*
 *	Set new terminal parameters.
 *	Note that there are three bytes of terminal characteristics,
 *	so we should make sure the fourth byte of the integer is unchanged.
 */
	iptr	= &(ttysnew.dev_characteristics.bcharacteristics);
	parameters	= *iptr;

	parameters	&= ~TT$M_ESCAPE;		/*  ESCAPE   OFF  */
	parameters	&= ~TT$M_HOSTSYNC;		/*  HOSTSYNC OFF  */
	parameters	|=  TT$M_NOECHO;		/*  NOECHO   ON   */
	parameters	|=  TT$M_PASSALL;		/*  PASSALL  ON   */
	parameters	&= ~TT$M_READSYNC;		/*  READSYNC OFF  */
	parameters	&= ~TT$M_TTSYNC;		/*  TTSYNC   OFF  */
	parameters	&= ~TT$M_WRAP;			/*  WRAP     OFF  */
	parameters	|= TT$M_EIGHTBIT;		/*  EIGHTBIT ON   */

	*iptr		= parameters;

	if (stty(&ttysnew) != SS_NORMAL)
		error("SETMODES:  error return from STTY", TRUE);
#endif
}



/* restore normal tty modes */
restoremodes(errcall)
int errcall;
{
/*  Device characteristic restoration for VMS  */
#ifdef vms
	if (stty(&ttys) != SS_NORMAL)		/*  Restore original modes  */
		{
		if (!errcall)
			error("Error restoring original terminal params.",
									FALSE);
		else
			{
			printf("vvrb/RESTOREMODES:  ");
			printf("Error restoring original terminal params.\n");
			}
		}
#endif
}

/*
	BBUFSIZ  - 128 or 1024 etc.
	raw_read(BBUFSIZ + 7, inbuf, 5 + 3 * (BBUFSIZ + 6));
*/




/* get a byte from data stream -- timeout if "dseconds" elapses */
/*	NOTE, however, that this function returns an INT, not a BYTE!!!  */
readline(dseconds)
{
	int seconds;
	int	c;

	seconds = dseconds/10;
	if (seconds < 5)
		seconds = 5;
#ifdef vms
	c	= raw_read(1, &c, seconds);

	if (c == SS$_TIMEOUT)
		return(TIMEOUT);

	return(c & 0377);  /* return the char */
#endif
}

/* junkpacket gets the line cleared */
junkpacket()
{
	int	c;

#ifdef vms
	for (;;) {
		c	= raw_read(1, &c, 1);

		if (c == SS$_TIMEOUT)
			return;
		}
#endif
}

/* send a byte to data stream */
sendline(data)
{
	char	dataout;

	dataout	= data;
#ifdef vms
	raw_write(dataout);
#endif

}


/* print error message and exit; if mode == TRUE, restore normal tty modes */
error(msg, mode)
char *msg;
int mode;
{
	if (mode)
		restoremodes(TRUE);  /* put back normal tty modes */
	printf("vvrb:  %s\n", msg);
	exit(SS_NORMAL);
}

