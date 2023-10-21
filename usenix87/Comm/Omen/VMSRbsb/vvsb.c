#define VERSION "vvsb 1.03 01-26-86"

/*
 * vvsb.c By Chuck Forsberg
 *
 * A small program for VMS which can send 1 or more
 * files with YMODEM protocol to computers running YAM or compatible programs.
 * (Use the "rb" command in YAM.)
 * Supports the CRC option or regular checksum.
 *
 * accepts -k option for 1kb record length.
 *
 * Can send one file with XOMDEM protocol: use "sb -X file"
 *
 * Typical compile and install sequence:
 *		cc vvsb.c
 *		cc vvmodem.c
 *		link vvsb,vvmodem
 *	sb :== $disk$user2:[username.subdir]vvsb.exe
 *
 *	Manual page is "sb.1" in the Unix version rbsb.sh file
 */

#define LOGFILE "sblog"

#include stdio.h
#include ctype.h
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
#define TIMEOUT (-2)
#define RETRYMAX 10
#define SECSIZ 128	/* cp/m's Magic Number record size */
#define KSIZE 1024

char Lastrx;
char Crcflg;
int Verbose=0;
int Quiet=0;		/* overrides logic that would otherwise set verbose */
int Modem=0;		/* MODEM - don't send pathnames */
int Fullname=0;		/* transmit full pathname */
int firstsec;
int errcnt=0;		/* number of files unreadable */
int blklen=SECSIZ;	/* length of transmitted records */
int Totsecs;		/* total number of sectors this file */
char txbuf[KSIZE];
int Filcnt=0;		/* count of number of files opened */

unsigned updcrc();
char *substr(), *getenv();



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

	npats=0;
	if (argc<2)
		goto usage;
#ifndef REGULUS
	setbuf(stdout, xXbuf);		
#endif
	while (--argc) {
		cp = *++argv;
		if (*cp == '-') {
			while ( *++cp) {
				switch(*cp) {
				case 'f':
					Fullname=TRUE; break;
				case 'k':
					blklen=KSIZE; break;
				case 'q':
					Quiet=TRUE; Verbose=0; break;
				case 'v':
					++Verbose; break;
				case 'x':
				case 'X':
					++Modem; break;
				default:
					goto usage;
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
	if (npats < 1) {
usage:
		fprintf(stderr,"%s by Chuck Forsberg\n", VERSION);
		fprintf(stderr,"Usage: sb [-fkqv] file ... (YMODEM)\n");
		fprintf(stderr,"or     sb -X [-kqv] file   (XMODEM)\n");
		exit(SS_NORMAL);
	}
	
	if (Verbose) {
		if (freopen(LOGFILE, "a", stderr)==NULL) {
			printf("Can't open log file %s\n",LOGFILE);
			exit(SS_NORMAL);
		}
		setbuf(stderr, NULL);
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

	setmodes();

	if (wcsend(npats, patts)==ERROR) {
		exitcode=0200;
		canit();
	}
	fflush(stdout);
	restoremodes(FALSE);
	exit(SS_NORMAL);
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

wcs(name)
char *name;
{
	register c;

	if ((in=fopen(name, "r"))==NULL) {
		++errcnt;
		return OK;	/* pass over it, there may be others */
	}
	++Filcnt;
	if (wctxpn(name)!= ERROR)
		return wctx();
	else {
		return ERROR;
	}
}

/*
 * generate and transmit pathname block consisting of
 *  pathname (null terminated),
 *  file length provided by RMS
 */
wctxpn(name)
char *name;
{
	register firstch;
	register char *p, *q;
	long filestat();

	uncaps(name);		/* Get it out of VMS RADIX50 */

	if (Modem)
		return OK;
	logent("\r\nAwaiting pathname nak for %s\r\n", *name?name:"<END>");
	if ((firstch=readline(800))==TIMEOUT) {
		logent("Timeout on pathname\n");
		return ERROR;
	}
	if (firstch==WANTCRC)
		Crcflg=TRUE;
	for (p=name, q=txbuf ; *p; )
		if ((*q++ = *p++) == '/' && !Fullname)
			q = txbuf;
	*q++ = 0;
	p=q;
	while (q < (txbuf + KSIZE))
		*q++ = 0;

	if (*name)
		sprintf(p, "%lu", filestat(name));

	/* force 1k blocks if name won't fit in 128 byte block */
	if (strlen(txbuf) > 127)
		blklen=KSIZE;
	if (wcputsec(txbuf, 0, SECSIZ)==ERROR)
		return ERROR;
	return OK;
}

wctx()
{
	register int sectnum, attempts, firstch;

	firstsec=TRUE;

	while ((firstch=readline(400))!=NAK && firstch != WANTCRC
	  && firstch!=TIMEOUT && firstch!=CAN)
		;
	if (firstch==CAN) {
		logent("Receiver CANcelled\n");
		return ERROR;
	}
	if (firstch==WANTCRC)
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
		readline(1);
		sendline(EOT);
		fflush(stdout);
		++attempts;
	}
		while ((firstch=(readline(100)) != ACK) && attempts < RETRYMAX);
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
		for (cp=txbuf, wcj=cseclen; wcj; wcj -= 128) {
			raw_wbuf(128, cp);
			cp += 128;
		}
		for (cp=txbuf, wcj=cseclen; --wcj>=0; ) {
			oldcrc=updcrc(*cp, oldcrc);
			checksum += (*cp++);
		}
		if (Crcflg) {
			oldcrc=updcrc(0,updcrc(0,oldcrc));
			sendline((int)oldcrc>>8);
			sendline((int)oldcrc);
		}
		else
			sendline(checksum);


		firstch = readline(400);
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
		default:
			logent("Got %02x for sector ACK\n", firstch);
			break;
		}
		for (;;) {
			Lastrx = firstch;
			if ((firstch = readline(400)) == TIMEOUT)
				break;
			if (firstch == NAK || firstch == WANTCRC)
				goto gotnak;
			if (firstch == CAN && Lastrx == CAN)
				goto cancan;
		}
	}
	if (Verbose)
		logent("Retry Count Exceeded\n");
	return ERROR;
}


/* fill buf with count chars padding with ^Z for CPM */
filbuf(buf, count)
register char *buf;
{
	register c, m;
	m=count;
	while ((c=getc(in))!=EOF) {
		*buf++ =c;
		if (--m == 0)
			break;
	}
	if (m==count)
		return 0;
	else
		while (--m>=0)
			*buf++ = CPMEOF;
	return count;
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


/* set tty modes for sb transfers */
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
			printf("sb/RESTOREMODES:  ");
			printf("Error restoring original terminal params.\n");
			}
		}
#endif
}

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
	printf("sb:  %s\n", msg);
	logent("Fatal Error:  %s\n", msg);
	exit(SS_NORMAL);
}


/* make string s lower case */
uncaps(s)
register char *s;
{
	for ( ; *s; ++s)
		if ( ! (*s & 0200))
			*s = tolower(*s);
}


