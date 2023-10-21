These changes are the primary hook into the operating system for detecting
a "remote" file.
***************
*** 149,154
  	int isdotdot;			/* != 0 if current name is ".." */
  	int flag;			/* op ie, LOOKUP, CREATE, or DELETE */
  	off_t enduseful;		/* pointer past last used dir slot */
  
  	lockparent = ndp->ni_nameiop & LOCKPARENT;
  	docache = (ndp->ni_nameiop & NOCACHE) ^ NOCACHE;

--- 149,157 -----
  	int isdotdot;			/* != 0 if current name is ".." */
  	int flag;			/* op ie, LOOKUP, CREATE, or DELETE */
  	off_t enduseful;		/* pointer past last used dir slot */
+ #ifdef REMOTEFS
+ 	int remote;
+ #endif
  
  	lockparent = ndp->ni_nameiop & LOCKPARENT;
  	docache = (ndp->ni_nameiop & NOCACHE) ^ NOCACHE;
***************
*** 202,207
  	 * Check accessiblity of directory.
  	 */
  	if ((dp->i_mode&IFMT) != IFDIR) {
  		u.u_error = ENOTDIR;
  		goto bad;
  	}

--- 205,226 -----
  	 * Check accessiblity of directory.
  	 */
  	if ((dp->i_mode&IFMT) != IFDIR) {
+ #ifdef REMOTEFS
+ 		remote = isremote(dp, cp, nbp->b_un.b_addr);
+ 		/*
+ 		 * If it is really local, then start again at the root.
+ 		 */
+ 		if (remote < 0) {
+ 			iput(dp);
+ 			dp = rootdir;
+ 			ILOCK(dp);
+ 			dp->i_count++;
+ 			fs = dp->i_fs;
+ 			cp = nbp->b_un.b_addr;
+ 			goto dirloop2;
+ 		}
+ 		else if (! remote)
+ #endif REMOTEFS
  		u.u_error = ENOTDIR;
  		goto bad;
  	}
***************
*** 642,647
  					u.u_error = EPERM;
  					goto bad;
  				}
  			}
  		}
  		nbp->av_forw = freenamebuf;

--- 661,677 -----
  					u.u_error = EPERM;
  					goto bad;
  				}
+ #ifdef REMOTEFS
+ 				/*
+ 				 * don't allow anyone to remove a remote mount
+ 				 * point.
+ 				 */
+ 				if (rmt_host(dp, &i)) {
+ 					iput(ndp->ni_pdir);
+ 					u.u_error = EBUSY;
+ 					goto bad;
+ 				}
+ #endif REMOTEFS
  			}
  		}
  		nbp->av_forw = freenamebuf;
