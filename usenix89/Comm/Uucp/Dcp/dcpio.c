/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */
/* DG and PC IO routines */
#include "dcp.h"
/*<FF>*/
/*************** BASIC I/O ***************************/
#ifdef  PC	/* Saltzers serial pkg */
/* Some notes: When pkts are flying in both directions, there seems to */
/* be some interupt handling problems as far as recieving. checksum errs*/
/* may therfore occur often even though we recover from them. This is */
/* especially true with sliding windows. Errors are very few in the VMS */
/* version. RH Lamb*/
#include "comm.h"
#define STOPBIT 1
#endif
#ifdef DG 	/* I/O routines should be re-written with sys calls */
#include <setjmp.h>
#include <signal.h>
jmp_buf env;
int     clkint();

clkint()
{longjmp(env,1);}

#include <termio.h>
#include <sys/ioctl.h>
setline(stty)
int stty;
{
int ret;
struct sgttyb ttybuf;
ret=ioctl(stty,TIOCGETP,&ttybuf);
ttybuf.sg_flags=RAW;  /* <-- faster RH Lamb */
ret=ioctl(stty,TIOCSETP,&ttybuf);/*
struct termio termbuf;
ret=ioctl(tty,TCGETA,&termbuf); printf("ret: %d ",ret);
termbuf.c_line = BELL_LD;
termbuf.c_iflag = 0;
termbuf.c_oflag = 0;
termbuf.c_lflag = 0;
ret=ioctl(tty,TCSETA,&termbuf); printf("%d\n",ret);
*/
}

receive_com()
{
int count,ret;
unsigned char c;
ret=ioctl(fpr,FIONREAD,&count);
printf("count: %d ret: %d\n",count,ret);
if(count < 1) return(-1);
read(fpr,&c,1); printf("char: %c\n",c);
return(c);
}
#endif DG

#ifdef VMS
#include <iodef.h>
#include <ttdef.h>
#include <tt2def.h>
#include <dcdef.h>
#include <ssdef.h>
#include <dvidef.h>
#include <descrip.h>
struct tt_mode {
       char class,type;
       short width;
       int basic : 24;
       char length;
       long extended;
};
struct iosb_struct {
       short status,size,terminator,termsize;
};
static struct tt_mode ttold,ttraw;
static struct iosb_struct ttiosb;
#endif VMS

swrite(data,num)
int num;
char *data;
{
int i;
#ifdef  PC
for(i=0;i<num;i++) {send_com(data[i]); }
#endif
#ifdef DG
write(fpw,data,num); i=num;
#endif
#ifdef VMS
sys$qiow(0,fpw,IO$_WRITEVBLK,0,0,0,data,num,0,0,0,0);
i=num;
#endif
return(i);
}

/* This should realy be the same as "sread" below but dont have time */
/* to look for subtlties. Workin on it though. DG,VAX using sys calls */
#ifdef DG
sread(buf,num,timeout)
int num,timeout;
unsigned char *buf;
{
int i,ii,jtime,jd; long tmp;
if(setjmp(env)) {if(debug) printmsg("ret i = %d\n",i); return(i);}
signal(SIGALRM,clkint); 
alarm(timeout+1);
for(i=0;i<num;i++) { read(fpr,&buf[i],1); }
alarm(0);
return(i);
}
#endif DG
/* non-blocking read essential to "g" protocol */
/* see "dcpgpkt.c" for description */
/* This all changes in a mtask systems. Requests for */
/* I/O should get qued and an event flag given. Then the */
/* requesting process (e.g.gmachine()) waits for the event */
/* flag to fire processing either a read or a write. */
/* Could be implemented on VAX/VMS or DG but not MS-DOS */
#ifdef PC
sread(buf,num,timeout)
char *buf;
int num,timeout;
{
long tmp,jtime;
int i,jj,jd;
jtime=time(&tmp);
while(-1) {
	jj=r_count_pending();
	if(debug > 2) printmsg("------->rcount=%d num=%d",jj,num);
	if(jj >= num ) {
		for(i=0;i<num;i++) buf[i]=receive_com();
		return(jj); }
	else {
		jd=time(&tmp)-jtime;
		if(jd>=timeout) return(jj); }
}
}
#endif PC
#ifdef VMS
sread(buf,num,timeout)
char *buf;
int num,timeout;
{
long tmp,jtime,trmmsk[2];
int i,jj,jd;
struct {
       short count;
       char c;
       char res1;
       long res2;	
       } hdcnt;
jtime=time(&tmp);
trmmsk[0]=0;
trmmsk[1]=0;
while(-1) {
	sys$qiow(0,fpr,(IO$_SENSEMODE|IO$M_TYPEAHDCNT),0,0,0,&hdcnt,sizeof(hdcnt),0,0,0,0);
	jj=hdcnt.count; 
       	if(debug > 3) printmsg("----v--->rcount=%d, num=%d",jj,num);
	if(jj >= num) {
		sys$qiow(0,fpr,IO$_READVBLK,&ttiosb,0,0,buf,num,0,trmmsk,0,0);
       		/*printf("%c",buf[0]);*/
       		return(jj); }
	else {
		jd=time(&tmp)-jtime;
		if(jd>=timeout) return(jj); }
}
}
#endif VMS


openline(name,baud)
char *name,*baud;
{
#ifdef DG
fpr=open(name,0); fpw=open(name,1); setline(fpw); setline(fpr);
if(fpr < 0) return(-1); if(fpw < 0) return(-1);
#endif
#ifdef PC
int i;
sscanf(name,"COM%d",&i); select_port(i); save_com(); install_com();
sscanf(baud,"%d",&i); open_com(i,'D','N',STOPBIT,'D');
dtr_on();
#endif
#ifdef VMS
int ii,spd,spds[] = {110,150,300,600,1200,1800,2400,4800,9600,19200,0};
static int spcodes[] = {TT$C_BAUD_110,TT$C_BAUD_150,TT$C_BAUD_300,TT$C_BAUD_600,TT$C_BAUD_1200,TT$C_BAUD_2400,TT$C_BAUD_4800,TT$C_
AUD_9600,TT$C_BAUD_19200,0};
$DESCRIPTOR(tmp,name);
tmp.dsc$w_length = strlen(name);
if((ii=sys$alloc(&tmp,0,0,0)) != 1) {
       if(debug > 3) printmsg("Alloc %d",ii);
       if(!remote) return(-1);
}
if((ii=sys$assign(&tmp,&fpr,0,0)) != 1) {
       if(debug > 3) printmsg("Assign %d",ii);
       return(-1);
}
fpw=fpr;
ii=sys$qiow(0,fpr,IO$_SENSEMODE,0,0,0,&ttold,sizeof(ttold),0,0,0,0);
if(debug && !remote) lib$signal(ii);
ttraw=ttold;
#ifdef TT2$M_PASTHRU
ttraw.extended |= TT2$M_PASTHRU;
#endif
#ifdef TT2$M_ALTYPEAHD;
ttraw.extended |= TT2$M_ALTYPEAHD;
#endif
ttraw.basic |= TT$M_PASSALL;
ttraw.basic |= TT$M_NOECHO|TT$M_EIGHTBIT;
ttraw.basic &= ~TT$M_WRAP & ~TT$M_HOSTSYNC & ~TT$M_TTSYNC;
sscanf(baud,"%d",&ii); 
for(spd=0; spds[spd] != 0 && spds[spd] != ii; spd++) ;
/*if(spds[spd]==0) err or default ?*/
if(debug) printmsg("speed code %d %d\n",spds[spd],spd);
ii=sys$qiow(0,fpr,IO$_SETMODE,0,0,0,&ttraw,sizeof(ttraw),spcodes[spd],0,0,0);
if(debug && !remote) lib$signal(ii);
#endif
return(0);
}

closeline()
{
#ifdef DG
close(fpr); close(fpw);
#endif
#ifdef PC
dtr_off();
close_com(); restore_com();
#endif
#ifdef VMS
int ii;
swrite("+++\r",4); /*helps at times */
ii=sys$qiow(0,fpr,(IO$_SETMODE|IO$M_HANGUP),0,0,0,0,0,0,0,0,0);
if(debug && !remote) lib$signal(ii);
sys$dassgn(fpr);
sys$dalloc(0,0);
#endif
}
