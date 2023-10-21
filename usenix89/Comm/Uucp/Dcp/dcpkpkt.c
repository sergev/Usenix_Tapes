/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */
/* Not quite the kermit proto but it works ok */
#include "dcp.h"

#define PKTSIZE 120
#define MAXSEQ  8
#define MAXERR  20
#define PKTKB   (1024/PKTSIZE)
static int nerrs,npkts,pktrec,pktsent;
kgetpkt(rdata,len)
int *len;
unsigned char *rdata;
{
int i,timeout=20;
unsigned char c,buf[5];
stpkt: if(nerrs >= MAXERR) return(-1);
c='a';
/*printmsg("kkget* 1"); */
while((c&0x7f) != '\001') if(sread(&c,1,timeout) < 1) return(-1);
/*printmsg("kkget** 2"); */
if(sread(buf,3,timeout) < 3) return(-1);
/*printmsg("kkget*** 3");       */
if(sread(rdata,PKTSIZE,timeout) < PKTSIZE) return(-1);
/*printmsg("kkget**** 4\n%s",rdata);     */
*len=buf[0]; 
if(debug > 2) {
printmsg("kget: len %d seq %d chk %d data",*len,buf[1],buf[2]);
printmsg("|%s|",rdata);
}
for(i=0;i<PKTSIZE;i++) {buf[2] ^= rdata[i]; /*printf("%x",rdata[i]);*/}
if(buf[2]) { nerrs++; swrite("\006",1); printmsg("ERR"); goto stpkt;}
swrite("\004",1);
if(buf[1] != pktrec) {nerrs++; printmsg("SEQ"); goto stpkt;}
/*printmsg("kkget***** 5");     */
pktrec=(pktrec+1)%MAXSEQ;
return(0);
}


ksendpkt(xdata,len,flg)
int len,flg;
unsigned char *xdata;
{
int i,timeout=4;
unsigned char c,kbuf[5];
if(flg) {len=PKTSIZE; for(i=strlen(xdata);i<len;i++) xdata[i]='\0';}
kbuf[0] = '\001'; kbuf[1]=len&0xff; kbuf[2]=pktsent&0xff;
kbuf[3]=0;
for(i=0;i<PKTSIZE;i++) {kbuf[3] ^= xdata[i]; /*printf("%x",xdata[i]);*/ }
sndpkt: if(nerrs >= MAXERR) return(-1);
/*printmsg("kksend****: 1\n%s",xdata); */
swrite(kbuf,4); swrite(xdata,PKTSIZE);
/*printmsg("kksend*****: 2\n%s",xdata);  */
if(debug>2) {
printmsg("ksend: len %d seq %d chk %d data",len,pktsent,kbuf[3]);
printmsg("|%s|",xdata);
}
/*printmsg("kksend******: 3");  */
if(flg==2) return(0); /* quit if flg=2 */
if(sread(&c,1,timeout) < 1) {nerrs++; printmsg("TIM"); goto sndpkt; }
if((c&0x7f) != '\004') {nerrs++; printmsg("NAK"); goto sndpkt; }
if(debug>2) printmsg("KB's xfered: %d errors %d",npkts/PKTKB,nerrs);
/*printmsg("kksend*******: 4"); */
pktsent=(pktsent+1)%MAXSEQ; npkts++;
return(0);
}

kopenpk()
{
msgtime=MSGTIME;
pktsize=PKTSIZE;
npkts=0;
nerrs=0;
pktrec=0;
pktsent=0;
return(0);
}
kclosepk()
{
return(0);
}


