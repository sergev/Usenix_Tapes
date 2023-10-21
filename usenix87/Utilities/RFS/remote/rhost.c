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
 * $Log:	rhost.c,v $
 * Revision 2.0  85/12/07  18:21:52  toddb
 * First public release.
 * 
 */
static char	*rcsid = "$Header: rhost.c,v 2.0 85/12/07 18:21:52 toddb Rel $";
#include	<stdio.h>
#include	"server.h"

static char	*rhostpath;
static FILE	*fd;
static rhost	rh;

setrhost(path)
	register char	*path;
{
	extern int	errno;

	if ((fd = fopen(path, "r")) != 0)
		debug2("rhost %s\n", path);
	errno = 0;
}

rhost *getrhostent(buf)
	register char	*buf;
{
	register char	*p;

	while (1)
	{
		if (fd == NULL || fgets(buf, BUFSIZ, fd) == NULL)
			return(NULL);

		/*
		 * assign the first token to rh_host and then look for the
		 * second token on the line.  If there is none, then
		 * don't return this entry because we can never map
		 * a remote user id name of "" to anything meaningful.
		 */
		rh.rh_host = buf;
		for (p=buf; *p && *p != ' ' && *p != '\n'; p++) ;
		if (*p == '\n' || *p == '\0')
			continue;

		/*
		 * remove the newline on the end
		 */
		rh.rh_user = p+1;
		*p = '\0';
		for (p++; *p && *p != ' ' && *p != '\n'; p++) ;
		*p = '\0';
		break;
	}
	return(&rh);
}

endrhost()
{
	if (fd)
	{
		fclose(fd);
		fd = NULL;
	}
}
