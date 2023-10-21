#
/*
 */

#include "../param.h"
#include "../systm.h"
#include "../user.h"
#include "../proc.h"
#include "../text.h"
#include "../inode.h"

/*
 * Swap out process p.
 * The ff flag causes its core to be freed--
 * it may be off when called to create an image for a
 * child process in newproc.
 * Os is the old size of the data area of the process,
 * and is supplied during core expansion swaps.
 *
 * panic: out of swap space
 * panic: swap error -- IO error
 */
xswap(p, ff, os)
int *p;
{
	register *rp, a;

# ifdef XBUF
	*ka5 = praddr;
# endif
	rp = p;
	if(os == 0)
		os = rp->p_size;
#ifdef XSWAP
	if(rp->p_flag&STGOUT)
		a = malloc2(swapmap, (rp->p_size+7)/8);
	else
#endif
	a = malloc(swapmap, (rp->p_size+7)/8);
	if(a == NULL)
		panic("out of swap space");
#ifdef XSWAP
	if(a < paddrx)	/* statistics */
		npriswp++;
	else
		nsecswp++;
#endif
	xccdec(rp->p_textp);
	rp->p_flag =| SLOCK;
	if(swap(a, rp->p_addr, os, 0))
		panic("swap error");
# ifdef XBUF
	*ka5 = praddr;
# endif
	if(ff)
		mfree(coremap, os, rp->p_addr);
	rp->p_addr = a;
#ifndef XSWAP
	rp->p_flag =& ~(SLOAD|SLOCK);
#endif
#ifdef XSWAP
	rp->p_flag =& ~(SLOAD|SLOCK|STGOUT);
#endif
	rp->p_time = 0;
	if(runout) {
		runout = 0;
		wakeup(&runout);
	}
}

/*
 * relinquish use of the shared text segment
 * of a process.
 */
xfree()
{
	register *xp, *ip;

# ifdef XBUF
	*ka5 = praddr;
# endif
	if((xp=u.u_procp->p_textp) != NULL) {
		u.u_procp->p_textp = NULL;
		xccdec(xp);
		if(--xp->x_count == 0) {
			ip = xp->x_iptr;
			if((ip->i_mode&ISVTX) == 0) {
				xp->x_iptr = NULL;
				mfree(swapmap, (xp->x_size+7)/8, xp->x_daddr);
				ip->i_flag =& ~ITEXT;
				iput(ip);
			}
		}
	}
}

/*
 * Attach to a shared text segment.
 * If there is no shared text, just return.
 * If there is, hook up to it:
 * if it is not currently being used, it has to be read
 * in from the inode (ip) and established in the swap space.
 * If it is being used, but is not currently in core,
 * a swap has to be done to get it back.
 * The full coroutine glory has to be invoked--
 * see slp.c-- because if the calling process
 * is misplaced in core the text image might not fit.
 * Quite possibly the code after "out:" could check to
 * see if the text does fit and simply swap it in.
 *
 * panic: out of swap space
 */
xalloc(ip)
int *ip;
{
	register struct text *xp;
	register *rp, ts;

# ifdef XBUF
	*ka5 = praddr;
# endif
	if(u.u_arg[1] == 0)
		return;
	rp = NULL;
	for(xp = &text[0]; xp < &text[NTEXT]; xp++)
		if(xp->x_iptr == NULL) {
			if(rp == NULL)
				rp = xp;
		} else
			if(xp->x_iptr == ip) {
				xp->x_count++;
				u.u_procp->p_textp = xp;
				goto out;
			}
	if((xp=rp) == NULL)
		panic("out of text");
	xp->x_count = 1;
	xp->x_ccount = 0;
	xp->x_iptr = ip;
	ts = ((u.u_arg[1]+63)>>6) & 01777;
	xp->x_size = ts;
#ifndef XSWAP
	if((xp->x_daddr = malloc(swapmap, (ts+7)/8)) == NULL)
#endif
#ifdef XSWAP
	if((xp->x_daddr = malloc2(swapmap, (ts+7)/8)) == NULL)
#endif
		panic("out of swap space");
	expand(USIZE+ts);
	estabur(0, ts, 0, 0);
	u.u_count = u.u_arg[1];
	u.u_offset[1] = 020;
	u.u_base = 0;
	readi(ip);
# ifdef XBUF
	*ka5 = praddr;
# endif
	rp = u.u_procp;
	rp->p_flag =| SLOCK;
	swap(xp->x_daddr, rp->p_addr+USIZE, ts, 0);
# ifdef XBUF
	*ka5 = praddr;
# endif
	rp->p_flag =& ~SLOCK;
	rp->p_textp = xp;
	rp = ip;
	rp->i_flag =| ITEXT;
	rp->i_count++;
	expand(USIZE);

out:
	if(xp->x_ccount == 0) {
		savu(u.u_rsav);
		savu(u.u_ssav);
		xswap(u.u_procp, 1, 0);
# ifdef XBUF
	*ka5 = praddr;
# endif
		u.u_procp->p_flag =| SSWAP;
		qswtch();
		/* no return */
	}
	xp->x_ccount++;
}

/*
 * Decrement the in-core usage count of a shared text segment.
 * When it drops to zero, free the core space.
 */
xccdec(xp)
int *xp;
{
	register *rp;

	if((rp=xp)!=NULL && rp->x_ccount!=0)
		if(--rp->x_ccount == 0)
			mfree(coremap, rp->x_size, rp->x_caddr);
}
