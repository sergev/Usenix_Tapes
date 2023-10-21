/*
 * add the following procedure to pipe.c
 */

empty() 	/* rand addition: see whether pipe or tty is empty */
{	register *fp, *ip;
	int v[3];
	fp = getf(u.u_ar0[R0]); 	/* get file ptr; copied from rdwr */
	if (fp != NULL && fp->f_flag&FREAD)
	{	ip = fp->f_inode;		/* from readp */
		if (fp->f_flag&FPIPE)
		{	/* test from readp */
		 u.u_ar0[R0] = (fp->f_offset[1] == ip->i_size1 /*char count*/
	      /* rand:bobg */  && !(ip->i_flag & IEOFP) /* no EOF waiting */
			       && ip->i_count >=2   /* check other end */
			       && (ip->i_mode&IWRITE)==0); /* more to write?*/
			return;
		}
		else
		{	/* relies on rand change to return 'non-empty flag'
			 * in u.u_arg[4] */
			sgtty(v);
			if (u.u_error == 0)
			{       u.u_ar0[R0] = u.u_arg[4];
				return;
			}
		}
	}
	u.u_error = EBADF;
}
