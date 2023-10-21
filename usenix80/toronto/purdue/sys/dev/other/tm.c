#
/*
 * TM tape driver
 * mods by j.b., winter and spring '76
 */

#include "/usr/sys/param.h"
#include "/usr/sys/buf.h"
#include "/usr/sys/conf.h"
#include "/usr/sys/user.h"

struct {
	int tmer;
	int tmcs;
	int tmbc;
	int tmba;
	int tmdb;
	int tmrd;
};

#define	TMADDR	0172520

struct	devtab	tmtab;
struct	buf	rtmbuf;

#define	GO	01
#define	RCOM	02
#define	WCOM	04
#define	WEOF	06
#define	SFORW	010
#define	SREV	012
#define	WIRG	014
#define	REW	016
#define UNLOAD	000
#define	IENABLE	0100
#define	CRDY	0200
#define SELR	0100
#define WLOCK	0004
#define GAPSD	010000
#define BOT	040
#define EOT	04000
#define	TUR	1

#define	HARD	0100200	/* ILC, NXM */
#define PROG	0003000	/* EOT, RLE */
#define PARITY	0034400	/* PAE, CRE, BGL, BTE,  */
#define	EOF	0040000	/* EOF */

#define	SSEEK	1
#define	SIO	2
#define SKIPR	3
#define SKIPF	4
#define N_RETRY	15
/*page*/
#define KDENS	 066000
#define D	0160000
#define NINE	 010000
#define CLREW	0100000

char	t_openf[8];
char	*t_blkno[8];
char	*t_nxrec[8];
int	tmlast[8];	/* last status */
int	tmd[8]	{D|NINE,D,D|NINE,D,D,D,D,D};

tmopen(dev, flag)
{
	register dminor;

# ifdef POWERFAIL
	if (dev == NODEV) {
		TMADDR->tmcs = IENABLE|UNLOAD|GO;  /* abort */
		return;
	}
# endif
	dminor = dev.d_minor;
	if (t_openf[dminor] || !tu_online(dminor,flag))
		u.u_error = ENXIO;
	else {
		t_openf[dminor]++;
		t_blkno[dminor] = 0;
		t_nxrec[dminor] = 65535;
	}
}

tmclose(dev, flag)
{
	register int dminor;

	dminor = dev.d_minor;
	t_openf[dminor] = 0;
	if (flag)
		tcommand(dminor, WEOF);
	if(tmd[dminor]&CLREW)
		tcommand(dminor,REW,0);
}

tcommand(unit, com, count)
{
	extern lbolt;

	while (tmtab.d_active || (TMADDR->tmcs & CRDY)==0)
		sleep(&lbolt, 1);
	TMADDR->tmbc = -count;
	TMADDR->tmcs = (tmd[unit]&KDENS)|com|GO | (unit<<8);
}
/*page*/
tmstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;
	register char **p;

	bp = abp;
	p = &t_nxrec[bp->b_dev.d_minor];
	if (*p <= bp->b_blkno) {
		if (*p < bp->b_blkno) {
			bp->b_flags =| B_ERROR;
			iodone(bp);
			return;
		}
		if (bp->b_flags&B_READ) {
			clrbuf(bp);
			iodone(bp);
			return;
		}
	}
	if ((bp->b_flags&B_READ)==0)
		*p = bp->b_blkno + 1;
	bp->av_forw = 0;
	spl5();
	if (tmtab.d_actf==0)
		tmtab.d_actf = bp;
	else
		tmtab.d_actl->av_forw = bp;
	tmtab.d_actl = bp;
	if (tmtab.d_active==0)
		tmstart();
	spl0();
}
/*page*/
tmstart()
{
	register struct buf *bp;
	register int com;
	int unit;
	register char *blkno;

    loop:
	if ((bp = tmtab.d_actf) == 0)
		return;
	unit = bp->b_dev.d_minor;
	blkno = t_blkno[unit];
	if (t_openf[unit] < 0 || (TMADDR->tmcs & CRDY)==0  ) {
    bad:
		bp->b_flags =| B_ERROR;
		tmtab.d_actf = bp->av_forw;
		iodone(bp);
		goto loop;
	}
	com = (unit<<8) | ((bp->b_xmem & 03) << 4) | IENABLE|(tmd[unit]&KDENS);
	if(bp == &rtmbuf){
	if((rtmbuf.b_flags&B_PHYS)==0){
		com =| bp->b_addr;
		if((bp->b_addr == SREV && tmlast[unit]&BOT) || (bp->b_addr==SFORW && tmlast[unit]&EOT))
			goto bad;
		tmtab.d_active = SIO;
		TMADDR->tmbc = bp->b_wcount;
		TMADDR->tmcs = com|GO;
		return;
	}
	}
	if (blkno != bp->b_blkno) {
		tmtab.d_active = SSEEK;
		if (blkno < bp->b_blkno) {
			com =| SFORW|GO;
			TMADDR->tmbc = blkno - bp->b_blkno;
		} else {
			if (bp->b_blkno == 0 && bp != &rtmbuf)
				com =| REW|GO;
			else {
				com =| SREV|GO;
				TMADDR->tmbc = bp->b_blkno - blkno;
			}
		}
		TMADDR->tmcs = com;
		return;
	}
	tmtab.d_active = SIO;
	TMADDR->tmbc = bp->b_wcount << 1;
	TMADDR->tmba = bp->b_addr;
	TMADDR->tmcs = com | ((bp->b_flags&B_READ)? RCOM|GO:
	    ((tmtab.d_errcnt)? WIRG|GO: WCOM|GO));
}
/*page*/
tmintr()
{
	register struct buf *bp;
	register int unit, *pp;

	if ((bp = tmtab.d_actf)==0)
		return;
	unit = bp->b_dev.d_minor;
	tmlast[unit] = TMADDR->tmer;
	if(TMADDR->tmcs < 0) {
		while(TMADDR->tmrd&GAPSD) {}
		if(TMADDR->tmer&HARD) goto hard;
		if(TMADDR->tmer&PROG) goto prog;
		if(TMADDR->tmer&EOF)
			if(bp == &rtmbuf){
				if(bp->b_flags&B_READ)
					TMADDR->tmbc = bp->b_wcount*2;
				goto next;
			} else
				goto eof;
		if(TMADDR->tmer&PARITY && tmtab.d_active == SIO) {
			if(++tmtab.d_errcnt < N_RETRY) {
				t_blkno[unit]++;
				goto retry;
			} goto prog;		/* retry count exhausted */
		} goto prog;			/* error on skip */
		printf("\n\"impossible\" mag-tape error:\n");
hard:						/*  hard(ware) error --irrecoverable  */
		deverror(bp,TMADDR->tmer,TMADDR->tmcs);
eof:						/* illegal EOF */
		t_openf[unit] = -1;		/* stop all subsequent i/o */
prog:						/* programmer or irrecoverable parity err */
		bp->b_flags =| B_ERROR;		/* mark buffer bad */
		tmtab.d_active = SIO;		/* so it'll get iodone */
	}					/* end of error handler */
next:
	if (tmtab.d_active == SIO) {
		tmtab.d_errcnt = 0;
		t_blkno[unit]++;
		tmtab.d_actf = bp->av_forw;
		tmtab.d_active = 0;
		bp->b_resid = TMADDR->tmbc;
		iodone(bp);
	} else
		t_blkno[unit] = bp->b_blkno;
retry:
	tmstart();
}
/*page*/
tmread(dev)
{
	tmsetup(dev);
	physio(tmstrategy, &rtmbuf, dev, B_READ);
/*	if(rtmbuf.b_flags&EOF==0)	*/
	u.u_count = -rtmbuf.b_resid;
}

tmwrite(dev)
{
	if(u.u_count == 0)
		tcommand(dev.d_minor,WEOF,0);
	else {
		tmsetup(dev);
		physio(tmstrategy, &rtmbuf, dev, B_WRITE);
		u.u_count = 0;
	}
}

tmsetup(dev)
{
	register unit, a;

	unit = dev.d_minor;
	a = lshift(u.u_offset, -9);
	t_blkno[unit] = a;
	t_nxrec[unit] = ++a;
	u.u_count =+ u.u_count&01;	/* odd to even */
}


tu_online(unit,flag)
{
	extern lbolt;

	while(tmtab.d_active || (TMADDR->tmcs&CRDY)==0)
		sleep(&lbolt,1);
		TMADDR->tmcs = unit<<8;
		if(TMADDR->tmer & SELR)
			if(!flag)
				return(1);
			else
				return(TMADDR->tmer&WLOCK ? 0 : 1);
		else
			return(0);
}
/*page*/
#define PEVEN	  04000
#define DEN556	      0
#define DEN800	 020000
#define DEN800D	 040000
#define DEN1600	 060000
#define BPI1600	  02000
#define DENSITY	 060000
#define CLREW	0100000
#define NINE	 010000
#define DENDIF	 020000
#define	UMASK	(DENSITY|PEVEN|CLREW|NINE)
#define SMASK	(UMASK|BPI1600)

tmsgtty(dev,av)
int dev,*av;
{
	register int *v,unit,x;
	unit = dev.d_minor;
	if(v=av) {			/* getty */
		*v++ = (tmd[unit]&UMASK) - (tmd[unit]&BPI1600? 0: DENDIF);
		*v++ = 0;
		*v++ = 0;
	}
	else {				/* setty */
		v = u.u_arg;
		if(*v & 0377) {
			u.u_error = tmxcom(dev,v);
		} else {
			x = *v;
			if((tmd[unit]&NINE) == NINE){
				if((x&DENSITY) < DEN800D)
					goto fail;
				x =| NINE;
			}
			if((x & DENSITY) == DEN1600 )
				if((tmd[unit]&NINE)==NINE)
					x =| BPI1600;
				else
					goto fail;
			else
				x =+ DENDIF;
			tmd[unit] = x & SMASK;
		}
	}
	return;
fail:
	u.u_error = EINVAL;
}
/*page*/
tmxcom(dev,av)
int *av;
{
	register int *v,count,com;
	int error;

	v = av;
	count = v[1];
	switch(*v&0377){
	default:			/* undefined operation */
		return(EINVAL);
	case 1:				/* endfile */
		com = WEOF;
		break;
	case 2:				/* skip files forward */
		error = 0;
		while(count-- > 0 && (error&EOT) == 0)
			error = tmphys(&rtmbuf,dev,SFORW,0100000);
		return(0);
/*
 *	case 3 is coded differently from the seemingly symmetrical
 *	case 2, because of the peculiar behavior of the DIGI-DATA
 *	tape drives near the BOT.
 *	God save us from whatever perils lurk beyond the EOT.
 */
	case 3:				/* skip files backward */
		error = 0;
		count++;
		while(count-- > 0 && (error&BOT) == 0){
			error = tmphys(&rtmbuf,dev,SREV,200);
			if((error&EOF) == 0)
				count++;
		}
		com = SREV;
		goto out;
	case 4:				/* skip records forward */
		com = SFORW;
		break;
	case 5:				/* skip records backward */
		com = SREV;
		if(count > 512)
			count = 512;
		break;
	case 6:				/* rewind */
		com = REW;
		break;
	case 7:				/* rewind & unload */
		com = UNLOAD;
		break;
	}
	error = tmphys(&rtmbuf,dev,com,count);
out:
	if((error&EOF) && com == SREV)
		error = tmphys(&rtmbuf,dev,SFORW,1);
	return(0);
}
/*page*/
tmphys(abp,dev,com,count)
struct buf *abp;
{
	register struct buf *bp;

	bp = abp;

	spl6();
	while(bp->b_flags&B_BUSY){
		bp->b_flags =| B_WANTED;
		sleep(bp,PRIBIO);
	}
	bp->b_flags =| B_BUSY;
	bp->b_flags =& ~(B_PHYS | B_DONE);

	bp->b_dev = dev;
	bp->b_wcount = -count;
	bp->b_addr = com;
	bp->b_xmem = 0;

	tmstrategy(bp);

	spl6();
	while((bp->b_flags&B_DONE)==0)
		sleep(bp,PRIBIO);
	if(bp->b_flags&B_WANTED)
		wakeup(bp);
	bp->b_flags =& ~(B_BUSY | B_WANTED);
	spl0();
	geterror(bp);
	return(tmlast[dev.d_minor]&(BOT|EOT|EOF));
}
