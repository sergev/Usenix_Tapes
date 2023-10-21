/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */
/* General sliding window program. Under developement. Has advanteges */
/* over "got-back-N". Was used initially as a vehicle to decypher  */
/* the "g: protocol. Modify dcp.h,dcp.c,dcpstart.c and leave out if desired*/
#include "dcp.h"

#define PKTSIZE 64      /*      framming data   */
#define HDRSIZ  5
#define SYNC    '\020'

#define MAXSEQ  8       /*      protocol data   */
#define NBUF    4    /*MAXSEQ/2 give explicitly! so type is integer*/
#define MAXERR  20
#define KPKT    1024/PKTSIZE
#define TIMEOUT 10     /* should be 4sec */
#define MAXTRY  10

#define between(a,b,c) ((a<=b && b<c) || (c<a && a<=b) || (b<c && c<a))

/* packet defin */
static int head,rwl,swl,swu,rwu,nerr,nbuffers,nonak,npkt;
static int ftimer[NBUF],fseq[NBUF],outlen[NBUF],inlen[NBUF],arr[NBUF];
static char outbuf[NBUF][PKTSIZE],inbuf[NBUF][PKTSIZE];

/***************************************************/
ropenpk()
{
int i,j,n1,n2,len;
char tmp[PKTSIZE];
msgtime=MSGTIME;
pktsize=PKTSIZE;
head=rwl=swl=swu=nerr=nbuffers=npkt=0;
rwu=NBUF; nonak=TRUE;
for(i=0;i<NBUF;i++) {ftimer[i]=0; arr[i]=FALSE; }
j=7;
ropkt:
if(j==4) return(0); /* ok */
for(i=0;i<MAXTRY;i++)
{rspack(j,0,0,0,tmp); if(rrpack(&n1,&n2,&len,tmp)==j) {j--; goto ropkt;}}
return(-1); /* no good */
}

rclosepk()
{
char tmp[PKTSIZE];
rspack(CLOSE,0,0,0,tmp);
rspack(CLOSE,0,0,0,tmp);
return(0);
}

/*************** rsendpkt *****************/
/*
*****   description:    Put at most a packet's worth of data  in the pkt state
*****                   machine for xmission.
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
rsendpkt(data,len,flg)
int len,flg;
char data[];
{
int i,i1;
long tmp;
if(debug>2) printmsg("\nSendpkt");
machine();
while(nbuffers >= NBUF) machine();
i1 = swu % NBUF;
if(flg) {strcpy(outbuf[i1],data); len=PKTSIZE; for(i=strlen(data);i<len;i++)
outbuf[i1][i]='\0';}
else { strncpy(outbuf[i1],data,len); }
outlen[i1] = len;
rspack(DATA,rwl,swu,outlen[i1],outbuf[i1]); ftimer[i1] = time(&tmp);
fseq[i1]=swu; npkt++;
nbuffers++;
swu = (1+swu)%MAXSEQ;
return(0);
}


/*************** rgetpkt **********/
/*
**** description: Gets no more than a packet's worth of data from
****               the "packet i/o state machine". May have to
****               periodically run the pkt machine to get some
****               packets.
* on input: dont care   getpkt(data,&len)  char *data int len
* on return: data+\0 and length in len. ret(0) if alls well
* ret(-1) if problems.(fail)
 */
rgetpkt(data,len)
int *len;
char data[];
{
int i2;
if(debug>2) printmsg("\nGetpkt");
machine();
while((arr[rwl%NBUF]) == 0) machine();
{
        if(debug>2) printmsg("ACK %d",rwl);
        rspack(ACK,rwl,0,0,0);
        i2 = rwl%NBUF;
        *len = inlen[i2];
        strncpy(data,inbuf[i2],*len);
        nonak = TRUE;
        arr[i2] = FALSE;
        rwl = (1+rwl)%MAXSEQ;
        rwu = (1+rwu)%MAXSEQ;
}
return(0);
}


/************     packet machine   *************/
machine()
{
int rack,rseq,rlen,i1,i2;
char rdata[PKTSIZE];
long tmp;
reply:
if(debug>3) printmsg("Kbytes transfered %d errors %d\r",npkt/KPKT,nerr);
if(nerr >= MAXERR) {return(ERROR);}
switch(rrpack(&rack,&rseq,&rlen,rdata))
{
case CLOSE: if(debug>2) printmsg("got CLOSE"); return(CLOSE);
case NAK: if(debug>2) printmsg("got NAK"); if(between(swl,rack,swu))
          {
             i1 = (i2 = rack)%NBUF;
             rspack(DATA,rwl,i2,outlen[i1],outbuf[i1]);ftimer[i1]=time(&tmp);
             nerr++; if(debug>2) printmsg("got NAK %d",rack);
          }
          goto reply;
case EMPTY:   if(debug>2) printmsg("got EMPTY");
            i1 = time(&tmp);
            for(i2=0;i2<NBUF;i2++)  {
                 if(ftimer[i2]) {if((i1-ftimer[i2]) >= TIMEOUT)
                 {
                        rspack(DATA,rwl,fseq[i2],outlen[i2],outbuf[i2]);
                        ftimer[i2]=time(&tmp);
                        nerr++; if(debug>2)
                                printmsg("timeout on %d",fseq[i2]);
            }    } }
            goto ssend;
case ACK: if(debug>2) printmsg("got ACK"); while(between(swl,rack,swu))
          {
                if(debug>2) printmsg("got ACK %d",rack);
                nbuffers--;
                ftimer[swl % NBUF] = 0;
                swl = (1+swl)%MAXSEQ;
          }
          goto reply;
case DATA: if(debug>2) printmsg("got DATA");
          if(rseq != rwl && nonak)
                {
                        /* can comment this out without loss in perfom*/
                        /*rspack(NAK,rwl,0,0,0);nonak=FALSE;*/
                        nerr++;   if(debug>2)
                                   printmsg("out of sequence %d",rseq);
                }
                if(between(rwl,rseq,rwu) && arr[rseq%NBUF] == FALSE)
                {
                        if(debug>2) printmsg("data pass %d",rseq);
                        i1 = rseq%NBUF;
                        arr[i1] = TRUE;
                        inlen[i1] = rlen;
                        strncpy(inbuf[i1],rdata,rlen);
                }
                goto reply;
case ERROR: nerr++; if(debug>2) printmsg("checksum error %d",rseq);
                if(nonak) {rspack(NAK,rwl,0,0,0);nonak=FALSE;}
                goto reply;
default: goto ssend;
}
ssend: {}
}
/*      framming        */
/********************
 *      read packet
 * on return: nt3=pkrec nt4=pksent len=length <=PKTSIZE  cnt1=data
 * ret(type) ok; ret(-1) input buf empty; ret(-2) bad header;
 *              ret(-3) lost pkt timeout; ret(-4) checksum error;ret(-5) ?
 rpack(nt3,nt4,len,cnt1)
 int *nt3,*nt4,*len;
 char cnt1[];
 *********************
 *      send a packet
 * nt2=type nt3=pkrec nt4=pksent len=length<=PKTSIZE cnt1= data
 * ret(0) always
 *
gspack(nt2,nt3,nt4,len,cnt1)
int nt2,nt3,nt4,len;
char cnt1[];
************************/
/******************** rspack *****************/
rspack(ttype,ack,seq,len,data)
int ttype,ack,seq,len;
char *data;
{
int i;
char msg[HDRSIZ];
msg[0] = SYNC; msg[1] = ttype;  msg[2] = ((ack<<4)&0xf0)|(seq&0x0f);
msg[3] = len;      msg[4] = msg[1]^msg[2]^msg[3];
if(ttype == DATA) for(i=0;i<PKTSIZE;i++) msg[4] ^= data[i];
swrite(msg,HDRSIZ);
if(ttype == DATA) swrite(data,PKTSIZE);
if(debug>2) {printmsg("rspack: ");
printmsg("type=%d seq=%d ack=%d len=%d chk=%d",ttype,seq,ack,len,msg[4]);
if(ttype==DATA)
        {data[len]='\0';  printmsg("|%s|",data); }
else    {printmsg("||"); }
}
}

/*********** rrpack **********************/
static unsigned char sbuf[512];
rrpack(ack,seq,len,data)
int *ack,*seq,*len;
char *data;
{
int i1,timeout=0;
/* ideally we only want to check the input buffer */
/* if its empty return(EMPTY) else get the rest of the packet */
if(sread(sbuf,1,timeout)<1) return(EMPTY);
i1=HDRSIZ-1;
if(sread(&sbuf[1],i1,1)<i1) return(ERROR);
if(sbuf[1]==DATA) if(sread(data,PKTSIZE,1)<PKTSIZE) return(ERROR);
/* deeeeeeeeeeee  all this que stuff just complicates stuff
*len = 0;
if(head>=HDRSIZ) goto rp2;
if(head>0) goto rp1;
sbuf[0] = (!SYNC);
while(sbuf[0] != SYNC) 
   {if(sread(&sbuf[0],1,timeout)<1) return(EMPTY);} head = 1;
rp1: i1=HDRSIZ;
if((head += sread(&sbuf[head],i1-head,timeout))<i1) return(EMPTY);
rp2: i1=HDRSIZ+PKTSIZE;
if(sbuf[1]==DATA)
if((head += sread(&sbuf[head],i1-head,timeout))<i1) return(EMPTY);
head=0; deeeeeeeeee  end de-complification */
*ack = (sbuf[2]>>4)&0x0f; *seq = sbuf[2]&0x0f; *len = sbuf[3];
sbuf[4] = sbuf[4]^sbuf[3]^sbuf[2]^sbuf[1];
if(sbuf[1] == DATA) for(i1=0;i1<PKTSIZE;i1++)
                        sbuf[4] ^=(data[i1]);/*=sbuf[HDRSIZ+i1]);*/
if(sbuf[4]&0xff) return(ERROR);
if(debug>2) {printmsg("rrpack: ");
printmsg("type=%d seq=%d ack=%d len=%d chk=%d",sbuf[1],*seq,*ack,*len,sbuf[4]);
data[*len]='\0'; printmsg("|%s|",data); }
return(sbuf[1]);
}
