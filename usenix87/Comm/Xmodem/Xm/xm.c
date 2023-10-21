/*
 *  XM - a UNIX-to-CP/M file transfer shell
 *	by Richard Conn
 *
 *  XM is based on UC version 1.2, which in turn was based on UMODEM 3.5,
 *  originally written by Lauren Weinstein, and mutated by Richard Conn,
 *  Bennett Marks, Michael Rubenstein, Ben Goldfarb, David Hinnant, and
 *  Lauren Weinstein. XM differs from UC in that it offers only a basic
 *  UNIX-to-CP/M file transfer facility using the Christensen XMODEM protocol
 *  in checksum mode.
 */
#define versmaj 1	/* Major Version */
#define versmin 0	/* Minor Version */
/*  Basics  */
#define FALSE	0
#define TRUE	~FALSE
/*  ASCII Characters  */
#define SOH	001
#define STX	002
#define ETX	003
#define EOT	004
#define ENQ	005
#define ACK	006
#define LF	012
#define CR	015
#define NAK	025
#define SYN	026
#define CAN	030
#define CTRLZ	032
#define ESC	033
/*  XM Constants  */
#define TIMEOUT      -1 	     /* Timeout Flag */
#define ERRMAX	10		/* Max errors tolerated */
#define BLOCKSZ 128		/* Size of transmission block */
#define CREATE	0644		/* Mode for New Files */
#define DELAY	5		/* Basic delay for transmission */
/*  Library Utilities  */
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sgtty.h>
#include	<signal.h>
#include	<ctype.h>
#include	<setjmp.h>
/*  Setjmp Environment */
jmp_buf rbenv;

main (argc, argv)
int argc;
char *argv[];
{
	int sendflg, recvflg, statflg;	/* major functions */
	char filetype;			/* tranfer type */
	FILE *fopen();			/* forward ref */
	int index;	/* index for arg parsing loop */
	char opt;	/* current option character */
	char *getenv(); /* getenv function defn */
	/* Print Banner */
	printf("XM Version %d.%d - Christensen XMODEM Protocol\n"
		,versmaj,versmin);
	printf("File Transfer Tool\n");
	/* Check for Help Request */
	if (argc == 1) {
		help();
		exit(0);
	}
	/* Init for Option Parsing */
	sendflg = FALSE;
	recvflg = FALSE;
	statflg = FALSE;
	/* Process Options */
	index = 0;
	while ((opt = argv[1][index++]) != '\0')
		switch (opt) {
		case '-' :              /* skip dash */
			break;
		case 'F' :
		case 'f' :
			statflg = TRUE; /* set file stat mode */
			break;
		case 'R' :
			recvflg = TRUE; /* set file recv mode */
			filetype = 'b';
			break;
		case 'r' :
			recvflg = TRUE; /* set file recv mode */
			filetype = 't';
			break;
		case 'S' :
			sendflg = TRUE; /* set file send mode */
			filetype = 'b';
			break;
		case 's' :
			sendflg = TRUE; /* set file send mode */
			filetype = 't';
			break;
		default :
			printf("Invalid Option %c\n", opt);
			break;
		}
	/* Select and Execute Major Mode */
	if (statflg) {		/* File Status Display */
		if (argc < 3) {
			printf("File Name NOT Given\n");
			exit(0);
		}
		fxstat(filetype,argv[2]);
		exit(0);
	}
	if (sendflg) {		/* Send File */
		if (argc < 3) {
			printf("File Name NOT Given\n");
			exit(0);
		}
		sendfile(filetype,argv[2]);
		exit(0);
	}
	if (recvflg) {		/* Receive File */
		if (argc < 3) {
			printf("File Name NOT Given\n");
			exit(0);
		}
		recv(filetype,argv[2]);
		exit(0);
	}
	printf("Major Mode NOT Selected\n");
	help();
	exit(0);
}
/* Print Help */
help()
{
	printf("Usage:  XM c filename\n");
	printf("\n");
	printf("where 'c' MUST be One of the Following Commands --\n");
	printf("\tR -- Receive Binary File\n");
	printf("\tr -- Receive Text File\n");
	printf("\tS -- Send Binary File\n");
	printf("\ts -- Send Text File\n");
	printf("\tf or F -- Show File Status\n");
	printf("\n");
	printf("Examples:\n");
	printf("\tXM S myfile <-- Send Binary File \"myfile\"\n");
	printf("\tXM s mytext <-- Send Text File \"mytext\"\n");
}
/* Send File */
sendfile(filetype,filename)
char filetype;
char *filename;
{
	FILE *fd, *fopen();
	int blocknum;			/* Current Block Number */
	int nlflg;			/* New Line for File Convert */
	int sending;			/* Xmit In-Progress Flag */
	int tries;			/* Attempt Count */
	int bufctr;			/* Counter for Buffer Build */
	int c;				/* Temp Char */
	int rcode;			/* Return Code */
	char buf[BLOCKSZ];		/* Buffer for Transfer */
	/* Print Banner */
	printf("XM Sending %s File: %s\n",
		(filetype == 't') ? "Text" : "Binary",
		filename);
	/* Open File for Input and Print Opening Messages */
	if ((fd = fopen(filename, "r")) == 0) {
		printf("Can`t Open File %s for Send\n", filename);
		return;
	}
	fxstat(filetype,filename);   /* Print File Status Info */
	printf("Ready to Send File\n");
	binary(TRUE,TRUE);	/* Open Binary Communications */
	/* Init Parameters */
	blocknum = 1;
	nlflg = FALSE;
	sending = TRUE;
	/* Synchronize */
	tries = 0;
	while (recvbyte(30) != NAK) {
		if (++tries > ERRMAX) {
			printf("Remote System Not Responding\n");
			return;
		}
	}
	/* Main Transmission Loop */
	while(sending) {
		/* Build Next Block into buf */
		for (bufctr = 0; bufctr < BLOCKSZ;) {
			if (nlflg) {	/* New Line */
				buf[bufctr++] = LF;	/* Store LF */
				nlflg = FALSE;
			}
			if (bufctr == BLOCKSZ) break;	/* Leave for Loop */
			c = getc(fd);	/* Get Next Byte from File */
			if (c == EOF) {
				sending = FALSE;	/* Done */
				if (!bufctr)	/* Avoid Extra Block */
					break;
				for(;bufctr < BLOCKSZ; bufctr++)
					buf[bufctr]= (filetype == 't')
						? CTRLZ : '\0' ;
				continue;	/* Exit for Loop */
			}
			if (c == LF && filetype == 't') {  /* NL? */
/*				buf[bufctr++] = CR;	/* Insert CR */
/*
   I commented out the above line, because the EOL on the Amiga is
   LF not CR/LF.  wpl
*/
				nlflg = TRUE;		/* New Line */
			}
			else buf[bufctr++] = c; 	/* Store Char */
		}
		/* Send Block */
		tries = 0;	/* Set Try Count */
		if (bufctr) do {
			putblock(filetype,buf,blocknum);     /* Send Block */
			rcode = recvbyte(10);	/* Get Response */
		}
		while (rcode != ACK && ++tries < ERRMAX);
		blocknum = (blocknum + 1) & 0xFF;
		if (tries == ERRMAX) sending = FALSE;  /* Error Abort */
	}
	/* Cleanup After Send */
	fclose(fd);	/* Close File */
	tries = 0;
	sendbyte(EOT);
	while (recvbyte(15) != ACK && ++tries < ERRMAX)
		sendbyte(EOT);
	binary(FALSE,TRUE);	/* Leave Binary Mode */
	sleep(3);
	printf("\n");
}
/* Send Buffer to Receiver */
putblock(filetype,buf,blocknum)
char filetype;
char *buf;
int blocknum;
{
	int i, j, checksum;
	sendbyte(SOH);		/* Send Start of Header */
	sendbyte(blocknum&0xff);     /* Send Block Number */
	sendbyte((-blocknum-1)&0xff);	     /* Send Block Complement */
	checksum = 0;
	for (i = 0; i < BLOCKSZ; i++) {
		sendbyte(*buf&0xff); /* Send Byte */
		checksum = (checksum + *buf++) & 0xff;
	}
	sendbyte(checksum&0xff);	     /* Checksum */
}
/* Receive File */
recv(filetype,filename)
char filetype;
char *filename;
{
	int fd; 		/* file descriptor */
	int blocknum;		/* next block to receive */
	int rbcnt;		/* total number of received blocks */
	int errorcnt;		/* number of errors on current block */
	int receiving;		/* continuation flag */
	int char1;		/* first char received in block */
	int rcode;		/* received block code */
	if (!access(filename,2)) {
		printf("File %s Exists -- Delete it? ", filename);
		if (!getyn()) {
			printf("Aborting\n");
			return;
		}
	}
	unlink(filename);	/* delete old file, if any */
	if ((fd = creat(filename, CREATE)) == -1) {	/* can't create */
		printf("Can't Create %s\n", filename);
		return;
	}
	/* We Have a GO */
	printf("XM Receiving %s File: %s\n",
		(filetype == 't') ? "Text" : "Binary",
		filename);
	printf("Ready to Receive\n");
	/* Init Counters et al */
	blocknum = 1;
	rbcnt = 0;
	errorcnt = 0;
	receiving = TRUE;
	/* Establish Binary Communications */
	binary(TRUE,TRUE);
	/* Synchronize with Sender */
	sendbyte(NAK);
	/* Receive Next Packet */
	while (receiving) {
		do {
			char1 = recvbyte(6);
		}
		while ((char1 != SOH) && (char1 != EOT) && (char1 != TIMEOUT));
		switch (char1) {
		case TIMEOUT :	     /* Timeout */
			if (++errorcnt == ERRMAX) {
				close(fd);	/* Close File */
				sleep(3);	/* Delay for Sender */
				binary(FALSE,TRUE);	/* Normal I/O */
				receiving = FALSE;
			}
			sendbyte(NAK);
			break;
		case EOT :	/* End of Transmission */
			sendbyte(ACK);
			while (recvbyte(3) != TIMEOUT);
			close(fd);	/* Close File */
			sleep(3);	/* Delay for Sender */
			binary(FALSE,TRUE);	/* Normal I/O */
			printf("\n");
			receiving = FALSE;
			break;
		case SOH :	/* New or Old Block */
			rcode = getblock(filetype,fd,blocknum);/* read block */
			switch (rcode) {
			case 0 :	/* OK */
				blocknum = ++blocknum & 0xff;
				rbcnt++;
			case 2 :	/* OK, but Duplicate Block */
				errorcnt = 0;
				sendbyte(ACK);
				break;
			case 1 :	/* Xmit Error, Non-Fatal */
				if (++errorcnt < ERRMAX) {
					sendbyte(NAK);
					break;
				}
			default :	/* Xmit Error, Fatal */
				close(fd);
				sendbyte(CAN);
				binary(FALSE,TRUE);
				while (recvbyte(3) != TIMEOUT);
				receiving = FALSE;
				break;
			}
			break;
		}
	}
}
/* Get Block from Sender */
getblock(filetype,fd,blocknum)
char filetype;
int fd, blocknum;
{
	int curblock, cmpblock;
	int recdone, checksum, inchecksum, byte, bufcnt, c;
	int startstx, endetx, endenq;
	int errflg, errchr;
	char buff[BLOCKSZ];
	int j;
	curblock = recvbyte(DELAY);
	if (curblock == TIMEOUT) return(1);
	cmpblock = recvbyte(DELAY);
	if (cmpblock == TIMEOUT) return(1);
	if ((curblock + cmpblock) != 0xff) {
		while (recvbyte(DELAY) != TIMEOUT);  /* Flush */
		return(1);
	}
	checksum = 0;		/* Init Checksum */
	byte = 0;		/* Init Buff Ptr */
	recdone = FALSE;	/* File Receive NOT Done */
	for (bufcnt=0; bufcnt<BLOCKSZ; bufcnt++) {
		c = recvbyte(DELAY);
		if (c == TIMEOUT) return(1);
		buff[byte] = c;
		checksum = (checksum + c) & 0xff;
		if (filetype != 't') {
			byte++; 	/* binary xfer, so advance */
			continue;
		}
		if (c == CR) continue;	/* skip CR */
		if (c == CTRLZ) {	/* done */
			recdone = TRUE;
			continue;
		}
		if (!recdone) byte++;		/* continue */
	}
	inchecksum = recvbyte(DELAY);
	if (inchecksum == TIMEOUT) return(1);
	errflg = FALSE;
	if (checksum != inchecksum) errflg = TRUE;
	if (errflg) return(1);
	if (curblock != blocknum) {
		if (curblock == ((blocknum+1)&0xff)) {
			return(99);
		}
		return(2);
	}
	if (write(fd,buff,byte) < 0) {
		return(99);
	}
	return(0);
}
/* File Status Display */
fxstat(filetype,filename)
char filetype;
char *filename;
{
	struct stat fsi;	/* file status info */
	if (stat (filename, &fsi) == -1) {	/* get file status info */
		printf("File %s Not Found\n", filename);
		return;
	}
	printf("File Size of %s is %ldK, %ld Blocks\n",
	filename,
	fsi.st_size%1024 ? (fsi.st_size/1024)+1 : fsi.st_size/1024,
	fsi.st_size%128 ? (fsi.st_size/128)+1 : fsi.st_size/128);
}
/*  SUPPORT ROUTINES  */
/* get yes or no response from user */
getyn()
	{
	int c;
	c = charx();	/* get char */
	if (c == 'y' || c == 'Y') {
		printf("Yes\n");
		return(TRUE);
	}
	else {
		printf("No\n");
		return(FALSE);
	}
}
/* get single char input */
charx()
{
	int binary();
	int c;
	binary(TRUE,FALSE);
	c = getchar();
	binary(FALSE,FALSE);
	return (c);
}
/* send byte to receiver */
sendbyte(data)
char data;
{
	write (1, &data, 1);	/* write the byte */
}
/* receive a byte from sender */
recvbyte(seconds)
unsigned seconds;
{
	char c;
	int alarmfunc();		/* forward declaration */
	signal(SIGALRM,alarmfunc);	/* catch alarms */
	if (setjmp(rbenv) < 0)		/* setup jump for alarm */
		return (TIMEOUT);
	alarm(seconds); 		/* set clock */
	read (0, &c, 1);	/* get char or timeout */
	alarm(0);			/* clear clock */
	return (c&0xff);
}
/* dummy alarm function */
alarmfunc()
{
	longjmp(rbenv,TIMEOUT); 	     /* jump to recvbyte */
	return;
}
/* set and clear binary mode */
binary(setflg,scope)
int setflg, scope;
{
	static struct sgttyb ttys, ttysold;
	if (setflg) {	/* set binary */
		if (gtty (0, &ttys) < 0) return(FALSE); /* failed */
		ttysold.sg_ispeed = ttys.sg_ispeed;	/* save old values */
		ttysold.sg_ospeed = ttys.sg_ospeed;
		ttysold.sg_erase = ttys.sg_erase;
		ttysold.sg_kill = ttys.sg_kill;
		ttysold.sg_flags = ttys.sg_flags;
		ttys.sg_flags |= RAW;		/* set for RAW Mode */
		ttys.sg_flags &= ~ECHO; 	/* set no ECHO */
		if (scope) {		/* cover all values? */
			ttys.sg_flags &= ~XTABS;	/* set no tab exp */
			ttys.sg_flags &= ~LCASE;	/* set no case xlate */
			ttys.sg_flags |= ANYP;		/* set any parity */
			ttys.sg_flags &= ~NL3;		/* no delays on nl */
			ttys.sg_flags &= ~TAB0; 	/* no tab delays */
			ttys.sg_flags &= ~TAB1;
			ttys.sg_flags &= ~CR3;		/* no CR delay */
			ttys.sg_flags &= ~FF1;		/* no FF delay */
			ttys.sg_flags &= ~BS1;		/* no BS delay */
		}
		if (stty (0, &ttys) < 0) return(FALSE); /* failed */
#ifdef	MESG
		if (scope) system("mesg n >/dev/null"); /* turn off messages */
#endif
		return(TRUE);
	}
	else {		/* clear binary */
		if (stty (0, &ttysold) < 0) return (FALSE);
#ifdef	MESG
		if (scope) system("mesg y >/dev/null"); /* turn on messages */
#endif
		return(TRUE);	/* OK */
	}
}
