/* char	sccsid[] = "@(#)logger.c 1.4 8/14/86"; */

/***************************************************
 *
 *	logger.c - log info about nbatcher
 *
 *	R.J. Esposito
 *	Bell of Penna.
 *	June 1986
 *
 **************************************************/

#include <stdio.h>
#include <time.h>
#include "nbatcher.h"

log_it (bytes)
long	bytes;
{
	struct tm	*localtime(), *tp;
	long	time(), clock;
	char	logfile[80], buf[BUFSIZ];
	char	pbuf[BUFSIZ];

	sprintf (logfile, "%s/%s", LIBDIR, "nbatcher.log");
	if (log == NULL) {
		if ((log=fopen(logfile, "a")) == NULL)
		   fprintf (stderr, "\ncan't append to logfile\n");
	}

	if (log != NULL)
		rewind (log, 0L, 2);	/* just incase */

	clock = time ((long *)0);
	tp = localtime (&clock);
	sprintf (buf, "%.2d/%.2d %.2d:%.2d %s: %d %s batched, %d %s queued\n",
	   tp->tm_mon+1, tp->tm_mday, tp->tm_hour, tp->tm_min, ep.site,
	   fcnt, (fcnt==1 ? "file" : "files"), scnt,
	   (scnt==1 ? "file" : "files"));

	if (bytes)
	   sprintf (pbuf, "%s\tmax bytes reached.  UUCP bytes was %ld, byte count = %ld\n",
			buf, n_bytes, bytes);
	else
		sprintf (pbuf, "%s", buf);

	if (vflg)
		fprintf (stdout, "%s",pbuf);

	if (log != NULL)
		fputs (pbuf, log);

}
