These changes modify chdirec(), which is called by chroot() and chdir(),
so that you can be allowed to do a chdir() to a remote mount point.
In addition, the changes ensure that we adjust internal pointers when doing
a chdir() OUT of a remote mount point.

***************
*** 67,72
  chdirec(ipp)
  	register struct inode **ipp;
  {
  	register struct inode *ip;
  	struct a {
  		char	*fname;

--- 67,75 -----
  chdirec(ipp)
  	register struct inode **ipp;
  {
+ #ifdef REMOTEFS
+ 	int	i;
+ #endif REMOTEFS
  	register struct inode *ip;
  	struct a {
  		char	*fname;
***************
*** 76,81
  	if (ip == NULL)
  		return;
  	if ((ip->i_mode&IFMT) != IFDIR) {
  		u.u_error = ENOTDIR;
  		goto bad;
  	}

--- 79,89 -----
  	if (ip == NULL)
  		return;
  	if ((ip->i_mode&IFMT) != IFDIR) {
+ #ifdef REMOTEFS
+ 		if (rmt_hostdir(ip, &i) != NULL)
+ 			u.u_error = remotechdir(i);
+ 		else
+ #endif REMOTEFS
  		u.u_error = ENOTDIR;
  		goto bad;
  	}
***************
*** 81,86
  	}
  	if (access(ip, IEXEC))
  		goto bad;
  	iunlock(ip);
  	if (*ipp)
  		irele(*ipp);

--- 89,97 -----
  	}
  	if (access(ip, IEXEC))
  		goto bad;
+ #ifdef REMOTEFS
+ 	remotechdir(-1);
+ #endif REMOTEFS
  	iunlock(ip);
  	if (*ipp)
  		irele(*ipp);
