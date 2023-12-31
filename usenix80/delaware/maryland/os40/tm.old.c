#
/*
 * TM tape driver
 *   Rewrite by Bruce S. Borden and S. Tucker Taft, Harvard Science Center
 *
 * This TM-11 driver is modeled after the TU-16
 *  driver written at Piscataway.  It has the following features:
 *
 *	- No rewind is done on close
 * for devices with the NOREW bit on in their minor device number.
 * On a close after a write, 2 tape marks are written, and then
 * a single backspace is done.  On a close after a read, a forward
 * seek is done to the next tape mark, leaving the tape positioned after
 * the tape mark.
 *	- With the raw device, it is possible to write a tape mark.
 * A write with zero bytes for a count will cause a tape mark to
 * be written.  Please note that the buffer address must still
 * be a legal address because "physio" checks it.  Writes of 2 bytes
 * on the raw device are automatically filled out to 4 bytes.
 *	- On a read, if a tape mark is encountered, an error is
 * returned for the cooked device (errno = EFBIG), and
 * a zero-length record is returned for the raw device.
 *	- Seeks are allowed on the raw device.  Each block is treated
 * as though it were exactly 512 bytes (including tape marks), so that
 * a seek-by-blocks system call will do the desired operation.
 * During a seek on the raw device, tape marks are counted as one block.
 * On the cooked device, an error will be returned if the seek tries
 * to cross a tape mark(EFBIG).  On the raw device,
 * an error is only returned if the seek tries to cross a double tape mark,
 * leaving the tape positioned after the first of the pair.
 *	- Please note:  If this driver may be used on an 11/70,
 * the symbol "CPU70" must be defined in param.h.
 *	-New feature as of 9/16/77: stty call for file positioning.
 * first word specifies operation, and second word specifies file count.
 * first word:	meaning of 2nd word:
 *	0	position relative to beginning of tape,
 *		skipping this many files.
 *	1	position to beginning of current file,
 *		then skip forward (positive count) or backward
 *		(negative count) specified number of files.
 *	2	position to end of tape (double tape mark) and
 *		then skip backward(count must be negative)
 *		specified number of files.
 *
 * e.g.	0,0 rewind
 *	1,0 position to beginning of this file
 *	2,0 position to end of tape, ready to add a new file
 *	1,-1 backspace file
 *	0,3 position to 4th file (or 3rd if you count starting with 0)
 *
 * On January 24, 1979 additional features added by
 * Robert L. Kirby, Computer Science Center, University of Maryland.
 *
 * 1.	Raw read and write request may specify an odd byte count.
 *	The buffer must still begin on a word boundary.
 * 2.	Odd byte length records have their true length returned
 *	rather than being rounded down.
 *	Internally, 0100000 is added to the negative word counts in
 *	u.u_wcount and bp->b_resid to indicate an odd byte count.
 * 3.	A different number of logical and physical units may be supported.
 * 4.	tmadjust has been rewritten to properly calculate the file
 *	offset when a long buffer reads a short record.
 *	i.e. buffer - actual > 0101000
 * 5.	The tmgstty routines are now optionally included
 *	by defining TMSTTY.
 */

#include "../h/param.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/user.h"
#include "../h/file.h"
#include "../h/reg.h"

struct {
	int tmer;
	int tmcs;
	int tmbc;
	int tmba;
	int tmdb;
	int tmrd;
};

struct	devtab	tmtab;
struct	buf	rtmbuf,	ctmbuf;

/*
 * The number of units NUNIT and the number of logical units NLUNIT
 * may differ on tape drives such as the DIGI-DATA where the
 * most significant bit of the unit number distinguishes
 * 800 (0) vs 1600 (4) FPI.
 * NUNIT and NLUNIT must be power of 2 (NUNIT-1 used as mask)
 */
#define NUNIT	1
#define NLUNIT	8
#define NOREW	040	/* no rewind on close bit (>= NLUNIT) */

struct tminfo {
	char	*t_blkno;	/* block num about to read/write */
	char	*t_rawtm;	/* blkno after last tape mark */
	char	*t_tmblk;	/* block number after last write */
	char	t_openf;	/* 0 = not open, 1 = open, -1 = fatal error */
	char	t_tmwritten;	/* 1 = two eof's already written */
} tminfo[NUNIT];

int	tm_sem	4;	/* tape buffer semaphore */
char	tm_done;	/* i/o done flag */

#define	TMADDR	0172520

#define RETRY	10
#define ETAPEMARK	EFBIG	/* use file big error for tape mark error */

/* commands (and tmcs bits) */
#define	NOCMD	00
#define	GO	01
#define	RCOM	02
#define	WCOM	04
#define	WEOF	06
#define	SFORW	010
#define	SREV	012
#define	WIRG	014
#define	REW	016
#define PWRCLR	010000
#define DENS	060000	/* 9-channel or 7-channel dump-mode */
#define	IENABLE	0100
#define	CRDY	0200
#define ERROR	0100000

/* status bits */
#define GAPSD	010000
#define	TUR	1
#define	HARD	0102200	/* ILC, EOT, NXM */
#define	EOF	0040000
#define	SELR	0100	/* select remote */
#define BOT	040
#define WRLOCK	04
#define	ILLCMD	0100000	/* illegal command error */

#define	SSEEKF	1
#define	SSEEKR	2
#define	SIO	3
#define SCOM	4


tmopen(dev, flag)
{
	register ds;
	register struct tminfo *mp;

	mp = &tminfo[dev & (NUNIT-1)];
	if (dev.d_minor & ~((NLUNIT-1)|NOREW) || mp->t_openf) {
		u.u_error = ENXIO;
		return;
	}
	mp->t_openf++;
	mp->t_blkno = 0;
	/* -1 is later interpreted as 65535 */
	mp->t_tmblk = mp->t_rawtm = -1;
	mp->t_tmwritten = 0;
	ds = tcommand(dev, NOCMD);	/* clear controller */
	if ((ds&SELR) == 0 || flag && (ds&WRLOCK)) {
		mp->t_openf = 0;
		prdev("Offline or needs ring", dev);
		u.u_error = ENXIO;
		return;
	}
}

tmclose(dev, flag)
{
	register int rdev;
	register struct buf *bp;
	register struct tminfo *mp;

	rdev = dev;
	mp = &tminfo[rdev & (NUNIT-1)];
	/* because t_rawtm and t_blkno are updated asyncronously */
	/*  must wait for all i/o to complete */
	tcommand(rdev, NOCMD);

	if(flag && !mp->t_tmwritten) {
		if (mp->t_rawtm != mp->t_blkno) tcommand(rdev, WEOF);
		tcommand(rdev, WEOF);
		tcommand(rdev, rdev&NOREW? SREV: REW);
	} else {
		tcommand(rdev, !(rdev&NOREW)? REW:
		    mp->t_rawtm != mp->t_blkno? SFORW: NOCMD);
	}

	mp->t_openf = 0;
	/* de-associate all blocks */
	for (bp = tmtab.b_forw; bp != &tmtab; bp = bp->b_forw)
		if (bp->b_dev == rdev) bp->b_dev.d_minor = -1;
}

tcommand(dev, com)
{
	register struct buf *bp;

	bp = &ctmbuf;
	spl5();
	while(bp->b_flags & B_BUSY) {
		bp->b_flags =| B_WANTED;
		sleep(bp, PRIBIO);
	}
	spl0();
	bp->b_dev = dev;
	bp->b_resid = com;
	bp->b_blkno = 0;
	bp->b_flags = B_BUSY | B_READ;
	tmstrategy(bp);
	iowait(bp);
	if(bp->b_flags & B_WANTED)
		wakeup(bp);
	bp->b_flags = 0;
	return(bp->b_resid);
}

tmstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;
	register struct tminfo *mp;
	register unit;

	bp = abp;
	unit = bp->b_dev&(NLUNIT-1);
	mp = &tminfo[unit & (NUNIT-1)];

	if (bp != &ctmbuf) { /* special commands skip positioning */
		if (mp->t_openf < 0) {
			/* fatal error */
			bp->b_flags =| B_ERROR;
			iodone(bp);
			return;
		}

		if (mp->t_tmblk < bp->b_blkno) {
			bp->b_error = ETAPEMARK;
			bp->b_flags =| B_ERROR;
			iodone(bp);
			return;
		}
		if (bp->b_flags&B_READ) {

			/* read */

			if (mp->t_tmblk == bp->b_blkno) {
				/* this may be read-before-write */
				if (bp == &rtmbuf)
					bp->b_resid = bp->b_wcount;
				else clrbuf(bp);
				iodone(bp);
				return;
			}
	/* check if he is about to backspace and read after write
	/* and write out two eof's now for him */
			tmchktm(unit);
		} else {

			/* write */

			mp->t_tmblk = bp->b_blkno + 1;
			mp->t_tmwritten = 0;
		}
#ifdef CPU70
		if (bp->b_flags&B_PHYS)
			mapalloc(bp);
#endif CPU70
	}

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
	sema_p(&tm_sem, PRIBIO);
	return;
}

tmstart()
{
	register struct buf *bp;
	register struct tminfo *mp;
	register com;

	tmtab.d_active = 0;
    loop:
	if ((bp = tmtab.d_actf) == 0)
		return;
	if (tm_done) {
		/* i/o done, chain to next */
		tmtab.d_actf = bp->av_forw;
		tmtab.d_errcnt = 0;
		tm_done = 0;
		iodone(bp);
		sema_v(&tm_sem);	/* release semaphore */
		goto loop;
	}
	com = bp->b_dev & (NLUNIT-1);
	mp = &tminfo[com & (NUNIT-1)];
	com = (com<<8) | ((bp->b_xmem & 03) << 4) | DENS;
	if(bp == &ctmbuf) {
		if(bp->b_resid == NOCMD) {
			/* don't want IENABLE bit or else */
			/* interrupt will occur immediately */
			TMADDR->tmcs = com;
			bp->b_resid = TMADDR->tmer;
			tm_done++;
			goto loop;
		}
		tmtab.d_active = SCOM;
		TMADDR->tmbc = 1;	/* i.e. infinity, or until eof */
		TMADDR->tmcs = bp->b_resid | com | IENABLE | GO;
		return;
	}
	if (mp->t_openf < 0 || (TMADDR->tmcs & CRDY)==0) {
		bp->b_flags =| B_ERROR;
		tm_done++;
		goto loop;
	}
	if (mp->t_blkno != bp->b_blkno) {
		if (mp->t_blkno < bp->b_blkno) {
			tmtab.d_active = SSEEKF;
			TMADDR->tmbc = mp->t_blkno - bp->b_blkno;
			mp->t_blkno =- TMADDR->tmbc;
			com =| SFORW|IENABLE|GO;
		} else {
			tmtab.d_active = SSEEKR;
			/* cannot do rewind even if b_blkno == 0 */
			/* because may not be in first file of tape */
			TMADDR->tmbc = bp->b_blkno - mp->t_blkno;
			/* extra backspace if error retry */
			if (tmtab.d_errcnt && (tmtab.d_errcnt&07) == 0 &&
			    bp->b_blkno != 0)
				TMADDR->tmbc--;
			mp->t_blkno =+ TMADDR->tmbc;
			com =| SREV|IENABLE|GO;
		}
		TMADDR->tmcs = com;
		return;
	}
	tmtab.d_active = SIO;
	/* wcount is a circularily shifted (possibly odd) byte count */
	TMADDR->tmbc = (bp->b_wcount << 1) + (bp->b_wcount.integ > 0);
	TMADDR->tmba = bp->b_addr;		/* core address */
	mp->t_blkno++;
	if (bp->b_flags&B_READ) {
		com =| IENABLE|RCOM|GO;	/* read */
	} else if (bp->b_wcount == -1) {
		com =| IENABLE|WEOF|GO;	/* write eof */
		mp->t_rawtm = mp->t_blkno;
	} else if (tmtab.d_errcnt) {
		com =| IENABLE|WIRG|GO;	/* write with 3 inch gap */
	} else {
		com =| IENABLE|WCOM|GO;	/* write */
		mp->t_rawtm = -1;
	}
	TMADDR->tmcs = com;
}

tmintr()
{
	register struct buf *bp;
	register unit;
	register struct tminfo *mp;

	if ((bp = tmtab.d_actf)==0)
		return;
	unit = bp->b_dev & (NLUNIT-1);
	mp = &tminfo[unit & (NUNIT-1)];
	/* ignore errors on SCOM */
	if (tmtab.d_active == SCOM) {
		bp->b_resid = TMADDR->tmbc -1;	/* return skip count */
		tm_done++;
		return(tmstart());
	}
	bp->b_resid = TMADDR->tmbc;
	/* circularily shift remaining count to get word count (almost) */
	bp->b_resid = ((bp->b_resid + (bp->b_resid.integ > 0)) << 15)
			+ (bp->b_resid >> 1);
	if (TMADDR->tmcs < 0 || TMADDR->tmer&EOF) {

	    /* if eof or error during seek, adjust t_blkno */
	    if (tmtab.d_active == SSEEKR)
		mp->t_blkno =- TMADDR->tmbc;
	    else if (tmtab.d_active == SSEEKF)
	    	mp->t_blkno =+ TMADDR->tmbc;

	    if (!(TMADDR->tmer&EOF)) {

		/* error */

		while(TMADDR->tmrd & GAPSD) ; /* wait for gap shutdown */
		if (TMADDR->tmer&HARD) {
			mp->t_openf = -1;
			prdev("Hard error", bp->b_dev);
		} else if (++tmtab.d_errcnt < RETRY) {
			return(tmstart());
		}

		deverror(bp, TMADDR->tmer, TMADDR->tmcs);
		bp->b_flags =| B_ERROR;
		tm_done++;
		return(tmstart());
	    } else {

		/* eof */

		if (tmtab.d_active == SSEEKR) return(tmstart());
		if (bp != &rtmbuf || mp->t_rawtm+1 == mp->t_blkno) {
			/* cooked eof, or two in a row for raw */
			/* must set error or cooked read will not stop */
			bp->b_error = ETAPEMARK;
			bp->b_flags =| B_ERROR;
			/* backspace over eof */
			tmtab.d_active = SCOM;
			TMADDR->tmbc = -1;
			TMADDR->tmcs = (unit<<8)|SREV|DENS|IENABLE|GO;
			mp->t_blkno--;
			return;
		}
		/* single eof on raw returns as zero length record */
		bp->b_resid = bp->b_wcount;
		mp->t_rawtm = mp->t_blkno;
	    }
	}
	if (tmtab.d_active == SIO) tm_done++;
	return(tmstart());
}

/* Indirect strategy routine fakes out physio for odd byte counts */
tmoddstr(abp)
struct buf *abp;
{
	register struct buf *bp;

	bp = abp;
	/* Set negative shifted count back down to odd */
	bp->b_wcount =+ 0100000;
	return(tmstrategy(bp));
}

tmread(dev)
{
	if(u.u_count & 1) {	/* Read an odd byte count */
		u.u_count++;	/* Round up to next higher even byte count */
		physio(tmoddstr, &rtmbuf, dev, B_READ);
	} else
		physio(tmstrategy, &rtmbuf, dev, B_READ);
/*
 * This depends on no other process using the buffer until this point.
 * I.e. No other process (even though just awakened)
 * can preempt this one to start using this buffer for raw I/O.
 * The high order bit contains the old low order bit of the
 * residual byte transfer count.
 */
	if(rtmbuf.b_resid.integ > 0) u.u_count--;
	tmadjust();
}

tmwrite(dev)
{
	if(u.u_count & 1) {	/* to write an odd byte count */
		u.u_count++;	/* first pretend its the next higher even */
		physio(tmoddstr, &rtmbuf, dev, B_WRITE);
	} else {
	/* PHYSIO complains on count of zero, */
	/* so count of zero is set to 2, indicating write-tape-mark */
	/* count of 2 is set to 4 to avoid confusion */
	/* and ridiculously small blocks */
		if (u.u_count < 4) u.u_count =+ 2;
		physio(tmstrategy, &rtmbuf, dev, B_WRITE);
	}
	u.u_count = 0;  /* avoid errors on write-tape-mark */
	tmadjust();
}

tmadjust()
{
	/* fudge file offset to make all blocks appear to be 512 bytes */
	register struct file *fp;
	register cnt, *p;

	fp = getf(u.u_ar0[R0]);
	p = &fp->f_offset[0];
	if(cnt = u.u_count - u.u_arg[1]) {
		dpadd(p, cnt);
		p[0]--;
	}
	dpadd(p, 512);
}

#ifdef	TMSTTY
tmstty(dev, v)
int *v;
{
	register struct file *fp;
	register int fc;
	register struct tminfo *mp;
	int ateof;
	struct { long lng; };

	if (v) {
		/* gtty returns status of drive */
		*v++ = TMADDR->tmer;
		*v++ = TMADDR->tmcs;
		*v++ = TMADDR->tmbc;
		return;
	}

	/* check if tape marks should be written now */
	tmchktm(dev);

	/* re-init for new file */
	mp = &tminfo[dev & (NUNIT-1)];
	ateof = (mp->t_blkno == mp->t_rawtm);	/*save eof info */
	mp->t_blkno = 0;
	mp->t_rawtm = -1;
	mp->t_tmblk = -1;
	mp->t_openf = 1;	/* clear error condition */

	/* set file offset to 0 */
	fp = getf(u.u_ar0[R0]);
	fp->f_offset->lng = 0;

	fc = u.u_arg[1];
	switch(u.u_arg[0]) {

	case 0:		/* rel to beginning */
		tcommand(dev, REW);
		if (fc == 0) return;
		break;

	case 1:		/* rel to current file */
		if (TMADDR->tmer&BOT && fc == 0) return;
		break;

	case 2:		/* rel to end of tape */
		/* scan for double eof */
		while (tcommand(dev, SFORW) > 1 || !ateof) ateof = 1;
		tcommand(dev, SREV);
		if (fc == 0) return;
		break;
	default:
		u.u_error = EINVAL;
		return;
	}
	if (fc > 0) {
		do {
			if (tcommand(dev, SFORW) <= 1 && ateof) {
				/* reached double eof */
				tcommand(dev, SREV);
				goto outoftape;
			}
			ateof = 1;
		} while (--fc > 0);
	} else {
		do {
			if (TMADDR->tmer&BOT)
				goto outoftape;
			tcommand(dev, SREV);
		} while (++fc <= 0);
		if ((TMADDR->tmer&BOT) == 0)
			tcommand(dev, SFORW);
	}
	return;

outoftape:
	u.u_error = ETAPEMARK;
	return;
}
#endif TMSTTY

tmchktm(dev)
{
	register struct tminfo *mp;

	/* write two tape marks now if about to backspace after writing */
	mp = &tminfo[dev & (NUNIT-1)];

	if (mp->t_tmblk != -1 && !mp->t_tmwritten) {
		tcommand(dev, WEOF);
		tcommand(dev, WEOF);
		tcommand(dev, SREV);
		tcommand(dev, SREV);
		mp->t_tmwritten++;
	}
}

sema_p(sem, pri)
int *sem;
{
	register sps;

	sps = PS->integ;
	spl6();
	while (--(*sem) < 0) sleep(sem, pri);
	PS->integ = sps;
}

sema_v(sem)
int *sem;
{
	register sps;

	sps = PS->integ;
	spl6();
	if (++(*sem) <= 0) {
		wakeup(sem);
		*sem = 1;
	}
	PS->integ = sps;
}
