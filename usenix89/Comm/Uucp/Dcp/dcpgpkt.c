/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */
/* 3-window "g" ptotocol */
/* Thanks got to John Gilmore for sending me a copy of Greg Chesson's 
UUCP protocol description-- Obviously invaluable
Thanks also go to Andrew Tannenbaum for the section on Siding window
protocols with a program example in his "Computer Networks" book 
*/
#include "dcp.h"
#define PKTSIZE         64
#define PKTSIZ2         2
#define HDRSIZE         6
#define MAXTRY          4

#define MAXERR  200		/* Dont want to quit in a middle of a long file*/
#define TIMEOUT 4		/* could be longer */
#define KPKT    1024/PKTSIZE
#define OK      -1

#define SWINDOW 3	/* fixed now, U make it variable */
#define RWINDOW 3
#define NBUF    8   /* always SAME as MAXSEQ ? */
#define MAXSEQ  8

#define between(a,b,c) ((a<=b && b<c) || (c<a && a<=b) || (b<c && c<a))

/* packet defin */
static int rwl,swl,swu,rwu,nerr,nbuffers,npkt,irec,timeout,GOT_SYNC,GOT_HDR;
static int fseq[NBUF],outlen[NBUF],inlen[NBUF],arr[NBUF];
static char outbuf[NBUF][PKTSIZE+1],inbuf[NBUF][PKTSIZE+1];
static unsigned char grpkt[HDRSIZE+1];
static long ftimer[NBUF],acktmr,naktmr;
/*<FF>*/
/******************SUB SUB SUB PACKET HANDLER************/
gopenpk()
{
int i,j,n1,n2,len;
char tmp[PKTSIZE+1];
pktsize=PKTSIZE; /* change it later after the init */
msgtime=MSGTIME; /* not sure I need this for "g" proto */
/* initialize proto parameters */
swl=nerr=nbuffers=npkt=0;
swl=swu=1;
rwl=0;
rwu=RWINDOW-1;
for(i=0;i<NBUF;i++) {ftimer[i]=0; arr[i]=FALSE; }
GOT_SYNC=GOT_HDR=FALSE;
/* 3-way handshake */
timeout=1; /* want some timeout capability here */
gspack(7,0,0,0,tmp);
rsrt:
if(nerr >=MAXERR) return(-1);
/* INIT sequence. Easy fix for variable pktsize and windows. */
/* I didnt since all the machines I talk to use W=3 PKTSZ=64 */
/* If you do this make sure to reflect the changes in "grpack" */
/* and "gspack" */
switch(grpack(&n1,&n2,&len,tmp)) {
case 7:	gspack(6,0,0,0,tmp);
		goto rsrt;
case 6: gspack(5,0,0,0,tmp);
		goto rsrt;
case 5: break;
default: nerr++; gspack(7,0,0,0,tmp);
		goto rsrt;
}
nerr=0;
return(0);      /* channel open */
}

gclosepk()
{
int i;
char tmp[PKTSIZE+1];
timeout=1;
for(i=0;i<MAXTRY;i++) {
        gspack(CLOSE,0,0,0,tmp);
        if(gmachine() == CLOSE) break;
}
if(debug) printmsg("number of errors %d and pkts xfered %d",nerr,npkt);
return(0);
}

/*
 *
 * ggetpkt
 ***** description: Gets no more than a packet's worth of data from
****               the "packet i/o state machine". May have to
****               periodically run the pkt machine to get some
****               packets.
* on input: dont care   getpkt(data,&len)  char *data int len
* on return: data+\0 and length in len. ret(0) if alls well
* ret(-1) if problems.(fail)
 */
ggetpkt(cdata,len)
int *len;
char cdata[];
{
int i2;
irec=1;
timeout=0;
/* WAIT FOR THE DESIRED PACKET */
while((arr[rwl]) == FALSE) if(gmachine()!=OK) return(-1);
/* GOT A PKT ! */
i2 = rwl; /*<-- mod(,rwindow) for larger than 8 seq no.s */
*len = inlen[i2];
strncpy(cdata,inbuf[i2],*len);
arr[i2]=FALSE;
rwu = (1+rwu)%MAXSEQ; /* bump rec window */
npkt++;
return(0);
}

/*
 *
 *  sendpkt
 *
*****   description:    Put at most a packet's worth of data  in the pkt state
 ****                   machine for xmission.
*****                   May have to run the pkt machine a few times to get
*****                   an available output slot.
 *
 * on input: char *data int len,flg; len=length of data in data.
 *           flg=2 just send the packet with no wait for ack.
 *           flg>0 zero out the unused part of the buffer. (for UUCP "msg"
 *                                                                   pkts)
 *           flg=0 normal data
 * return:   ret(0) if alls well ret(-1) if problems (fail)
 *
*/
gsendpkt(cdata,len,flg)
int len,flg;
char *cdata;
{
int i,i1;
long ttmp;
irec=0;
timeout=0; /* non-blocking reads */
/* WAIT FOR INPUT i.e. if weve sent SWINDOW pkts and none have been */
/* acked, wait for acks */
while(nbuffers >= SWINDOW) if(gmachine()!= OK) return(-1);
i1 = swu;  /* <--If we ever have more than 8 seq no.s, must mod() here*/
/* PLACE PACKET IN TABLE AND MARK UNACKED */
/* fill with zeros or not */
if(flg) {
        strcpy(outbuf[i1],cdata);
        len=PKTSIZE;
        for(i=strlen(cdata);i<len;i++) outbuf[i1][i]='\0';
} else {
        strncpy(outbuf[i1],cdata,len);
        outbuf[i1][len]='\0';
}
/* mark packet  */
outlen[i1] = len;
ftimer[i1] = time(&ttmp);
fseq[i1]=swu;
swu = (1+swu)%MAXSEQ;	/* bump send window */
nbuffers++;
npkt++;
/* send it */
gspack(DATA,rwl,fseq[i1],outlen[i1],outbuf[i1]); 
/* send it once then let the pkt machine take it. wouldnt need this for */
/* mtasking systems */
gmachine();
return(0);
}

/************     packet machine   ****** RH Lamb 3/87 */
/* Idealy we would like to fork this process off in an infinite loop */
/* and send and rec pkts thru "inbuf" and "outbuf". Cant do this in MS-DOS*/
/* so we setup "getpkt" and "sendpkt" to call this routine often and return */
/* only when the input buffer is empty thus "blocking" the pkt-machine task. */
gmachine()
{
int rack,rseq,rlen,i1,i2,dflg;
char rdata[PKTSIZE+1];
long ttmp,itmp;
reply:
if(debug>2) printmsg("*send %d<W<%d, rec %d<W<%d, err %d",swl,swu,rwl,rwu,nerr);
/* waiting for ACKs for swl to swu-1. Next pkt to send=swu */
/* rwl=expected pkt */
if(debug>3) printmsg("Kbytes transfered %d errors %d\r",npkt/KPKT,nerr);
if(nerr >= MAXERR) goto close;
dflg=0;
switch(grpack(&rack,&rseq,&rlen,rdata))
{
case CLOSE:     if(debug>2) printmsg("**got CLOSE");
                goto close;
case NAK:       nerr++; 
				acktmr=naktmr=0; /* stop ack/nak timer */
                if(debug>2) printmsg("**got NAK %d",rack);
nloop:          if(between(swl,rack,swu)) { /* resend rack->(swu-1) */
                        i1 = rack;
                        gspack(DATA,rwl,rack,outlen[i1],outbuf[i1]);
            			if(debug>2) printmsg("***resent %d",rack);
                        ftimer[i1]=time(&ttmp);
                        rack=(1+rack)%MAXSEQ;
                        goto nloop;
                }
				if(dflg) return(OK);
                goto reply; /* any other stuff ? */
case EMPTY:     if(debug>2) printmsg("**got EMPTY");
                itmp = time(&ttmp);
				if(acktmr) if((itmp-acktmr) >= TIMEOUT) { /* ack timed out*/
					gspack(ACK,rwl,0,0,rdata);
					acktmr=itmp;
				}
				if(naktmr) if((itmp-naktmr) >= TIMEOUT) { /*nak timed out*/
					gspack(NAK,rwl,0,0,rdata);
					naktmr=itmp;
				}
				/* resend any timed out un-acked pkts */
                for(i2=swl;between(swl,i2,swu);i2=(1+i2)%MAXSEQ)  {
					acktmr=naktmr=0; /* reset ack/nak */
					i1=i2;
					if(debug > 2)
						printmsg("--->seq,elapst %d %ld",i2,(itmp-ftimer[i1]));
                    if((itmp-ftimer[i1]) >= TIMEOUT) {
						if(debug>2) printmsg("***timeout %d",i2);
						/* since "g" is "go-back-N", when we time out we */
						/* must send the last N pkts in order. The generalized*/
						/* sliding window scheme relaxes this reqirment */
                        nerr++;
						dflg=1;  /* same hack */
                        rack=i2;
						goto nloop;
                     }
                }
                return(OK);
case ACK:       if(debug>2) printmsg("**got ACK %d",rack);
				acktmr=naktmr=0; /* disable ack/nak's */
aloop:          if(between(swl,rack,swu)) {   /* S<-- -->(S+W-1)%8 */
						if(debug>2) printmsg("***ACK %d",swl);
						ftimer[swl]=0;
                        nbuffers--;
                        swl = (1+swl)%MAXSEQ;
						goto aloop;
                } 
				if(dflg) return(OK); /* hack for non-mtask sys's */
									 /* to empty "inbuf[]" */
                goto reply;
case DATA:      if(debug>2) printmsg("**got DATA %d %d",rack,rseq);
                i1=(1+rwl)%MAXSEQ; /* (R+1)%8 <-- -->(R+W)%8 */
                i2=(1+rwu)%MAXSEQ;
                if(between(i1,rseq,i2)) {
                        if(i1 == rseq) {
                                i1 = rseq;
                                arr[i1] = TRUE;
                                inlen[i1] = rlen;
                                strncpy(inbuf[i1],rdata,rlen);
                                rwl=(rwl+1)%MAXSEQ;
								if(debug>2) printmsg("***ACK d %d",rwl);
                                gspack(ACK,rwl,0,0,rdata);
								acktmr=time(&ttmp); /* enable ack/nak tmout*/
								dflg=1; /*return to call when finished*/
								/* in a mtask system, unneccesary */
                        }
                        else {nerr++; if(debug>2) printmsg("***unexpect %d ne %d",rseq,rwl);}
                } else {nerr++; if(debug>2) printmsg("***wrong seq %d",rseq);}
                goto aloop;
case ERROR:     nerr++;
                if(debug>2) printmsg("**got BAD CHK");
                gspack(NAK,rwl,0,0,rdata);
				naktmr=time(&ttmp); /* set nak timer */
				if(debug>2) printmsg("***NAK d %d",rwl);
                goto reply;
default:        if(debug>2) printmsg("**got SCREW UP");
                goto reply; /* ignore it */
}
close:          gspack(CLOSE,0,0,0,rdata);
                return(CLOSE);
}

/*<FF>*/
/*************** FRAMMING *****************************/
/*
 *
 *
 *      send a packet
 * nt2=type nt3=pkrec nt4=pksent len=length<=PKTSIZE cnt1= data * ret(0) always
 */
gspack(nt2,nt3,nt4,len,cnt1)
int nt2,nt3,nt4,len;
char cnt1[];
{
unsigned int check,i;
unsigned char c2,pkt[HDRSIZE+1],dpkerr[10];
if(len > 64) len = 64;
if(len == 0) cnt1[0] = '\0';
/**Link testing mods- create artificial errors ***/ /*
printf("**n:normal,e:error,l:lost,p:partial,h:bad header,s:new seq--> ");
gets(dpkerr); 
if(dpkerr[0] == 's') { sscanf(&dpkerr[1],"%d",&nt4); } /**/
/** End Link testing mods ***/
if(debug > 2) 
	printmsg("send packet type %d, num=%d, n=%d, len=%d",nt2,nt3,nt4,len);
if(debug > 3)
	printmsg("data =\n|%s|",cnt1);
c2 = '\0';
pkt[0] = '\020';
pkt[4] = nt2<<3;
nt2 &= 7;
switch(nt2) {
case 1:      break;                  /* stop protocol */
case 2:      pkt[4] += nt3; break;   /* reject        */
case 3:      break;
case 4:      pkt[4] += nt3; break;   /* ack          */
case 5:      pkt[4] += SWINDOW;   break;   /* 3 windows */
case 6:      pkt[4] += 1;   break;   /* pktsiz = 64 (1) */
case 7:      pkt[4] += SWINDOW;   break;   /* 3 windows */
case 0:      pkt[4] += 0x80 + nt3 + (nt4<<3);
                c2 = (PKTSIZE - len)&0xff; 
                /* havnt set it up for VERY LONG pkts with a few */
                /* bytes yet (-128) */
                if(c2) { /* short packet handling */
                        pkt[4] += 0x40;   /* if len < PKTSIZE */
                        for(i=PKTSIZE-1;i>0;i--) cnt1[i]=cnt1[i-1];
                        cnt1[0]=c2;
                }
                break;
}
pkt[4] &= 0xff;
if(nt2) {
        pkt[1] = 9;             /* control packet size = 0 (9) */
        check = (0xaaaa - pkt[4])&0xffff;
} else {
        pkt[1] = PKTSIZ2;             /* data packet size = 64 (2) */
        check = checksum(cnt1,PKTSIZE);
		i = pkt[4];/* got to do this on PC for ex-or high bits */
		i &= 0xff;
		check = (check ^ i) & 0xffff;
        check = (0xaaaa - check) & 0xffff;
}
pkt[2] = check & 0xff;
pkt[3] = (check>>8) & 0xff;
pkt[5] = (pkt[1]^pkt[2]^pkt[3]^pkt[4])&0xff;
/***More Link testing MODS ******/ /*
switch(dpkerr[0]) {
case 'e':   cnt1[10] = -cnt1[10];
			break;
case 'h':	pkt[5] = -pkt[5];
			break;
case 'l':	return;
case 'p':	swrite(pkt,HDRSIZE); 
			if(pkt[1] != 9) swrite(cnt1,PKTSIZE-3);
			return;
default:	break;
}  /**/
/******End Link Testing Mods **********/
swrite(pkt,HDRSIZE);       /* header is 6-bytes long */
/*      write(flog,pkt,HDRSIZE);   */
if(pkt[1] != 9) { swrite(cnt1,PKTSIZE);     /* data is always 64
bytes long */       /*    write(flog,cnt1,PKTSIZE);   */
}
}
/*<FF>*/
/*
 *
 *      read packet
 * on return: nt3=pkrec nt4=pksent len=length <=PKTSIZE  cnt1=data *
 * ret(type) ok; ret(EMPTY) input buf empty; ret(ERROR) bad header;
 *          ret(EMPTY) lost pkt timeout; ret(ERROR) checksum error;ret(-5) ?
****NOTE :
***sread(buf,n,timeout)
	while(TRUE) {
		if(# of chars available >= n) (without dec internal counter)
			read n chars into buf (decrenent internal char counter)
			break
		else
			if(time>timeout) break
	}
	return(# of chars available)
****END NOTE
 */
grpack(nt3,nt4,len,cnt1)
int *nt3,*nt4,*len;
char cnt1[];
{
unsigned int nt1,check,checkchk,i;
unsigned char c,c2;
int ii;
if(GOT_SYNC) goto get_hdr; 
if(GOT_HDR) goto get_data;
c = '\0';
while((c & 0x7f) != '\020') 
	if(sread(&c,1,timeout) == 0) 
		return(EMPTY);
GOT_SYNC=TRUE;
get_hdr:
if(sread(&grpkt[1],HDRSIZE-1,timeout)< (HDRSIZE-1)) return(EMPTY);
GOT_SYNC=FALSE;
i = grpkt[1]^grpkt[2]^grpkt[3]^grpkt[4]^grpkt[5];
i &= 0xff;
if(i) {               /*  bad header */
	if(debug)  printmsg("****bad header****");
    return(ERROR); /* Im not sure whether "g" considers it an empty or error */
}
GOT_HDR=TRUE;
if((grpkt[1] &=0x7f) == 9) {       /* control packet */
        *len = 0;
        c = grpkt[4] & 0xff;
        nt1  = c>>3;
        *nt3 = c & 7;
        *nt4 = 0;
        check = 0;
        checkchk = 0;
        cnt1[*len] = '\0';
		GOT_HDR=FALSE;
} else {       /* data packet */
        if(grpkt[1] != PKTSIZ2) return(-5);   /* cant handle other than 64*/
get_data:
		if(sread(cnt1,PKTSIZE,timeout) < PKTSIZE) return(EMPTY);
		GOT_HDR=FALSE;
        nt1  = 0;
        c2 = grpkt[4] & 0xff;
        c = c2&0x3f;
        *nt4 = c>>3;
        *nt3 = c & 7;
		i = grpkt[3];
		i = (i<<8)&0xff00;
		check = grpkt[2];
		check = i|(check&0xff);
        checkchk = checksum(cnt1,PKTSIZE);
		i = grpkt[4]|0x80;
		i &= 0xff;
        checkchk = 0xaaaa - (checkchk^i);
        checkchk &= 0xffff;
        if(checkchk != check) {
                if(debug>0) printmsg("***checksum error***");
                return(ERROR);
        }
        *len = PKTSIZE;
		/* havnt set it up for very long pkts yet (>128) RH Lamb */
        if(c2&0x40) {
                ii = (cnt1[0]&0xff);
                *len = (*len-ii)&0xff;
                for(ii=0;ii<*len;ii++) cnt1[ii] = cnt1[ii+1];
        }
        cnt1[*len] = '\0';
}
if(debug > 2) 
	printmsg("rec  packet type %d, num=%d, n=%d, len=%d",nt1,*nt3,*nt4,*len);
if(debug > 3)
    printmsg("  checksum rec = %x comp = %x, data=\n|%s|",check,checkchk,cnt1);
        ii = nt1;
        return(ii);
}

unsigned checksum(data,len)
int len;
char data[];
{
unsigned int i,j,tmp,chk1,chk2;
chk1 = 0xffff;
chk2 = 0;
j = len;
for(i=0;i<len;i++)
{
        if(chk1&0x8000) { chk1 <<= 1; chk1++; }
        else { chk1 <<= 1; }
        tmp = chk1;
        chk1 += (data[i]&0xff);
        chk2 += chk1^j;
        if((chk1&0xffff) <= (tmp&0xffff)) chk1 ^= chk2;
        j--;
}
return(chk1&0xffff);
}
