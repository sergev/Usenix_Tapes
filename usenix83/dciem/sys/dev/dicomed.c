/*
 *	DR11B driver, initially developed for DICOMED D48 film recorder.
 *	Martin Tuori, DCIEM Toronto, 1979.
 *	Modified for V7, June 1981, by Martin Tuori.
 */

#include "../h/local.h"
/*#include "../h/d48opcode.h"*/
#include "../h/param.h"
#include "../h/dir.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/mascot.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/seg.h"

struct drdev {
	unsigned int	drwc;		/* word count */
	caddr_t		drba;		/* bus address */
	unsigned int	drcsr;		/* control and status register */
	unsigned int	drdb;		/* data buffer */
};

#define DRADDR ((struct drdev *)0172410)
/* second DRADDR is 0172420 */
/* interrupt vectors are at 0124 and 0130 */

/* sleep's should be interruptable, due to device's tendency to hang */
#define PRIDR11B (PZERO+1)

struct	buf	drtab;
struct	buf	drbuf;
int drbusy= 0;
extern lbolt;
int filmstatus = 0;
#define S_CRITICAL 0340


/* Control and Status bits */
#define DRERR	0100000
#define DRNXM	0040000
#define DRATTN	0020000
#define DRMAINT	0010000
#define DRSTATA	0004000
#define DRSTATB	0002000
#define DRSTATC	0001000
#define DRCYCLE	0000400
#define DRREADY	0000200
#define DRIENBL	0000100
#define DRXMEM	0000060
#define DRFUNC3	0000010
#define DRFUNC2	0000004
#define DRFUNC1	0000002
#define DRGO	0000001

#define READ_N_RDY  DRSTATA
#define WRITE_N_RDY DRSTATB
#define D48_WRITE   DRFUNC1
#define D48_ATTN    DRFUNC2
#define D48_STAT    DRFUNC3

dropen(){
	if(drbusy)
		u.u_error= EBUSY;
	else{
		drbusy++;
		filmstatus= 0;
	}
}

drclose(){
	register struct buf *bp;

	spl6();
	drbusy= 0;
	if(drtab.b_active){
		DRADDR->drcsr&= ~DRIENBL;
		drtab.b_active= 0;
		bp= drtab.b_actf;
		bp->b_flags|= B_DONE;
		bp->b_flags&= ~B_BUSY;
		wakeup(bp);
	}
	spl0();
}

drstrategy(abp)
struct buf *abp;
{
	register struct buf *bp;

	bp = abp;
        if (drtab.b_actf==0)
		drtab.b_actf = bp;
	else {
		u.u_error= EBUSY;
		iodone(bp);
	}
	if (drtab.b_active==0)
		drstart();
}

drstart()
{
	register struct buf *bp;
	register int com;

	if ((bp = drtab.b_actf) == 0)
		return;
	drtab.b_active++;
	/* get status */
	/* it would be nice to keep a count of lbolts here, and give up after
		awhile */
	while(DRADDR->drcsr & WRITE_N_RDY) sleep (&lbolt,PRIDR11B);
	DRADDR->drcsr= 0; DRADDR->drcsr= D48_STAT; DRADDR->drcsr= 0;
	while((DRADDR->drcsr & DRREADY) ==0) sleep(&lbolt,PRIDR11B);
	filmstatus= DRADDR->drdb;
	if(filmstatus & S_CRITICAL)
		printf("Film status CRITICAL, %o, get tech help\n",filmstatus);

	DRADDR->drwc= -(bp->b_bcount>>1);
	DRADDR->drba= bp->b_un.b_addr;
	com= DRGO | DRIENBL | ((bp->b_xmem & 03) << 4) | D48_WRITE;
	while(DRADDR->drcsr & WRITE_N_RDY) sleep (&lbolt,PRIDR11B);
	DRADDR->drcsr= DRCYCLE|D48_WRITE; /* prime a bus cycle */
	DRADDR->drcsr= com;	/* now do the request */
}

drintr()
{
	register struct buf *bp;

	if (drtab.b_active == 0)
		return;
	bp = drtab.b_actf;
	drtab.b_active = 0;
	if(DRADDR->drcsr & DRERR){	/* error bit */
		deverror(bp, DRADDR->drcsr);
		bp->b_flags |= B_ERROR;
	}
	drtab.b_errcnt = 0;
	drtab.b_actf = bp->av_forw;
	bp->b_resid = DRADDR->drwc;
	iodone(bp);
	drstart();
}

drsgtty(dev,v)
	int *v;
{
	if(v){
		*v++= filmstatus;
		*v=   DRADDR->drcsr;
	}
	else u.u_error= EINVAL;
}

drwrite(dev)
{

	drphysio(drstrategy, &drbuf, dev, B_WRITE);
}

/* copy of physio, which does not sleep at PRIBIO
 * waiting for io to complete; the film recorder has
 * a tendency to hang up, never interrupt, and leave
 * us stranded. This way, the user can kill it.
 */

/*
 * Raw I/O. The arguments are
 *	The strategy routine for the device
 *	A buffer, which will always be a special buffer
 *	  header owned exclusively by the device for this purpose
 *	The device number
 *	Read/write flag
 * Essentially all the work is computing physical addresses and
 * validating them.
 */
drphysio(strat, bp, dev, rw)
register struct buf *bp;
int (*strat)();
{
	register unsigned base;
	register int nb;
	int ts;

	base = (unsigned)u.u_base;
#ifdef S_OVRLAY
	/* Verify and modify base if page mapping in effect.
	 * Program appears to be I/D separated, but data starts
	 * at virtual 8192.  Subtract this to fool rest of code.
	 */
	if (SEP_ID(u.u_sep) && u.u_pagelim!=0) {
		if (base < ctob(stoc(1)))
			goto bad;
		base -= ctob(stoc(1));
	}
#endif
	/*
	 * Check odd base, odd count, and address wraparound
	 */
	if (base&01 || u.u_count&01 || base>=base+u.u_count)
		goto bad;
	ts = (u.u_tsize+127) & ~0177;
	if (SEP_ID(u.u_sep))
		ts = 0;
	nb = (base>>6) & 01777;
	/*
	 * Check overlap with text. (ts and nb now
	 * in 64-byte clicks)
	 */
	if (nb < ts)
		goto bad;
	/*
	 * Check that transfer is either entirely in the
	 * data or in the stack: that is, either
	 * the end is in the data or the start is in the stack
	 * (remember wraparound was already checked).
	 */
	if ((((base+u.u_count)>>6)&01777) >= ts+u.u_dsize
	    && nb < 1024-u.u_ssize)
		goto bad;
	spl6();
	/* no need to wait for a buffer pointer that this driver owns
	while (bp->b_flags&B_BUSY) {
		bp->b_flags |= B_WANTED;
		sleep((caddr_t)bp, PRIBIO+1);
	}
	*/
	bp->b_flags = B_BUSY | B_PHYS | rw;
	bp->b_dev = dev;
	/*
	 * Compute physical address by simulating
	 * the segmentation hardware.
	 */
	ts = (SEP_ID(u.u_sep)? UDSA: UISA)->r[nb>>7] + (nb&0177);
	bp->b_un.b_addr = (caddr_t)((ts<<6) + (base&077));
	bp->b_xmem = (ts>>10) & 077;
	bp->b_blkno = u.u_offset >> BSHIFT;
	bp->b_bcount = u.u_count;
	bp->b_error = 0;
	u.u_procp->p_flag |= SLOCK;
	(*strat)(bp);
	spl6();
	while ((bp->b_flags&B_DONE) == 0)
		sleep((caddr_t)bp, PRIDR11B);
	u.u_procp->p_flag &= ~SLOCK;
	if (bp->b_flags&B_WANTED)
		wakeup((caddr_t)bp);
	spl0();
	bp->b_flags &= ~(B_BUSY|B_WANTED);
	u.u_count = bp->b_resid;
	geterror(bp);
	return;
    bad:
	printf("DR11B physio error.\n");
	u.u_error = EFAULT;
}
