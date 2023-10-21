This change repairs a semantic change in ino_close() that assumes
the only other file descriptor type is DTYPE_SOCKET.  If patch tells
you
	Reversed (or previously applied) patch detected!
Then you already have the fix and you must undo what patch does.
***************
*** 516,522
  	for (ffp = file; ffp < fileNFILE; ffp++) {
  		if (ffp == fp)
  			continue;
! 		if (ffp->f_type == DTYPE_SOCKET)		/* XXX */
  			continue;
  		if (ffp->f_count && (ip = (struct inode *)ffp->f_data) &&
  		    ip->i_rdev == dev && (ip->i_mode&IFMT) == mode)

--- 516,522 -----
  	for (ffp = file; ffp < fileNFILE; ffp++) {
  		if (ffp == fp)
  			continue;
! 		if (ffp->f_type != DTYPE_INODE)	/* semantic fix (toddb@du) */
  			continue;
  		if (ffp->f_count && (ip = (struct inode *)ffp->f_data) &&
  		    ip->i_rdev == dev && (ip->i_mode&IFMT) == mode)
