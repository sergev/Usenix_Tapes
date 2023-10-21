The following changes implement local execution of an object file that
lives on another host.
***************
*** 28,33
  #include "../h/uio.h"
  #include "../h/nami.h"
  #include "../h/acct.h"
  
  #ifdef vax
  #include "../vax/mtpr.h"

--- 28,39 -----
  #include "../h/uio.h"
  #include "../h/nami.h"
  #include "../h/acct.h"
+ #ifdef REMOTEFS
+ /*
+  * needed only if EISREMOTE isn't in /usr/include/errno.h
+  */
+ #include "../h/errno.h"
+ #endif REMOTEFS
  
  #ifdef vax
  #include "../vax/mtpr.h"
***************
*** 57,62
  	int na, ne, ucp, ap, c;
  	int indir, uid, gid;
  	char *sharg;
  	struct inode *ip;
  	swblk_t bno;
  	char cfname[MAXCOMLEN + 1];

--- 63,72 -----
  	int na, ne, ucp, ap, c;
  	int indir, uid, gid;
  	char *sharg;
+ #ifdef REMOTEFS
+ 	struct inode *ip; /* have to take address */
+ 	int	remote = -1;
+ #else REMOTEFS
  	struct inode *ip;
  #endif REMOTEFS
  	swblk_t bno;
***************
*** 58,63
  	int indir, uid, gid;
  	char *sharg;
  	struct inode *ip;
  	swblk_t bno;
  	char cfname[MAXCOMLEN + 1];
  	char cfarg[SHSIZE];

--- 68,74 -----
  	int	remote = -1;
  #else REMOTEFS
  	struct inode *ip;
+ #endif REMOTEFS
  	swblk_t bno;
  	char cfname[MAXCOMLEN + 1];
  	char cfarg[SHSIZE];
***************
*** 64,69
  	int resid;
  
  	if ((ip = namei(uchar, LOOKUP, 1)) == NULL)
  		return;
  	bno = 0;
  	bp = 0;

--- 75,86 -----
  	int resid;
  
  	if ((ip = namei(uchar, LOOKUP, 1)) == NULL)
+ #ifdef REMOTEFS
+ 		if (u.u_error == EISREMOTE)
+ 			remote = remote_execinfo(&ip, &uid, &gid,
+ 				((struct execa *)u.u_ap)->fname);
+ 	if (u.u_error)
+ #endif REMOTEFS
  		return;
  	bno = 0;
  	bp = 0;
***************
*** 68,73
  	bno = 0;
  	bp = 0;
  	indir = 0;
  	uid = u.u_uid;
  	gid = u.u_gid;
  	if (ip->i_mode & ISUID)

--- 85,93 -----
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
*** 106,111
  	if (u.u_error)
  		goto bad;
  	u.u_count = resid;
  #ifndef lint
  	if (u.u_count > sizeof(u.u_exdata) - sizeof(u.u_exdata.Ux_A) &&
  	    u.u_exdata.ux_shell[0] != '#') {

--- 126,137 -----
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
  	if (u.u_count > sizeof(u.u_exdata) - sizeof(u.u_exdata.Ux_A) &&
  	    u.u_exdata.ux_shell[0] != '#') {
***************
*** 171,176
  		    (unsigned)(u.u_dent.d_namlen + 1));
  		cfname[MAXCOMLEN] = 0;
  		indir = 1;
  		iput(ip);
  		ip = namei(schar, LOOKUP, 1);
  		if (ip == NULL)

--- 197,205 -----
  		    (unsigned)(u.u_dent.d_namlen + 1));
  		cfname[MAXCOMLEN] = 0;
  		indir = 1;
+ #ifdef REMOTEFS
+ 		if (remote < 0)
+ #endif REMOTEFS
  		iput(ip);
  		ip = namei(schar, LOOKUP, 1);
  #ifdef REMOTEFS
***************
*** 173,178
  		indir = 1;
  		iput(ip);
  		ip = namei(schar, LOOKUP, 1);
  		if (ip == NULL)
  			return;
  		goto again;

--- 202,219 -----
  #endif REMOTEFS
  		iput(ip);
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
*** 175,180
  		ip = namei(schar, LOOKUP, 1);
  		if (ip == NULL)
  			return;
  		goto again;
  	}
  

--- 216,222 -----
  #else REMOTEFS
  		if (ip == NULL)
  			return;
+ #endif REMOTEFS
  		goto again;
  	}
  
***************
*** 244,249
  		bcopy((caddr_t)cfname, (caddr_t)u.u_dent.d_name,
  		    (unsigned)(u.u_dent.d_namlen + 1));
  	}
  	getxfile(ip, nc + (na+4)*NBPW, uid, gid);
  	if (u.u_error) {
  badarg:

--- 286,294 -----
  		bcopy((caddr_t)cfname, (caddr_t)u.u_dent.d_name,
  		    (unsigned)(u.u_dent.d_namlen + 1));
  	}
+ #ifdef REMOTEFS
+ 	getxfile(ip, nc + (na+4)*NBPW, uid, gid, remote);
+ #else REMOTEFS
  	getxfile(ip, nc + (na+4)*NBPW, uid, gid);
  #endif REMOTEFS
  	if (u.u_error) {
***************
*** 245,250
  		    (unsigned)(u.u_dent.d_namlen + 1));
  	}
  	getxfile(ip, nc + (na+4)*NBPW, uid, gid);
  	if (u.u_error) {
  badarg:
  		for (c = 0; c < nc; c += CLSIZE*NBPG) {

--- 290,296 -----
  	getxfile(ip, nc + (na+4)*NBPW, uid, gid, remote);
  #else REMOTEFS
  	getxfile(ip, nc + (na+4)*NBPW, uid, gid);
+ #endif REMOTEFS
  	if (u.u_error) {
  badarg:
  		for (c = 0; c < nc; c += CLSIZE*NBPG) {
***************
*** 303,308
  /*
   * Read in and set up memory for executed file.
   */
  getxfile(ip, nargc, uid, gid)
  	register struct inode *ip;
  	int nargc, uid, gid;

--- 349,357 -----
  /*
   * Read in and set up memory for executed file.
   */
+ #ifdef REMOTEFS
+ getxfile(ip, nargc, uid, gid, remote)
+ #else REMOTEFS
  getxfile(ip, nargc, uid, gid)
  #endif REMOTEFS
  	register struct inode *ip;
***************
*** 304,309
   * Read in and set up memory for executed file.
   */
  getxfile(ip, nargc, uid, gid)
  	register struct inode *ip;
  	int nargc, uid, gid;
  {

--- 353,359 -----
  getxfile(ip, nargc, uid, gid, remote)
  #else REMOTEFS
  getxfile(ip, nargc, uid, gid)
+ #endif REMOTEFS
  	register struct inode *ip;
  	int nargc, uid, gid;
  {
***************
*** 309,314
  {
  	register size_t ts, ds, ss;
  	int pagi;
  
  	if (u.u_exdata.ux_mag == 0413)
  		pagi = SPAGI;

--- 359,367 -----
  {
  	register size_t ts, ds, ss;
  	int pagi;
+ #ifdef REMOTEFS
+ 	int	oldtextsize;
+ #endif REMOTEFS
  
  	if (u.u_exdata.ux_mag == 0413)
  		pagi = SPAGI;
***************
*** 314,319
  		pagi = SPAGI;
  	else
  		pagi = 0;
  	if (u.u_exdata.ux_tsize!=0 && (ip->i_flag&ITEXT)==0 &&
  	    ip->i_count!=1) {
  		register struct file *fp;

--- 367,383 -----
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
  	if (u.u_exdata.ux_tsize!=0 && (ip->i_flag&ITEXT)==0 &&
  	    ip->i_count!=1) {
  		register struct file *fp;
***************
*** 370,375
  	u.u_smap = u.u_csmap;
  	vgetvm(ts, ds, ss);
  
  	if (pagi == 0)
  		u.u_error =
  		    rdwri(UIO_READ, ip,

--- 434,444 -----
  	u.u_smap = u.u_csmap;
  	vgetvm(ts, ds, ss);
  
+ #ifdef REMOTEFS
+ 	if (remote >= 0)
+ 		u.u_error = remote_execread(remote, oldtextsize);
+ 	else
+ #endif REMOTEFS
  	if (pagi == 0)
  		u.u_error =
  		    rdwri(UIO_READ, ip,
