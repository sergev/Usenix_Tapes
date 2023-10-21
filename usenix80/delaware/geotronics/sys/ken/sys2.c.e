201a
tell()
{
	register struct file *fp;

	if (fp = getf(u.u_ar0[R0])) {
		u.u_ar0[R0] = fp->f_offset[0];
		u.u_ar0[R1] = fp->f_offset[1];
	}
}

/*
 * link system call
 */
.
200c
 * Tell -- discover offset of file R/W pointer
.
89c
			goto err;
.
86a
		if ( (u.u_cdir->i_nlink == 0) &&
		     (fubyte( u.u_arg[0] ) != '/') )  {
			u.u_error = ENOENT;
    err:		iput( u.u_pdir );    /* namei left parent dir locked */
			return;
		}
.
1a
/*
 * sys2.c - more system calls
 *
 *	modified 03-Jun-1980 by D A Gwyn:
 *	1) added "tell" system call;
 *	2) fixed "creat" bugs - orphaned files and maknode failure.
 */
.
w
q
