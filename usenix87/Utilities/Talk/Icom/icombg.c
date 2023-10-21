/*	backgroud task		*/
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "icom.h"

int	icomlog = 2;
long	itime;

char	sts[512];

struct	s_shar	*sp;
int	shmid;
int	msgid;
int	msg2use[NUMUSERS];

struct	s_msg smsg, wms;

char	pname[NUMUSERS][32];

main()
{
	int	i,j,k;
	long	ti,oti;
	int	closeup();
	int	sigcatch();
	struct	s_shar *shmat();
	struct	shmid_ds zs;

	signal(SIGTERM,closeup);
	signal(SIGQUIT,closeup);
	signal(SIGPIPE,SIG_IGN);
	signal(SIGHUP,SIG_IGN);
	signal(SIGINT,SIG_IGN);

	shmid = shmget(SHMKEY,sizeof(struct s_shar),IPC_CREAT|0777);
	if(shmid<0) {
		perror("icombg@shmget");
		exit(1);
	}
	msgid = msgget(SHMKEY,IPC_CREAT|0777);
	if(msgid<0) {
		perror("icombg@msgget");
		blowexit(1);
	}
	for(i=0; i<NUMUSERS; i++) {
		msg2use[i] = open(fifolist[i],2);
		if(msg2use[i]<0) {
			sprintf(sts,"icombg@openfifo%d",i);
			perror(sts);
			blowexit(1);
		}
	}
	sp = shmat(shmid,NULL,SHM_R,SHM_W);
	if((long)sp == -1) {
		perror("icombg@shmat");
		blowexit(1);
	}

	icomlog = creat(ICOMSTAT,0666);

	if(icomlog<0) {
		perror("icombg@openlog");
		exit(1);
	}
	fillj(-1,sp,sizeof(struct s_shar));
	sp->cpid = getpid();
	sp->cstatus = 0;
	j = 0;
	for(;;) {
		zpr("rw");
		i = msgrcv(msgid,&smsg,sizeof(smsg)-sizeof(smsg.mtype),0,0);
		sprintf(sts,"fr: ty %d sz %d pn %d (%x %x %x %x %x %x %x %x)"
			,smsg.mtype ,i ,smsg.pnum ,smsg.u.ta[0]&255
			,smsg.u.ta[1]&255 ,smsg.u.ta[2]&255 ,smsg.u.ta[3]&255
			,smsg.u.ta[4]&255 ,smsg.u.ta[5]&255 ,smsg.u.ta[6]&255
			,smsg.u.ta[7]&255);
		zpr(sts);
		if(i==-1) {
			continue;
		}
		checkactions();
	}
}

checkactions()
{
	int	hs;
	register pn,i,j,k,l;
	hs = nstat(1);
	pn = smsg.pnum;	
	switch(smsg.mtype) {
	case CLOGIN:
		for(i=0; i<NUMUSERS; i++)
			if(sp->pp[i].pnum==-1) break;
		if(i>=NUMUSERS) {
			signal(pn,SIGQUIT);
			break;
		}
		sp->pp[i].ppid = pn;
		strcpy(pname[i],smsg.u.ta);
		for(j=0; j<NUMUSERS; j++) {
			if(sp->pp[j].pnum<0) continue;
			s_wmessage(i,j,CLOGIN,pname[j]);
		}
		sp->pp[i].pnum = 0;
		a_wmessage(i,CLOGIN,smsg.u.ta);
		break;
	case CLOGOUT:
		a_wcmd(CLOGOUT,pn);
		fillj(-1,&sp->pp[pn],sizeof(sp->pp[pn]));
		break;
	case CDCHAR:
		a_wchar(pn,smsg.u.ch);
		break;
	}
	nstat(hs);
}

sigcatch(s)
int s;
{
	signal(s,SIG_IGN);
	signal(s,sigcatch);
}

closeup(s)
int s;
{
	int i;
	signal(s,SIG_IGN);
	close(icomlog);
	blowexit(0);
}

blowexit(n)
int n;
{
	int i;
	shmctl(shmid,IPC_RMID,0);	/* kill share */
	msgctl(msgid,IPC_RMID,0);	/* kill msg */
	for(i=0; i<NUMUSERS; i++)
		close(msg2use[i]);
	exit(n);
}

fillj(c,s,size)
char c;
char *s;
int size;
{
	while(--size>=0) *s++ = c;
}

s_wmessage(to,from,type,text)
int to,from,type;
char *text;
{
	wms.mtype = type;
	wms.pnum = from;
	strcpy(wms.u.ta,text);
	writemsg(to,&wms,strlen(wms.u.ta)+sizeof(wms.pnum)+1);
}

a_wmessage(from,type,text)
int from,type;
char *text;
{
	wms.mtype = type;
	wms.pnum = from;
	strcpy(wms.u.ta,text);
	masswrite(strlen(text)+sizeof(wms.pnum)+1);
}

s_wcmd(to,command,from)
int to,command,from;
{
	wms.mtype = command;
	wms.pnum = from;
	writemsg(to,&wms,sizeof(wms.pnum));
}

a_wcmd(command,from)
int command,from;
{
	wms.mtype = command;
	wms.pnum = from;
	masswrite(sizeof(wms.pnum));
}

s_wchar(to,from,ch)
int to,from,ch;
{
	wms.mtype = CDCHAR;
	wms.pnum = from;
	wms.u.ch = ch;
	write(to,&wms,sizeof(wms.pnum)+sizeof(wms.u.ch));
}

a_wchar(from,ch)
int from,ch;
{
	wms.mtype = CDCHAR;
	wms.pnum = from;
	wms.u.ch = ch;
	masswrite(sizeof(wms.pnum)+sizeof(wms.u.ch));
}

masswrite(l)
int l;
{
	register int i;
	for(i=0; i<NUMUSERS; i++) {
		if(sp->pp[i].pnum<0) continue;
		writemsg(i,&wms,l);
	}
}

writemsg(to,ptr,len)
int to;
struct s_msg *ptr;
int len;
{
	int hs;
	int i;
	hs = nstat(4);
	sprintf(sts,"to %d: type %d size %d pnum %d (%x %x %x %x %x %x %x %x)"
		,to ,ptr->mtype ,len ,ptr->pnum ,ptr->u.ta[0]&255
		,ptr->u.ta[1]&255 ,ptr->u.ta[2]&255 ,ptr->u.ta[3]&255
		,ptr->u.ta[4]&255 ,ptr->u.ta[5]&255 ,ptr->u.ta[6]&255
		,ptr->u.ta[7]&255);
	zpr(sts);
	if(to==NUMUSERS) {
		i = msgsnd(msgid,ptr,len,IPC_NOWAIT);
	} else {
		len += sizeof(ptr->mtype);
		write(msg2use[to],&len,sizeof(len));
		write(msg2use[to],ptr,len);
	}
}

nstat(i)
int i;
{
	int j;
	j = sp->cstatus;
	sp->cstatus = i;
	return j;
}

zpr(s)
char *s;
{
	int i;
	i = strlen(s);
	write(icomlog,s,i);
	write(icomlog,"\n",1);
}
