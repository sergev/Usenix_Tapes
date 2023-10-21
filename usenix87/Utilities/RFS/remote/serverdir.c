/*
 * Copyright 1985, Todd Brunhoff.
 *
 * This software was written at Tektronix Computer Research Laboratories
 * as partial fulfillment of a Master's degree at the University of Denver.
 * This is not Tektronix proprietary software and should not be
 * confused with any software product sold by Tektronix.  No warranty is
 * expressed or implied on the reliability of this software; the author,
 * the University of Denver, and Tektronix, inc. accept no liability for
 * any damage done directly or indirectly by this software.  This software
 * may be copied, modified or used in any way, without fee, provided this
 * notice remains an unaltered part of the software.
 *
 * $Log:	serverdir.c,v $
 * Revision 2.0  85/12/07  18:22:28  toddb
 * First public release.
 * 
 */
static char	*rcsid = "$Header: serverdir.c,v 2.0 85/12/07 18:22:28 toddb Rel $";
#include	"server.h"
#include	<sys/dir.h>
#include	<sys/stat.h>
#include	<errno.h>

extern hosts	*host;
extern long	errno;
extern char	byteorder[];
extern struct stat	filetypes[];

/*
 * Check to see type open file type... we may have to massage input
 * it if the user wants to read this file descriptor and it is a directory.
 */
checkfiletype(fd)
	register int    fd;
{
	struct stat	statb, *statp = &statb;

	if (fd < 0)
		return;
	fstat(fd, statp);
	filetypes[ fd ] = statb;
}

/*
 * If byte-ordering is different between this machine and our client,
 * the directories must be massaged into the right byte order.
 */
fixdir(fd, buf, size)
	register long	size,
			fd;
	register char	*buf;
{
	register struct direct *dirp;
	register char	*next, *last;
	register u_char	*clientorder = host->h_byteorder;
	short		fixshort();

	if (size < 0)
		return(errno);
	if (fd >= NOFILE || (filetypes[fd].st_mode & S_IFDIR) == 0)
		return(0);

	/*
	 * we don't know this client's byteorder...  can't do it right
	 */
	if (!host->h_mounted)
		return(EIO);
	dirp = (struct direct *)buf;
	last = buf;
	debug7("nuxi directory entry buf=0%x, size=%d, end @%x\n",
		buf, size, buf+size);
	while(last < buf + size && dirp->d_reclen)
	{
		dirp = (struct direct *)last;
		next = last + dirp->d_reclen;

		debug7("dir @0x%x (next+%d @0x%x): %x %x %x %s -->",
			last, dirp->d_reclen, next,
			dirp->d_ino,
			(unsigned)dirp->d_reclen,
			(unsigned)dirp->d_namlen,
			dirp->d_name);
		dirp->d_ino = fixlong(clientorder, &dirp->d_ino);
		dirp->d_reclen = fixshort(clientorder, &dirp->d_reclen);
		dirp->d_namlen = fixshort(clientorder, &dirp->d_namlen);
		debug7(" %x %x %x %s\n",
			dirp->d_ino,
			(unsigned)dirp->d_reclen,
			(unsigned)dirp->d_namlen,
			dirp->d_name);
		last = next;
	}
	return(0);
}

fixlong(clto, from)
	register char	*clto,	/* clients byte order */
			*from;	/* data to be fixed */
{
	register char	*srvo,	/* server's byte order */
			*to;
	long		result;

	to = (char *)&result;
	srvo = byteorder;
	to[ clto[0] ] = from[ srvo[0] ];
	to[ clto[1] ] = from[ srvo[1] ];
	to[ clto[2] ] = from[ srvo[2] ];
	to[ clto[3] ] = from[ srvo[3] ];
	return(result);
}

short fixshort(clto, from)
	register char	*clto,	/* clients byte order */
			*from;	/* data to be fixed */
{
	register char	*srvo,	/* server's byte order */
			*to;
	short		result;

	to = (char *)&result;
	srvo = byteorder;
	to[ clto[0]&0x1 ] = from[ srvo[0]&0x1 ];
	to[ clto[1]&0x1 ] = from[ srvo[1]&0x1 ];
	return(result);
}
