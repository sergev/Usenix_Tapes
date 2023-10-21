#
/*
 *	getpw(uid, buffer);
 *
 * This routine looks through the special seek-format password
 * file for the line of user identification associated with a
 * given uid.  This version uses a seek based upon the uid to
 * locate the desired line much faster than a line-by-line search.
 *
 */

#define	SEEK	"/etc/u-seek"

int fd_FD_	-1;

getpw(uid, buffer)
int uid;
char buffer[];
{
	char buf[64];
	int k;

	if(fd_FD_ == -1) if ((fd_FD_=open(SEEK,0)) < 0){
		fd_FD_=0;
		return(-1);
	}

	if (seek(fd_FD_, uid/8, 3) < 0) return(-1);
	if (seek(fd_FD_, (uid%8)*64, 1) < 0) return(-1);
	k = read(fd_FD_, buf, 64);
	if (k < 64) return(-1);
	if (buf[0] == 0) return(-1);
	for (k=0; (buffer[k] = buf[k]) != 0; k++);
	return(0);
}
