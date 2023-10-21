The following changes implement local execution of an object file that
lives on another host.
***************
*** 27,32
  #include "acct.h"
  #include "exec.h"
  
  #ifdef vax
  #include "../vax/mtpr.h"
  #endif

--- 27,36 -----
  #include "acct.h"
  #include "exec.h"
  
+ #ifdef REMOTEFS
+ #include "../h/errno.h"
+ #endif REMOTEFS
+ 
  #ifdef vax
  #include "../vax/mtpr.h"
  #endif
***************
*** 55,61
  	int na, ne, ucp, ap, len, cc;
  	int indir, uid, gid;
  	char *sharg;
! 	struct inode *ip;
  	swblk_t bno;
  	char cfname[MAXCOMLEN + 1];
  #define	SHSIZE	32

--- 59,70 -----
  	int na, ne, ucp, ap, len, cc;
  	int indir, uid, gid;
  	char *sharg;
! #ifdef REMOTEFS
! 	struct inode *ip; /* have to take address */
! 	int	remote = -1;
! #else REMOTEFS
! 	register struct inode *ip;
! #endif REMOTEFS
  	swblk_t bno;
  	char cfname[MAXCOMLEN + 1];
  #define	SHSIZE	32
***************
*** 71,76
  	ndp->ni_segflg = UIO_USERSPACE;
  	ndp->ni_dirp = ((struct execa *)u.u_ap)->fname;
  	if ((ip = namei(ndp)) == NULL)
  		return;
  	bno = 0;
  	bp = 0;

--- 80,91 -----
  	ndp->ni_segflg = UIO_USERSPACE;
  	ndp->ni_dirp = ((struct execa *)u.u_ap)->fname;
  	if ((ip = namei(ndp)) == NULL)
+ #ifdef REMOTEFS
+ 		if (u.u_error == EISREMOTE)
+ 			remote = remote_execinfo(&ip, &uid, &gid,
+ 				&exdata, ((struct execa *)u.u_ap)->fname);
+ 	if (u.u_error)
+ #endif REMOTEFS
  		return;
  	bno = 0;
  	bp = 0;
***************
*** 75,80
  	bno = 0;
  	bp = 0;
  	indir = 0;
  	uid = u.u_uid;
  	gid = u.u_gid;
  	if (ip->i_mode & ISUID)

--- 90,99 -----
  	bno = 0;
  	bp = 0;
  	indir = 0;
+ 
+ #ifdef REMOTEFS
+ if (remote < 0) {
+ #endif REMOTEFS
  	uid = u.u_uid;
  	gid = u.u_gid;
  	if (ip->i_mode & ISUID)
***************
*** 112,117
  	    0, 1, &resid);
  	if (u.u_error)
  		goto bad;
  #ifndef lint
  	if (resid > sizeof(exdata) - sizeof(exdata.ex_exec) &&
  	    exdata.ex_shell[0] != '#') {

--- 131,143 -----
  	    0, 1, &resid);
  	if (u.u_error)
  		goto bad;
+ #ifdef REMOTEFS
+ }
+ 
+ remote_again:
+ 
+ #endif REMOTEFS
+ 
  #ifndef lint
  	if (resid > sizeof(exdata) - sizeof(exdata.ex_exec) &&
  	    exdata.ex_shell[0] != '#') {
***************
*** 170,176
  				bcopy((caddr_t)cp, (caddr_t)cfarg, SHSIZE);
  		}
  		indir = 1;
! 		iput(ip);
  		ndp->ni_nameiop = LOOKUP | FOLLOW;
  		ndp->ni_segflg = UIO_SYSSPACE;
  		ip = namei(ndp);

--- 196,205 -----
  				bcopy((caddr_t)cp, (caddr_t)cfarg, SHSIZE);
  		}
  		indir = 1;
! #ifdef REMOTEFS
! 		if (remote < 0)
! #endif REMOTEFS
! 			iput(ip);
  		ndp->ni_nameiop = LOOKUP | FOLLOW;
  		ndp->ni_segflg = UIO_SYSSPACE;
  		ip = namei(ndp);
***************
*** 174,179
  		ndp->ni_nameiop = LOOKUP | FOLLOW;
  		ndp->ni_segflg = UIO_SYSSPACE;
  		ip = namei(ndp);
  		if (ip == NULL)
  			return;
  		bcopy((caddr_t)ndp->ni_dent.d_name, (caddr_t)cfname,

--- 203,220 -----
  		ndp->ni_nameiop = LOOKUP | FOLLOW;
  		ndp->ni_segflg = UIO_SYSSPACE;
  		ip = namei(ndp);
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
*** 176,181
  		ip = namei(ndp);
  		if (ip == NULL)
  			return;
  		bcopy((caddr_t)ndp->ni_dent.d_name, (caddr_t)cfname,
  		    MAXCOMLEN);
  		cfname[MAXCOMLEN] = '\0';

--- 217,223 -----
  #else REMOTEFS
  		if (ip == NULL)
  			return;
+ #endif REMOTEFS
  		bcopy((caddr_t)ndp->ni_dent.d_name, (caddr_t)cfname,
  		    MAXCOMLEN);
  		cfname[MAXCOMLEN] = '\0';
***************
*** 268,274
  		bdwrite(bp);
  	bp = 0;
  	nc = (nc + NBPW-1) & ~(NBPW-1);
! 	getxfile(ip, &exdata.ex_exec, nc + (na+4)*NBPW, uid, gid);
  	if (u.u_error) {
  badarg:
  		for (cc = 0; cc < nc; cc += CLSIZE*NBPG) {

--- 310,322 -----
  		bdwrite(bp);
  	bp = 0;
  	nc = (nc + NBPW-1) & ~(NBPW-1);
! 
! #ifdef REMOTEFS
!         getxfile(ip, &exdata.ex_exec, nc + (na+4)*NBPW, uid, gid, remote);
! #else REMOTEFS
!         getxfile(ip, &exdata.ex_exec, nc + (na+4)*NBPW, uid, gid);
! #endif REMOTEFS
! 
  	if (u.u_error) {
  badarg:
  		for (cc = 0; cc < nc; cc += CLSIZE*NBPG) {
***************
*** 376,381
  /*
   * Read in and set up memory for executed file.
   */
  getxfile(ip, ep, nargc, uid, gid)
  	register struct inode *ip;
  	register struct exec *ep;

--- 424,432 -----
  /*
   * Read in and set up memory for executed file.
   */
+ #ifdef REMOTEFS
+ getxfile(ip, ep, nargc, uid, gid, remote)
+ #else REMOTEFS
  getxfile(ip, ep, nargc, uid, gid)
  #endif REMOTEFS
  	register struct inode *ip;
***************
*** 377,382
   * Read in and set up memory for executed file.
   */
  getxfile(ip, ep, nargc, uid, gid)
  	register struct inode *ip;
  	register struct exec *ep;
  	int nargc, uid, gid;

--- 428,434 -----
  getxfile(ip, ep, nargc, uid, gid, remote)
  #else REMOTEFS
  getxfile(ip, ep, nargc, uid, gid)
+ #endif REMOTEFS
  	register struct inode *ip;
  	register struct exec *ep;
  	int nargc, uid, gid;
***************
*** 383,388
  {
  	size_t ts, ds, ids, uds, ss;
  	int pagi;
  
  	if (ep->a_magic == 0413)
  		pagi = SPAGI;

--- 435,443 -----
  {
  	size_t ts, ds, ids, uds, ss;
  	int pagi;
+ #ifdef REMOTEFS
+ 	int	oldtextsize;
+ #endif REMOTEFS
  
  	if (ep->a_magic == 0413)
  		pagi = SPAGI;
***************
*** 388,393
  		pagi = SPAGI;
  	else
  		pagi = 0;
  	if (ip->i_flag & IXMOD) {			/* XXX */
  		u.u_error = ETXTBSY;
  		goto bad;

--- 443,459 -----
  		pagi = SPAGI;
  	else
  		pagi = 0;
+ #ifdef REMOTEFS
+ 	if (remote >= 0) {
+ 		/*
+ 		 * Prevent xalloc() from making a shared or paged text.
+ 		 */
+ 		pagi = 0;
+ 		oldtextsize = ep->a_text;
+ 		ep->a_data += ep->a_text;
+ 		ep->a_text = 0;
+ 	}
+ #endif REMOTEFS
  	if (ip->i_flag & IXMOD) {			/* XXX */
  		u.u_error = ETXTBSY;
  		goto bad;
***************
*** 452,457
  	u.u_smap = u.u_csmap;
  	vgetvm(ts, ds, ss);
  
  	if (pagi == 0)
  		u.u_error =
  		    rdwri(UIO_READ, ip,

--- 518,528 -----
  	u.u_smap = u.u_csmap;
  	vgetvm(ts, ds, ss);
  
+ #ifdef REMOTEFS
+ 	if (remote >= 0)
+ 		u.u_error = remote_execread(remote, oldtextsize, ep);
+ 	else
+ #endif REMOTEFS
  	if (pagi == 0)
  		u.u_error =
  		    rdwri(UIO_READ, ip,
