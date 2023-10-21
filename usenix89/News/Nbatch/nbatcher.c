/* char	sccsid[] = "@(#)nbatcher.c 1.4 8/14/86"; */

/****************************************************
 *
 *	nbatcher.c - where it really happens.
 *
 *	R.J. Esposito
 *	Bell of Penna.
 *	June 1986
 *
 ***************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "nbatcher.h"

batch_it ()
{
	struct stat	st;
	FILE	*bfp, *afp;
	char	fbuf[BUFSIZ], lckfile[40];
	char	tbuf[80];
	short	count;
	int	c;

	if (chdir(BATCHDIR) < 0)
		xerror ("can't chdir to %s\n", BATCHDIR);

	/* we create a lock file for two purposes,
	   first to make sure a previous nbatcher
	   didn't blowup and leave the lock file
	   laying around, and second to put the
	   remaining news article filenames when
	   we go over the max UUCP bytes and there's
	   still files remaining for batching.
	*/

	sprintf (lckfile, ".%s.lock", ep.site);
	if (!access(lckfile, 0))
		xerror ("lockfile already exists for %s\n", ep.site);

	if ((lfp=fopen(lckfile, "w")) == NULL)
		xerror ("can't create lockfile for %s\n", ep.site);

	/* now that we've locked ourselves for this site,
	   lets carry on */

	if ((bfp=fopen(ep.site, "r")) == NULL)
		xerror ("can't open %s/%s for reading\n", BATCHDIR, ep.site);

	if (tfile == NULL) {
		tfile = mktemp("/tmp/bnewsXXXXXX");
		if ((tfp=fopen(tfile, "w")) == NULL)
			xerror ("can't open %s for writing\n", tfile);
	}

	count = fcnt = scnt = 0;
	cu_bytes = 0;
	while ((fgets(fbuf, sizeof(fbuf), bfp)) != NULL) {
		fbuf[strlen(fbuf)-1] = '\0';	/* remove the newline */
		if ((afp=fopen(fbuf, "r")) == NULL) {
		   fprintf (stderr, "\nbypassing article %s: can't read it\n",
			    fbuf);
			continue;
		}
		if (fstat(fileno(afp), &st) < 0)
			xerror ("fstat failed on %s\n", fbuf);

		cu_bytes += st.st_size;

		/* if the max byte count is exceeded,
		   save the remaining files for later */

		if ((cu_bytes + n_bytes) > ep.m_bytes) {
			fprintf (lfp, "%s\n", fbuf); /* put the '\n' back */
			while ((fgets(fbuf, sizeof(fbuf), bfp)) != NULL)
				fputs (fbuf, lfp);
			fclose (bfp);
			fclose (lfp);
			fclose (afp);
			unlink (ep.site);
			if (link(lckfile, ep.site) < 0)
			   xerror ("can't link lockfile to %s\n", ep.site);
			unlink (lckfile);
			chown (ep.site, NEWSUID, NEWSGID);
			if (count)
				spoolit ();
			if (cu_bytes - st.st_size)
				log_it (cu_bytes - st.st_size);

			return;
		}
		sprintf (tbuf, "#! rnews %ld\n", st.st_size);
		fputs (tbuf, tfp);
		while ((c=getc(afp)) != EOF)
			putc (c, tfp);
		fclose (afp);

		if (++count == nfiles) {
			spoolit ();
			count = 0;
		}
		fcnt++;
	}

	/* The final spool if lest than nfiles
	   is encountered.  Then zero out the
	   batchfile and unlink the lock file */

	spoolit ();
	fclose (bfp);
	fclose (lfp);
	close (creat(ep.site, 0664));
	chown (ep.site, NEWSUID, NEWSGID);
	unlink (lckfile);

	/* here we log what we've done, and
	   if vflg is set, a copy to stdout
	   as well */

	log_it (0);
	if (chdir(LIBDIR) < 0)
		xerror ("can't chdir back to %s\n", LIBDIR);

}

spoolit ()
{
	struct stat	st;
	char	cmd[BUFSIZ], cfile[80];
	FILE	*pfp;
	int	c;

	fclose (tfp);
	stat (tfile, &st);

	/* if for some reason the temp file
	   is zero, just return */

	if (st.st_size == 0)
		return;

	/* if ep.c_bits is set use COMPRESS to compress
	   the temp file first
	*/

	if (ep.c_bits) {
		sprintf (cmd, "%s -b%d %s", COMPRESS, ep.c_bits, tfile);
		if (system(cmd) != 0)
			xerror ("system(%s) failed\n", cmd);

		strcpy (cfile, tfile);
		strcat (cfile, ".Z");
		if ((tfp=fopen(cfile, "r")) == NULL)
			xerror ("can't open %s for reading\n", cfile);

		/* if ep.command has a specific command
		   for UUCP spooling, use it.  If not,
		   use UUX.
		*/

		if (ep.command[0] != '\0')
			strcpy (cmd, ep.command);
		else
			sprintf (cmd, "%s %s!rnews", UUX, ep.site);

		/* now popen the command for writing
		   and send it the contents of tempfile */

		if ((pfp=popen(cmd, "w")) == NULL)
			xerror ("popen failed on %s\n", cmd);

		/********************************************
		 * for version 2.10.3 and above,
		 * prepend `#! cunbatch'.
		 *
		 * NOTE: The remote site MUST be able to
		 *       except this format, or it will
		 *       be lost!!!
		 *******************************************/

		fputs ("#! cunbatch\n", pfp);
		while ((c=getc(tfp)) !=  EOF)
			putc (c, pfp);

		pclose (pfp);
		fclose (tfp);
		unlink (cfile);
	} else {			/* regular batching here */
		if ((tfp=fopen(tfile, "r")) == NULL)
			xerror ("can't open %s for reading\n", tfile);

		/* if ep.command has a specific command
		   for UUCP spooling, use it.  If not,
		   use UUX.
		*/

		if (ep.command[0] != '\0')
			strcpy (cmd, ep.command);
		else
			sprintf (cmd, "%s %s!rnews", UUX, ep.site);

		if ((pfp=popen(cmd, "w")) == NULL)
			xerror ("popen failed on %s\n", cmd);

		while ((c=getc(tfp)) != EOF)
			putc (c, pfp);

		pclose (pfp);
		fclose (tfp);
	}
	if ((tfp=fopen(tfile, "w")) == NULL)
		xerror ("can't re-open %s\n", tfile);

	scnt++;
}
