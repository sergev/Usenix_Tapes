#

/*
 *      con - connect mx virtual terminal
 */


#include "mxvt.h"               /* escape char definitions */

#define EINTR   4               /* interrupted sys call error */
#define SZBUF   256             /* standard buffer size */
#define ALARM   14              /* alarm clock signal */

#define bgetc(b)   (--(b)->b_cnt>=0 ? *(b)->b_next++&0377 : bfill(b))
#define bputc(c,b) (--(b)->b_cnt>=0 ? (*(b)->b_next++=(c)) : bflushc((c),b))


struct buf {
	int     b_fd;           /* file descr */
	int     b_cnt;          /* count left */
	char    *b_next;        /* next char pointer */
	char    b_buf[SZBUF];
} ib,ob;

int     pid     -1;             /* child pid */
extern  errno;                  /* sys call error number */
int     receiving;              /* this half is receiving */
int     fd;                     /* net fd */
char    ostty[6];               /* original stty */
char    nstty[6];               /* new stty */
int     flush;                  /* flushing net receive stream until C_DM */

extern  onintr(),onquit(),sync(),synca();


main(argc,argv)
char **argv;
{
	gtty(0,ostty);
	gtty(0,nstty);
	if((fd = mxfile()) < 0)
		out("no mx files\n");
	if(argc == 1)
		out("need host name\n");
	if(mxscon(fd,argv[1],1) < 0)
		out("connect failed\n");
	ps("connect\n");

	while((pid = receiving = fork()) == -1)
		sleep(10);
	if(receiving)
		recv();
	else
		send();
}

send()
{
	register c;
	extern par_uid;
	pid = par_uid;
	signal(2,onintr);
	signal(3,onquit);
	ib.b_fd = 0;            /* read keyboard */
	ob.b_fd = fd;           /* write net */
	for(;;) {
		if((c = bgetc(&ib)) == IAC)
			bputc(IAC,&ob);
		bputc(c,&ob);
	}
}

recv()
{
	register c,i;
	struct buf ab;
	signal(2,sync);
	signal(3,sync);
	ib.b_fd = fd;           /* read net */
	ob.b_fd = 1;            /* write tty */
	ab.b_fd = fd;           /* write net */
	ab.b_cnt = ab.b_next = 0;
	for(;;) {
		if((c = bgetc(&ib)) != IAC) {
			if(flush) continue;
			bputc(c,&ob);
			continue;
		}
		switch(bgetc(&ib)) {

		case IAC:
			if(!flush) bputc(IAC,&ob);
			break;

		case C_DM:
			flush = 0;
			bclear(&ob);
			alarm(0);
			break;

		case C_GT:
			gtty(0,nstty);
			bputc(IAC,&ab);  bputc(C_ST,&ab);
			for(i=0 ; i<sizeof nstty ; i++)
				bputc(nstty[i],&ab);
			bflush(&ab);
			break;

		case C_ST:
			for(i=0 ; i<sizeof nstty ; i++)
				nstty[i] = bgetc(&ib);
			stty(0,nstty);
			break;

		default:
			ps("bad IAC recv\n");
			break;
		}
	}
}

out(s)
{
	stty(0,ostty);
	ps(s);
	kill(pid,9);
	exit();
}

ps(s)
char *s;
{
	register char *cp;
	for(cp=s ; *cp++ ;);
	write(1,s,cp-s-1);
}

sync()
{
	signal(2,sync);
	signal(3,sync);
	bclear(&ob);
	signal(ALARM,synca);
	alarm(2);
	flush = 1;
}

synca()
{
	flush = 0;
}

bfill(b)
register struct buf *b;
{
	register i;
	bflush(&ob);    /* next read might block; ensure output flushed */
	for(;;) {
		i = read(b->b_fd, b->b_buf, SZBUF);
		if(i == 0) {
			eof(b);
			continue;
		}
		if(i < 0) {
			if(errno == EINTR) continue;
			out("read error\n");
		}
		break;
	}
	b->b_next = b->b_buf;
	b->b_cnt = i;
	return(bgetc(b));
}

bflush(b)
register struct buf *b;
{
	register i,j;
	if(!b->b_next) {
		bclear(b);
		return;
	}
	if(b->b_next == b->b_buf) return;
	for(;;) {
		i = write(b->b_fd, b->b_buf, (j=b->b_next-b->b_buf));
		if(i != j) {
			if(errno == EINTR) break;      /* was continue */
			out("write error\n");
		}
		break;
	}
	bclear(b);
}

bflushc(c,b)
struct buf *b;
{
	bflush(b);
	bputc(c,b);
}

bclear(b)
register struct buf *b;
{
	b->b_next = b->b_buf;
	b->b_cnt = SZBUF;
}

eof(b)
struct buf *b;
{
	if(b != &ib) return;
	if(receiving)
		out("disconnect\n");
	bputc(IAC,&ob);  bputc(C_EF,&ob);
	bflush(&ob);
}

onintr()
{
	static char iac_in[] { IAC, C_IN };
	signal(2,onintr);
	write(fd,iac_in,2);
}

onquit()
{
	static char iac_qu[] { IAC, C_QU };
	signal(3,onquit);
	write(fd,iac_qu,2);
}
