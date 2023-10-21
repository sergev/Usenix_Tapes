/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */
/* Dialer,Proto negotiator,identify routines */
#include "dcp.h"
#define PROTOS  "trkg"
#define MAXLOGTRY       3

Proto Protolst[] = {  'g',ggetpkt,gsendpkt,gopenpk,gclosepk,
                      'k',kgetpkt,ksendpkt,kopenpk,kclosepk,
                      'r',rgetpkt,rsendpkt,ropenpk,rclosepk,
                      't',tgetpkt,tsendpkt,topenpk,tclosepk,'0'};

procref         getpkt,sendpkt,openpk,closepk;

/*<FF>*/
/*
**
**
**startup
**
**
*/
startup()
{
char msg[80],tmp1[20],tmp2[20];
if(! remote) 
{
msgtime = 2*MSGTIME;
if(rmsg(msg,2)== -1) return('Y');
if(debug > 1) printmsg("1st msg = %s",msg);
if(strncmp(&msg[6],rmtname,7)) return('Y');
sprintf(msg,"S%.7s -Q0 -x%d",myname,debug);  /* -Q0 -x16 remote debug set */
wmsg(msg,2);
if(rmsg(msg,2)== -1) return('Y');
if(debug > 1) printmsg("2nd msg = %s",msg);
if(strncmp(&msg[1],"OK",2)) return('Y');
if(rmsg(msg,2)== -1) return('Y');
if(debug > 1) printmsg("3rd msg = %s",msg);
if(msg[0] != 'P' || mindex(&msg[1],proto[0]) == -1)
{
wmsg("UN",2);
return('Y');
}
sprintf(msg,"U%c",proto[0]);
wmsg(msg,2);
setproto(proto[0]);
return('D');
}
else 
{
msgtime = 2*MSGTIME;
sprintf(msg,"Shere=%s",myname);
wmsg(msg,2);
if(rmsg(msg,2)== -1) return('Y');
sscanf(&msg[1],"%s %s %s",rmtname,tmp1,tmp2);
sscanf(tmp2,"-x%d",&debug);
if(debug > 0) printmsg("debug level = %d",debug);
if(debug > 1) printmsg("1st msg from remote = %s",msg);
if(checkname(rmtname)) return('Y');
wmsg("ROK",2);
sprintf(msg,"P%s",PROTOS);
wmsg(msg,2);
if(rmsg(msg,2)== -1) return('Y');
if(msg[0] != 'U' || mindex(PROTOS,msg[1]) == -1) return('Y');
proto[0] = msg[1];
setproto(proto[0]);
return('R');
}
}


/******* set the protocol **********/
setproto(pr)
char pr;
{
int i;
Proto *tproto;
for(tproto=Protolst;tproto->type != '\0' && pr != tproto->type; tproto++)
{if(debug>2) printmsg("setproto: %c %c",pr,tproto->type); }
if(tproto->type =='\0') {
                  printmsg("setproto:You said I had it but I cant find it");
                        exit(1); }
getpkt = tproto->a; sendpkt = tproto->b;
openpk =tproto->c;  closepk =tproto->d;
}



/*<FF>*/
/*
**
**callup
** script processor - nothing fancy! 
*/
callup()
{
int flg,kk,jj,ll,firstflg;
char buf2[132],*prsend;
if(openline(device,speed)) return('G');
jj=0;
kk=0;
while(TRUE)
{
        if(loginseq[jj] == 0x5c && loginseq[jj+1] == 'n')
        {
                buf2[kk] = '\n';
                kk++;
                jj += 2;
                continue;
        }
        if(loginseq[jj] == 0x5c && loginseq[jj+1] == 'r')
        {
                buf2[kk] = '\r';
                kk++;
                jj += 2;
                continue;
        }
        buf2[kk] = loginseq[jj];
        if(buf2[kk] == '\0') break;
        kk++;
        jj++;
}
flg = -1;       /* start by sending */
firstflg = -1;
jj=0;
prsend = &buf2[jj];
msgtime = 2*MSGTIME;
while(buf2[jj])
{
        kk=jj;
        while(buf2[kk] != '-' && buf2[kk] != '\0')
                kk++;
        if(buf2[kk]=='\0') buf2[kk+1]='\0';
        buf2[kk] = '\0';
        if(flg)
        {
                prsend = &buf2[jj];
                if(firstflg) {
                slowrite(&buf2[jj]);
                } else {
                wmsg(&buf2[jj],0);
                }
                flg = 0;
                firstflg = -1; /* change this to 0 for normal use */
        }
        else
        {
                ll = 1;
                while(getstring(&buf2[jj]))
                {
                      if(ll++ >= MAXLOGTRY ) return('Y');
                      wmsg(prsend,0);
                      /* try resending before giving up */
                }
                flg = -1;
        }
        jj = kk + 1;
}
return('P');
}


/*<FF>*/
/*
**
**      slowrite
** comunication slow write. needed for auto-baud modems
*/
slowrite(st)
register char *st;
{
int len,j;
char c;
len=strlen(st);
if(debug>2) printmsg("sent %s",st);
for(j=0;j<len;j++) {
        swrite(&st[j],1);
        ddelay(80000);
        }
}
