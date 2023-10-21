#include "devfd.h"
#if NDEVFD > 0
#include	"../h/param.h"
#include	"../h/dir.h"
#include	"../h/file.h"
#include	"../h/inode.h"
#include	"../h/user.h"

/*
 * 4.2 BSD pseudo-device driver for the "devfd" device.  An attempt to
 * open this device returns a file descriptor which is a "duplicate" of
 * the open file descriptor corresponding to the minor device number of the
 * particular filedesc device opened.  For example, the device /dev/stdin
 * is a devfd device with minor device 0.  Opening /dev/stdin returns
 * a duplicate of file descriptor 0, equivalent to dup(0).  The open mode
 * must be the same as the mode of the file descriptor being duplicated,
 * except that a read/write file descriptor can be duped for just read or
 * just write.
 *
 * The idea of "/dev/stdin" originated in Bell Labs UNIX 8th Edition
 * (research version 8) and was described by Rob Pike at the Summer 1985
 * Usenix Conference in Portland.  We use the names "/dev/stdin",
 * "/dev/stdout", and "/dev/stderr" for minor devices 0,1, and 2, as well
 * as the names "/dev/fdXX" for XX ranging from 0 to NOFILE-1.
 * The protection mode of the device inodes should be 0666.
 *
 * "devfd" driver written 16-Jun-85 by Darwyn Peachey, Computer Science
 * Research Lab, University of Saskatchewan, Saskatoon, Canada S7N 0W0
 * Phone: (306) 966-4909   UUCP: ihnp4!sask!kimnovax!peachey
 */

devfdopen(dev, mode)
	dev_t dev;
	int mode;
{
	register int fd, oldfd;
	register struct file *fp;

	/*
	 * Luckily, the new fd is still in u.u_r.r_val1 from the call to
	 * falloc(kern_descrip.c) done by copen(ufs_syscalls.c).
	 */
	fd = u.u_r.r_val1;
	if (fd < 0 || fd >= NOFILE)	/* something terrible has happened */
		return (EINVAL);

	/* validate minor device number */
	oldfd = minor(dev);
	if (oldfd < 0 || oldfd >= NOFILE || fd == oldfd
	  || (fp = u.u_ofile[oldfd]) == NULL)
		return (EBADF);

	/* check open mode "mode" versus mode of descriptor being duped */
	if ((mode & (fp->f_flag & (FREAD|FWRITE))) != (mode & (FREAD|FWRITE)))
		return (EACCES);

	/* release the file structure allocated for the opening of devfd */
	if ((fp = u.u_ofile[fd]) != NULL) {	/* should always be true */
		u.u_ofile[fd] = NULL;
		if (fp->f_data)
			irele((struct inode *)(fp->f_data));
		fp->f_count = 0;
	}

	/* duplicate the oldfd */
	fp = u.u_ofile[fd] = u.u_ofile[oldfd];
	u.u_pofile[fd] = u.u_pofile[oldfd];
	fp->f_count++;
	return (0);
}
#endif
