/* char	sccsid[] = "@(#)checkwork.c 1.4 8/14/86"; */

/************************************************************
 *
 *	checkwork.c - look to see if there's any work
 *		      to do for a site.
 *
 *	R.J. Esposito
 *	Bell of Penna.
 *	June 1986
 *
 ************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/dir.h>
#include <ctype.h>
#include <time.h>
#include "nbatcher.h"

work_to_do ()
{
	register char	*p;
	struct tm	*localtime(), *tp;
	struct stat	st;
	char	buf[BUFSIZ];
	long	time(), clock;
	int	hour;
	short	num, upper, lower;

	sprintf (buf, "%s/%s", BATCHDIR, ep.site);

	if (stat(buf, &st) < 0)
		xerror ("bad stat on %s\n", buf);

	/* if the size of the batch file is
	   zero, return FALSE
	*/

	if (st.st_size == 0)
		return (FALSE);

	/* now see if it time to do anything */

	clock = time ((long *)0);
	tp = localtime (&clock);
	hour = tp->tm_hour;

	p = (char *) ep.hour;

	if (*p == '*')		/* match any hour */
		return (check_uucp());

	if (strncmp(p, "off", 3) == 0)	/* just what it says, off */
		return (FALSE);

	/* parse thru hour field to see if
	   this is the hour to do work */

	num = 0;
	do {
		num = num*10 + (*p - '0');
	} while (isdigit(*++p));
	if (num == hour)
		return (check_uucp());

	if (*p == '-') {
		lower = num;
		p++;
		num = 0;
		do {
			num = num*10 + (*p - '0');
		} while (isdigit(*++p));
		upper = num;

		if (lower < upper) {	/* normal hour range */
			if (hour >= lower && hour <= upper)
				return (check_uucp());
		} else if (lower > upper) {	/* 24 hr. cycle thru */
			if (hour >= lower || hour <= upper)
				return (TRUE);
		} else
			return (FALSE);
	}

	if (*p == ',') {
		p++;
		while (*p) {
			num = 0;
			do {
				num = num*10 + (*p - '0');
			} while (isdigit(*++p));
			if (num == hour)
				return (check_uucp());
			p++;
		}
	}

	return (FALSE);
}

/*	If check_uucp cannot find the remote site
 *	directory, just bypass the byte counting
 *	routine.  This is necessary because the
 *	uucpcleanup daemon, on some sites, removes
 *	the site directory when there's nothing there.
 */

check_uucp()
{
	struct utsname	utsn;
	struct direct	dp;
	struct stat	st;
	FILE	*dfp;
	char	u_name[9], buf[80];
	short	prefix_len;

	if (uname(&utsn) < 0)
		xerror ("can't get local nodename\n");

	sprintf (buf, "%s/%s", UUCPDIR, ep.site);
	if (chdir(buf) < 0) {
	   fprintf (stderr, "\nnbatcher: can't chdir to %s - bypassing UUCP check\n", buf);
	   return (TRUE);
	}

	if ((dfp=fopen(".", "r")) == NULL) {
	   fprintf (stderr, "\nnbatcher: fopen error on %s - bypassing UUCP check\n", UUCPDIR);
	   return (TRUE);
	}

	sprintf (buf, "D.%s", utsn.nodename);
	prefix_len = (short) strlen(buf);
	n_bytes = 0;
	while ((fread((char *)&dp, sizeof(dp), 1, dfp)) == 1) {
		if (dp.d_ino == 0 || dp.d_name[0] == '.')
			continue;
		if (strncmp(dp.d_name, buf, prefix_len))
			continue;
		if (stat(dp.d_name, &st) < 0) {
		  fprintf (stderr, "\nnbatcher: bad stat on UUCP_file %s - bypassing\n", dp.d_name);
		  continue;
		}
		n_bytes += st.st_size;
		if (n_bytes > ep.m_bytes) {
			fclose (dfp);
			return (FALSE);
		}
	}
	fclose (dfp);
	if (chdir(LIBDIR) < 0)
		xerror ("can't chdir back to %s\n", LIBDIR);

	return (TRUE);
}
