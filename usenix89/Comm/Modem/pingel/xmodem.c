
/* 
This is a simple pc communications utility.  Protocols supported are
xmodem and xon/xoff.  Other protocols may be added in future.
All commands are available via meta key, ie. <alt>.
Be warned that this code contains hardware dependant routines, see notes.

Terminal emulation is the normal mode.  <alt><command-letter> invokes
file transfers etc.  See main() for commands.
*/

/* Notes:
	a) Timing loops are for 8 Mhz 80186.  Decrease by factor of ~3 for
	   IBM PC.  See defines at start of code.  If you have system
	   wait calls throw out the loops.
	b) Hardware dependant code exists at end of this file.  Convert
	   com* routines to use interrupt 14 bios calls where available.
	c) You will need to change MS-DOS bdos calls to conform to your
	   libraries.
	d) This communications utility was developed for use with Hayes
	   compatible modems.  The modem must be set for V0 and E0 prior
	   to use.  These settings provide for numerical responses and no
	   echo, respectively.
	e) Terminal emulation depends on ansi.sys for handling of tabs
	   and escape sequences.
	f) Putcnb() is simply non-buffered putc.  This was used to eliminate
	   need for newline in display.  Replace with putc if possible.
*/


/* 
This program is placed in the public domain.  Distribution is allowed
provided charges levied do not exceed distribution costs.  Modified
versions may be distributed if this notice is included with a list
of modifications.
Copyright 1986 Lee D. R. Pingel
*/


#include <stdio.h>
#include <dos.h>
#include <string.h>
#include <ctype.h>

/*	installation dependant parameters		*/
#define CONTM	10	/* seconds to dial and connect */
#define MCMDTM	3	/* seconds for modem to respond to command */
#define COMINPS	40000L	/* loops per second, approximately */
#define LPS	180000L	/* simple wait loop interations per second */



#define	ERROR	-1
#define FALSE	0
#define	TRUE	~FALSE
#define DATAMASK 0x7f
#define BLKSIZ 128
#define BPBUF 32
#define BUFFSIZ (BLKSIZ * BPBUF)
#define RETRYMAX 5
#define MAXLOGIN 5

/* special character defines */
#define TIMEOUT -1
#define QUIT 0x1b
#define SOH 1
#define EOT 4
#define ACK 6
#define NAK 0x15
#define XON 0x11
#define XOFF 0x13
#define CAN 0x18

#define ALTP 25
#define ALTR 19  
#define ALTS 31
#define ALTQ 16
#define ALTX 45
#define ALTB 48
#define ALTC 46
#define ALTD 32

#define SYSFILE "systems.txt"

/* global parameters */
int	connected = FALSE;		/* state of phone line */
char	buffer[BUFFSIZ];		/* general I/O buffer */
int	baud = 1200;			/* normal serial settings */
int	bits = 7;
int	sbits = 1;
char	parity = 'E';
char	proto = 'x';
/* I/O trace flag */
int	trace = FALSE;


/* xmodem receive and send */

recvxmodem(file)
char *file;
{
	int blkcur, blk, blkcomp;
	int checksum, moddata, firstchar, attempts;
	int fd;
	unsigned int j, bufptr;
	
	fd = creat(file);
	if(fd == ERROR){
		return(FALSE);
	}
	blkcur = 1;
	bufptr = 0;
 	attempts = 0;
	serialset(baud, 8, 2, 'N');
	comout(NAK);
	do{
		/* initial synchronization */
		do {
			firstchar = cominw(5);
		}while(firstchar != SOH
			&& firstchar != EOT
			&& firstchar != CAN
			&& firstchar != TIMEOUT);
		if (firstchar == TIMEOUT){
			comout(NAK);
			printmsg("Timeout error.");
			attempts++;
			continue;
		}
		if (firstchar == CAN){
			comout(ACK);
			printmsg("Transfer canceled by host.");
			break;
		}
		if (firstchar == EOT){
			printmsg("Transfer complete.");
			comout(ACK);
			break;
		}
		blk = cominw(1);
		blkcomp = cominw(1);
		if(blk == (~blkcomp & 0xff) && blk == ((blkcur - 1) & 0xff)){
			comout(ACK);
			printmsg("Duplicate block %x.", blkcur);
			while(cominw(1) != TIMEOUT);
			attempts++;
			continue;
		}
		if(blk != (~blkcomp & 0xff) /*  || blk != (blkcur & 0xff) */ ){
			/* commented out as some xmodem go 255,1,2 */
			comout(NAK);
			printmsg("Block number error, wanted %x and got %x.",
				blkcur, blk);
			while(cominw(1)!= TIMEOUT);
			attempts++;
			continue;
		}
		checksum = 0;
		for(j = bufptr; j <(bufptr + BLKSIZ); j++){
			moddata = cominw(1);
			if(moddata == TIMEOUT) break;
			buffer[j] = moddata;
			checksum = ((checksum + moddata) & 0xff);
		}
		moddata = cominw(1);
		if(checksum != (moddata & 0xff)){
			comout(NAK);
			printmsg("Checksum error, got %x expected %x.",
				checksum, moddata & 0xff);
			attempts++;
			continue;
		}
		printmsg("Block %d.",blkcur);
		bufptr += BLKSIZ;
		blkcur++;
		if(bufptr >= BUFSIZ){
			bufptr = 0;
			if(write(fd, buffer, BUFSIZ)== ERROR){
				comout(CAN);
				printmsg("Error writing file.");
				break;
			}
		}
		comout(ACK);
	}while (attempts < RETRYMAX);
	if(attempts >= RETRYMAX)
		comout(CAN);
	serialset(baud, bits, sbits, parity);
	comout(ACK);
	if(write(fd, buffer, bufptr)== ERROR){
		printmsg("Error writing file.");
	}
	close(fd);
}



sendxmodem(file)
char *file;
{
	int	fd, blks, attempts, blkcur, checksum;
	unsigned j, bufptr;
	char	sync;

	fd = open(file, 2);
	if(fd == ERROR){
		printmsg("Cannot open %s.", file);
		return;
	}
	blkcur = 1;
	serialset(baud, 8, 2, 'N');
	for(attempts = 0; attempts < 10;){
		sync = cominw(1);
		if(sync == NAK)
			break;
		else if(sync == TIMEOUT)
			attempts++;
	}
	attempts = 0;	
	while((blks = read(fd,buffer,BUFFSIZ)) > 0 &&
		attempts < RETRYMAX){
		if(blks == ERROR){
			printmsg("Error reading file.");
			close(fd);
			return;
		}
		blks = (blks % BLKSIZ) == 0 ? blks/BLKSIZ :(blks/BLKSIZ) + 1;
		bufptr = 0;
		do{
			attempts = 0;
			do{
				comout(SOH);
				comout(blkcur & 0xff);
				comout(~blkcur & 0xff);
				checksum = 0;
				for(j=bufptr;j<(bufptr+BLKSIZ);j++){
					comout(buffer[j]);
					checksum = (checksum + buffer[j]) & 0xff;
				}
				comout(checksum);
			}while(cominw(1)!= ACK && attempts++ < RETRYMAX);
			printmsg("Block %d.",blkcur);
			bufptr += BLKSIZ;
			blkcur++;
			blks--;
		}while(blks && attempts < RETRYMAX);
	}
	if(attempts >= RETRYMAX){
		printmsg("No acknowledgment of block, aborting.");
		comout(CAN);
	}
	else{
		attempts = 0;
		do{
			comout(EOT);
		}while(cominw(1)!= ACK && attempts++ < RETRYMAX);
		if(attempts >= RETRYMAX){
			printmsg("No acknowledgement of EOF.");
		}
	}
	serialset(baud, bits, sbits, parity);
	close(fd);
	return;
}

recvascii(file)
char	*file;
{
	char	kbdata, moddata;
	int	fd;
	char	*bp,*limit, tmp[16];

	fd = creat(file);
	if(fd == ERROR){
		printmsg("Cannot create %s.",file);
		return;
	}
	bp = buffer;
	limit = bp + BUFFSIZ;
	kbdata = '\0';
	purgeline();
	while( comcts()&&(kbdata != QUIT)){
		if(kbdrdy()){
			kbdata = kbdin();
			if (kbdata != QUIT)
				comout(kbdata);
		}
		if(cominrdy()){
			moddata = comin();
			putcnb(moddata);
			*bp++ = moddata;
			if(bp >= limit){
				comout(XOFF);
				bp = tmp;
				while((*bp = cominw(1)) != TIMEOUT){
					putcnb(*bp);
					bp++;
				}
				*bp = '\0';
				write(fd, buffer, BUFFSIZ);
				strcpy(buffer, tmp);
				bp = buffer + (bp - tmp);
				comout(XON);
			}
		}
	}
	*bp = CTRLZ;
	write(fd,buffer,bp - buffer);
	close(fd);
}


sendascii(file)
char	*file;
{
	int	fd,size;
	char	moddata;
	char	*cp,*limit;

	fd = open(file, 0);
	if(fd == ERROR){
		printmsg("Cannot open %s.", file);
		return;
	}
	
	while((size = read(fd,buffer,BUFFSIZ)) > 0){
		for(cp = buffer, limit = buffer + size; cp < limit; cp++){
			comout(*cp);
			if(comin() == XOFF){
				do{
					moddata = cominw(10);
				}while (moddata != XON && moddata != TIMEOUT);
			}
		}
	}
	close(fd);
}

/*
Connect uses a file (systems.txt) with one line per remote system:
<name> <phone-number> [<delimiter><host-prompt><delimter><response>etc.]

asys 555-1212 'login:'mylogin'password:'mypassword'

Note that prompt/response string is optional.  Normal use is to login
to the remote system.  The as many prompt/response pairs may be used as
needed.
*/

connect(namenum)
char	*namenum;
{
	char	name[32], number[32], logseq[80];
	char	buffer[80];
	char	*cp, *prompt;
	char	delimiter;
	int	moddata;
	int	attempts;
	FILE	*fd;

	disconnect();
	if(isdigit(namenum[0])){
		dial(namenum);
		return;
	}
	if((fd = fopen(SYSFILE,"r")) == NULL){
		printmsg("Unable ot open %s.",SYSFILE);
		return;
	}
	do{
		name[0] = '\0';
		logseq[0] = '\0';
		logseq[1] = '\0';	/* prompt starts here */
		number[0] = '\0';
		if(fgets(buffer,80,fd) == NULL){
			fclose(fd);
			printmsg("Unable to find %s.",namenum);
			return;
		}
		sscanf(buffer,"%s %s %s\n",name,number,logseq);
	}while(strcmp(namenum,name) != 0);
	dial(number);
	if(!connected){
		fclose(fd);
		return;
	}
	attempts = 0;
	delimiter = logseq[0];
	prompt = logseq + 1;
	cp = prompt;
	comout('\n');
	while(*cp != '\0' && attempts < MAXLOGIN){
		cp = prompt;
		moddata = 0x20;
		while(*cp != '\0' && *cp != delimiter && moddata != TIMEOUT){
			moddata = cominw(3);
			if (*cp == (moddata & 0xff))
				cp++;
			else
				cp = prompt;
		}
		if(moddata == TIMEOUT){
			comout('\n');
			attempts++;
			prompt = logseq + 1;
		}else if(*cp != '\0'){
			cp++;
			for(;*cp != delimiter && *cp != '\0';cp++){
				comout(*cp);
			}
			comout('\n');
			prompt = cp;
			if(*cp == delimiter)
				prompt++;
		}
	}
	if(*cp != '\0'){
		disconnect();
	}
	fclose(fd);
	
}

dial(num)
char	*num;
{
	long	time;
	char	*cp;
	connected = TRUE;
	for(cp="AT DT "; *cp != '\0'; cp++){
		comout(*cp);
	}
	for(cp=num; *cp != '\0'; cp++){
		comout(*cp);
	}
	comout('\015');
	for(time = COMINPS * CONTM; comin() != '1' && time > 0; time--);
	if(!time){
		disconnect();
		printmsg("Failed dial.");
	}
}



disconnect()
{
	long	time;
	char	*cp;

	if(!connected)
		return;
	for(time =  LPS; time > 0; time--);	/* wait 2 seconds */
	for(cp = "+++"; *cp != '\0'; cp++){
		comout(*cp);
	}
	for(time =  LPS; time > 0; time--);
	purgeline();
	for(cp = "ATH\015"; *cp != '\0'; cp++){
		comout(*cp);
	}
	for(time = COMINPS * CONTM; comin() != '0' && time > 0; time--);
	for(time =  LPS; time > 0; time--);	/* let the line settle */
	connected = FALSE;
}



cmd(c)
char	c;
{
	char	cbuf[16];

	switch(c){
		case ALTX:
			if(trace){
				printmsg("trace off");
				trace = FALSE;
			}else{
				printmsg("trace on");
				trace = TRUE;
			}
			break;
		case ALTB:
			printmsg("baud rate (300,1200,2400,4800,9600):%d ",baud);
			scanf("%d",&baud);
			printmsg("bits per character:%d ",bits);
			scanf("%d",&bits);
			printmsg("stop bits:%d ",sbits);
			scanf("%d",&sbits);
			printmsg("parity (Even|Odd|None):%c ",parity);
			scanf("%s",cbuf);
			switch (cbuf[0] & 0x5f){
			case 'E':
				parity = 'E';
				break;
			case 'O':
				parity = 'O';
				break;
			case 'N':
				parity = 'N';
				break;
			}
			serialset(baud,bits,sbits,parity);
			break;
		case ALTD:
			printmsg("disconnect in progress");
			disconnect(cbuf);
			break;
		case ALTC:
			printmsg("connect (name|number):");
			scanf("%s",cbuf);
			connect(cbuf);
			break;
		case ALTR:
			printmsg("File:");
			scanf("%s",cbuf);
			switch(proto){
			case 'a':
				recvascii(cbuf);
				break;
			case 'x':
				recvxmodem(cbuf);
				break;
			}
			putcnb("\007");
			break;
		case ALTS:
			printmsg("File:");
			scanf("%s",cbuf);
			switch(proto){
			case 'a':
				sendascii(cbuf);
				break;
			case 'x':
				sendxmodem(cbuf);
				break;
			}
			putcnb("\007");
			break;
		case ALTP:
			printmsg("protocol (Ascii|Xmodem):[%c]",proto);
			while (!kbdrdy());
			switch(kbdin() & 0x5f){
			case 'A':
				proto = 'a';
				break;
			case 'X':
				proto = 'x';
				break;
			default:
				break;
			}
			putcnb(proto);			
			break;
	}
}


char	cmdmsg[] = 
"commands: <Alt><Connect|Disconnect|Send|Recv|Protocol|Baudrate|Quit>";

main()
{
	char	kbdata, moddata;
	long	time;
	char	*cp;
		
	serialset(baud,bits,sbits,parity);
	for(time =  LPS; time > 0; time--);	/* wait 2 seconds */
	for(cp = "+++"; *cp != '\0'; cp++){
		comout(*cp);
	}
	for(time =  LPS; time > 0; time--);	/* wait 2 seconds */
	printmsg("%s",cmdmsg);
	purgeline();		/* remove echo */
	kbdata = 0;
	while(1){
		if(kbdrdy()){
			kbdata = kbdin();
			if(kbdata == '\0'){
				kbdata = kbdin();
				if(kbdata == ALTQ)
					break;
				else
					cmd(kbdata);
			}
			else{
				comout(kbdata);
			}
		}
		if(cominrdy()){
			moddata = comin();
			putcnb(moddata);
		}
	}
	disconnect();
}

/* MESSY! use ansi.sys where available */
printmsg(fmt,arg1,arg2,arg3)
char	*fmt;
int	arg1,arg2,arg3;
{
	char	buf[160];
	char	*cp;
	int	n;

	if (trace) printf("\n");
	putcnb('\015');
	for(n = 79; n > 0; n--){
		putcnb(' ');
	}
	putcnb('\015');
	sprintf(buf,fmt,arg1,arg2,arg3);
	for(cp = buf;*cp!= '\0'; cp++){
		putcnb(*cp);
	}
}

/* I/O trace display */
/* format of report: >15(nak) <01(soh) <e3 <f0 <61(a) <0a(nl) >06(ack) */

char ascii[] = "\
nul soh stx etx eot enq ack bel \
bs  ht  nl  vt  np  cr  so  si  \
dle dc1 dc2 dc3 dc4 nak syn etb \
can em  sub esc fs  gs  rs  us  \
sp  !   \"   #   $   %   &   '   \
(   )   *   +   ,   -   .   /   \
0   1   2   3   4   5   6   7   \
8   9   :   ;   <   =   >   ?   \
@   A   B   C   D   E   F   G   \
H   I   J   K   L   M   N   O   \
P   Q   R   S   T   U   V   W   \
X   Y   Z   [   \\   ]   ^   _   \
`   a   b   c   d   e   f   g   \
h   i   j   k   l   m   n   o   \
p   q   r   s   t   u   v   w   \
x   y   z   {   |   }   ~   del ";


iotrace(c,flag)
char	c;
{
	char	*cp, *xcp;
	char	buffer[16];
		
	sprintf(buffer,"%c%x",flag,(c & 0xff));
	cp = buffer + strlen(buffer);
	if(c < 0x80 && c > 0){
		*cp++ = '(';
		for(xcp = &ascii[c << 2]; *xcp != ' '; xcp++){
			*cp++ = *xcp;
		}
		*cp++ = ')';
	}
	*cp++ = ' ';
	*cp++ = '\0';
	for(cp = buffer; *cp != '\0'; cp++){
		putcnb(*cp);
	}
}



kbdrdy()
{
	struct	reg	sreg,dreg;
	sreg.r_ax = 0x0b00;
	intcall(&sreg,&dreg,0x21);
	return(dreg.r_ax & 0xff);
}

char	kbdin()
{
	struct	reg	sreg,dreg;
	sreg.r_ax = 0x0700;
	intcall(&sreg,&dreg, 0x21);
	return(dreg.r_ax & 0xff);
}



/*********************************************************/
/* device specific code 				*/
/********************************************************/

#define MR1	0
#define MR2	0
#define SR	2
#define CSR	2
#define CR	4
#define HR	6

#define STRTCTR	0x1c
#define	SPB	0x1c
#define	STPCTR	0x1e
#define	IP	0x1a
#define	OPCR	0x1a
#define	IPCR	0x8
#define	ACR	0x8
#define	ISR	0xa
#define	IMR	0xa
#define	CTU	0xc
#define	CTL	0xe


static	int	duart = 0x80;	/* base of duart channel */


/* the following code is written for the 1681 DUART */

serialset(baud,bits,sbits,parity)
int	baud,bits,sbits;
char	parity;
{

	int	mr1,mr2,csr;
	mr1 = 0x80;
	mr2 = 0x30;

	switch(baud){
	case 300:
		csr = 0x44;
		break;
	case 1200:
		csr = 0x66;
		break;
	case 2400:
		csr = 0x88;
		break;
	case 4800:
		csr = 0x99;
		break;
	case 9600:
		csr = 0xbb;
		break;
	}
	switch(bits){
	case 7:
		mr1 |= 2;
		break;
	case 8:
		mr1 |= 3;
		break;
	}
	switch(sbits){
	case 1:
		mr2 |= 7;
		break;
	case 2:
		mr2 |= 0xf;
		break;
	}
	switch(parity){
	case 'E':
		break;
	case 'O':
		mr1 |= 4;
		break;
	case 'N':
		mr1 |= 0x10;
		break;
	}
	outb(duart + IMR, 0x0);
	outb(duart + CR, 0x15);
	outb(duart + CSR, csr);
	outb(duart + MR1, mr1);
	outb(duart + MR2, mr2);
}

purgeline()
{
	while(cominw(1) != TIMEOUT);
}


cominw(seconds)			/* int to differentiate TIMEOUT */
unsigned seconds;
{
	int	c;
	long	idle;

	idle = seconds * COMINPS;
	while(!cominrdy()&& idle)
		--idle;
	if(!idle)
		return(TIMEOUT);
	c = inb(duart + HR);
	c &= 0xff;	/* ensure 0xff doesn't appear as 0xffff */
	if (trace) iotrace(c,'>');
	return(c);
}


comin()			/* int so that TIMEOUT is different from data */
{
	int	c;

	if(inb(duart+SR) & 1){
		c = inb(duart + HR);
		c &= 0xff;	/* ensure 0xff doesn't appear as 0xffff */
		if (trace) iotrace(c,'>');
	}
	else{
		c = '\0';
	}
	return(c);
}

comout(c)
char c;
{
	int	time;

	if (trace) iotrace(c,'<');
	time = COMINPS/10;
	while(!comoutrdy() && time--);
	outb(duart + HR,c);
}

comoutrdy()
{
	return(inb(duart+SR) & 0x4);
}


cominrdy()
{
	return(inb(duart+SR) & 1);
}


comcts()
{
	return TRUE;
}


