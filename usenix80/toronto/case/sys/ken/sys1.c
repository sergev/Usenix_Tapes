#
/*
 */

#include "../param.h"
#include "../systm.h"
#include "../user.h"
#include "../proc.h"
#include "../buf.h"
#include "../reg.h"
#include "../inode.h"
#include "../file.h"    /* rand:bobg for eofp call */

/*
 * exec system call.
 * Since we no longer use an I/O buffer to store the callers
 * arguments (arg copy to swapdev swiped from Purdue/Ver 7),
 * the NEXEC define has been replaced by NCARGS, the maximum
 * number of args allowed (must be multiple of 512) in param.h.
 */
#define EXPRI	-1

exec()
{
	register struct buf *bp;
	char *na, *nc, *ucp;
	int ts, ds, ss, bno, sep;
	register c;
	register char *cp;
	char *ap;
	struct inode *ip;
	extern uchar;
	int *iip;

	/*
	 * pick up file names
	 * and check various modes
	 * for execute permission
	 */

	ip = namei(&uchar, 0);
	if(ip == NULL)
		return;
	bno = bp = 0;
	if(access(ip, IEXEC) || (ip->i_mode&IFMT)!=0)
		goto bad;

	/*
	 * pack up arguments into a file on the swapdev,
	 * (allows > 512 bytes of args).
	 */

	na = 0;
	nc = 0;
	if((bno = malloc(swapmap, (NCARGS+511)/512)) == 0)
		panic("Out of swap");		/* not very elegant */
	if(u.u_arg[1])
		while(ap = fuword(u.u_arg[1])){
			na++;
			if(ap == -1)
				u.u_error = EFAULT;
			u.u_arg[1] += 2;
			do{
				if(nc >= NCARGS-1)
					u.u_error = E2BIG;
				if((c = fubyte(ap++)) < 0)
					u.u_error = EFAULT;
				if(u.u_error)
					goto bad;
				if((nc & 0777) == 0){
					if(bp)
						bawrite(bp);
					bp = getblk(swapdev, bno+(nc >> 9));
					cp = bp->b_addr;
				}
				nc++;
				*cp++ = c;
			}while(c > 0);
		}
	if(bp)
		bawrite(bp);
	bp = 0;
	if((nc&1) != 0) 
		nc++;

	/*
	 * read in first 8 bytes
	 * of file for segment
	 * sizes:
	 * w0 = 407/410/411 (410 implies RO text) (411 implies sep ID)
	 * w1 = text size
	 * w2 = data size
	 * w3 = bss size
	 */

	u.u_base = &u.u_arg[0];
	u.u_count = 8;
	u.u_offset[1] = 0;
	u.u_offset[0] = 0;
	u.u_segflg = 1;
	readi(ip);
	u.u_segflg = 0;
	if(u.u_error)
		goto bad;
	sep = 0;
	if(u.u_arg[0] == 0407) {
		u.u_arg[2] =+ u.u_arg[1];
		u.u_arg[1] = 0;
	} else
	if(u.u_arg[0] == 0411)
		sep++; else
	if(u.u_arg[0] != 0410) {
		u.u_error = ENOEXEC;
		goto bad;
	}
	if(u.u_arg[1]!=0 && (ip->i_flag&ITEXT)==0 && ip->i_count!=1) {
		u.u_error = ETXTBSY;
		goto bad;
	}

	/*
	 * find text and data sizes
	 * try them out for possible
	 * exceed of max sizes
	 */

	ts = ((u.u_arg[1]+63)>>6) & 01777;
	ds = ((u.u_arg[2]+u.u_arg[3]+63)>>6) & 01777;
	ss = SSIZE + ((nc + na*2) >> 6);
	if(estabur(ts, ds, ss, sep))
		goto bad;

	/*
	 * allocate and clear core
	 * at this point, committed
	 * to the new image
	 */

	u.u_prof[3] = 0;
	xfree();
	expand(USIZE);
	xalloc(ip);
	c = USIZE+ds+ss;
	expand(c);
	while(--c >= USIZE)
		clearseg(u.u_procp->p_addr+c);

	/*
	 * read in data segment
	 */

	estabur(0, ds, 0, 0);
	u.u_base = 0;
	u.u_offset[1] = 020+u.u_arg[1];
	u.u_count = u.u_arg[2];
	readi(ip);

	/*
	 * initialize stack segment
	 */

	u.u_tsize = ts;
	u.u_dsize = ds;
	u.u_ssize = ss;
	u.u_sep = sep;
	estabur(u.u_tsize, u.u_dsize, u.u_ssize, u.u_sep);
	/*
	 * copy back arglist from swapdev
	 */
	ucp = -nc;
	ap = ucp - na*2 -4;
	u.u_ar0[R6] = ap;
	suword(ap, na);
	nc = 0;
	while(na--){
		suword(ap += 2, ucp);
		do{
			if((nc & 0777) == 0){
				if(bp)
					brelse(bp);
				bp = bread(swapdev, bno + (nc >> 9));
				cp = bp->b_addr;
			}
			subyte(ucp++, (c = *cp++));
			nc++;
		}while(c & 0377);
	}
	suword(ap+2, -1);

	/*
	 * set SUID/SGID protections, if no tracing
	 */

	if ((u.u_procp->p_flag&STRC)==0) {
		if(ip->i_mode&ISUID)
			if(u.u_uid != 0) {
				u.u_uid = ip->i_uid;
				u.u_procp->p_uid = ip->i_uid;
			}
		if(ip->i_mode&ISGID)
			u.u_gid = ip->i_gid;
	}

	/*
	 * clear sigs, regs and return
	 */

	for(iip = &u.u_signal[0]; iip < &u.u_signal[NSIG]; iip++)
		if((*iip & 1) == 0)
			*iip = 0;
	for(cp = &regloc[0]; cp < &regloc[6];)
		u.u_ar0[*cp++] = 0;
	u.u_ar0[R7] = 0;
	for(iip = &u.u_fsav[0]; iip < &u.u_fsav[25];)
		*iip++ = 0;

bad:
	if(bp)
		brelse(bp);
	if(bno)
		mfree(swapmap, (NCARGS+511)/512, bno);
	iput(ip);
}

/*
 * exit system call:
 * pass back caller's r0
 */
rexit()
{

	u.u_arg[0] = u.u_ar0[R0] << 8;
	exit();
}

/*
 * Release resources.
 * Save u. area for parent to look at.
 * Enter zombie state.
 * Wake up parent and init processes,
 * and dispose of children.
 */
exit()
{
	register int *a;
	register struct proc *p, *q;

	p = u.u_procp;
	p->p_flag =& ~STRC;
	p->p_clktim = 0;
	for(a = &u.u_signal[0]; a < &u.u_signal[NSIG];)
		*a++ = 1;
	for(a = &u.u_ofile[0]; a < &u.u_ofile[NOFILE]; a++)
		if (*a) {
			closef(*a);
			*a = NULL;
		}
	plock(u.u_cdir);
	iput(u.u_cdir);
	xfree();
	mfree(coremap, p->p_size, p->p_addr);
	p->p_stat = SZOMB;
	p->xp_xstat = u.u_arg[0];
	p->xp_utime = u.u_cutime + u.u_utime;
	p->xp_stime = u.u_cstime + u.u_stime;
	for(q = &proc[0]; q < &proc[NPROC]; q++)
		if(q->p_ppid == p->p_pid) {
			wakeup(&proc[1]);
			q->p_ppid  = 1;
			if (q->p_stat == SSTOP)
				setrun(q);
		}
	for (q = &proc[0]; q < &proc[NPROC]; q++)
		if (p->p_ppid == q->p_pid) {
			wakeup(q);
			swtch();
			/* no return */
		}
	printf("Init proc dead\n");
	swtch();
}

/*
 * Wait system call.
 * Search for a terminated (zombie) child,
 * finally lay it to rest, and collect its status.
 * Look also for stopped (traced) children,
 * and pass back status from them.
 */
wait()
{
	register f;
	register struct proc *p;

	f = 0;

loop:
	for(p = &proc[0]; p < &proc[NPROC]; p++)
	if(p->p_ppid == u.u_procp->p_pid) {
		f++;
		if(p->p_stat == SZOMB) {
			u.u_ar0[R0] = p->p_pid;
			u.u_ar0[R1] = p->xp_xstat;
			u.u_cutime =+ p->xp_utime;
			u.u_cstime =+ p->xp_stime;
			p->p_stat = NULL;
			p->p_pid = 0;
			p->p_ppid = 0;
			p->p_sig = 0;
			p->p_ttyp = 0;
			p->p_flag = 0;
			p->p_wchan = 0;
			return;
		}
		if(p->p_stat == SSTOP) {
			if((p->p_flag&SWTED) == 0) {
				p->p_flag =| SWTED;
				u.u_ar0[R0] = p->p_pid;
				u.u_ar0[R1] = (p->p_sig<<8) | 0177;
				return;
			}
			continue;
		}
	}
	if(f) {
		sleep(u.u_procp, PWAIT);
		goto loop;
	}
	u.u_error = ECHILD;
}

/*
 * The following was added by rand:bobg. Sfork is identical to fork
 * except that it sets the SNOSIG flag in the child inhibiting signals
 * from the teletype.
 */
sfork()
{
	fork1(1);
}

fork()
{
	fork1(0);
}

/*
 * fork system call. (Changed to fork1 by rand:bobg.)
 */
fork1(sflag)
{
	register struct proc *p1, *p2;

	p1 = u.u_procp;
	for(p2 = &proc[0]; p2 < &proc[NPROC]; p2++)
		if(p2->p_stat == NULL)
			goto found;
	u.u_error = EAGAIN;
	goto out;

found:
	if(newproc()) {
		u.u_ar0[R0] = p1->p_pid;
		u.u_cstime = 0;
		u.u_stime = 0;
		u.u_cutime = 0;
		u.u_utime = 0;
		if (sflag) u.u_procp->p_flag =| SNOSIG;
			  /* rand:bobg--set NOSIG flag in child if an sfork */
		return;
	}
	u.u_ar0[R0] = p2->p_pid;

out:
	u.u_ar0[R7] =+ 2;
}

/*
 * break system call.
 *  -- bad planning: "break" is a dirty word in C.
 */
sbreak()
{
	register a, n, d;
	int i;

	/*
	 * set n to new data size
	 * set d to new-old
	 * set n to new total size
	 */

	n = (((u.u_arg[0]+63)>>6) & 01777);
	if(!u.u_sep)
		n =- nseg(u.u_tsize) * 128;
	if(n < 0)
		n = 0;
	d = n - u.u_dsize;
	n =+ USIZE+u.u_ssize;
	if(estabur(u.u_tsize, u.u_dsize+d, u.u_ssize, u.u_sep))
		return;
	u.u_dsize =+ d;
	if(d > 0)
		goto bigger;
	a = u.u_procp->p_addr + n - u.u_ssize;
	i = n;
	n = u.u_ssize;
	while(n--) {
		copyseg(a-d, a);
		a++;
	}
	expand(i);
	return;

bigger:
	expand(n);
	a = u.u_procp->p_addr + n;
	n = u.u_ssize;
	while(n--) {
		a--;
		copyseg(a-d, a);
	}
	while(d--)
		clearseg(--a);
}

/*
 * gprocs system call (rand:bobg)
 * copies proc table into buffer supplied by user
 * returns size of proc table
 */

gprocs()
{
	register int p;

	if ((p = u.u_ar0[R0]) & 1)      /* get addr of buffer from R0 */
	{                               /* won't allow odd address */
		u.u_error = EINVAL;
		return;
	}
	if (p) copyout(&proc[0],p,sizeof(proc));
				/* if adr not 0, copyout proc tbl to user */
	u.u_ar0[R0] = NPROC;    /* return NPROC as value in any case */
}

/*
 * eofp system call (rand:bobg)
 * write an end-of-file on a pipe
 */
eofpipe()
{
	register int *p;

	p = getf(u.u_ar0[R0]);  /* get file desc from r0 */
	if (p == NULL) return;  /* couldn't */
	if (!(p->f_flag & FPIPE) || !(p->f_flag & FWRITE))
	      return(u.u_error = EBADF); /* must be the write end of a pipe */
	eofp(p->f_inode);       /* call eof code on the inode */
}


