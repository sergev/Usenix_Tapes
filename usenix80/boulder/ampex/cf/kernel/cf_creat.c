#include "../param.h"
#include "../user.h"
#include "../inode.h"
#include "../reg.h"
#include "../file.h"
#include "../cf.h"

long nblks, firstb;

cf_creat()		/* system call to allocate new contiguous file(s)
			   args are new file name, mode, #files, 
			   and #chunks */
{
	register struct inode *nfp;
	long lastb;
	extern long dpmul();
	unsigned nfiles, i, nchunks;

	nfiles = u.u_arg[2];
	nchunks = u.u_arg[3];
	creat();
	if(u.u_error)
		goto bad;
				/* at this point 1st file created & open */
	nfp = u.u_ofile[u.u_ar0[R0]]->f_inode;
	if(cf_start(nfp->i_dev)) {	/* open free list */
		if(nchunks > cf_sb.bound && (i = nchunks % cf_sb.bound))
			nchunks =+ (cf_sb.bound - i);
		nblks = dpmul(nchunks, cf_sb.chunksiz);	/* = # blocks/file */
		nchunks =* nfiles;		/* = total # chunks sought */
		firstb = dpmul(cf_getfree(nchunks), cf_sb.chunksiz)
			 + cf_sb.cfb0;
	}
	cf_relse();		/* close free list */
	if(u.u_error) {
		iput(nfp);
    bad:
		u.u_ar0[R0] = -1;
		return;
	}
	lastb = firstb + dpmul(nchunks, cf_sb.chunksiz) - 1;
	for(i=1; i<= nfiles; i++) {
		fixi(nfp);
		if(i < nfiles) {
			incr();
			c_creat();	/* create but don't open file */
			if(u.u_error) {
				nfp->i_addr->i_cbase = lastb;
				return;
			}
		}
		if(i > 1)
			iput(nfp);
		nfp = u.u_ar0[R1];
	}
}

fixi(ip)
register *ip;
{
	ip->i_mode =& ~IFMT;
	ip->i_mode =| IFCHR;
	ip = ip->i_addr;
	ip->i_cflag = ICFLAG;
	ip->i_cdev = cf_sb.devc;
	ip->i_bdev = cf_sb.devb;
	ip->i_cbase = firstb;
	ip->i_clast = (firstb =+ nblks) - 1;
}

incr()
{
	register char *dp;

	for(dp = u.u_dbuf; *dp; )
		dp++;
	while((*--dp)++ >= '9')
		*dp = '0';
}
