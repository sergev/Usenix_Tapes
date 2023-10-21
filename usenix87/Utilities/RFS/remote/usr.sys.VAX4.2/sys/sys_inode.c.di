This change repairs a semantic change in ino_close() that assumes
the only other file descriptor type is DTYPE_SOCKET.  If patch tells
you
	Reversed (or previously applied) patch detected!
Then you already have the fix and you must undo what patch does.

***************
*** 351,357
  	 * two different inodes.
  	 */
  	for (fp = file; fp < fileNFILE; fp++) {
! 		if (fp->f_type == DTYPE_SOCKET)		/* XXX */
  			continue;
  		if (fp->f_count && (ip = (struct inode *)fp->f_data) &&
  		    ip->i_rdev == dev && (ip->i_mode&IFMT) == mode)

--- 351,357 -----
  	 * two different inodes.
  	 */
  	for (fp = file; fp < fileNFILE; fp++) {
! 		if (fp->f_type != DTYPE_INODE)	/* XXX */
  			continue;
  		if (fp->f_count && (ip = (struct inode *)fp->f_data) &&
  		    ip->i_rdev == dev && (ip->i_mode&IFMT) == mode)
