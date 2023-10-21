/*	forgroud task		*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <curses.h>
#include "icom.h"

#define	MSGLINES	5

char	ppname[32];
int	mynum;
int	childpid;
struct	s_shar *sp;
int	shmid;
int	istt;
int	msgid;
int	msg2use[NUMUSERS];
struct	s_msg smsg, wms;
struct	termio	ttmy;
WINDOW	*winds[4];

main()
{
	int	mpid,i,j;
	char 	c,fname[32], *ptname;
	struct	s_shar *shmat();
	WINDOW	*subwin();
	long	ti,oti;
	int	scatch();
	int	msglen;

	shmid = shmget(SHMKEY,sizeof(struct s_shar),0777);
	if(shmid<0) {
		fprintf(stderr,"Sorry, the icom controller is not running\n");
		exit(1);
	}
	sp = shmat(shmid,NULL,NULL);

	msgid = msgget(SHMKEY,0777);
	if(msgid<0) {
		fprintf(stderr,"Sorry, the icom controller is not running\n");
		exit(1);
	}
	for(i=0; i<NUMUSERS; i++) {
		msg2use[i] = open(fifolist[i],2);
		if(msg2use[i]<0) {
			perror("icom@openfifo");
			exit(1);
		}
	}
	sp = shmat(shmid,NULL,NULL);
	initscr();
	signal(SIGHUP,scatch);
	signal(SIGINT,scatch);
	signal(SIGTERM,scatch);
	signal(SIGQUIT,scatch);

	signal(SIGUSR2,SIG_IGN);

	mpid = getpid();
	clear();

	printw("icom\n\n");
	printw("What is your name? ");
	refresh();
	getstr(ppname);

	ptname = ppname;
	while(*ptname) {
		if(*ptname=='\n')
			*ptname = '\0';
		else
			*ptname++;
	}

	noecho();
	cbreak();
	ioctl(fileno(stdin),TCGETA,&ttmy);
	ttmy.c_cc[VMIN]=1;
	ttmy.c_cc[VTIME]=0;
	ioctl(fileno(stdin),TCSETA,&ttmy);
	strcpy(smsg.u.ta,ppname);
	smsg.pnum = mpid;
	smsg.mtype = CLOGIN;
	clear();
	winds[0] = subwin(stdscr,5,80,1,0); scrollok(winds[0],TRUE);
	wmove(winds[0],4,0);
	winds[1] = subwin(stdscr,5,80,7,0); scrollok(winds[1],TRUE);
	wmove(winds[1],4,0);
	winds[2] = subwin(stdscr,5,80,13,0); scrollok(winds[2],TRUE);
	wmove(winds[2],4,0);
	winds[3] = subwin(stdscr,5,80,19,0); scrollok(winds[3],TRUE);
	wmove(winds[3],4,0);
	drawscreen();
	refresh();
	writemsg(NUMUSERS,&smsg,strlen(ppname)+sizeof(smsg.pnum)+1);

	i = 0;
	for(;;) {
		if(sp->pp[i].ppid == mpid) break;
		++i;
		i %= NUMUSERS;
	}

	mynum = i;

	childpid = fork();
	if(childpid==0) {
		childbgc();
		exit(0);
	}

	for(;;) {
		read(msg2use[mynum],&msglen,sizeof(msglen));
		read(msg2use[mynum],&smsg,msglen);
		domsg();
	}
}

domsg()
{
	register int pn;
	pn = smsg.pnum;
	switch(smsg.mtype) {
	case CLOGIN:
		move(smsg.pnum*6,8);
		printw(" %s ",smsg.u.ta);
		refresh();
		return;
	case CLOGOUT:
		move(smsg.pnum*6,8);
		printw("------------------------");
		refresh();
		if(pn==mynum) icomexit();
		return;
	case CDCHAR:
		wprintw(winds[smsg.pnum],"%c",smsg.u.ch);
		wrefresh(winds[smsg.pnum]);
		return;
	}
}

scatch(s)
int s;
{
	icomexit();
}


icomexit()
{
	int retc;
	kill(childpid,SIGINT);
	wait(&retc);
	move(23,0);
	refresh();
	endwin();
	exit(0);
}

drawscreen()
{
	drawli(0);
	drawli(6);
	drawli(12);
	drawli(18);
	refresh();
}

drawli(c)
int c;
{
	int i;
	move(c,0);
	printw("-------------------------------------------------------------------------------");
}

static int diemonster;

childbgc()
{
	int i;
	int childsig();
	signal(SIGHUP,childsig);
	signal(SIGINT,childsig);
	signal(SIGTERM,childsig);
	signal(SIGQUIT,childsig);

	signal(SIGUSR1,SIG_IGN);
	signal(SIGUSR2,SIG_IGN);

	diemonster = 0;

	for(;;) {
		i = getchar()&255;
		if(i==0x1b || i==4 || diemonster) {
			s_wcmd(NUMUSERS,CLOGOUT,mynum);
			return;
		}
		s_wchar(NUMUSERS,mynum,i);
	}
}

childsig(s)
int s;
{
	int childsig();
	++diemonster;
	signal(s,childsig);
}

fillj(c,s,size)
char c;
char *s;
int size;
{
	while(--size>=0) *s++ = c;
}

s_wcmd(to,command,from)
int to,command,from;
{
	wms.mtype = command;
	wms.pnum = from;
	writemsg(to,&wms,sizeof(wms.pnum));
}

s_wchar(to,from,ch)
int to,from,ch;
{
	wms.mtype = CDCHAR;
	wms.pnum = from;
	wms.u.ch = ch;
	writemsg(to,&wms,sizeof(wms.pnum)+sizeof(wms.u.ch));
}

writemsg(to,ptr,len)
int to;
struct s_msg *ptr;
int len;
{
	int hs;
	int i;
	if(to==NUMUSERS) {
		i = msgsnd(msgid,ptr,len,IPC_NOWAIT);
	} else {
		i = msgsnd(msg2use[to],ptr,len,IPC_NOWAIT);
	}
}
