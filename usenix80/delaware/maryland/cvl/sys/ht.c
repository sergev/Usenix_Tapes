#
/*
 * HT tape driver
 *  Rewritten by Robert L. Kirby from his TM11 driver at
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
 * 1. htadjust has been rewritten to properly calculate the file
 * offset when a long buffer reads a short record.
 * i.e. buffer - actual > 0101000
 */

#include "/usr/sys/h/param.h"
#include "/usr/sys/h/buf.h"
#include "/usr/sys/h/conf.h"
#include "/usr/sys/h/user.h"
#include "/usr/sys/h/file.h"
#include "/usr/sys/h/inode.h"
#include "/usr/sys/h/reg.h"

/*
 * NUNIT must be power of 2 (NUNIT-1 used as mask)
 */
#define NUNIT	1
#define	HTADDR	0172440
#define ETAPEMARK EFBIG /* use file big error for tape mark error */
#define HTPRIO  5       /* interruptable tape operation priority */
#define RETRY	20
#define NSEEK	040	/* size of opening/closing seek increments */
#define DEVMSK	0140	/* dev open/close type mask for inodes */
#define FOWARD	0040	/* forward on close */
#define BACKUP	0100	/* backup on open */
#define REWIND	0140	/* rewind on close */
#define DENS	0200	/* Low density flag in inode */

struct	devtab	httab;
/* d_active status types */
#define SSEEKF	1	/* seek forward before i/o */
#define SSEEKR	2	/* seek reverse before i/o */
#define SIO	3	/* i/o */
#define SEEKF	4	/* seek forward while opening/closing */
#define SEEKR	5	/* seek reverse while opening/closing */
#define SWEOF	6	/* writing eof before closing or reverse seek */
#define SGAP	7	/* waiting for gap shutdown or rewind */

int	ht_sem	4;	/* tape buffer semaphore */
int	htodd;		/* odd count flag for raw */
char	ht_done;	/* i/o done flag */

struct  buf     rhtbuf; /* raw i/o buffer */

struct htinfo {
	char	t_flags;
	char	t_tmwrite;	/* tape marks yet to write */
	char	*t_blkno;	/* block num about to read/write */
	char	*t_tmback;	/* tape marks yet to back up over */
	char	*t_bkftm;	/* blkno after last tape mark */
	char	*t_last;	/* first unusable block */
	struct	file *t_fp;	/* save the file pointer until closing */
	struct  buf chtbuf;     /* buffer for open and close */
} htinfo[NUNIT];
/* t_flags open and close status (also includes FWRITE 0002) */
#define TBOT	0001	/* file was at BOT after open */
#define TOPEN	0004	/* device exclusively assigned */
#define TADV	0010	/* advance over tape mark during open/close */
#define TREW	0020	/* rewind on close */
#define TRAW	0100	/* indicate raw device during forward closing */
#define TERROR	0200	/* fatal error */

struct {		/* device register configuration */
	int	htcs1;
	int	htwc;
	int	htba;
	int	htfc;
	int	htcs2;
	int	htds;
	int	hter;
	int	htas;
	int	htck;
	int	htdb;
	int	htmr;
	int	htdt;
	int	htsn;
	int	httc;
	int	htbae;	/* 11/70 bus extension */
};
/* htcs command and status bits */
#define	GO	01
#define	REW	06
#define DCLR	010
#define ERASE	024
#define	WEOF	026
#define	SFORW	030
#define	SREV	032
#define	IENABLE	0100
#define	CRDY	0200
#define TRE	040000

/* htds status bits */
#define BOT	02
#define	EOF	04
#define SDWN	020
#define PES	040
#define EOT	02000
#define WRL	04000
#define	MOL	010000	/* medium on line */
#define PIP	020000	/* Positioning in progress */
/* hters status bits */
#define HARD	064073	/* hard drive errors */
#define FCE	001000	/* frame count error */
#define CS	002000	/* frame count error */
#define COR	0100000	/* frame count error */
/* other status bits */
#define CHARD	077400	/* htcs2 hard ctlr errors */
#define P800	01300	/* 800FPI density & pdp11 mode */
#define P1600	02300	/* 1600FPI density & pdp11 mode */

struct { char *chp;};

htopen(adev, flag)
{
	register struct htinfo *mp;
	register struct buf *bp;
	register int dev;

	dev = adev;
	mp = &htinfo[dev & (NUNIT - 1)];
	if((dev.d_minor & ~((NUNIT - 1) | DEVMSK | DENS)) != 0
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
	bp = &mp->chtbuf;
	if((dev & DEVMSK) == BACKUP) { /* backup before opening */
		spl6(); /* lock out timeout interrupt from htgap */
		mp->t_tmback++;
	}
	if((bp->b_flags & B_BUSY) == 0) {
		spl0(); /* lower priority when rewinding can't change flags */
		bp->b_dev = dev;
		bp->b_flags = B_BUSY | B_READ;
		bp->b_error = 0;
		mp->t_last = mp->t_bkftm = -1;
		mp->t_flags =& ~(TERROR | TBOT | TADV | TREW | TRAW);
		htstrategy(bp);
	}
	spl5();
	while((bp->b_flags & B_DONE) == 0) {
		bp->b_flags =| B_WANTED;
		sleep(bp, (mp->t_fp == NULL ? PRIBIO : HTPRIO));
	}
	spl0();
	if(mp->t_flags & TERROR) {
		u.u_error = EIO;
		mp->t_flags = 0;
		mp->t_tmback = 0;
		return;
	}
	mp->t_tmwrite = (flag ? 3 : 0);
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

htclose(dev, flag)
{
	register int rdev;
	register struct buf *bp;
	register struct htinfo *mp;

	rdev = dev;
	mp = &htinfo[rdev & (NUNIT-1)];
	spl5();
	/* de-associate all blocks */
	for (bp = httab.b_forw; bp != &httab; bp = bp->b_forw)
		if (bp->b_dev == rdev)
			bp->b_dev.d_major =| ~((DENS|DEVMSK) >> 8);
	if(mp->t_flags & TERROR)
		mp->t_flags =& ~TADV;
	mp->t_flags =& ~(TOPEN | TERROR | FWRITE);
	spl0();
	bp = &mp->chtbuf;
	if((bp->b_flags & B_BUSY) == 0) {
		if(mp->t_fp != NULL) {
			/* seek forward can suppress writing tape marks */
			bp->b_blkno = lshift(&mp->t_fp->f_offset[0], -9);
			if((mp->t_flags & TADV) != 0
			   && (mp->t_fp->f_inode->i_mode & IFMT) == IFCHR)
				mp->t_flags =| TRAW;
		}
		bp->b_flags = B_BUSY | B_READ;
		htstrategy(bp);
	}
}

htstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;
	register struct htinfo *mp;

	bp = abp;
	mp = &htinfo[bp->b_dev & (NUNIT-1)];

	if (mp->t_flags & TERROR) { /* fatal error */
		bp->b_flags =| B_ERROR;
		if(bp->b_wcount == 0)
			bp->b_flags =& ~B_BUSY;
		iodone(bp);
		return;
	}
	/* Use negative frame count instead of negative word count */
	bp->b_wcount =<< 1;
	if(bp == &rhtbuf) bp->b_wcount =+ htodd;

	bp->av_forw = 0;
	spl5();
	if (httab.d_actf==0)
		httab.d_actf = bp;
	else
		httab.d_actl->av_forw = bp;
	httab.d_actl = bp;
	if(httab.d_active == 0)
		htstart();
	spl0();
	if(bp->b_wcount != 0)
		sema_p(&ht_sem, PRIBIO);
	return;
}

htstart()
{
	register struct buf *bp;
	register struct htinfo *mp;
	register unit;

    loop:
	if((bp = httab.d_actf) == 0) {
		httab.d_active = 0;
		return;
	}
	if(ht_done) {
		httab.d_actf = bp->av_forw;
		httab.d_errcnt = 0;
		ht_done = 0;
		if(bp->b_wcount == 0) {
			if(bp->b_flags & B_WANTED) wakeup(bp);
			bp->b_flags = B_DONE; /* turns off B_ERROR & B_BUSY */
		} else {
			iodone(bp);
			sema_v(&ht_sem); /* release semaphore */
		}
		goto loop;
	}
	unit = bp->b_dev & (NUNIT - 1);
	mp = &htinfo[unit];
	HTADDR->htcs2 = unit>>3;
	HTADDR->httc = unit&07 | (bp->b_dev & DENS ? P800 : P1600);
	HTADDR->htcs1 = TRE | DCLR | GO;
	HTADDR->htfc = 0;
	if(&mp->chtbuf == bp) { /* closing actions */
		if(mp->t_blkno < bp->b_blkno) {
			if(mp->t_blkno == 0)
				mp->t_tmwrite = 0;
			else if(mp->t_tmwrite != 0) {
				bp->b_blkno = mp->t_blkno;
				bp->b_flags =| B_ERROR;
			}
		}
		if(mp->t_tmwrite != 0) {
			if(htgap(0)) return;
			httab.d_active = SWEOF;
			mp->t_tmwrite--;
			mp->t_bkftm = ++(mp->t_blkno);
			HTADDR->htcs1 = IENABLE | WEOF | GO;
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
				HTADDR->htcs1 = REW | GO;
				ht_done++;
				goto loop;
			}
		}
	}
	if(mp->t_flags & TERROR) {
		bp->b_flags =| B_ERROR;
		ht_done++;
		goto loop;
	}
	if(mp->t_last <= bp->b_blkno) { /* end of reel protection */
		bp->b_flags =| B_ERROR;
		bp->b_error = ETAPEMARK;
		bp->b_resid = bp->b_wcount;
		if(bp != &mp->chtbuf) {
			ht_done++;
			goto loop;
		} else if((bp->b_blkno = mp->t_last) != 0)
			bp->b_blkno--;
	}
	if (mp->t_blkno > bp->b_blkno) { /* backward positioning */
		if((mp->t_tmwrite != 0) && (bp->b_flags & B_READ) != 0) {
			httab.d_active = SWEOF;
			mp->t_tmwrite--;
			mp->t_bkftm = ++(mp->t_blkno);
			HTADDR->htcs1 = IENABLE | WEOF | GO;
			return;
		}
		if(htgap(bp)) return;
		httab.d_active = SSEEKR;
		mp->t_last = -1;
		HTADDR->htfc = bp->b_blkno - mp->t_blkno;
		/* extra backspace if error retry */
		if(httab.d_errcnt && (httab.d_errcnt & 07) == 0
		    && bp->b_blkno != 0)
			HTADDR->htfc--;
		if((mp->t_blkno =+ HTADDR->htfc) != 0
		     || (mp->t_flags & TBOT) == 0) {
			HTADDR->htcs1 = SREV | IENABLE | GO;
			return;
		} else { /* rewind to backup */
			HTADDR->htcs1 = REW | GO;
			goto loop;
		}
	}
	if(mp->t_blkno < bp->b_blkno) { /* forward positioning */
		if(mp->t_tmwrite == 0 || mp->t_blkno == 0) {
			if(htgap(0)) return;
			mp->t_tmwrite = 0;
			httab.d_active = SSEEKF;
			HTADDR->htfc = mp->t_blkno - bp->b_blkno;
			mp->t_blkno =- HTADDR->htfc;
			HTADDR->htcs1 = SFORW|IENABLE|GO;
			return;
		} else {
			bp->b_error = ETAPEMARK;
			bp->b_flags =| B_ERROR;
			ht_done++;
			goto loop;
		}
	}
	if((bp->b_flags & B_READ) == 0) {
		mp->t_tmwrite = 3;
		if(htgap(0)) return;
		if (bp->b_wcount == 0) { /* write eof */
			httab.d_active = SIO;
			mp->t_bkftm = ++mp->t_blkno;
			HTADDR->htcs1 = IENABLE | WEOF | GO;
			return;
		}
		if (httab.d_errcnt & 1) {
			/* erase before writing on odd retries */
			httab.d_errcnt++;
			httab.d_active = SSEEKF;
			HTADDR->htcs1 = IENABLE | ERASE | GO;
			return;
		}
	} else if(bp->b_wcount.chp > 1 && (bp->b_flags & B_ERROR) == 0) {
		if(mp->t_tmwrite != 0) {
			if(bp == &rhtbuf) {
				if(mp->t_blkno == 0)
					mp->t_tmwrite = 0;
				else {
				      bp->b_flags =| B_ERROR;
				      ht_done++;
				      goto loop;
				}
			} else if(mp->t_blkno == 0 && mp->t_fp != NULL
				&& (mp->t_fp->f_flag & FREAD))
				mp->t_tmwrite = 0;
			else {
				clrbuf(bp);
				ht_done++;
				goto loop;
			}
		}
	} else if(bp == &mp->chtbuf) {
		if(mp->t_bkftm == bp->b_blkno)
			mp->t_flags =& ~TADV;
		mp->t_flags =& ~(TBOT | TREW | TRAW);
		bp->b_blkno = mp->t_blkno = 0;
		mp->t_bkftm = -1;
	/*
	 * Restart after NSEEK records if no BOT or EOF found
	 * to prevent excessive waiting for byte counter at BOT
	 * and to allow aborted forward closing.
	 */
		HTADDR->htfc = - NSEEK;
		if(mp->t_tmback != 0) {
			if(htgap(bp)) return;
			httab.d_active = SEEKR;
			mp->t_last = -1;
			if((mp->t_flags & TADV) == 0)
				mp->t_flags =| TADV;
			else
				mp->t_tmback--;
			if(HTADDR->htds & BOT)
				htintr();
			else
				HTADDR->htcs1 = IENABLE | SREV | GO;
			return;
		}
		if(mp->t_last != -1) {	/* back up from end of reel */
			mp->t_tmback++;
			bp->b_flags =| B_ERROR;
			goto loop;
		}
		if((mp->t_flags & TADV) != 0) {
			if(htgap(0)) return;
			httab.d_active = SEEKF;
			mp->t_flags =& ~TADV;
			HTADDR->htcs1 = IENABLE | SFORW | GO;
			return;
		}
		if((HTADDR->htds & MOL) == 0 || (bp->b_flags & B_ERROR)
		   || ((HTADDR->htds & WRL) && (mp->t_flags & FWRITE)))
			mp->t_flags =| TERROR;
		else if(htgap(0))
			return;
		else if((HTADDR->htds & BOT) != 0)
			mp->t_flags =| TBOT;
		ht_done++;
		goto loop;
	/* wait for gap shutdown after backward reposition */
	} else if(htgap(0))
		return;
	else {  /* positioning only */
		ht_done++;
		goto loop;
	}
	if(htgap(0)) return;
	httab.d_active = SIO;
	mp->t_blkno++;
	unit = bp->b_wcount; /* rhstart expects a word count */
	bp->b_wcount =>> 1;
	rhstart(bp, &HTADDR->htfc, unit, &HTADDR->htbae);
	bp->b_wcount = unit;
}

/*
 * Wait for rewind to complete or for tape settledown within
 * interrecord gap when tape direction will be switched.
 * Backf is zero iff the next action goes forward.
 * If delay is needed returns non-zero and restarts htstart after delay.
 */
htgap(backf)
{
	if((HTADDR->htds & (SDWN | PIP)) == 0) return(0);
	if((HTADDR->htds & PIP) == 0 && httab.d_active != SGAP) {
		if(httab.d_active == SSEEKR || httab.d_active == SEEKR) {
			if(backf) return(0);
		} else if(!backf) return(0);
		timeout(htstart, 0, 7);
	} else
		timeout(htstart, 0, 50);
	httab.d_active = SGAP;
	return(1);
}

htintr()
{
	register struct buf *bp;
	register struct htinfo *mp;
	register rest;

	if((bp = httab.d_actf) == 0)
		return;
	rest = HTADDR->htfc;
	/* read returns positive number of frames read in htfc */
	if(httab.d_active == SIO) {
		bp->b_resid = rest;
		if(bp->b_flags & B_READ)
			bp->b_resid =+ bp->b_wcount;
	}
	mp = &htinfo[bp->b_dev & (NUNIT-1)];
	if((HTADDR->htcs1 & TRE) || (HTADDR->htds & (BOT | EOF))) {
		switch(httab.d_active) {
		case SSEEKR:
			mp->t_blkno =- rest;
			break;
		case SSEEKF:
			mp->t_blkno =+ rest;
			break;
		}
		if(HTADDR->htds & EOT) {
			switch(httab.d_active) {
			case SEEKF:
				mp->t_tmback++;
				if((HTADDR->htds & EOF) == 0)
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
			return(htstart());
		} else if(HTADDR->htds & EOF) {
			switch(httab.d_active) {
			case SSEEKR:
			case SEEKR:
			case SEEKF:
				return(htstart());
			case SIO:
				bp->b_resid = bp->b_wcount;
			}
			if(mp->t_bkftm + 1 == mp->t_blkno
			   || (bp != &rhtbuf && (mp->t_flags & TRAW) == 0)) {
				if(bp != &rhtbuf && httab.d_active == SIO
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
				return(htstart());
			}
			mp->t_bkftm = mp->t_blkno;
		} else if((HTADDR->htds & BOT) != 0 && bp->b_blkno == 0) {
			if(httab.d_active == SEEKR) {
				if((mp->t_flags & TADV) == 0
				  || mp->t_tmback != 0) {
					bp->b_flags =| B_ERROR;
					mp->t_tmback = 0;
				}
				mp->t_flags =& ~TADV;
				return(htstart());
			}
		/* ignore raw frame count errors and correctable errors */
		} else if(HTADDR->htcs2 & CHARD || bp != &rhtbuf
		     || (HTADDR->hter != FCE && ((HTADDR->htds & PES) == 0
		     || (HTADDR->hter & ~(FCE|COR|CS)) != 0))) {
			if(HTADDR->hter & HARD || HTADDR->htcs2 & CHARD) {
				mp->t_flags =| TERROR;
				printf("fc=%o ba=%o ",
					HTADDR->htfc, HTADDR->htba);
			} else if(++httab.d_errcnt < RETRY)
				return(htstart());
			deverror(bp, HTADDR->hter, HTADDR->htcs2);
			bp->b_flags =| B_ERROR;
			ht_done++;
			return(htstart());
		}
	}
	switch(httab.d_active) {
	case SIO:
		if(&(-bp->b_wcount)->chp < rest.chp) {
			/* record longer than buffer */
			bp->b_flags =| B_ERROR;
			bp->b_resid = 0;
		}
		ht_done++;
		break;
	case SEEKR:
		mp->t_tmback++;
		break;
	case SEEKF:
		mp->t_flags =| TADV;
		break;
	}
	return(htstart());
}

htread(dev)
{
	register rdev;

	rdev = dev;
	htseek(rdev, B_READ);
	htodd = u.u_count & 1;
	u.u_count =+ htodd;
	physio(htstrategy, &rhtbuf, rdev, B_READ);
	htadjust(rdev);
}

htwrite(dev)
{
	register rdev;

	rdev = dev;
	htseek(rdev, B_READ);
	htodd = u.u_count & 1;
	u.u_count =+ htodd;
	if(u.u_count == 0) { /* write tape mark */
		htseek(rdev, B_WRITE);
	} else {
		physio(htstrategy, &rhtbuf, rdev, B_WRITE);
	}
	htadjust(rdev);
}

/* fudge file offset to make all blocks appear to be 512 bytes */
htadjust(dev)
{
	register cnt, *p;

	u.u_count = -rhtbuf.b_resid;
	p = htinfo[dev & (NUNIT - 1)].t_fp->f_offset;
	if(cnt = u.u_count - u.u_arg[1]) {
		dpadd(p, cnt);
		p[0]--;
	}
	dpadd(p, 512);
}

htseek(dev, rw)
int dev;
int rw;
{
	register struct buf *bp;

	bp = &rhtbuf;
	spl5();
	while(bp->b_flags & B_BUSY) {
		bp->b_flags =| B_WANTED;
		sleep(bp, HTPRIO);
	}
	spl0();
	htodd = 0;
	bp->b_flags = B_BUSY | rw;
	bp->b_dev = dev;
	bp->b_blkno = lshift(u.u_offset, -9);
	bp->b_wcount = 0;
	bp->b_error = 0;
	htstrategy(bp);
	spl5();
	while ((bp->b_flags & B_DONE) == 0) {
		bp->b_flags =| B_WANTED;
		sleep(bp, HTPRIO);
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
