/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */
/* NO error checking protocol. Good for UUCP over virtual circuits */
/* such as TCPIP or X.25. Also a good starting place to write your */
/* own protocols */
#include "dcp.h"
#define PKTSIZE 120

topenpk()
{
msgtime=MSGTIME;
pktsize=PKTSIZE;
return(0);
}

tclosepk()
{
return(0);
}

tgetpkt(data,len)
int *len;
char *data;
{
int nt1,nt2;
trpack(len,data);
swrite("2",1);
return(0);
}

tsendpkt(data,len,msg)
int len,msg;
char *data;
{
int i;
char c;
if(msg) { len=PKTSIZE; for(i=strlen(data);i<len;i++) data[i]='\0'; }
tspack(DATA,len,data);
sread(&c,1,30);
return(0);
}

tspack(nt1,len,data)
int nt1,len;
unsigned char *data;
{
unsigned char c;
if(debug>2) {printmsg("spack: type=%d len=%d",nt1,len);
if(nt1==DATA) {data[len]='\0'; printmsg("|%s|",data); }}
c=nt1; swrite(&c,1); c=len; swrite(&c,1);
swrite(data,len);
}

trpack(len,data)
int *len;
unsigned char *data;
{
unsigned char c,ct;
int timeout=30;
sread(&c,1,timeout); ct=c; sread(&c,1,timeout); *len=c;
sread(data,*len,timeout); data[*len]='\0';
if(debug>2) {printmsg("rpack: type=%d len=%d",ct,*len);
if(*len > 0) printmsg("data=|%s|",data);}
return(ct);
}
