#ifndef lint
static char RCSid[] = "$Header: ulimit.c,v 1.4 85/12/09 13:02:22 arnold Exp $";
#endif

/*
 * $Log:	ulimit.c,v $
 * Revision 1.4  85/12/09  13:02:22  arnold
 * changed the declaration of limit to keep lint happy
 * 
 * Revision 1.3  85/12/03  16:18:42  arnold
 * Fixed original code to use symbolic constants from the header files
 * Added many more commands for use with the ksh style ulimit command,
 * for the BSD limits.
 * 
 * Revision 1.2  85/11/18  10:04:46  arnold
 * Updated from bare S5R2 to my first release
 * 
 * Revision 1.1  85/11/15  12:19:05  arnold
 * Initial revision
 * 
 * 
 */

/*
	ulimit -- system call emulation for Bourne shell on 4.2BSD

	last edit:	22-Aug-1983	D A Gwyn
*/

#include	<sys/time.h>
#include	<sys/resource.h>
#include	<errno.h>

extern int	getrlimit(), setrlimit();
extern int	errno;

#define BLOCK	512L	/* all these things are done in "blocks" */

long
ulimit (cmd, newlimit)
int	cmd;			/* subcommand */
long	newlimit;		/* desired new limit */
{
	struct rlimit limit;	/* data being gotten/set */

	switch (cmd) {
	case 1: 			/* get file size limit */
		if (getrlimit (RLIMIT_FSIZE, &limit) != 0 )
			return -1L;	/* errno is already set */
		return limit.rlim_max / BLOCK;

	case 2: 			/* set file size limit */
		limit.rlim_cur = limit.rlim_max = newlimit * BLOCK;
		return setrlimit (RLIMIT_FSIZE, &limit);

	case 3: 			/* get maximum break value */
		if (getrlimit (RLIMIT_DATA, &limit) != 0)
			return -1L;	/* errno is already set */
		return limit.rlim_max;

	case 6:		/* get core file size limit */
		if (getrlimit (RLIMIT_CORE, &limit) != 0)
			return -1L;	/* errno already set */
		return limit.rlim_max / BLOCK;

	case 7:		/* set core file size limit */
		limit.rlim_cur = limit.rlim_max = newlimit * BLOCK;
		return setrlimit (RLIMIT_CORE, &limit);

	case 8:		/* get data segment size limit */
		if (getrlimit (RLIMIT_DATA, &limit) != 0)
			return -1L;	/* errno already set */
		return limit.rlim_max / BLOCK;

	case 9:		/* set data segment size limit */
		limit.rlim_cur = limit.rlim_max = newlimit * BLOCK;
		return setrlimit (RLIMIT_DATA, &limit);

	case 10:	/* get physical memory limit */
		if (getrlimit (RLIMIT_RSS, &limit) != 0)
			return -1L;	/* errno already set */
		return limit.rlim_max / BLOCK;

	case 11:	/* set physical memory limit */
		limit.rlim_cur = newlimit * BLOCK;
		return setrlimit (RLIMIT_RSS, &limit);

	case 12:	/* get cpu time limit */
		if (getrlimit (RLIMIT_CPU, &limit) != 0)
			return -1L;	/* errno already set */
		return limit.rlim_max / 1000L;	/* system uses milliseconds */

	case 13:	/* set cpu time limit */
		limit.rlim_cur = limit.rlim_max = newlimit * 1000L;
		return setrlimit (RLIMIT_CPU, &limit);

	case 4:
	case 5:
		/* These two are for getting and setting the pipe */
		/* size under UNIX/RT -- not applicable here */
		/* so fall thru and complain */
	default:
		errno = EINVAL;
		return -1L;
	}
}
