The following changes implement local execution of an object file that
lives on another host.
***************
*** 58,63
  #include "../h/cmap.h"
  #include "../h/vmmac.h"
  #include "../h/debug.h"
  
  int  *swapstack();
  

--- 58,69 -----
  #include "../h/cmap.h"
  #include "../h/vmmac.h"
  #include "../h/debug.h"
+ #ifdef REMOTEFS
+ /*
+  * needed only if EISREMOTE isn't in /usr/include/errno.h
+  */
+ #include "../h/errno.h"
+ #endif REMOTEFS
  
  int  *swapstack();
  
***************
*** 76,81
  	register char *cp;
  	register struct buf *bp;
  	register int na, ne, ucp, ap, c, i;
  	register int indir, uid, gid;
  	register char *sharg;
  	register struct inode *ip;

--- 82,93 -----
  	register char *cp;
  	register struct buf *bp;
  	register int na, ne, ucp, ap, c, i;
+ #ifdef REMOTEFS
+ 	register int indir;
+ 	int uid, gid; /* have to take address */
+ 	struct inode *ip; /* have to take address */
+ 	int	remote = -1;
+ #else REMOTEFS
  	register int indir, uid, gid;
  	register struct inode *ip;
  #endif REMOTEFS
***************
*** 77,83
  	register struct buf *bp;
  	register int na, ne, ucp, ap, c, i;
  	register int indir, uid, gid;
- 	register char *sharg;
  	register struct inode *ip;
  	register swblk_t bno;
  	char cfname[MAXNAMLEN + 1];

--- 89,94 -----
  	int	remote = -1;
  #else REMOTEFS
  	register int indir, uid, gid;
  	register struct inode *ip;
  #endif REMOTEFS
  	register char *sharg;
***************
*** 79,84
  	register int indir, uid, gid;
  	register char *sharg;
  	register struct inode *ip;
  	register swblk_t bno;
  	char cfname[MAXNAMLEN + 1];
  	char cfarg[SHSIZE];

--- 90,97 -----
  #else REMOTEFS
  	register int indir, uid, gid;
  	register struct inode *ip;
+ #endif REMOTEFS
+ 	register char *sharg;
  	register swblk_t bno;
  	char cfname[MAXNAMLEN + 1];
  	char cfarg[SHSIZE];
***************
*** 105,110
   *	(7) do a ret
   */
  	if ((ip = namei(uchar, LOOKUP, 1)) == NULL)
  		return;
  	sysVinfo.sV_sysexec++;
  	bno = 0;

--- 118,128 -----
   *	(7) do a ret
   */
  	if ((ip = namei(uchar, LOOKUP, 1)) == NULL)
+ #ifdef REMOTEFS
+ 		if (u.u_error == EISREMOTE)
+ 			remote = remote_execinfo(&ip, &uid, &gid, fname);
+ 	if (u.u_error)
+ #endif REMOTEFS
  		return;
  	sysVinfo.sV_sysexec++;
  	bno = 0;
***************
*** 110,115
  	bno = 0;
  	bp = 0;
  	indir = 0;
  	uid = u.u_uid;
  	gid = u.u_gid;
  	if (ip->i_mode & ISUID)

--- 128,136 -----
  	bno = 0;
  	bp = 0;
  	indir = 0;
+ #ifdef REMOTEFS
+ if (remote < 0) {
+ #endif REMOTEFS
  	uid = u.u_uid;
  	gid = u.u_gid;
  	if (ip->i_mode & ISUID)
***************
*** 148,153
  	if (u.u_error)
  		goto bad;
  	u.u_count = resid;
  #ifndef lint
  	if (resid > sizeof(u.u_exdata) - sizeof(u.u_exdata.Ux_A) &&
  	    u.u_exdata.ux_shell[0] != '#') {

--- 169,180 -----
  	if (u.u_error)
  		goto bad;
  	u.u_count = resid;
+ #ifdef REMOTEFS
+ }
+ 
+ remote_again:
+ 
+ #endif REMOTEFS
  #ifndef lint
  	if (resid > sizeof(u.u_exdata) - sizeof(u.u_exdata.Ux_A) &&
  	    u.u_exdata.ux_shell[0] != '#') {
***************
*** 224,229
  		    (unsigned)(u.u_dent.d_namlen + 1));
  		cfname[MAXCOMLEN] = 0;
  		indir = 1;
  		iput(ip);
  		/* Security hole fix: don't allow SUID-root exec'able
  		   scripts whose name starts with '-'.... allows

--- 251,259 -----
  		    (unsigned)(u.u_dent.d_namlen + 1));
  		cfname[MAXCOMLEN] = 0;
  		indir = 1;
+ #ifdef REMOTEFS
+ 		if (remote < 0)
+ #endif REMOTEFS
  		iput(ip);
  		/* Security hole fix: don't allow SUID-root exec'able
  		   scripts whose name starts with '-'.... allows
***************
*** 233,238
  			return;
  		}
  		ip = namei(schar, LOOKUP, 1);
  		if (ip == NULL)
  			return;
  		goto again;

--- 263,280 -----
  			return;
  		}
  		ip = namei(schar, LOOKUP, 1);
+ #ifdef REMOTEFS
+ 		if (ip == NULL) {
+ 			if (u.u_error == EISREMOTE)
+ 				remote = remote_execinfo(&ip, 0, 0, 0);
+ 			if (u.u_error)
+ 				return;
+ 			if (ip == NULL)
+ 				goto remote_again;
+ 		}
+ 		else
+ 			remote = -1;
+ #else REMOTEFS
  		if (ip == NULL)
  			return;
  #endif REMOTEFS
***************
*** 235,240
  		ip = namei(schar, LOOKUP, 1);
  		if (ip == NULL)
  			return;
  		goto again;
  	}
  

--- 277,283 -----
  #else REMOTEFS
  		if (ip == NULL)
  			return;
+ #endif REMOTEFS
  		goto again;
  	}
  
***************
*** 332,337
  		bcopy((caddr_t)cfname, (caddr_t)u.u_dent.d_name,
  		    (unsigned)(u.u_dent.d_namlen + 1));
  	}
  	getxfile(ip, (int)(nc + (na+4)*NBPW), uid, gid);
  	if (u.u_error) {
  badarg:

--- 375,383 -----
  		bcopy((caddr_t)cfname, (caddr_t)u.u_dent.d_name,
  		    (unsigned)(u.u_dent.d_namlen + 1));
  	}
+ #ifdef REMOTEFS
+ 	getxfile(ip, (int)(nc + (na+4)*NBPW), uid, gid, remote);
+ #else REMOTEFS
  	getxfile(ip, (int)(nc + (na+4)*NBPW), uid, gid);
  #endif REMOTEFS
  	if (u.u_error) {
***************
*** 333,338
  		    (unsigned)(u.u_dent.d_namlen + 1));
  	}
  	getxfile(ip, (int)(nc + (na+4)*NBPW), uid, gid);
  	if (u.u_error) {
  badarg:
  		for (c = 0; c < nc; c += CLSIZE*PAGSIZ) {

--- 379,385 -----
  	getxfile(ip, (int)(nc + (na+4)*NBPW), uid, gid, remote);
  #else REMOTEFS
  	getxfile(ip, (int)(nc + (na+4)*NBPW), uid, gid);
+ #endif REMOTEFS
  	if (u.u_error) {
  badarg:
  		for (c = 0; c < nc; c += CLSIZE*PAGSIZ) {
***************
*** 627,632
   * Set up page tables and other structures for the process to be
   * executed, and read in from the file.
   */
  getxfile(ip, nargc, uid, gid)
  	register struct inode *ip;
  {

--- 674,682 -----
   * Set up page tables and other structures for the process to be
   * executed, and read in from the file.
   */
+ #ifdef REMOTEFS
+ getxfile(ip, nargc, uid, gid, remote)
+ #else REMOTEFS
  getxfile(ip, nargc, uid, gid)
  #endif REMOTEFS
  	register struct inode *ip;
***************
*** 628,633
   * executed, and read in from the file.
   */
  getxfile(ip, nargc, uid, gid)
  	register struct inode *ip;
  {
  	register size_t ts, ds, uss, css;

--- 678,684 -----
  getxfile(ip, nargc, uid, gid, remote)
  #else REMOTEFS
  getxfile(ip, nargc, uid, gid)
+ #endif REMOTEFS
  	register struct inode *ip;
  {
  	register size_t ts, ds, uss, css;
***************
*** 632,637
  {
  	register size_t ts, ds, uss, css;
  	register int pagi;
  	register struct proc *p;
  	register struct file *fp;
  	register int flag;		/* used to keep proc flags */

--- 683,691 -----
  {
  	register size_t ts, ds, uss, css;
  	register int pagi;
+ #ifdef REMOTEFS
+ 	register int	oldtextsize;
+ #endif REMOTEFS
  	register struct proc *p;
  	register struct file *fp;
  	register int flag;		/* used to keep proc flags */
***************
*** 650,655
  		pagi = SPAGI;
  	else
  		pagi = 0;
  	if (u.u_exdata.ux_tsize!=0 && (ip->i_xflag&ITEXTFILE)==0 &&
  						ip->i_count!=1) {
  		SPINLOCK(&file_lock);

--- 704,720 -----
  		pagi = SPAGI;
  	else
  		pagi = 0;
+ #ifdef REMOTEFS
+ 	if (remote >= 0) {
+ 		/*
+ 		 * Prevent xalloc() from making a shared or paged text.
+ 		 */
+ 		pagi = 0;
+ 		oldtextsize = u.u_exdata.ux_tsize;
+ 		u.u_exdata.ux_dsize += u.u_exdata.ux_tsize;
+ 		u.u_exdata.ux_tsize = 0;
+ 	}
+ #endif REMOTEFS
  	if (u.u_exdata.ux_tsize!=0 && (ip->i_xflag&ITEXTFILE)==0 &&
  						ip->i_count!=1) {
  		SPINLOCK(&file_lock);
***************
*** 802,807
  	 * Read in the data segment if we are not going to page in this
  	 * process.
  	 */
  	if (pagi == 0) {
  		u.u_error =
  		    rdwri(UIO_READ, ip,

--- 867,877 -----
  	 * Read in the data segment if we are not going to page in this
  	 * process.
  	 */
+ #ifdef REMOTEFS
+ 	if (remote >= 0)
+ 		u.u_error = remote_execread(remote, oldtextsize);
+ 	else
+ #endif REMOTEFS
  	if (pagi == 0) {
  		u.u_error =
  		    rdwri(UIO_READ, ip,
