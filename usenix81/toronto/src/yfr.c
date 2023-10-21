/*
 * yfr -- minor local modification of xfr, the "standard" Toronto UNIX
 * file transfer program.  The only changes of note are the improvement
 * of error messages so that the name of the remote file is printed,
 * and the correction of a bug which caused checksums to be (a) incorrectly
 * encoded, and (b) ignored.
 *
 * For reasons of compatibility with other UNIX systems xfr itself should
 * remain untouched (at least until any changes can be distributed to all
 * installations using it), but for local UNIX-to-UNIX transfers the use of
 * yfr is to be recommended over xfr.
 *
 * Modifications by Ron Gomes -- 30 January 1979
 */

/*
 * UNIX to UNIX file transfer program.
 * Intended for dialup lines.
 * 
 * Unlimited useage rights are granted provided the following notice
 * appears in all source copies:
 *
 *	Copyright (c) 1978 Michael D. Tilson
 *	Human Computing Resources Corporation
 *	10 St. Mary Street
 *	Toronto, Ont., Canada
 *	416-922-1937
 *
 *
 */

/*
 * Communications protocol:
 *
 *	The local (master) sends a UNIX command to the remote (slave)
 *	system, to start a remote copy of xfr.  (This is the main
 *	drawback of xfr -- a remote copy is required.  However, one does
 *	not have to worry about various conventions concerning tabbing,
 *	newline expansion, erase characters, etc.  Also, most errors
 *	are detected and corrected.)
 *
 *	The remote xfr responds with REC_READY when it is started, or
 *	with an error message (which always begins with '*').  Any
 *	other response means that the remote copy did not start.
 *
 *	Once communications is established, records are sent as follows:
 *		Sender			Receiver
 *		------			--------
 *
 *		REC_READY -->		wait for REC_READY
 *		REC_READY -->		    .
 *		2 digit length -->	read to END_OF_RECORD
 *		record			    .
 *		2 digit chksum -->	    .
 *		END_OF_RECORD
 *		END_OF_RECORD
 *					<-- GOT_IT or RE_XMIT
 *		if any other response,
 *		SEND_LAST_CONTROL -->
 *					<-- GOT_IT or RE_XMIT
 *
 *	Three END_OR_RECORDS in a row terminates either side.  Normal
 *	end of transmission is a zero length record.  Within a sent
 *	record, any one character may be changed or deleted without
 *	loss of data.
 *
 *
 *
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sgtty.h>

#define USAGE	"**yfr error: usage:  yfr [-l line] [-s nnnn] [-p localfile] remotefile\n"

char *tty =	"/dev/cul0";
char *linespeed = "1200";
/* BT Oct/80 add to make the -s speed option actually work!! */
int speedcode;
char *speedtab[] {
	"0","50","75","110","135.5","150","200","300",
	"600","1200","1800","2400","4800","9600", 0
};
/* End BT Oct/80 add */
FILE *iofile;
int haveoutput;
int line;
struct sgttyb savstty;
int sending;
char *sendfile;
int master;
char *remotefile;
char *ttyname();

/*
 * communications protocol chars
 */
#define REC_READY	002
#define GOT_IT		006
#define SEND_LAST_CONTROL 007
#define RE_XMIT		025
#define END_OF_RECORD	'\n'

#define MODULUS		99
#define MODE		(EVENP|ODDP|RAW)

int savprot;

/* The following must be two digits or less. */
#define RECSIZE		50
char record[2*RECSIZE];	/* allow for variations on other peoples xfrs */
int recsize;
int eorcount;


extern quit();

main(argc, argv)
	char **argv;
{
	master++;

	for(argc--,argv++; argc>0 && argv[0][0]=='-'; argc--,argv++) {
		switch(argv[0][1]) {
		case 'r':
			/* not intended for users.  used by xfr on
			 * remote system to to indicate slave status.
			 */
			master = 0;
			if ((tty=ttyname(1)) == NULL)
				tty = "????????";
			break;

		case 'p':
			argc--;
			argv++;
			if(argc<1)
				usage();
			sendfile = argv[0];
			sending++;
			break;

		case 'l':
			argc--;
			argv++;
			if(argc<1)
				usage();
			else
				tty = argv[0];
			break;

		case 's':
			argc--;
			argv++;
			if(argc<=0)
				usage();
			linespeed = argv[0];
			break;
		default:
			usage();
			break;
		}
	}
	if((master || !sending) && (argc != 1))
		usage();

	remotefile = argv[0];

	/* BT Oct/80 more speed handling stuff.
	 * If we are a remote yfr, just use whatever
	 * line speed is inherited from the connection.
	 */
	if(master) {
		for(speedcode=0; strcmp(linespeed, speedtab[speedcode]); )
			if(speedtab[++speedcode] == 0)
				error("bad line speed");
	} else speedcode = -1;
	/* End BT Oct/80 add */
	openline();
	if(signal(SIGHUP,SIG_IGN)!=SIG_IGN) signal(SIGHUP, quit);
	if(signal(SIGINT,SIG_IGN)!=SIG_IGN) signal(SIGINT, quit);
	if(sending)
		send();
	else
		receive();
	quit(0);
}

usage() {
	fprintf(stderr, USAGE);
	quit(1);
}

quit(x) {
	signal(SIGHUP,SIG_IGN);
	signal(SIGINT,SIG_IGN);
	if(line) {
		stop();
		resetline();
	}
	if(haveoutput) {
		fflush(iofile);
	}
	exit(x);
}

struct stat statbuf;

openline() {
	line = open(tty, 2);
	if(line<0)
		error("can't open communications line");
	gtty(line, &savstty);
	stat(tty, &statbuf);
	savprot = statbuf.st_mode;
	chmod(tty, 0600);
}

send() {
	start();
	do {
		encode();
		sendrecord();
	} while(recsize>0);
}

receive() {
	start();
	while(getrecord())
		decode();
}

start() {
	register reply;
	openfile();
	if(master) {
		setline();
		sendxfrcommand();
		reply = readc();
		switch(reply) {
		case REC_READY:
			return;
		case '*':	/* start of error message from remote */
			error("remote yfr returns error");
		default:
			error("remote yfr does not start");
		}
	}
	else {
		setline();
		writec(REC_READY);
	}
}

stop() {
	register int i;
	for(i=0; i<6; i++)
		writec(END_OF_RECORD);	/* enough for all possible cases */
}

openfile() {
	if(sending) {
		if ((iofile=fopen(sendfile, "r")) == NULL)
			error("can't open input");
	}
	else {
		if(master)
			iofile = stdout;
		else if ((iofile=fopen(remotefile, "w")) == NULL)
			error("can't create output");
		haveoutput++;
	}
}

sendxfrcommand() {
	char work[200];
	register char *p;
	register c;
	if(sending)
		sprintf(work, "yfr -r %s", remotefile);
	else
		sprintf(work, "yfr -r -p %s", remotefile);
	p = work;
	while(*p)
		writec(*p++);
	writec('\n');
	p = work;
	while( *p == (c=readc()))
		p++;
	if(*p == 0) {
		if(c=='\r')
			c = readc();
		if(c=='\n')
			return;
	}
printf("expected '%s'; got %c\n", p, c);
	error("remote yfr command botched");
}

setline() {
	struct sgttyb buf;
	if(!line)
		return;
	gtty(line, &buf);
	if(speedcode > 0)
		buf.sg_ispeed = buf.sg_ospeed = speedcode;
	buf.sg_flags = MODE;
	stty(line, &buf);
}

resetline() {
	chmod(tty, savprot);
	stty(line, &savstty);
}

getrecord() {
	register int c, sum, csum;
	int i, nbytes;
	char num[3];

loop:
	while(readc()!=REC_READY)
		;
	while((c=readc()) == REC_READY)
		;
	for(i=0; i<sizeof num -1; i++) {
		if(digit(c))
			num[i] = c;
		else
			goto bad;
		c = readc();
	}
	num[i] = '\0';
	nbytes = atoi(num);
	if(nbytes<0 || nbytes>sizeof record)
		goto bad;
	sum = 0;
	for(i=0; i<nbytes; i++) {
		if(control(c) || c==END_OF_RECORD)
			goto bad;
		record[i] = c;
		sum += c;
		c = readc();
	}
	sum %= MODULUS;
	csum = 0;
	while(digit(c)) {
		csum = csum*10 + (c-'0');
		c = readc();
	}
	if((c!=END_OF_RECORD) || (csum!= sum))  /* csum formerly ignored */
		goto bad;
	writec(GOT_IT);
	return(recsize = nbytes);
bad:
	while(c!=END_OF_RECORD)
		c = readc();
	writec(RE_XMIT);
	goto loop;
}

sendrecord() {
	register int i, n, c;
	char work[3];
	int sum, retrys;
	retrys = 0;
loop:
	writec(REC_READY);
	writec(REC_READY);
	if(recsize<10)
		sprintf(work,"0%d",recsize);
	else
		sprintf(work,"%d", recsize);
	n = strlen(work);
	for(i=0; i<n; i++)
		writec(work[i]);
	sum = 0;
	for(i=0; i<recsize; i++) {
		c = record[i];
		sum += c;
		writec(c);
	}
	sum %= MODULUS;
	if(sum<10)
		sprintf(work,"0%d", sum);  /* Bug fix: "recsize" -> "sum" */
	else
		sprintf(work,"%d", sum);
	n = strlen(work);
	for(i=0; i<n; i++)
		writec(work[i]);
	writec(END_OF_RECORD);
	writec(END_OF_RECORD);
	while((c=readc())!=GOT_IT && c!=RE_XMIT)
		writec(SEND_LAST_CONTROL);
	if(c==RE_XMIT) {
		retrys++;
		if(retrys>5)
			error("too many retrys");
		goto loop;
	}
}

encode() {
	register int i, n;
	register c;
	char work[10];
	char *p;

	for(i=0; i<RECSIZE; i++) {
		c = getch();
		if(c== EOF)
			break;
		if(control(c)) {
			if(i>=RECSIZE-3) {
				ungetch(c);
				break;
			}
			if(c=='\n')
				ungetch('n');
			else {
				sprintf(work, "00%o", c);
				n = strlen(work);
				p = &work[n];
				if(n>3)
					n=3;
				do
					ungetch(*--p);
				while(--n);
			}
			c = '\\';
		}
		else if(c=='\\') {
			if(i>=RECSIZE -1) {
				ungetch(c);
				break;
			}
			record[i++] = c;
		}
		record[i] = c;
	}
	recsize = i;
}

#define NORMAL	0
#define NUMBER	1
#define ESCAPE	2
decode() {
	register int i, state;
	register c;
	int ec, numcount;

	state = NORMAL;
	for(i=0; i<recsize; i++) {
		c = record[i];
		if(state==ESCAPE) {
			if(c=='n') {
				state=NORMAL;
				putch('\n');
			}
			else if(c=='\\') {
				state=NORMAL;
				putch(c);
			}
			else if(digit(c)) {
				state = NUMBER;
				numcount = 1;
				ec = c-'0';
			}
			else {
				putch('\\');
				putch(c);
				state = NORMAL;
			}
		}
		else if(state==NUMBER) {
			if(numcount==3 || !digit(c)) {
				state = NORMAL;
				putch(ec);
				goto Norm;
			}
			numcount++;
			ec = ec*8 + (c-'0');
		}
		else {
		Norm:
			if(c=='\\')
				state = ESCAPE;
			else
				putch(c);
		}
	}
	if(state==NUMBER)
		putch(ec);
	else if(state==ESCAPE)
		error("invalid coding");
}


int lastsent;

readc() {
	int c;
loop:
	c = 0;
	if(read(line, (char *)&c, 1)!=1)
		error("line dropped");
	c &= 0177;
	if(c==END_OF_RECORD || c=='\r')
		eorcount++;
	else
		eorcount = 0;
	if(eorcount>=3)
		error("termination requested");
	if(c==SEND_LAST_CONTROL && control(lastsent)) {
		writec(lastsent);
		goto loop;
	}
	return(c);
}

writec(c) {
	write(line, (char *)&c, 1);
	lastsent = c;
}

digit(c) {
	return(c>='0' && c<='9');
}

control(c) {
	return(!((c>=040 && c<0177) || c=='\t'));
}

error(s)
char *s;
{
	fprintf(stderr, "**yfr error: %s. Remote file = %s.\n", s, remotefile);
	quit(1);
}

putch(c) {
	putc(c, iofile);
}

char pushback[10];
int npushed;

getch() {
	if(npushed)
		return(pushback[--npushed]&0377);
	return(getc(iofile));
}

ungetch(c) {
	pushback[npushed++] = c;
}
