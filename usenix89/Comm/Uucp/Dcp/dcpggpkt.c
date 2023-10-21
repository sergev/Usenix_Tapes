/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */
/* second 1-window "g" protocol */
#include "dcp.h"
#define PKTSIZE	64
#define PKTSIZ2	2
#define HDRSIZE 6
#define MAXTRY	5	/* ought to be greater than RWINDOW */
#define SWINDOW	3
#define MAXERR	5

static int      pkrec,pksent,pknerr;
/*<FF>*/
/******************SUB SUB SUB PACKET HANDLER************/
gclosepk()
{
char tmp[PKTSIZE];
gspack(1,0,0,0,tmp);
gspack(1,0,0,0,tmp);
return(0);
}

gopenpk()
{
int i,j,n1,n2,len;
char tmp[PKTSIZE];
pkrec=0; pksent=1;
pktsize=PKTSIZE;
msgtime=MSGTIME;
pknerr=0;
j=7;
gsta:
if(j == 4 ) return(0); /* if ok */
for(i=0;i<MAXTRY;i++)
{gspack(j,0,0,0,tmp); if(grpack(&n1,&n2,&len,tmp)==j) {j--; goto gsta;} }
return(-1); /* fail */
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
ggetpkt(data,len)
int *len;
char *data;
{
int npkrec,npksent,i,nlen,nakflg;
char tmp[PKTSIZE];
msgtime=2*MSGTIME;
nakflg=0;
for(i=0;i<MAXTRY;i++)
{
switch(grpack(&npkrec,&npksent,&nlen,data))
{
case DATA: if(npksent != ((pkrec+1)%8)) {
                if(debug > 0) printmsg(" unexpected pkt %d ",pkrec);
                break;
        }  
        pkrec = (pkrec+1)%8;
        *len = nlen;
        if(nakflg) 	
        	gspack(NAK,pkrec,0,0,tmp);
        else
      		gspack(ACK,pkrec,0,0,tmp);
        return(0);
case ERROR: nakflg=1;
        if(debug > 0) printmsg(" checksum error pkt %d ",pkrec);
        break;  
case CLOSE:	return(-1);
default: break; /* anything else:BADHDR,LOST,PARTIAL,UNKNOWN*/
}
}
pknerr++;
if(pknerr >= MAXERR) return(-1);
return(0);
}
/*
 *
 *  sendpkt
 *
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
gsendpkt(data,len,msg)
int len,msg;
char data[];
{
int nlen,npkrec,npksent,i;
char tmp[PKTSIZE];
msgtime=MSGTIME;
if(msg) {len=PKTSIZE; for(i=strlen(data);i<len;i++) data[i] = '\0'; }
for(i=0;i<MAXTRY;i++)
{
gspack(DATA,pkrec,pksent,len,data); if(msg == 2) return(0);
switch(grpack(&npkrec,&npksent,&nlen,tmp))
{
case ACK: /* Ignore wrong ACK's */
		if(npkrec != pksent) {
	        if(debug > 0) printmsg(" wrong RR %d (%d) ",pksent,npkrec);
			break;
        } 
        pksent = (1+pksent)%8;
        return(0);
case NAK: if(debug > 0) printmsg(" REJ %d ",npkrec);
        if(npkrec != pksent) {
        	if(debug >0) printmsg(" wrong NAK RJ %d (%d)",pksent,npkrec);
			break;
		}
		pksent=(1+pksent)%8; 
		return(0);
case CLOSE:	return(-1);
default: break;
}
}
pknerr++;
if(pknerr >= MAXERR) return(-1);
return(0);
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
char *cnt1;
{
unsigned int check,i;
unsigned char c2,pkt[HDRSIZE],dpkerr[10];
if(len > 64) len = 64;
if(len == 0) cnt1[0] = '\0';
/**Link testing mods***/ /*
printf("**n:normal,e:error,l:lost,p:partial,h:bad header,s:new seq--> ");
gets(dpkerr); 
if(dpkerr[0] == 's') { sscanf(&dpkerr[1],"%d",&nt4); } /**/
/** End Link testing mods ***/
if(debug > 0) {
printmsg("send packet type %d",nt2);
printmsg("  num = %d n = %d",nt3,nt4);
printmsg("  len = %d data =\n|%s|",len,cnt1);
}
c2 = '\0';
pkt[0] = '\020';
pkt[4] = nt2<<3;
nt2 &= 7;
switch(nt2)
{
case 1:      break;                  /* stop protocol */
case 2:      pkt[4] += nt3; break;   /* reject        */
case 3:      break;
case 4:      pkt[4] += nt3; break;   /* ack          */
case 5:      pkt[4] += SWINDOW;   break;   /* 3 windows */
case 6:      pkt[4] += 1;   break;   /* pktsiz = 64 (1) */
case 7:      pkt[4] += SWINDOW;   break;   /* 3 windows */
case 0:      pkt[4] += 0x80 + nt3 + (nt4<<3);
                c2 = (PKTSIZE - len)&0xff;
                if(c2)
                {
                        pkt[4] += 0x40;   /* if len < PKTSIZE */
                        for(i=PKTSIZE-1;i>0;i--) cnt1[i]=cnt1[i-1];
                        cnt1[0]=c2;
                }
                break;
}
pkt[4] &= 0xff;
if(nt2 != DATA) {
        pkt[1] = 9;             /* control packet size = 0 (9) */
		check=pkt[4];
} else {
        pkt[1] = PKTSIZ2;             /* data packet size = 64 (2) */
        check = checksum(cnt1,PKTSIZE);
		i = pkt[4];/* got to do this on PC for ex-or high bits */
		i &= 0xff;
		check = (check ^ i) & 0xffff;
}
check = (0xaaaa - check) & 0xffff;
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
if(len==0 && nt2==0) write(flog,pkt,HDRSIZE);   
if(pkt[1] != 9) { 
	swrite(cnt1,PKTSIZE);     /* data is always 64 bytes long */ 
if(len==0 && nt2==0)	write(flog,cnt1,PKTSIZE); 
}
}

/*<FF>*/
/*
 *
 *      read packet
 * on return: nt3=pkrec nt4=pksent len=length <=PKTSIZE  cnt1=data *
ret(type) ok; ret(-1) input buf empty; ret(-2) bad header;
 *              ret(-3) lost pkt timeout; ret(-4) checksum error;ret(-5) ?
 */
grpack(nt3,nt4,len,cnt1)
int *nt3,*nt4,*len;
char *cnt1;
{
unsigned int nt1,check,checkchk,i;
unsigned char c,c2,pkt[HDRSIZE];
int ii;
c = '\0';
while((c & 0x7f) != '\020') if(sread(&c,1,msgtime) < 1) return(-1);
if(sread(&pkt[1],HDRSIZE-1,msgtime) < (HDRSIZE-1)) return(-1);
/*write(flog,&c,1); write(flog,&pkt[1],HDRSIZE-1); */
/* header is 6-bytes long */
i = pkt[1]^pkt[2]^pkt[3]^pkt[4]^pkt[5];
i &= 0xff;
if(i) {                        /*  bad header */
        if(debug) printmsg("****bad header****");
        return(-2);
}
if((pkt[1] &=0x7f) == 9) {       /* control packet */
        *len = 0;
        c = pkt[4] & 0xff;
        nt1  = c>>3;
        *nt3 = c & 7;
        *nt4 = 0;
        check = 0;
        checkchk = 0;
        cnt1[*len] = '\0';
} else {       /* data packet */
        if(pkt[1] != PKTSIZ2) return(-5);   /* cant handle other than 64*/
        nt1  = 0;
        c2 = pkt[4] & 0xff;
        c = c2&0x3f;
        *nt4 = c>>3;
        *nt3 = c & 7;
        if(sread(cnt1,PKTSIZE,msgtime) < PKTSIZE) return(-3);
/*        write(flog,cnt1,PKTSIZE);     */
        /* 64 byte packets even if partial */
		i = pkt[3];
		i = (i<<8)&0xff00;
		check = pkt[2];
		check = i|(check&0xff);
        checkchk = checksum(cnt1,PKTSIZE);
		i = pkt[4]|0x80;
		i &= 0xff;
        checkchk = 0xaaaa - (checkchk^i);
        checkchk &= 0xffff;
        if(checkchk != check) {
                if(debug) printmsg("***checksum error***");
                return(ERROR);
        }
        *len = PKTSIZE;
        if(c2&0x40) {
                ii = (cnt1[0]&0xff);
                *len = (*len-ii)&0xff;
                for(ii=0;ii<*len;ii++) cnt1[ii] = cnt1[ii+1];
        }
        cnt1[*len] = '\0';
}
if(debug > 0) {
        printmsg("receive packet type %d ",nt1);
        printmsg("  num = %d n = %d",*nt3,*nt4);
        printmsg("  checksum rec = %x comp = %x",check,checkchk);
        printmsg("  len = %d data =\n|%s|",*len,cnt1);
}
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
