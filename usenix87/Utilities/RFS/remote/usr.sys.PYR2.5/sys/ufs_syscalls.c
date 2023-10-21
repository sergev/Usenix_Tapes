These changes modify chdirec(), which is called by chroot() and chdir(),
so that you can be allowed to do a chdir() to a remote mount point.
In addition, the changes ensure that we adjust internal pointers when doing
a chdir() OUT of a remote mount point.
***************
*** 104,109
  	register struct inode **ipp;
  {
  	register struct inode *ip;
  
  	ip = namei(uchar, LOOKUP, 1);
  	if (ip == NULL)

--- 104,112 -----
  	register struct inode **ipp;
  {
  	register struct inode *ip;
+ #ifdef REMOTEFS
+ 	int	i;
+ #endif REMOTEFS
  
  	ip = namei(uchar, LOOKUP, 1);
  	if (ip == NULL)
***************
*** 109,114
  	if (ip == NULL)
  		return;
  	if ((ip->i_mode&IFMT) != IFDIR) {
  		u.u_error = ENOTDIR;
  		goto bad;
  	}

--- 112,123 -----
  	if (ip == NULL)
  		return;
  	if ((ip->i_mode&IFMT) != IFDIR) {
+ #ifdef REMOTEFS
+ 		if (rmt_hostdir(ip, &i) != NULL)
+ 			u.u_error = remotechdir(i);
+ 		else
+ 			u.u_error = ENOTDIR;
+ #else REMOTEFS
  		u.u_error = ENOTDIR;
  #endif REMOTEFS
  		goto bad;
***************
*** 110,115
  		return;
  	if ((ip->i_mode&IFMT) != IFDIR) {
  		u.u_error = ENOTDIR;
  		goto bad;
  	}
  	if (access(ip, IEXEC))

--- 119,125 -----
  			u.u_error = ENOTDIR;
  #else REMOTEFS
  		u.u_error = ENOTDIR;
+ #endif REMOTEFS
  		goto bad;
  	}
  	if (access(ip, IEXEC))
***************
*** 114,119
  	}
  	if (access(ip, IEXEC))
  		goto bad;
  	iunlock(ip);
  	if (*ipp)
  		irele(*ipp);

--- 124,132 -----
  	}
  	if (access(ip, IEXEC))
  		goto bad;
+ #ifdef REMOTEFS
+ 	remotechdir(-1);
+ #endif REMOTEFS
  	iunlock(ip);
  	if (*ipp)
  		irele(*ipp);
