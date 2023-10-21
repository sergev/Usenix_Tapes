/*
 * @(#)uuslave.c	Version hoptoad-1.11	87/03/23
 *
 * (C) Copyright 1987 by John Gilmore.
 * Copying and use of this program are controlled by the terms of the Free
 * Software Foundation's GNU Emacs General Public License.
 *
 * Derived from:
 * i[$]uuslave.c	1.7 08/12/85 14:04:20
 * which came from the ACGNJ BBS system at +1 201 753 9758.  Original
 * author unknown.
 */

char version[] = "Version hoptoad-1.11";

/*
 * Modified to compile on BSD systems by John Gilmore (hoptoad!gnu), Jan 1987.
 * 
 * Separated into layers and acknowledgement (middle) layer added, Feb 1987,
 * by John Gilmore.
 *
 * Incorporates a few of the "very extensive and brutal hacks by Marcus J.
 * Ranum",  posted to the net 6 Feb 1987 by mjranum@osiris.uucp.
 *
 * Modified to compile on CP/M by Larry Marek (ihnp4!strg8!larry), March '87.
 *
 * Modified to compile with Microsoft C 4.0 by Tim Pozar (hoptoad!pozar),
 * February and March of 1987:
 *
 *    ver 0.1     11.Mar.1987
 *    Since there isn't any bps rate detection at this point, the machine will 
 *    only work with whatever it was last set with.  Also, PC-DOS and IBM's
 *    serial port seem to have a slight problem with buffering and PC-DOS's
 *    stupid way of handling "read file" errors with the com ports.  PC-DOS
 *    isn't graceful in it's handling and will expect an operator to be at
 *    the physical system console to tell it to "retry".  BPS rate detection
 *    and buffered I/O will be the next code to be plugged into this beast. 
 *    
 *    Basically this guy shouldn't be released yet...
 *
 *    ver 0.2     13.Mar.1987
 *    Hacked to support the COMPORT.ASM/OBJ/H to speed up handshaking and 
 *    char interchange without DOS problems.
 *  
 * Changes from trwrb!sansom (Richard Sansom), 24 Feb '87, to get it to
 * compile on the Atari ST.
 *
 * Above changes merged back into master sources by John Gilmore, 23Mar87.
 *
 */

/*

This program implements the uucp (Unix-to-Unix CoPy) protocol as a
slave (the recipient of a phone call from another Unix system).  This
protocol is used to transfer mail, files, and Usenet news from Unix
machine to Unix machine.  UUCP comes with Unix (unless you get a
sleazeoid version like Xenix, where they charge you extra for it).  You
can buy a commercial program for MSDOS, called UULINK, which also
implements this protocol.  UULINK costs $300 and you don't get sources,
though.

The protocol requires a full 8-bit data path with no characters inserted
or deleted (e.g. ^S and ^Q are used as DATA characters).  Simple serial
ports and modems do this; most complicated networks do not, at least without
setting up odd modes and such.  Telenet's PC Pursuit works fine though.

The basic flow of the protocol is that the calling machine will send down
a line of text saying what it wants to do (send a file, receive a file,
or hang up).  (The lines of text are encapsulated into packets; see below.)
The called machine responds with a "yes" or "no" answer, and if the answer
was yes, it sends or receives the file.  Files are terminated with a
packet containing 0 bytes of data.  Then the system that received the file
sends a "copy succeeded" or "copy failed" line to the other end, and 
they go back to "what do we do now".  A request to hang up should be
answered "no" if the called machine has some mail or files it wants to
send to the calling machine; the two machines reverse roles and the calling
machine goes into "what do we do now".  If a hangup request is answered "yes",
the call is terminated.

The data flow described above is actually sent in packets containing
checksums and acknowledgements.  Each packet can either hold a short
control message, e.g. an ack, or a data block.  The data blocks are
numbered with a 3-bit counter, and sent with a checksum.  If the sender
has not received an acknowledgement for a data block within a certain
time, it retransmits the block.  The size of a data block is negotiated
at the start of a call.  To send a block with fewer bytes, a "short
data" block is sent, which is just as big as a "long data" block, but
contains a 1- or 2-byte count of "how many bytes in this block are just
padding".  This is a cute trick since it always works (e.g. if you want
to send 1023 out of 1024 bytes, you only need one byte for the count;
while if you want to send 1 byte out of 1024 then you have enough space
for the count to be 2 bytes).

The short control messages are used to start the call and negotiate the
packet size and the "window size", to acknowledge or reject packets,
and to terminate the packet protocol at the end of a call.  The window
size is how many packets one side can send before it will stop and wait
for an acknowledgement from the other side.  A window size of 1 makes
for a half-duplex protocol (which is what uuslave currently
implements), but also makes it easy to implement on micros that don't
handle serial lines with interrupts.  In window 1, you just keep
sending the same packet until the other side acknowledges it.  Unix
always uses a window size of 3, which is the max that can be dealt with
given the 3-bit packet numbers (for reasons that would take more space
than I want to spend here).  This gives much better throughput, but
requires full duplex serial port handling and more complicated
acknowledgement strategies.

At the low level, full 8-bit bytes are sent out and received on an async serial
port.  The received data is scanned for a DLE (hex 10) which indicates the
start of a packet, and the next 5 bytes are read and checked.  If they
pass, this is a good packet and it is acted upon.  (If it's a data
packet, we have to read in and check the data part too.)  If the checks
fail, all the bytes read so far should be scanned for another DLE.

Include files for various supported systems:
Note that NAMESIZE should be the max length of a file name, including
all its directories, drive specifiers, extensions, and the like.
E.g. on a Unix with 14-char file names, NAMESIZE is several hundred
characters, since the 14-char names can be nested.
*/

#ifdef BSD
/* Unix Berserkeley systems */
#include <stdio.h>
#include <ctype.h>
#include <sgtty.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/time.h>
#include <signal.h>
#include <setjmp.h>

#define	UNIX
#define	NAMESIZE	MAXPATHLEN
#endif

#ifdef SYSV
/* Unix System V */
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <termio.h>
#include <signal.h>
#include <setjmp.h>

#define	UNIX
#endif

#ifdef UNIX
/* Stuff common to all Unix systems */
#define	MULTITASK
#define	STDIN		0
#define	SPOOLDIR	"/usr/spool/uucp"
#define	PUBDIR		"/usr/spool/uucppublic"
#define	LOGFILE		"LOGFILE"
#endif

#ifdef CPM
/* CP/M-80 */
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>

#define	NAMESIZE	50		/* No directories... */
#endif

#ifdef MSDOS
/* Microsoft DOS */
/* Turn on support for the interrupt driven comm port routines */
#define	COMPORT

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>
#include <dos.h>
#include <conio.h>
#ifdef COMPORT
#include <comport.h>
#endif

typedef struct timetype {
	unsigned hour;
	unsigned minute;
	unsigned sec;
	unsigned hsec;
} TIME, *TIME_PTR;

#define	NAMESIZE	512		/* Pulled out of a hat */
#endif

#ifdef ST
/* Atari ST */
#include <stdio.h>
#include <ctype.h>
#include <osbind.h>
#include <signal.h>
#include <setjmp.h>

#define O_RDONLY	0	/* for read only open() */
#define AUX		1	/* rs232 port */
#define CON		2	/* console */
#define NAMESIZE	13	/* filename size */
#define CTRL(X)	(X & 037)

#endif

#define	MAGIC	0125252		/* checksum is subtracted from this */

/*
 * What is sent from one machine to the other is a control byte and
 * sometimes a data block following.
 * A packet is a just this, with a frame around the control byte and the
 * data, if any, after the frame.  The frame is 6 bytes long, and looks like:
 *	DLE K C0 C1 C X
 * where:
 *	DLE is a literal ASCII DLE (Data Link Escape) character
 *	K is binary, 9 for a control packet, or the packet size log 2, minus 4
 *		e.g. K = 2 means 64 byte packet since  (K+4) is  6 or 64.
 *						      2         2
 *	C0 and C1 are low, high order checksums for the entire data section, or
 *		are low, high order of (MAGIC minus the control byte).
 *	C is the control byte for the message.
 *	X is the xor of K, C0, C1, and C.
 * If a packet does not satisfy all of the above checks, it is invalid.
 */
#define	DLE	0x10		/* Start of packet indicator */

#define	LENBYTE	1		/* Byte offset from DLE to length */
#define	CBYTE	4		/* Byte offset from DLE to control */
#define	FRAMEBYTE 5		/* Byte offset from DLE to end of frame */

#define	KCONTROL 9		/* K value for control packets */

/*
 * A control byte is split into three bit fields: type, x, and y.
 * 	TT XXX YYY
 * Here are the types:
 */
#define	CONTROL	0		/* Control message */
#define	ALTCHN	1		/* Alternate channel message, unused in UUCP */
#define	LONGDATA	2	/* Long data block -- full size spec'd by K */
#define	SHORTDATA	3	/* Short data block -- first byte or two is
				   count.  Full K-size packet is sent, even
				   though the useful data is shorter. */

char *tt_pr[] = {"CONTROL", "ALTCHN", "LONGDATA", "SHORTDATA"};

/* If TT == CONTROL (also K will == KCONTROL) then
   the x field is: 			and the Y field means: */
#define	CLOSE	1	/* End of communication */
#define	RJ	2	/* Reject packet	last good packet # seen */
#define	SRJ	3	/* Selective reject	seq # of bad packet, resend
			   SRJ is not used by UUCP. */
#define	RR	4	/* Receiver Ready	last good packet # seen */
#define	INITC	5	/* Init phase C		window size to hand sender */
#define	INITB	6	/* Init phase B		max data segment size (K val)
						to hand sender */
#define	INITA	7	/* Init phase A		window size to hand sender */

char *ctrl_pr[] = {"*ZERO*",
	"CLOSE", "RJ", "SRJ", "RR", "INITC", "INITB", "INITA"};

/*
 * If TT == LONGDATA or SHORTDATA then x field is the sequence # of this packet
 * and y field is the last good packet # seen.
 *
 * In both data and RJ/RR packets, the "last good packet # seen" starts off
 * as zero.
 */


/*
 * Timeout for raw characters -- if we don't hear a char within BYTE_TIMEOUT
 * seconds, we assume the other side has gone away.  Has nothing to do with
 * retransmission timeouts (if any!).
 */
#define	BYTE_TIMEOUT	60

#define	MAX_PACKET	4096
#define	SLOP		10		/* Frame header, ctrl, slop */
#define	MAX_FLAGS	40

#ifndef LOG
#define	logit(one, two)	/* Nothing */
#endif

extern int errno;

unsigned char	msgi[MAX_PACKET+SLOP],	/* Incoming packet */
		msgo[MAX_PACKET+SLOP];	/* Outgoing packet */

char	ttynam[NAMESIZE],		/* Name of tty we use as serial port */
	cmnd[8],			/* Trashplace to put command letter */
	srcnam[NAMESIZE],		/* Source file name */
	dstnam[NAMESIZE],		/* Dest file name */
	dskbuf[MAX_PACKET],		/* Disk I/O buffer */
	who[NAMESIZE] = "-",		/* Who sent the file */
	flags[MAX_FLAGS],		/* Flags from file xfer cmd */
	temp[NAMESIZE],			/* Temp file name */
	msgbld[(NAMESIZE*4)+SLOP];	/* Top level message storage */
int	fdtty,				/* Terminal line (to unix uucp) file
					   descriptor */
	logfd,				/* file desc of uucp logfile */
	ourpid = 0,			/* Our process ID */
	firstslave,			/* First packet of slave's session */
	mode,				/* File mode from file xfer cmd */
	msgsize,			/* Size of data part of msg */
	tt, xxx, yyy,			/* Fields from control byte */
	rseq,				/* Last good packet # we received */
	his_rseq,			/* Last good packet # HE received */
	wseq;				/* Next packet # we will send */

int	last_op;			/* Last data op: OP_READ or OP_WRITE */
#define	OP_READ		0
#define	OP_WRITE	1

int	reject;				/* Packet # to reject or NOREJECT */
#define	NOREJECT	-1

#ifndef CPM
jmp_buf alarming;			/* For read timeouts */
#endif

#ifdef BSD
	struct sgttyb atermio, btermio;
#endif
#ifdef SYSV
	struct termio atermio, btermio;
#endif

/* Segments are encoded as (log2 length) - 3, except in INITB packet */
int wndsiz = 1;			/* Ask for window of 1 messages flying */
int segsiz = 2;			/* Ask for 64 byte messages */
int sendseg = 2;		/* Size segments other guy wants to see */
int sendwin = 1;		/* Size window other guy wants to see */

int sendbytes;			/* sendseg, in bytes */

int	segbytes[10] = {	/* K value (encoded segment size) */
		-1,		/* 0 */
		32,		/* 1 */
		64,		/* 2 */
		128,		/* 3 */
		256,		/* 4 */
		512,		/* 5 */
		1024,		/* 6 */
		2048,		/* 7 */
		4096,		/* 8 */
		0,		/* 9 = KCONTROL */
};

#ifdef MSDOS
char hayesinit[] = "\r\r\rATS0=1\r";
#endif

/* We print these prompts */
char msgo0[] = "login: ";
char msgo1[] = "Password:";
char msgo2[] = "\20Shere\0";
char msgo3[] = "\20ROK\0";
char msgo3a[]= "\20Pg\0";
char msgo4[] = "\20OOOOOOO\0";

/* We expect to receive these strings */
char msgi0[] = "uucp\n";
char msgi1[] = "s8000\n";
char msgi2[] = "\20S*\0";
char msgi3[] = "\20Ug\0";
char msgi4[] = "OOOOOO";

/*
 * Basement level I/O routines
 *
 * xwrite() writes a character string to the serial port
 * xgetc() returns a character from the serial port, or an EOF for timeout.
 * sigint() restores the state of the serial port on exit.
 */

#ifdef CPM
#define	abort()	exit(1)
extern xgetc(), xwrite(), sioinit();
#endif

sigint()
{
	/* Restore terminal settings on dialout line */
#ifdef BSD
	ioctl(fdtty, TIOCSETN, &atermio);
        close(fdtty);
#endif

#ifdef SYSV
	ioctl(fdtty,TCSETA,&atermio);
        close(fdtty);
#endif

#ifdef MSDOS
#ifndef COMPORT
        close(fdtty);
#else
        uninit_comm();
        reset_tty();
#endif
#endif
#ifdef ST
	/* No need to do anything here? */
#endif

	exit(0);
}

#ifdef MSDOS
xwrite(fd,buf,ctr)
int fd;
char *buf;
int ctr;
{
#ifndef COMPORT
	return write(fd,buf,ctr);
#else
	int i;

	for (i=0;i<=ctr;i++) {
		outp_char(buf[i]);
	}
	return ctr;
#endif
}
#endif

#ifdef ST
/*
 * xwrite(dummy, buf, n) - write "n" bytes from buffer "buf" to rs232 port.
 */
xwrite(dummy, buf, n)
	int dummy, n;
	char *buf;
{
	register char *c = buf;

	while (n) {
		/* wait for rs232 to be ready */
		while (Bcostat(AUX) == 0)
			;

		/* write next character in buffer */
		Bconout(AUX, c++);
		n--;
	}
}
#endif

#ifdef UNIX
#define	xwrite	write		/* Just use write system call */
#endif

/*
 * Serial port reading routines 
 */

#ifdef UNIX
sigalrm()
{
	longjmp(alarming, 1);
}

/*
 * FIXME:  This is really slow; it does 4 system calls per byte!
 */
xgetc()
{
	char data;
	int status;

	signal(SIGALRM,sigalrm);
	alarm(BYTE_TIMEOUT);
	if (setjmp(alarming) == 0) 
	{
		status = read(fdtty,&data,1);
		alarm(0);
		if (status == 1)	/* the read worked, returning 1 char */
			return(data & 0xFF);
	}
	/* Error on serial port, or timeout. */
	return(EOF);
}
#endif

#ifdef MSDOS
xgetc()
{
	char data;

#ifndef COMPORT
	/* Warning: No timeouts... */
	read(fdtty,&data,1);
	return(data & 0xFF);
#else
	int i;
	unsigned s;
	TIME n;

	i = 0;
	get_time(&n);
	s = n.sec;

	/*
	 * Implement timeouts by staring at the clock while we wait.
	 * When the second hand moves, bump our counter.  This is a lot
	 * easier than figuring out the time when we'd time out (in hours,
	 * minutes, and seconds!) and comparing against that, which is
	 * what people tend to do in Unix where the time is just an integer
	 * number of seconds.
	 */
	while (i < BYTE_TIMEOUT) {
		while (s == n.sec) {
			if(inp_cnt() != 0) {
				data = inp_char();
				return (data & 0xFF);
			}
			get_time (&n);
		}
		s = n.sec;
		++i;
	}
	return(EOF);
#endif
}
#endif /* MSDOS */

#ifdef ST
/*
 * Atari ST routines for reading the comm port.
 *
 * The following routines come by way of J. R. Bammi
 */

long prtime = 0L;			/* present time			*/
long alrmtime = 0L;			/* alarm time			*/
long *hz200 = (long *)0x0004ba;		/* address of system 200 hz clk	*/
jmp_buf abortenv;			/* used to catch user aborts	*/

/*
 * read200hz() - read the system 200 hz clock.
 */
void read200hz()
{
	prtime = *hz200;
}

/*
 * alarm(n) - set the alarm time to n seconds.
 */
void alarm(n)
	unsigned int n;
{
	/* if n != 0, then set the alarm time */
	if (n) {
		Supexec(read200hz);
		alrmtime = prtime + (long)(200 * n);

	/* n == 0, reset alarm time */
	} else
		alrmtime = 0L;
}

/*
 * xgetc() - get a character from the rs232 port, catch timeouts & aborts.
 */
xgetc()
{
	if (setjmp(alarming))
		return(EOF);
	alarm(BYTE_TIMEOUT);
	while (1) {
		/* catch ^C (user abort) */
		if (Bconstat(CON))
			if ((int)Bconin(CON) == CTRL('C'))
				longjmp(abortenv, 1);

		/* check for a char at rs232 port */
		if (Bconstat(AUX)) {
			alarm(0);
			return((int)Bconin(AUX));

		/* no char, check for timeout */
		} else if (alrmtime) {
			Supexec(read200hz);
			if (prtime >= alrmtime)
				longjmp(alarming, 1);
		}
	}
}
#endif

/*
 * Low level output routines.  These send packets without checking
 * whether they got properly received.
 *
 * writeframe():
 *
 * Finish off an outgoing frame in msgo and queue it up to be written
 * to the serial port.
 *
 * This routine is called to send each and every packet.
 */
int
writeframe(cksm)
	int cksm;
{
	
	msgo[0] = DLE;
	msgo[2] = cksm;
	msgo[3] = cksm >> 8;
	msgo[5] = msgo[1] ^ msgo[2] ^ msgo[3] ^ msgo[4];

#ifdef DEBUG
	{
		int tt, xxx, yyy, index, maxlen;

		printf("T ");
		maxlen = segbytes[msgo[LENBYTE]] + 6;
		for (index = 0; index < maxlen; index++)
			printf("%02x  ",msgo[index] & 0xFF);
		putchar('\n');
		tt = msgo[CBYTE] >> 6;
		xxx = (msgo[CBYTE] >> 3) & 7;
		yyy = msgo[CBYTE] & 7;
		if (tt == CONTROL)
			printf("Sent: CONTROL %s %d\n",
				ctrl_pr[xxx], yyy);
		else
			printf("Sent: %s %d %d\n",
				tt_pr[tt], xxx, yyy);
	}
#endif

	/*
	 * In our window=1 implementation, we just queue the packet
	 * up for transmission here (by leaving it in msgo[]).  It
	 * will be written next time we go through inpkt().
	 */
	last_op = OP_WRITE;	/* Remember to avoid overwriting the packet */
	return 0;		/* Never aborts */
}

/* Send an ack */
int
ackmsg()
{

	msgo[1] = KCONTROL;
	if (reject != NOREJECT)
		msgo[4] = (CONTROL << 6) | (RJ << 3) | reject;
	else
		msgo[4] = (CONTROL << 6) | (RR << 3) | rseq;
	reject = NOREJECT;
	return writeframe(MAGIC - msgo[4]);
}


/* Send a control message other than an ack */
int
ctlmsg(byte)
char byte;
{

	msgo[1] = KCONTROL;
	msgo[4] = (CONTROL << 6) | byte;
	return writeframe(MAGIC - msgo[4]);
}

/*
 * Medium level output routine.  This sends a short or long data packet
 * and figures out when to retransmit and/or insert acknowledgements as
 * needed.
 */
sendpacket(s, n, sorl)
	char *s;
	int n;
	int sorl;			/* SHORTDATA or LONGDATA */
{
	int cksm, offset, difflen;

	if (last_op == OP_WRITE) {
		/* Better get the first one sent and ack'd first */
		/* FIXME, this will change for window > 1 */
		if (inpkt()) return 1;
	}

	bzero(msgo+6, sendbytes);
	msgo[1] = sendseg;
	msgo[4] = (sorl << 6) + (wseq << 3) + rseq;

	switch(sorl) {
	case LONGDATA:
		if (n > sendbytes) abort();
		offset = 6;
		break;

	case SHORTDATA:
		difflen = sendbytes - n;
		if (difflen < 1) abort();
		offset = 7;
		if (difflen <= 127) {
			msgo[6] = difflen;	  /* One byte count */
		} else {
			msgo[6] = 128 | difflen;  /* low byte, with 0x80 on */
			msgo[7] = difflen >> 7;   /* High byte */
			offset = 8;
		}
	}

	bcopy(s, msgo+offset, n);		/* Move the data */

	cksm = MAGIC - (chksum(&msgo[6], sendbytes) ^ (0377 & msgo[4]));
	wseq = (wseq + 1) & 7;			/* Bump sent pkt sequence # */
	return writeframe(cksm);
}

/*
 * Medium level input routine.
 *
 * Look for an input string for the send-expect sequence.
 * Return 0 for matching string, 1 for timeout before we found it.
 * FIXME:  we only time out if the other end stops sending.  If it
 *	   keeps sending, we keep listening forever.
 */
instr(s,n)
register char *s;
register int n;
{
	int data,count,j;
	register int i;

	count = 0;
#ifdef DEBUG
	printf("Expecting ");
	for (i = 0; i < n; i++)
		printf("%02x%c ",s[i] & 0xFF, isprint(s[i])? s[i]: ' ');
	printf("\nR ");
#endif
	while ((data = xgetc()) != EOF)
	{
		msgi[count++] = data & 0x7F;
#ifdef DEBUG
		printf("%02x%c ",msgi[count-1],
			 isprint(msgi[count-1])? msgi[count-1]: ' ');
#endif
		if (count >= n)
		{
			for (i = n - 1, j = count - 1; i >= 0; i--, j--)
				if (*(s+i) == '*' || *(s+i) != msgi[j])
					break;
			if (i < 0 || *(s+i) == '*')
			{
#ifdef DEBUG
				putchar('\n');
#endif
				return(0);
			}
		}
	}
#ifdef DEBUG
	putchar('\n');
#endif
	msgi[count] = 0;
	return(1);
}

/*
 * Medium level input routine.
 *
 * Write a packet to the serial port, then read a packet from the serial port.
 * Return 1 if other side went away, 0 if good packet received.
 *
 * With window size of 1, we send a packet and then receive one.
 * FIXME, when we implement a larger window size, this routine will become
 * more complicated and will callers will not be able to depend on msgo[]
 * being sent and acknowledged when it returns.
 */
int
inpkt()
{
	int data,count,need;
	register int i;
	short pktsum, oursum;
	int status;
	/*
	 * Next vars are for re-queueing received chars to rescan them
	 * for a valid packet after an error.
	 */
	int queued = -1;  /* <0: off, 0: just finished, >0: # chars pending */
	unsigned char *qp;
	unsigned char qbuf[sizeof msgi];	/* This can be static if 4K
						   on the stack is too much */
#	define	bad(str) {printf str; goto oops; }

	if (firstslave) {
		firstslave = 0;
		goto again;
	}

xmit:
	i = segbytes[msgo[LENBYTE]] + 6;
	status = xwrite(fdtty,msgo,i);
	if (status != i) {
#ifdef DEBUG
		printf("xmit %d bytes failed, status %d, errno %d", 
			i, status, errno);
#endif
		return 1;		/* Write failed */
	}

again:
	count = 0;

#ifdef DEBUG
	printf("R ");
#endif

	while (1) {
		if (queued >= 0) {
			/*
			 * Process some stuff from a string.
			 * If we just finished the last char queued, and
			 * we are still scanning for a DLE, re-xmit our
			 * last packet before we continue reading.
			 * On the other hand, if we have a valid packet
			 * header accumulating, just keep reading the serial
			 * port.
			 */
			if (--queued < 0)
				if (count == 0) {
#ifdef DEBUG
					printf("End queue.  Re-xmit.\n");
#endif
					goto xmit; /* No packet comin' in */
				} else {
#ifdef DEBUG
					printf("End queue.  Keep reading\n");
#endif
					goto readser; /* Seems to be sumpin' */
				}
			data = *qp++;		/* Just grab from queue */
		} else {
readser:
			data = xgetc();
			if (data == EOF) break;
		}
#ifdef DEBUG
		printf("%02x%c ",data & 0xFF, isprint(data)? data: ' ');
#endif
		switch (count)
		{
		case 0:
			/* Look for DLE */
			if (data == DLE)
				msgi[count++] = DLE;
			break;

		case LENBYTE:
			/* Check length byte */
			if (data > KCONTROL || data == 0)
				bad(("packet size"));
			if (segbytes[data] > MAX_PACKET) {
				bad(("packet too long for buffer"));
		oops:
				printf(" bad in above packet\n");

				/* FIXME, decode packet header here,
				   if enough of it has come in. */

				/* See if any DLEs in the bad packet */
				/* Skip 0, we know that's a DLE */
				for (i = 1; i < count; i++) {
					if (msgi[i] == DLE) {
						/* Reprocess from the DLE.
						 * if queued, back up the q.
						 * if not, make one.
						 */
						if (queued) {
							queued += count - i;
							qp -= count - i;
						} else {
							bcopy(msgi+i, qbuf, 
								count - i);
							qp = qbuf;
							queued = count - i;
						}
#ifdef DEBUG
						printf("Rescan input\n");
#endif
						goto again;
					}
				}

				if (queued >= 0) {
#ifdef DEBUG
					printf("Continue scan\n");
#endif
					goto again;
				} else {
#ifdef DEBUG
					printf("Re-xmit previous packet\n");
#endif
					goto xmit;	/* Xmit then rcv */
				}
			}
			msgi[count++] = data;		/* Save it */
			msgsize = segbytes[data];	/* Save Packet size */
			need = 6 + msgsize;
			break;

		case CBYTE:
			/* Break up control byte as well as storing it */
			msgi[count++] = data;		/* Save it */
			tt = (data >> 6) & 3;
			xxx = (data >> 3) & 7;
			yyy = data & 7;

			/* Now check it a bit */
			switch (tt) {		/* Switch on msg type */
			case CONTROL:
				/* Control msg must have KCONTROL size */
				if (msgsize != 0) bad(("K versus Control"));
				/* We don't implement SRJ, nor does Unix */
				switch (xxx) {
				case SRJ:
					bad(("SRJ received"));
				case RJ:
				case RR:
					if (yyy != (7 & (wseq - 1)))
						bad(("didn't ack our pkt"));
				}
				break;
			
			case ALTCHN:
				bad(("ALTCHN received")); /* Unsupported */

			case SHORTDATA:
			case LONGDATA:
				if (msgsize == 0) bad (("KCONTROL with data"));
				if (((xxx - rseq) & 7) > wndsiz) {
					/* Atari ST cpp has problems with
					 * macro args broken across lines?
					 * That's why funny indent here
					 */
bad (("data out of window, xxx=%d rseq=%d", xxx, rseq));
				}
				/* FIXME, below enforces window size == 1 */
				/* Note that this is also how we guarantee
				   that msgo has been received OK by the time
				   we exit inpkt() too.  Don't change it unless
				   you know what you are doing. */
				if (yyy != (7 & (wseq - 1)))
					bad(("didn't ack our pkt"));
				break;
			}
			break;

		case FRAMEBYTE:
			/* See whole frame, check it a bit. */
			msgi[count++] = data;
			if (data != (msgi[1] ^ msgi[2] ^ msgi[3] ^ msgi[4]))
				bad(("frame checksum"));
			pktsum = msgi[2] + (msgi[3] << 8);

			if (tt == CONTROL) {
				/* Check checksums for control packets */
				oursum = MAGIC - msgi[4];
				if (pktsum != oursum)
					bad(("control checksum"));
				/*
				 * We have a full control packet.
				 * Update received seq number for the ones
				 * that carry one.
				 */
				switch (xxx) {
				case RJ: case RR:
					if (((wseq - yyy) & 7) > sendwin) {
bad (("RJ/RR out of window, yyy=%d wseq=%d", yyy, wseq));
					}
					his_rseq = yyy;
				}
				goto done;
			} else {
				/*
				 * Received frame of data packet.
				 *
				 * Now that the checksum has been verified,
				 * we can believe the acknowledgement (if
				 * any) in it.
				 */
				if (((wseq - yyy) & 7) > sendwin) {
bad (("data ack out of window, yyy=%d wseq=%d", yyy, wseq));
				}
				his_rseq = yyy;
			}
			break;

		default:
			msgi[count++] = data;
			if (count >= need) {
				/* We have received a full data packet */
				oursum = MAGIC - (chksum(&msgi[6], sendbytes)
						  ^ (0377 & msgi[4]));
				if (pktsum != oursum) {
					/* Send a reject on this pkt */
					reject = xxx - 1;
bad(("\ndata checksum in packet %x, ours=%x", pktsum, oursum));
				}
				/* FIXME, this may change for window>1 */
				if (xxx != (rseq+1)%8 ) {
bad(("Not next packet xxx=%d rseq=%d", xxx, rseq));
				}
				rseq = xxx;	/* We saw this pkt OK */
		done:
#ifdef DEBUG
				putchar('\n');
				if (tt == CONTROL)
					printf("Rcvd: CONTROL %s %d\n",
						ctrl_pr[xxx], yyy);
				else
					printf("Rcvd: %s %d %d\n",
						tt_pr[tt], xxx, yyy);
#endif
				last_op = OP_READ;
				return(0);
			}
			break;
		}
	}
#ifdef DEBUG
	printf(" EOF\n");
#endif
	return(1);
}

int
chksum(s,n)
register unsigned char *s;
register n;
{
	register short sum;
	register unsigned short t;
	register short x;

	sum = -1;
	x = 0;
	do {
		if (sum < 0)
		{
			sum <<= 1;
			sum++;
		}
		else
			sum <<= 1;
		t = sum;
		sum += *s++ & 0377;
		x += sum ^ n;
		if ((unsigned short)sum <= t)
			sum ^= x;
	} while (--n > 0);

	return(sum);
}

/*
 * Medium level packet driver input routine.
 *
 * Read a data packet from the other side.  If called twice in succession,
 * we send an ack of the previous packet.  Otherwise we tend to piggyback
 * the acks on data packets.
 *
 * Result is 0 if we got a data packet, 1 if we got some other kind, or
 * a hangup timeout.
 */
int
indata()
{

	while (1) {
		if (last_op == OP_READ) {
			ackmsg();		/* Send an ack */
		}
		if (inpkt()) return 1;

		switch (tt) {
		case ALTCHN:
			return 1;		/* Unsupported - yet */

		case LONGDATA:
		case SHORTDATA:
			/*
			 * We got a data packet.  That's what we want,
			 * so return.
			 */
			return 0;		/* We are done. */

		case CONTROL:	
			switch (xxx) {
			default:
				return 1;	/* Bad packet type */

			case RJ:		/* Reject prev pkt */
			case RR:		/* OK but no data */
				break;		/* Ack and try again */
			}
		}
	}
}

/*
 * Open a conversation in the g protocol.  Medium level routine.
 * Returns 0 for success, 1 for failure.
 */
int
gpro_open(mastermode)
	int mastermode;
{
	int tries = 0;
	int expect = 0;
	static int which[] = {INITA, INITB, INITC};

	/* initialize protocol globals, e.g. packet sequence numbers */

	rseq = 0;		/* Last good packet # we have seen from him */
	wseq = 1;		/* Next packet # we will send */
	his_rseq = 0;		/* Last good Packet # he has seen from us */
	reject = NOREJECT;	/* Don't reject first packet */
	firstslave = mastermode? 0: 1;	/* About to do first slave packet? */

	if (mastermode) goto master_start;

	while (++tries <= 10) {
		/* Receive an initialization packet and handle it */
		if (inpkt() == 0 && tt == CONTROL && xxx == which[expect]) {
			/* Remember we've seen it, grab value */
			switch (xxx) {
			case INITA:
			case INITC:
				sendwin = yyy;
				break;
			case INITB:	
				/*
				 * Get preferred packet size for other guy,
				 * but don't overrun our buffer space.
				 * The encoded segment size is off-by-1 from
				 * the one used in the K field in each packet.
				 */
				do {
					sendseg = yyy+1;
					sendbytes = segbytes[sendseg];
				} while (sendbytes > MAX_PACKET && --yyy);
				break;
			}
		} else {
			expect = -1;
		}

master_start:
		/*
		 * Transmit an initialization packet.
		 *
		 * Send whichever packet we expected, if we got it.
		 * If we didn't, send INITA.
		 */
		switch (expect) {
		case -1:
		case 0:
			ctlmsg((INITA << 3) | wndsiz);
			break;
		case 1:
			ctlmsg((INITB << 3) | (segsiz - 1));
			break;
		case 2:
			ctlmsg((INITC << 3) | wndsiz);
			break;
		}
		
		if (++expect > 2)
			return 0;		/* We are done */
	}
	return 1;				/* Failure */
}

/*
 * Close a conversation in the G protocol.  Medium level routine.
 */
int
gpro_close()
{

	/* In windowed protocol, we have to check if prev one's been ack'd */
	if (last_op == OP_WRITE) {
		if (inpkt()) return 1;
	}

	do {
		ctlmsg(CLOSE << 3);
		if (inpkt())
			return 1;
	} while (tt != CONTROL && xxx != CLOSE);

	return 0;
}

/*
 * MAIN ROUTINE.
 *
 * This is called at program startup.  It parses the arguments to the
 * program (if any) and sets up to receive a call on the modem.
 *
 * If there are no arguments, we assume the caller is already on standard
 * input, waiting to do uucp protocols (past the login prompt), and we
 * just handle one caller.
 *
 * If there is an argument, it is the name of the tty device where we
 * should listen for multiple callers and handle login and password.
 */
main(argc,argv)
int argc;
char *argv[];
{
	int ontheline = 1;

	/* If argument provided, use it as name of comm port */
	if (argc > 1) {
		ontheline = 0;
		strcpy(ttynam, argv[1]);
	}

#ifdef UNIX
	if (ontheline) {
		if (chdir(SPOOLDIR)) {
			perror("Can't chdir to Spool directory");
			exit(2);
		}
	}
#endif UNIX

#ifdef LOG
	if (0 > (logfd = open(LOGFILE, O_CREAT|O_WRONLY|O_APPEND, 0644))) {
		perror("Can't open LOGFILE");
		exit(2);
	}
	/* Log our presence so we humans reading the logs can find the
	   entries created by uuslave. */
	logit("UUSLAVE", ontheline? version: ttynam);
#endif

#ifdef DEBUG
	if (ontheline) {
		freopen("uuslave.log", "a", stdout);
	}
#endif

#ifdef SYSV
	setbuf(stdout, (char *)NULL);	/* Unbuffered debug output */
#endif

#ifdef BSD
	setbuf(stdout, (char *)NULL);	/* Unbuffered debug output */
#endif

#ifdef MSDOS
	setbuf(stdout, (char *)NULL);	/* Unbuffered debug output */
#endif

#ifdef DEBUG
	{
		long clock;

		time(&clock);
		printf("\014\nuuslave log starting %s", ctime(&clock));
	}
#endif

#ifdef COMPORT
	set_tty();      /* read old settings and set up port
			   so it is a full 8 bit channel at
			   1200 baud. */

	printf("\n Initializing modem.");
	xwrite(fdtty,hayesinit,sizeof(hayesinit)-1);
#endif

	do {
		/*
		 *  Set up serial channel, wait for incoming call. 
		 */

#ifdef DEBUG
		printf("\nrestarting\n");
#endif

#ifdef CPM
		/* FIXME, we should implement ontheline here */
		sioinit();
#endif

#ifdef MSDOS
		/* FIXME, we should implement ontheline here */
#ifndef COMPORT
		if ((fdtty = open(ttynam, O_RDWR)) < 0)
		{
			printf("Cannot open %s for read/write %d\n",
				ttynam, errno);
			exit(1);
		}
#endif
#ifdef COMPORT
		printf("\n Waiting for call.  Strike any key to abort.\n");
		while ((get_msr() & CD) == 0){
			if (kbhit() != 0){
				printf("\nAborting UUSLAVE at user's request.");
				exit(1);
			}
		}
		init_comm();
		inp_flush();
		sleep(3);
#endif
		signal(SIGINT,sigint);
#endif

#ifdef ST
		/* FIXME, we should implement ontheline here */
		/* set 1200 baud, no flow ctrl. */
		Rsconf(7, 0, -1, -1, -1, -1);
		/* FIXME, these numbers (at least the 7!) should be replaced
		 * with symbolic constants, preferably from a system header
		 * file */

		/* setup user abort */
		if (setjmp(abortenv))
			exit(0);
#endif

#ifdef BSD
		/* Berserkeley version */
		if (ontheline) {
			fdtty = STDIN;
		} else if ((fdtty = open(ttynam, O_RDWR)) < 0) {
			perror(ttynam);
			exit(1);
		}
		ioctl(fdtty, TIOCGETP, &atermio);
		btermio = atermio;
		btermio.sg_flags |= RAW;
		btermio.sg_flags &= ~(ECHO|XTABS);
		if (!ontheline)
			btermio.sg_ispeed = btermio.sg_ospeed = B1200;
		ioctl(fdtty, TIOCSETN, &btermio);
		signal(SIGINT,sigint);
#endif

#ifdef SYSV
		/* Missed'em Five version */
		if (ontheline) {
			fdtty = STDIN;
		} else if ((fdtty = open(ttynam, O_RDWR)) < 0) {
			perror(ttynam);
			exit(1);
		}
		ioctl(fdtty,TCGETA,&atermio);
		btermio = atermio;
		btermio.c_iflag = btermio.c_oflag = btermio.c_lflag = 0;
		btermio.c_cc[VMIN] = 1;
		btermio.c_cc[VTIME] = 0;
		if (!ontheline)
			btermio.c_cflag = (btermio.c_cflag & ~CBAUD) | B1200;
		ioctl(fdtty,TCSETA,&btermio);
		signal(SIGINT,sigint);
#endif

		do_session(ontheline);

#ifdef UNIX
		(void) close (fdtty);
#endif

#ifdef MSDOS
#ifndef COMPORT
		(void) close (fdtty);
#else
		uninit_comm();
#endif
#endif

	} while (!ontheline);

	sleep(3);		/* Let output drain? makes hangup work? */
}


/* Handle a single uucp login session */
do_session(ontheline)
	int ontheline;
{

	if (!ontheline) {
		/* output login request, verify uucp */
		xwrite(fdtty,msgo0,sizeof(msgo0)-1);
		if (instr(msgi0,sizeof(msgi0)-1))
			goto bort;

		/* output password request, verify s8000 */
		xwrite(fdtty,msgo1,sizeof(msgo1)-1);
		if (instr(msgi1,sizeof(msgi1)-1))
			goto bort;
	}

	/* output here message, wait for response */
	xwrite(fdtty,msgo2,sizeof(msgo2)-1);
	if (instr(msgi2,sizeof(msgi2)-1))
		goto bort;

	/* output ok message, output protocol request, wait for response */
	xwrite(fdtty,msgo3,sizeof(msgo3)-1);
	xwrite(fdtty,msgo3a,sizeof(msgo3a)-1);
	if (instr(msgi3,sizeof(msgi3)-1))
		goto bort;

	if (gpro_open(0)) goto bort;
	logit("OK", "startup");
	while (1)
		if (top_level()) goto bort;
	/* NOTREACHED */

bort:	
	printf("...aborting...\n");
	;
}

/*
 * Handle a transaction "at top level", as Unix uucp's debug log says.
 * This really means "receive a command from the other side and handle
 * it".  Return 1 to abort, 0 to continue.
 */
int
top_level()
{

	msgbld[0] = '\0';		/* No command yet */
	do {
		if (indata() || tt != LONGDATA)
			return 1;
		msgi[6+msgsize] = '\0';	/* Null terminate packet */
		strcat(msgbld,&msgi[6]);	/* Tack on to command */
	} while (strlen(msgi+6) == msgsize);	/* Loop if no null in pkt */

#ifdef DEBUG
	/* Print it for easy debugging */
	printf("\n\nCommand: %s\n\n", msgbld);
#endif

	switch (msgbld[0]) {
	case 'S' :
		if (send_file(msgbld)) return 1;
		break;
	case 'R' :
		if (receive_file(msgbld)) return 1;
		break;
	case 'H' :
		if (yesno('H', 1))
			return 1;
		if (indata() || tt != LONGDATA)
			return 1;
		if (!strcmp(&msgi[6],"HY"))
		{
			/* Shut down the packet protocol */
			gpro_close();

			/* Write the closing sequence */
			xwrite(fdtty,msgo4,sizeof(msgo4)-1);
			(void) instr(msgi4,sizeof(msgi4)-1);
			xwrite(fdtty,msgo4,sizeof(msgo4)-1);
			logit("OK", "conversation complete");
			return 1;	/* Go byebye */
		}
		break;
	default:
		/* Unrecognized packet from the other end */
		printf("\nBad control packet type %c refused.\n", msgbld[0]);
		if (yesno(msgbld[0], 0))
			return 1;
		break;
	}

	return 0;
}

/* Send a "yes or no" packet with character 'c'. */
int
yesno(c, true)
	char c;
	int true;
{
	char buf[20];

	buf[0] = c;
	buf[1] = 'Y';
	buf[2] = 0;
	/* FIXME, errno might not be the right return code here */
	if (!true) 
		sprintf(buf,"%cN%d", c, errno);

	return sendpacket(buf, strlen(buf), LONGDATA);
}

/*
 * Create a temporary file name for receiving a file into.
 * "name" is the name we will actually eventually want to use for the file.
 * We currently ignore it, but some OS's that can't move files around
 * easily might want to e.g. put the temp file into the same directory
 * that this file is going into.
 */
char *
temp_filename(name)
	register char *name;
{
	static char tname[NAMESIZE];

	if (ourpid == 0)
		ourpid = getpid();
	sprintf(tname, "TM.u%d", ourpid);
#ifdef DEBUG
	printf("Using temp file %s\n", tname);
#endif
	return tname;
}


/*
 * Transform a filename from a uucp packet (in Unix format) into a local
 * filename that will work in the local file system.
 */
char *
munge_filename(name)
	register char *name;
{
	register char *p;

#ifdef DEBUG
	printf("Munge_filename  input: %s\n", name);
#endif

#ifdef CPM
	for (p = name + strlen(name); p != name && *(p-1) != '/'; p--) ;
#endif

#ifdef UNIX
	{static char buffer[NAMESIZE+SLOP];

	/* FIXME: Security checking goes here! */

	if (name[0] == '~') {
		/* Handle user-relative names -- ~ or ~uucp turns to PUBDIR */
		if (name[1] == '/')
			p = &name[1];
		else if (!strncmp("~uucp/", name))
			p = &name[5];
		else {
			p = NULL;		/* Neither of the above */
			goto out;
		}
		strcpy(buffer, PUBDIR);
		strcat(buffer, p);
		p = buffer;
		goto out;
	}
#ifdef SUBDIR
	/* Berkeley Unix subdirectory hack.
	 * Full pathnames go through OK.
	 * D.myname*	-> D.myname/D.myname*
	 * D.*		-> D./D.*
	 * C.*		-> C./C.*
	 * otherwise left alone (e.g. X.*).
	 * FIXME: we punt D.myname since we don't know our own name yet.
	 * FIXME: I hear Honey Danber has a slightly different scheme.
	 */
	if (name[0] != '/') {
		if (name[1] == '.' &&
		    (name[0] == 'C' || name[0] == 'D')) {
			strncpy(buffer, name, 2);
			buffer[2] = '/';
			strcpy(buffer+3, name);
			p = buffer;
			goto out;
		}
	}
#endif

	p = name;		/* Let it through as-is. */
	}
#endif

#ifdef MSDOS
	p = name;		/* FIXME */
#endif

out:
#ifdef DEBUG
	printf("Munge_filename output: %s\n", p);
#endif
	return p;
}

/*
 * Master wishes to send a file to us -- we receive it.
 * Return 1 to abort the call, 0 to continue.
 */
int
send_file(msg)
	char *msg;
{
	char *p, *q;	/* File names */
	int offset;
	int fddsk;	/* Disk file descriptor */
	int status;
	int error = 0;	/* No errors so far */

	sscanf(msg,"%s %s %s %s %s %s %o",
		cmnd, srcnam, dstnam, who, flags, temp, &mode);
	logit("REQUESTED", msgbld);
	q = munge_filename(dstnam);	/* Translate to local customs */
	p = temp_filename(q);		/* Create a handy temp file */
	if (p && (fddsk = open(p, O_CREAT|O_EXCL|O_WRONLY, mode|0600)) >= 0) {
		/* FIXME: Are the above permissions right?? */
		/* FIXME: Should we create directories for the file? */
		if (yesno('S',1))		/* Say yes */
			return 1;
		do {
			/* Read a packet, handle the data in it */
			if (indata())
				return 1;

			switch (tt) {
			case LONGDATA:
				/* FIXME, check this write */
				offset = 6;
				goto writeit;
			case SHORTDATA:
				if (msgi[6] & 0x80) {
					msgsize -=
					  (msgi[7] << 7) | (127&msgi[6]);
					offset = 8;
				} else {
					msgsize -= msgi[6];
					offset = 7;
				}

			writeit:
				if (msgsize != 0) {
					status = 
					  write(fddsk, &msgi[offset], msgsize);
					if (status != msgsize) error++;
				}
				break;
			}
		} while (msgsize != 0);
		status = close(fddsk);
		if (status != 0) error++;

		/* Move the file from its temp location to its real loc */
		/* FIXME:  This needs to be able to copy the file, if 
		   a simple rename does not suffice. */
		status = rename(p, q);
		if (status != 0) {
#ifdef DEBUG	
		printf("Cannot rename file %s to %s, errno=%d\n",
			p, q, errno);
#endif DEBUG
			error++;
		}

		logit("COPY", error? "FAILED": "SUCCEEDED");
		if (yesno('C', error == 0))	/* Send yes or no */
			return 1;
	}
	else
	{
		/* Can't open file -- send error response */
#ifdef DEBUG
		printf("Cannot open file %s (%s) for writing, errno=%d\n",
			p, dstnam, errno);
#endif
		logit("REQUEST", "FAILED -- HMM");
		if (yesno('S', 0))
			return 1;
	}

	return 0;
}

/*
 * Master wants to Recieve a file from us -- we send it.
 * Return 1 to abort the call, 0 to continue.
 */
int
receive_file(msg)
	char *msg;
{
	char *p;
	int count;
	int fddsk;		/* Disk file descriptor */

	sscanf(msg,"%s %s %s",cmnd,srcnam,dstnam);
	logit("REQUESTED", msg);
	p = munge_filename(srcnam);
	if (p && (fddsk = open(p, O_RDONLY)) >= 0) {
		if (yesno('R',1))
			return 1;
		do {
			count = read(fddsk, dskbuf, sendbytes);
			if (sendpacket(dskbuf, count,
				(count == sendbytes)? LONGDATA: SHORTDATA))
					return 1;
		} while (count);
		close(fddsk);
		/* Await the "CY" or "CNddd" packet, and toss it. */
		while (1) {
			if (indata() || tt != LONGDATA)
				return 1;
			if (msgi[6] != 'C') {
#ifdef DEBUG
				printf("\nDidn't get 'CY' or 'CN', got %s\n",
					&msgi[6]);
#endif
			} else {
				logit("REQUESTED", &msgi[6]);
				break;
			}
		}
	} else {
		/* Can't open file for reading; return error packet */
#ifdef DEBUG
		printf("Cannot open file %s (%s) for reading, errno=%d\n",
			p, srcnam, errno);
#endif
		logit("DENIED", "CAN'T OPEN");
		if (yesno('R', 0))
			return 1;
	}

	return 0;
}

#ifdef LOG
/*
 * Log file writing subroutine.
 *
 * Makes incredibly ugly log entries that look *just like* Unix uucp's
 * incredibly ugly log entries.
 *
 * Once we don't care about compatability, we should do this much better.
 */
logit(one, two)
	char *one, *two;
{
	char logbuf[(NAMESIZE*4)+SLOP+50];	/* Temp buffer for logs */
	long clock;
	struct tm *tm;
	int len;

	(void) time(&clock);
	tm = localtime(&clock);
	if (ourpid == 0)
		ourpid = getpid();

	sprintf(logbuf, "%s uuslave (%d/%d-%d:%02d-%d) %s (%s)\n",
		who, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min,
		ourpid, one, two);

#ifdef DEBUG
	printf("%s", logbuf);
#endif

	len = strlen(logbuf);
	if (len != write(logfd, logbuf, len)) {
#ifdef DEBUG
		printf("Can't log to logfd, terminating");
		perror(LOGFILE);
#endif
		exit(39);		/* Terminate if we can't log */
	}

}
#endif

#ifndef UNIX
/* CP/M and MSDOS and ST need these routines.  Probably should use
 * the new names, but for now...
 */
bzero(s, cnt)
register char	*s;
register int	cnt;
{
	register int	i;
	for (i = 0; i < cnt; i++) {
		*s++ = '\0';
	}
}

bcopy(from, to, cnt)
register char	*from;
register char	*to;
register int	cnt;
{
	register int	i;
	for (i = 0; i < cnt; i++) {
		*to++ = *from++;
	}
}
#endif

#ifdef COMPORT
/*
 * MSDOS routines for handling the comm port.
 *
 * get_time(n)
 * TIME_PTR n;
 *
 * fills timetype structure n with current time using DOS interrupt 21
 *
 */

get_time(n)
TIME_PTR n;
{
  union REGS inregs;
  union REGS outregs;

  inregs.h.ah = 0x2c;		/* Please make a #define for this, Tim */

  int86(0x21, &inregs, &outregs);/* Please #define the 0x21 too */

  n->hour = outregs.h.ch;
  n->minute  = outregs.h.cl;
  n->sec  = outregs.h.dh;
  n->hsec = outregs.h.dl;

  return(0);
}

sleep(x)
int x;
{
  int i;
  unsigned s;
  TIME n;               /* current time record */

  i = 0;
  get_time(&n);
  s = n.sec;

  while (i < x){
    while (s == n.sec)
      get_time(&n);
    s = n.sec;
    ++i;
  }
}
#endif
