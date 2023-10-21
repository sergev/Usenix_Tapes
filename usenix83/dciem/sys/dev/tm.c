#include "../h/mascot.h"
#

/*
 * TM tape driver
 */

/*
 * HCR:  SP-1 - 23 JUNE 81.
 *		-add ioctl
 *		-forward & backward spacing
 *		-writing end of file mark
 *		-no rewind/rewind on close, that overides
 *		 mode given by minor device number.
 *		-The state of dev error printing is remembered through
 *		 opens & closes, but other states are restored (i.e
 *		 rewind on close).
 *
 * HCR:	SP-2 - 25 JUNE 81.
 *		Added checking for invalid unit numbers.
 *		Fixed type in tmclose that used 077 instead of 0177.
 *
 * MIT01	Martin Tuori, DCIEM, Toronto, Mar 3, 1982.
 *		added deverror(), and increase retries to 10.
 */
#include "../h/local.h"	/* SP-1 */
#include "../h/param.h"
#include "../h/buf.h"
#include "../h/dir.h"
#include "../h/conf.h"
#include "../h/file.h"
#include "../h/user.h"
#ifdef dciemtape
#include "../h/tty.h"
#endif

struct device {
	int	tmer;
	int	tmcs;
	int	tmbc;
	char	*tmba;
	int	tmdb;
	int	tmrd;
};

struct	buf	tmtab;
struct	buf	ctmbuf;
struct	buf	rtmbuf;

char	t_flags[8];
char	t_openf[8];
daddr_t	t_blkno[8];
daddr_t	t_nxrec[8];

#define	TMADDR ((struct device *)0172520)

#define	GO	01
#define	RCOM	02
#define	WCOM	04
#define	WEOF	06
#define	NOP	0100
#define	SFORW	010
#define	SREV	012
#define	WIRG	014
#define	REW	016
#define	DENS	060000		/* 9-channel */
#define	IENABLE	0100
#define	CRDY	0200
#define GAPSD	010000
#define	TUR	1
#define	HARD	0102200	/* ILC, EOT, NXM */
#define RLE	01000
#define	EOF	0040000
#define	WL	04

#define	SSEEK	1
#define	SIO	2
#define	SCOM	3

#define T_WRITTEN 1
# ifdef dciemtape	/* SP-1 */
# define T_DRWC		04	/* don't rewind on close */
# define T_DEVR		010	/* print deverrors */
# define T_SOFLN	020	/* tape has been 'stty'ed off-line */

/* stty modes */

# define NRWC		1	/* no rewind on close */
# define RWC		2	/* rewind on close */
# define EOT		3	/* write end of file mark */
# define BOT		4	/* rewind */
# define FSR		5	/* forward space record */
# define BSR		6	/* backward space record */
# define OFLIN		7	/* take drive offline */
# define DEVR		8	/* turn on deverrors */
# define NDEVR		9	/* turn off deverrors */

/* additional drive commands */

# define OFLN		0

# define NTM		2	/* number of tape units */
# define TMMAXERR 10
# endif

tmopen(dev, flag)
{
	register unit, ds;

	unit = minor(dev) & 0177;
#ifndef dciemtape
	if (t_openf[unit]) {
#else
	if(unit >= NTM || t_openf[unit]) {
#endif
		u.u_error = ENXIO;
		return;
	}
	t_blkno[unit] = 0;
	t_nxrec[unit] = 65535;
# ifndef dciemtape
	t_flags[unit] = 0;
# else
	t_flags[unit] &= ~(T_WRITTEN|T_DRWC|T_SOFLN); /* other things are remembered*/
# endif

	tmtab.b_flags |= B_TAPE;
	ds = tcommand(dev, NOP);
	if ((ds&TUR)==0) {
# ifdef dciemtape
		/*
		 * At this point we want to double check the on-line
		 * bit. This time we will check using the old (V6) way.
		 */
		{
			extern lbolt;

			spl5();
			while(tmtab.b_actf || (TMADDR->tmcs&CRDY)==0)
				sleep(&lbolt, 1);
			TMADDR->tmcs = DENS | (unit << 8);
			ds = TMADDR->tmer;
			spl0();
		}
		if((ds&TUR)==0){
#endif
		printf("mt%d off line, ds = 0%o\n", unit, ds);
		u.u_error = ENXIO;
#ifdef dciemtape
		}
#endif
	}
	if (flag && ds&WL) {
		printf("mt%d needs write ring\n",unit);
		u.u_error = ENXIO;
	}
	if (u.u_error==0)
#ifdef dciemtape
		{
		if(minor(dev) & 0200)
			t_flags[unit] |= T_DRWC;	/* subject to sttys*/
#endif
		t_openf[unit]++;
#ifdef dciemtape
	}
#endif
}

tmclose(dev, flag)
dev_t dev;
int flag;
{

	if ( flag == FWRITE ||
	((flag&FWRITE) && (t_flags[minor(dev)&0177]&T_WRITTEN))) {
		tcommand(dev, WEOF);
		tcommand(dev, WEOF);
		tcommand(dev, SREV);
	}
#ifndef dciemtape
	if ((minor(dev)&0200) == 0)
#else
	if (!(t_flags[minor(dev)&0177] & T_DRWC))
#endif
		tcommand(dev, REW);
#ifndef dciemtape
	t_openf[minor(dev)&077] = 0;
#else
	t_openf[minor(dev)&0177] = 0;
#endif
}

tcommand(dev, com)
{
	register struct buf *bp;

	bp = &ctmbuf;
	spl5();
	while (bp->b_flags&B_BUSY) {
		bp->b_flags |= B_WANTED;
		sleep((caddr_t)bp, PRIBIO);
	}
	bp->b_flags = B_BUSY|B_READ;
	spl0();
	bp->b_dev = dev;
	bp->b_resid = com;
	bp->b_blkno = 0;
	tmstrategy(bp);
	iowait(bp);
	if (bp->b_flags&B_WANTED)
		wakeup((caddr_t)bp);
	bp->b_flags = 0;
	return(bp->b_resid);
}

tmstrategy(bp)
register struct buf *bp;
{
	register daddr_t *p;

	if(bp->b_flags&B_PHYS)
		mapalloc(bp);
	if (bp != &ctmbuf) {
		p = &t_nxrec[minor(bp->b_dev)&0177];
		if (*p <= bp->b_blkno) {
			if (*p < bp->b_blkno) {
				bp->b_flags |= B_ERROR;
				iodone(bp);
				return;
			}
			if (bp->b_flags&B_READ) {
				clrbuf(bp);
				bp->b_resid = 0;
				iodone(bp);
				return;
			}
		}
		if ((bp->b_flags&B_READ) == 0) {
			t_flags[minor(bp->b_dev)&0177] |= T_WRITTEN;
			*p = bp->b_blkno+1;
		}
	}
	bp->av_forw = 0;
	spl5();
	if (tmtab.b_actf == NULL)
		tmtab.b_actf = bp;
	else
		tmtab.b_actl->av_forw = bp;
	tmtab.b_actl = bp;
	if (tmtab.b_active == NULL)
		tmstart();
	spl0();
}

tmstart()
{
	register struct buf *bp;
	register int com;
	int unit;
	register daddr_t *blkno;

    loop:
	if ((bp = tmtab.b_actf) == 0)
		return;
	unit = minor(bp->b_dev)&0177;
	blkno = &t_blkno[unit];
	if (t_openf[unit] < 0 || (TMADDR->tmcs & CRDY) == NULL) {
		bp->b_flags |= B_ERROR;
		goto next;
	}
	if (bp == &ctmbuf) {
		if (bp->b_resid == NOP) {
			bp->b_resid = TMADDR->tmer;
			goto next;
		}
		tmtab.b_active = SCOM;
		TMADDR->tmcs = DENS|bp->b_resid|GO| (unit<<8) | IENABLE;
		return;
	}
	com = (unit<<8) | ((bp->b_xmem & 03) << 4) | IENABLE|DENS;
	if (*blkno != bp->b_blkno) {
		tmtab.b_active = SSEEK;
		if (*blkno < bp->b_blkno) {
			com |= SFORW|GO;
			TMADDR->tmbc = *blkno - bp->b_blkno;
		} else {
			if (bp->b_blkno == 0)
				com |= REW|GO;
			else {
				com |= SREV|GO;
				TMADDR->tmbc = bp->b_blkno - *blkno;
			}
		}
		TMADDR->tmcs = com;
		return;
	}
	tmtab.b_active = SIO;
	TMADDR->tmbc = -bp->b_bcount;
	TMADDR->tmba = bp->b_un.b_addr;
	TMADDR->tmcs = com | ((bp->b_flags&B_READ)? RCOM|GO:
	    ((tmtab.b_errcnt)? WIRG|GO: WCOM|GO));
	return;

next:
	tmtab.b_actf = bp->av_forw;
	iodone(bp);
	goto loop;
}

tmintr()
{
	register struct buf *bp;
	register int unit;
	int	state;

	if ((bp = tmtab.b_actf) == NULL)
		return;
	unit = minor(bp->b_dev)&0177;
	state = tmtab.b_active;
	tmtab.b_active = 0;
	if (TMADDR->tmcs < 0) {		/* error bit */
		if(t_flags[unit]&T_DEVR){
			if(TMADDR->tmer & 0137600)
				/* report any error excpet EOF */
				deverror(bp,TMADDR->tmer,TMADDR->tmcs);
			if((tmtab.b_errcnt+1) >= TMMAXERR)
				printf("tm max retries on unit %d\n",unit);
		}
		while(TMADDR->tmrd & GAPSD) ; /* wait for gap shutdown */
		if (TMADDR->tmer&EOF) {
			t_nxrec[unit] = bp->b_blkno;
			state = SCOM;
			TMADDR->tmbc = -bp->b_bcount;
			goto out;
		}
		if ((TMADDR->tmer&HARD) == 0 && TMADDR->tmer&RLE) {
			state = SIO;
			goto out;
		}
		if ((TMADDR->tmer&(HARD|EOF)) == NULL && state==SIO) {
			if (++tmtab.b_errcnt < TMMAXERR) {
				t_blkno[unit]++;
				tmtab.b_active = 0;
				tmstart();
				return;
			}
		} else
			if (t_openf[unit]>0 && bp!=&rtmbuf &&
				(TMADDR->tmer&EOF)==0 ) {
				t_openf[unit] = -1;
				deverror(bp, TMADDR->tmer, 0);
			}
		bp->b_flags |= B_ERROR;
		state = SIO;
	}
out:
	switch ( state ) {
	case SIO:
		t_blkno[unit] += (bp->b_bcount>>BSHIFT);
	case SCOM:
		tmtab.b_errcnt = 0;
		tmtab.b_actf = bp->av_forw;
		bp->b_resid = -TMADDR->tmbc;
		iodone(bp);
		break;
	case SSEEK:
		t_blkno[unit] = bp->b_blkno;
		break;
	default:
		return;
	}
	tmstart();
}

tmread(dev)
{
	tmphys(dev);
	physio(tmstrategy, &rtmbuf, dev, B_READ);
}

tmwrite(dev)
{
	tmphys(dev);
	physio(tmstrategy, &rtmbuf, dev, B_WRITE);
}

tmphys(dev)
{
	register unit;
	daddr_t a;

	unit = minor(dev) & 0177;
	if(unit < 8) {
		a = u.u_offset >> 9;
		t_blkno[unit] = a;
		t_nxrec[unit] = a+1;
	}
}
#ifdef dciemtape
tmioctl(dev, com, addr, flag)
dev_t dev;
caddr_t addr;
{
	register int code;
	register char *t;

	if(com != TIOCSETP)
		u.u_error = ENXIO;
	else{
		code = fuword(addr);
		t = &t_flags[minor(dev)&0177];
		switch(code){
		case NRWC:
			*t |= T_DRWC;
			break;
		case RWC:
			*t &= ~T_DRWC;
			break;
		case EOT:
			if(flag)
				tcommand(dev, WEOF);
			else
				u.u_error = EACCES;
			break;
		case BOT:
			tcommand(dev, REW);
			break;
		case FSR:
			tcommand(dev, SFORW);
			break;
		case BSR:
			tcommand(dev, SREV);
			break;
		case OFLIN:
			tcommand(dev, OFLN);
			break;
		case DEVR:
			*t |= T_DEVR;
			break;
		case NDEVR:
			*t &= ~T_DEVR;
			break;
		default:
			u.u_error = ENXIO;
			return;
		}
	}
}
#endif
