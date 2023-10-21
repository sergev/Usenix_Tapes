#
/*
 * TM tape driver
 *  Rewritten by Robert L. Kirby,
 *  Computer Science Center, University of Maryland, College Park
 * New features:
 * 1. Four file types (cooked and raw) are available:
 *  A. Standard is after close to return to starting position.
 * This is incompatible with Harvard drives that rewind
 * but is compatible with the original driver.
 *  B. Advance to just after next file mark after closing position
 * (like Harvard no-rewind except raw seeks first).
 *  C. Rewind the file when closed (like Harvard standard).
 *  D. Backup to just after previous file mark on open
 * and return to this position when closed.
 * 2. Opens, closes, and raw file seeks may be made asychrononously.
 * 3. EOT does not permit further forward motions.
 * 4. Raw read and write request may specify an odd byte count.
 * The buffer must still begin on a word boundary.
 * 5. Odd byte length records have their true length returned
 * rather than being rounded down.
 * 6. A different number of logical and physical units may be supported.
 *
 * Using modified ideas of
 *  Bruce S. Borden and S. Tucker Taft, Harvard Science Center
 * who followed a TU16 driver written at Piscataway by Larry Wehr:
 * 1. With the raw device, a write with zero bytes for a count
 * will cause a tape mark to be written.
 * 2. Seeks are allowed on the raw device.
 * For using seek-by-blocks, each block is treated
 * as though it were exactly 512 bytes (including tape marks).
 * 3. On a read or seek, if a tape mark is encountered, an error is
 * returned for the cooked device (errno = EFBIG), and
 * a zero-length record is returned for the raw device.
 * On the raw device, an error is only returned
 * if the seek or read tries to cross a double tape mark,
 * leaving the tape positioned after the first of the pair.
 *
 * Bug fix by Robert L. Kirby:
 * 1. tmadjust has been rewritten to properly calculate the file
 * offset when a long buffer reads a short record.
 * i.e. buffer - actual > 0101000
 *
 * If this driver may be used on an 11/70,
 * the symbol "CPU70" must be defined in param.h.
 */

#include "../param.h"
#include "../buf.h"
#include "../conf.h"
#include "../user.h"
#include "../file.h"
#include "../inode.h"
#include "../reg.h"

/*
 * The number of units NUNIT and the number of logical units NLUNIT
 * may differ on tape drives such as the DIGI-DATA where the
 * most significant bit of the unit number distinguishes
 * 800 (0) vs 1600 (4) FPI.
 * NUNIT and NLUNIT must be power of 2 (NUNIT-1 used as mask)
 */
#define NUNIT	1
#define NLUNIT	8
#define	TMADDR	0172520
#define ETAPEMARK EFBIG /* use file big error for tape mark error */
#define TMPRIO  5       /* interruptable tape operation priority */
#define RETRY	10
#define NSEEK	040	/* size of opening/closing seek increments */
#define DEVMSK	0140	/* dev open/close type mask for inodes */
#define FOWARD	0040	/* forward on close */
#define BACKUP	0100	/* backup on open */
#define REWIND	0140	/* rewind on close */

struct	devtab	tmtab;
/* d_active status types */
#define SSEEKF	1	/* seek forward before i/o */
#define SSEEKR	2	/* seek reverse before i/o */
#define SIO	3	/* i/o */
#define SEEKF	4	/* seek forward while opening/closing */
#define SEEKR	5	/* seek reverse while opening/closing */
#define SWEOF	6	/* writing eof before closing or reverse seek */
#define SGAP	7	/* waiting for gap shutdown or rewind */

int	tm_sem	4;	/* tape buffer semaphore */
int	tmodd;		/* odd count flag for raw */
char	tm_done;	/* i/o done flag */

struct  buf     rtmbuf; /* raw i/o buffer */

struct tminfo {
	char	t_flags;
	char	t_tmytw;	/* tape marks yet to write */
	char	*t_blkno;	/* block num about to read/write */
	char	*t_tmback;	/* tape marks yet to back up over */
	char	*t_bkftm;	/* blkno after last tape mark */
	char	*t_last;	/* first unusable block */
	struct	file *t_fp;	/* save the file pointer until closing */
	struct  buf ctmbuf;     /* buffer for open and close */
} tminfo[NUNIT];
/* t_flags open and close status (also includes FWRITE 0002) */
#define TBOT	0001	/* file was at BOT after open */
#define TOPEN	0004	/* device exclusively assigned */
#define TADV	0010	/* advance over tape mark during open/close */
#define TREW	0020	/* rewind on close */
#define TRAW	0100	/* indicate raw device during forward closing */
#define TERROR	0200	/* fatal error */

struct {		/* device register configuration */
	int tmer;
	int tmcs;
	int tmbc;
	int tmba;
	int tmdb;
	int tmrd;
};
/* tmer status bits */
#define TUR     0000001
#define RWS	0000002 /* rewind in progress */
#define WRL	0000004 /* write lock */
#define SDWN	0000010 /* slowing down during gap */
#define BOT     0000040
#define SELR	0000100 /* select remote */
#define RLE	0001000 /* record length error */
#define EOT	0002000 /* end of tape */
#define EOF	0040000
#define HARD	0102200 /* ILC, EOT, NXM */

/* commands (and tmcs bits) */
#define	GO	01
#define	RCOM	02
#define	WCOM	04
#define	WEOF	06
#define	SFORW	010
#define	SREV	012
#define	WIRG	014
#define	REW	016
#define IENABLE 0000100
#define CRDY	0000200
#define PWRCLR	0010000
#define DENS	0060000	/* 7-channel dump-mode or 9-channel */
#define ERROR	0100000

struct { char *chp;};

tmopen(adev, flag)
{
	register struct tminfo *mp;
	register struct buf *bp;
	register int dev;

	dev = adev;
	mp = &tminfo[dev & (NUNIT - 1)];
	if((dev.d_minor & ~((NLUNIT - 1) | DEVMSK)) != 0
	    || (mp->t_flags & TOPEN) != 0) {
		u.u_error = ENXIO;
		return;
	}
	mp->t_flags =| (TOPEN | flag);
	bp.chp = u.u_ar0[R0];	/* use file pointer if not mounting */
	if(bp.chp >= NOFILE || (mp->t_fp = u.u_ofile[bp.integ]) == NULL
		|| ((bp = mp->t_fp->f_inode)->i_mode & IFCHR & IFBLK) == 0
		|| bp->i_addr[0] != dev)
			mp->t_fp = NULL;
	bp = &mp->ctmbuf;
	if((dev & DEVMSK) == BACKUP) { /* backup before opening */
		spl6(); /* lock out timeout interrupt from tmgap */
		mp->t_tmback++;
	}
	if((bp->b_flags & B_BUSY) == 0) {
		spl0(); /* lower priority when rewinding can't change flags */
		bp->b_dev = dev;
		bp->b_flags = B_BUSY | B_READ;
		bp->b_error = 0;
		mp->t_last = mp->t_bkftm = -1;
		mp->t_flags =& ~(TERROR | TBOT | TADV | TREW | TRAW);
		tmstrategy(bp);
	}
	spl5();
	while((bp->b_flags & B_DONE) == 0) {
		bp->b_flags =| B_WANTED;
		sleep(bp, (mp->t_fp == NULL ? PRIBIO : TMPRIO));
	}
	spl0();
	if(mp->t_flags & TERROR) {
		u.u_error = EIO;
		mp->t_flags = 0;
		mp->t_tmback = 0;
		return;
	}
	mp->t_tmytw = (flag ? 3 : 0);
/*
 * By waiting until motion stops and no error exists
 * TREW and TADV are never both on.
 * Interrupting open suppresses closing actions.
 */
	switch(dev & DEVMSK) {
	case REWIND:	/* rewind on close */
		mp->t_flags =| TREW;
		return;
	case FOWARD:	/* forward on close */
		mp->t_flags =| TADV;
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
	spl5();
	/* de-associate all blocks */
	for (bp = tmtab.b_forw; bp != &tmtab; bp = bp->b_forw)
		if (bp->b_dev == rdev) bp->b_dev.d_minor =| NLUNIT;
	if(mp->t_flags & TERROR)
		mp->t_flags =& ~TADV;
	mp->t_flags =& ~(TOPEN | TERROR | FWRITE);
	spl0();
	bp = &mp->ctmbuf;
	if((bp->b_flags & B_BUSY) == 0) {
		if(mp->t_fp != NULL) {
			/* seek forward can suppress writing tape marks */
			bp->b_blkno = lshift(&mp->t_fp->f_offset[0], -9);
			if((mp->t_flags & TADV) != 0
			   && (mp->t_fp->f_inode->i_mode & IFMT) == IFCHR)
				mp->t_flags =| TRAW;
		}
		bp->b_flags = B_BUSY | B_READ;
		tmstrategy(bp);
	}
}

tmstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;
	register struct tminfo *mp;

	bp = abp;
	mp = &tminfo[bp->b_dev & (NUNIT-1)];

	if (mp->t_flags & TERROR) { /* fatal error */
		bp->b_flags =| B_ERROR;
		if(bp->b_wcount == 0)
			bp->b_flags =& ~B_BUSY;
		iodone(bp);
		return;
	}
#ifdef CPU70
	if (bp->b_flags&B_PHYS)
		mapalloc(bp);
#endif CPU70
	/* Use negative frame count instead of negative word count */
	bp->b_wcount =<< 1;
	if(bp == &rtmbuf) bp->b_wcount =+ tmodd;

	bp->av_forw = 0;
	spl5();
	if (tmtab.d_actf==0)
		tmtab.d_actf = bp;
	else
		tmtab.d_actl->av_forw = bp;
	tmtab.d_actl = bp;
	if(tmtab.d_active == 0)
		tmstart();
	spl0();
	if(bp->b_wcount != 0)
		sema_p(&tm_sem, PRIBIO);
	return;
}

tmstart()
{
	register struct buf *bp;
	register struct tminfo *mp;
	register com;

    loop:
	if((bp = tmtab.d_actf) == 0) {
		tmtab.d_active = 0;
		return;
	}
	if(tm_done) {
		tmtab.d_actf = bp->av_forw;
		tmtab.d_errcnt = 0;
		tm_done = 0;
		if(bp->b_wcount == 0) {
			if(bp->b_flags & B_WANTED) wakeup(bp);
			bp->b_flags = B_DONE; /* turns off B_ERROR & B_BUSY */
		} else {
			iodone(bp);
			sema_v(&tm_sem); /* release semaphore */
		}
		goto loop;
	}
	com = bp->b_dev & (NLUNIT - 1);
	mp = &tminfo[com & (NUNIT - 1)];
	if((TMADDR->tmcs & CRDY) == 0) {
		deverror(bp, TMADDR->tmer, TMADDR->tmcs);
		mp->t_flags =| TERROR;
		bp->b_flags =| B_ERROR;
		tm_done++;
		goto loop;
	}
	com =<< 8;
	com =| ((bp->b_xmem & 03) << 4) | DENS;
	TMADDR->tmcs = com;
	if(&mp->ctmbuf == bp) { /* closing actions */
		if(mp->t_blkno < bp->b_blkno) {
			if(mp->t_blkno == 0)
				mp->t_tmytw = 0;
			else if(mp->t_tmytw != 0) {
				bp->b_blkno = mp->t_blkno;
				bp->b_flags =| B_ERROR;
			}
		}
		if(mp->t_tmytw != 0) {
			if(tmgap(0)) return;
			tmtab.d_active = SWEOF;
			mp->t_tmytw--;
			mp->t_bkftm = ++(mp->t_blkno);
			TMADDR->tmcs = com | IENABLE | WEOF | GO;
			return;
		}
		if((mp->t_flags & TADV) == 0) {
			bp->b_blkno = 0;
			if((mp->t_flags & TREW) != 0) {
				mp->t_blkno = 0;
				mp->t_last = mp->t_bkftm = -1;
				mp->t_flags =&
				    ~(TBOT | TREW | TERROR | TADV);
				if(mp->t_tmback != 0) {
					mp->t_flags =| TERROR;
					mp->t_tmback = 0;
				}
				TMADDR->tmcs = com | REW | GO;
				tm_done++;
				goto loop;
			}
		}
	}
	if(mp->t_flags & TERROR) {
		bp->b_flags =| B_ERROR;
		tm_done++;
		goto loop;
	}
	if(mp->t_last <= bp->b_blkno) { /* end of reel protection */
		bp->b_flags =| B_ERROR;
		bp->b_error = ETAPEMARK;
		bp->b_resid = bp->b_wcount;
		if(bp != &mp->ctmbuf) {
			tm_done++;
			goto loop;
		} else if((bp->b_blkno = mp->t_last) != 0)
			bp->b_blkno--;
	}
	if (mp->t_blkno > bp->b_blkno) { /* backward positioning */
		if((mp->t_tmytw != 0) && (bp->b_flags & B_READ) != 0) {
			tmtab.d_active = SWEOF;
			mp->t_tmytw--;
			mp->t_bkftm = ++(mp->t_blkno);
			com =| IENABLE|WEOF|GO;
		} else {
			if(tmgap(bp)) return;
			tmtab.d_active = SSEEKR;
			mp->t_last = -1;
			TMADDR->tmbc = bp->b_blkno - mp->t_blkno;
			/* extra backspace if error retry */
			if(tmtab.d_errcnt && (tmtab.d_errcnt & 03) == 0 &&
			    bp->b_blkno != 0)
				TMADDR->tmbc--;
			if((mp->t_blkno =+ TMADDR->tmbc) != 0
				|| (mp->t_flags & TBOT) == 0)
				com =| SREV | IENABLE | GO;
			else { /* rewind to backup */
				TMADDR->tmcs = com | REW | GO;
				goto loop;
			}
		}
	} else if(mp->t_blkno < bp->b_blkno) { /* forward positioning */
		if(mp->t_tmytw == 0 || mp->t_blkno == 0) {
			if(tmgap(0)) return;
			mp->t_tmytw = 0;
			tmtab.d_active = SSEEKF;
			TMADDR->tmbc = mp->t_blkno - bp->b_blkno;
			mp->t_blkno =- TMADDR->tmbc;
			com =| SFORW|IENABLE|GO;
		} else {
			bp->b_error = ETAPEMARK;
			bp->b_flags =| B_ERROR;
			tm_done++;
			goto loop;
		}
	} else {
		if((bp->b_flags & B_READ) == 0) {
			mp->t_tmytw = 3;
			if (bp->b_wcount == 0) {
				com =| IENABLE|WEOF|GO; /* write eof */
				mp->t_bkftm = mp->t_blkno + 1;
			} else if (tmtab.d_errcnt)
				/* write with 3 inch gap */
				com =| IENABLE|WIRG|GO;
			else
				com =| IENABLE|WCOM|GO; /* write */
		} else if(bp->b_wcount.chp > 1
		       && (bp->b_flags & B_ERROR) == 0) {
			if(mp->t_tmytw != 0) {
				if(bp == &rtmbuf) {
					if(mp->t_blkno == 0)
						mp->t_tmytw = 0;
					else {
					      bp->b_flags =| B_ERROR;
					      tm_done++;
					      goto loop;
					}
				} else if(mp->t_blkno == 0 && mp->t_fp != NULL
				     && (mp->t_fp->f_flag & FREAD))
					mp->t_tmytw = 0;
				else {
					clrbuf(bp);
					tm_done++;
					goto loop;
				}
			}
			com =| IENABLE|RCOM|GO; /* read */
		} else if(bp == &mp->ctmbuf) {
			if(mp->t_bkftm == bp->b_blkno)
				mp->t_flags =& ~TADV;
			mp->t_flags =& ~(TBOT | TREW | TRAW);
			bp->b_blkno = mp->t_blkno = 0;
			mp->t_bkftm = -1;
			if(mp->t_tmback != 0) {
				if(tmgap(bp)) return;
				tmtab.d_active = SEEKR;
				mp->t_last = -1;
				if((mp->t_flags & TADV) == 0)
					mp->t_flags =| TADV;
				else
					mp->t_tmback--;
				com =| IENABLE | SREV | GO;
			} else if(mp->t_last != -1) {
				mp->t_tmback++;
				bp->b_flags =| B_ERROR;
				goto loop;
			} else if((mp->t_flags & TADV) != 0) {
				if(tmgap(0)) return;
				tmtab.d_active = SEEKF;
				mp->t_flags =& ~TADV;
				com =| IENABLE | SFORW | GO;
			} else {
				if((TMADDR->tmer & SELR) == 0
				   || (bp->b_flags & B_ERROR)
				   || ((TMADDR->tmer & WRL)
				       && (mp->t_flags & FWRITE)))
					mp->t_flags =| TERROR;
				else if(tmgap(0))
					return;
				else if((TMADDR->tmer & BOT) != 0)
					mp->t_flags =| TBOT;
				tm_done++;
				goto loop;
			}
		/*
		 * restart after NSEEK records if no BOT
		 * or EOF found to prevent excessive
		 * waiting for byte counter at BOT
		 * and to allow aborted forward closing
		 */
			TMADDR->tmbc = - NSEEK;
			TMADDR->tmcs = com;
			return;
		/* wait for gap shutdown after backward reposition */
		} else if(tmgap(0))
			return;
		else {  /* positioning only */
			tm_done++;
			goto loop;
		}
		if(tmgap(0)) return;
		tmtab.d_active = SIO;
		TMADDR->tmbc = bp->b_wcount;
		TMADDR->tmba = bp->b_addr;
		mp->t_blkno++;
	}
	TMADDR->tmcs = com;
}

/*
 * Wait for rewind to complete or for tape settledown within
 * interrecord gap when tape direction will be switched.
 * Backf is zero iff the next action goes forward.
 * If delay is needed returns non-zero and restarts tmstart after delay.
 */
tmgap(backf)
{
	if((TMADDR->tmer & (SDWN | RWS)) == 0) return(0);
	if((TMADDR->tmer & RWS) == 0 && tmtab.d_active != SGAP) {
		if(tmtab.d_active == SSEEKR || tmtab.d_active == SEEKR) {
			if(backf) return(0);
		} else if(!backf) return(0);
		timeout(tmstart, 0, 7);
	} else
		timeout(tmstart, 0, 50);
	tmtab.d_active = SGAP;
	return(1);
}

tmintr()
{
	register struct buf *bp;
	register struct tminfo *mp;
	register rest;

	if((bp = tmtab.d_actf) == 0)
		return;
	rest = TMADDR->tmbc;
	if(tmtab.d_active == SIO)
		bp->b_resid = rest;
	mp = &tminfo[bp->b_dev & (NUNIT-1)];
	if ((TMADDR->tmcs < 0) || (TMADDR->tmer & BOT)) {
		switch(tmtab.d_active) {
		case SSEEKR:
			mp->t_blkno =- rest;
			break;
		case SSEEKF:
			mp->t_blkno =+ rest;
			break;
		}
		if(TMADDR->tmer & EOT) {
			switch(tmtab.d_active) {
			case SEEKF:
				mp->t_tmback++;
				if((TMADDR->tmer & EOF) == 0)
					mp->t_flags =| TADV;
				break;
			case SIO:
				if(rest == bp->b_wcount && rest != 0
				     && bp->b_blkno != 0)
					mp->t_blkno--;
			}
			if((mp->t_last = mp->t_blkno) != 0)
				mp->t_last--;
			prdev("eot", bp->b_dev);
			bp->b_flags =| B_ERROR;
			return(tmstart());
		} else if(TMADDR->tmer & EOF) {
			switch(tmtab.d_active) {
			case SSEEKR:
			case SEEKR:
			case SEEKF:
				return(tmstart());
			case SIO:
				bp->b_resid = bp->b_wcount;
			}
			if(mp->t_bkftm + 1 == mp->t_blkno
			   || (bp != &rtmbuf && (mp->t_flags & TRAW) == 0)) {
				if(bp != &rtmbuf && tmtab.d_active == SIO
				   && (mp->t_flags & FWRITE) != 0) {
					/* read before write extends */
					clrbuf(bp);
					bp->b_wcount = 1; /* only position */
				} else {
			/* cooked eof, or two in a row for raw */
			/* must set error or cooked read will not stop */
					bp->b_error = ETAPEMARK;
					bp->b_flags =| B_ERROR | B_READ;
				}
				/* backspace over eof */
				bp->b_blkno = mp->t_blkno - 1;
				return(tmstart());
			}
			mp->t_bkftm = mp->t_blkno;
		} else if((TMADDR->tmer & BOT) != 0 && bp->b_blkno == 0) {
			if(tmtab.d_active == SEEKR) {
				if((mp->t_flags & TADV) == 0
				  || mp->t_tmback != 0) {
					bp->b_flags =| B_ERROR;
					mp->t_tmback = 0;
				}
				mp->t_flags =& ~TADV;
				return(tmstart());
			}
		} else {
			if(TMADDR->tmer & HARD) {
				mp->t_flags =| TERROR;
				printf("bc=%o ba=%o ",
					TMADDR->tmbc, TMADDR->tmba);
			} else if(((TMADDR->tmer & RLE) == 0)
			    && (++tmtab.d_errcnt < RETRY))
				return(tmstart());
			deverror(bp, TMADDR->tmer, TMADDR->tmcs);
			bp->b_flags =| B_ERROR;
			tm_done++;
			return(tmstart());
		}
	}
	switch(tmtab.d_active) {
	case SIO:
		tm_done++;
		break;
	case SEEKR:
		mp->t_tmback++;
		break;
	case SEEKF:
		mp->t_flags =| TADV;
		break;
	}
	return(tmstart());
}

tmread(dev)
{
	register rdev;

	rdev = dev;
	tmseek(rdev, B_READ);
	tmodd = u.u_count & 1;
	u.u_count =+ tmodd;
	physio(tmstrategy, &rtmbuf, rdev, B_READ);
	tmadjust(rdev);
}

tmwrite(dev)
{
	register rdev;

	rdev = dev;
	tmseek(rdev, B_READ);
	tmodd = u.u_count & 1;
	u.u_count =+ tmodd;
	if(u.u_count == 0) { /* write tape mark */
		tmseek(rdev, B_WRITE);
	} else {
		physio(tmstrategy, &rtmbuf, rdev, B_WRITE);
	}
	tmadjust(rdev);
}

/* fudge file offset to make all blocks appear to be 512 bytes */
tmadjust(dev)
{
	register cnt, *p;

	u.u_count = -rtmbuf.b_resid;
	p = tminfo[dev & (NUNIT - 1)].t_fp->f_offset;
	if(cnt = u.u_count - u.u_arg[1]) {
		dpadd(p, cnt);
		p[0]--;
	}
	dpadd(p, 512);
}

tmseek(dev, rw)
int dev;
int rw;
{
	register struct buf *bp;

	bp = &rtmbuf;
	spl5();
	while(bp->b_flags & B_BUSY) {
		bp->b_flags =| B_WANTED;
		sleep(bp, TMPRIO);
	}
	spl0();
	tmodd = 0;
	bp->b_flags = B_BUSY | rw;
	bp->b_dev = dev;
	bp->b_blkno = lshift(u.u_offset, -9);
	bp->b_wcount = 0;
	bp->b_error = 0;
	tmstrategy(bp);
	spl5();
	while ((bp->b_flags & B_DONE) == 0) {
		bp->b_flags =| B_WANTED;
		sleep(bp, TMPRIO);
	}
	spl0();
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
