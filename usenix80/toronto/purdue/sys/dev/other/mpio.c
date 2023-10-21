#

/*
 *	DMC11 mpx physical link driver
 */

#define ADDR	0160200
#define	BUFR	4
#define	BUFX	0
#define	BASE	3
#define CTRL	1
#define	RDY	0200
#define	IE	0100
#define	RQ	040
#define MCLR	0300
#define	FATAL	01772
#define ORUN	04

#define	BSIZE	256
#define XWAIT_TICS 2
#define XWATCH_TICS 2

extern  errlg;

struct
{
	char	sin;
	char	smaint;
	char	sout;
	char	zz;
	int	sel4;
	int	sel6;
};

struct  dmbuf
{
	char	bseq;
	char	btype;
	char	bf[BSIZE];
};

struct
{
	char	xstate;
	char	abort;
	char	orun;
	char	rseq;
	char	first;
	char	xclock;
	int	qin;
	int	qout;
	int     q[5*3];
	int	base[128];
	struct  dmbuf bx;
	struct  dmbuf br;
}mpd;

mpxintr()
{
	register char *cp;
	register i,j;
	if(mpd.xstate) return;
	if((i=mpsx()) < 0) return;
	mpd.xstate = 1;
	cp = mpd.bx.bf;
	j = BSIZE;
	while(i>=0) {
		*cp++ = i;
		if(--j == 0) break;
		i = mpsx();
	}
	mpd.bx.btype = 1;
	mpdmcin(BUFX,&mpd.bx,cp - &mpd.bx);
}

mprintr()
{
	int o2,o4,o6;
	register char *cp;
	register j;
	int mpxwait();
	o2 = ADDR->sout;  o4 = ADDR->sel4;  o6 = ADDR->sel6;
	ADDR->sout =& ~RDY;
	switch(o2 & 7) {

	case BUFR:
		if(mpd.br.bseq != mpd.rseq++) {
			prf("DMC seq err\n");
			goto die;
		}
		if(mpd.br.btype) {
			j = o6 - 2;
			cp = mpd.br.bf;
			while(j--)
				mpsr(*cp++ & 0377);
		}
		mpdmcin(BUFR,&mpd.br,sizeof mpd.br);
		return;

	case BUFX:
		if(mpd.xstate == 1) {
			mpd.bx.bseq++;
			timeout(mpxwait,0,XWAIT_TICS);
			mpd.xclock = 0;
			mpd.xstate = 2;
		}
		return;

	case CTRL:
		if(o6 == ORUN)
			if(mpd.orun++ & 077) return;
		prf("DMC cntlo %o\n",o6);
		if(o6 & FATAL)
			goto die;
		return;

	default:
		prf("DMC ??? %o %o %o\n",o2,o4,o6);
	}
die:
	mpreset();
}

mpdreset()
{
	int i;
	mpd.xclock = 0;
	if(mpd.first == 0) {
		mpxwatch();
		mpd.first++;
	}
	mpd.bx.bseq = mpd.rseq = 0;
	ADDR->smaint = MCLR;
	for(i=0;i<1000;i++);
	mpd.qin = mpd.qout = 0;
	mpdmcin(BASE,mpd.base,0);
	mpdmcin(CTRL,0,0);
	mpdmcin(BUFR,&mpd.br,sizeof mpd.br);
	ADDR->sout =| IE;
	mpd.xstate = 0;
	mpd.abort = 1;
}

mpdmcin(i,s4,s6)
{
	int c,oqin;
	oqin = mpd.qin;
	mpd.q[mpd.qin++] = i;
	mpd.q[mpd.qin++] = s4;
	mpd.q[mpd.qin++] = s6;
	if(oqin != mpd.qout) return;
	while(mpd.qin != mpd.qout) {
start:		ADDR->sin = mpd.q[mpd.qout] | RQ;
		c = 0;
wait:		while(ADDR->sin >= 0  &&  ADDR->sout >= 0)
			if(c++ > 500)goto start;
		if(ADDR->sout < 0) {
			mpd.abort = 0;
			mprintr();
			if(mpd.abort) return;
			goto start;
		}
		mpd.qout++;
		ADDR->sel4 = mpd.q[mpd.qout++];
		ADDR->sel6 = mpd.q[mpd.qout++];
		ADDR->sin =& ~RQ;
		while(ADDR->sin & RDY);
	}
	mpd.qin = mpd.qout = 0;
}

mpxnull()
{
	if(mpd.xstate) return;
	mpd.xstate = 1;
	mpd.bx.btype = 0;
	mpdmcin(BUFX,&mpd.bx,2);
}

mpxwatch()
{
	if(mpd.xclock++ >= 2)
		pireq(mpxnull,5);
	timeout(mpxwatch,0,XWATCH_TICS);
}

mpxwait()
{
	mpd.xstate = 0;
	pireq(mpxintr,5);
}

prf(x1,x2,x3,x4,x5)
{
	errlg++;
	printf(x1,x2,x3,x4,x5);
	errlg = 0;
}
