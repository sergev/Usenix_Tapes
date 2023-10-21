char	sccsid[] = "nbatcher 1.4 8/14/86";

/********************************************
 *
 *	main.c - for nbatcher
 *
 *	R.J. Esposito
 *	Bell of Penna.
 *	June 1986
 *
 ********************************************/

#include <stdio.h>
#include "nbatcher.h"

main()
{
	int	uid, nowork;
	FILE	*cfp;
	char	fbuf[BUFSIZ];

	uid = getuid();

	if (uid && uid != NEWSUID)
		xerror ("permission denied - not NEWSUSER\n");

	if (chdir(LIBDIR) < 0)
		xerror ("can't chdir to %s\n", LIBDIR);

	if ((cfp=fopen("nbatcher.ctl", "r")) == NULL)
		xerror ("no `batcher.ctl' file found\n");

	if (isatty(0)) {
		vflg = TRUE;
		(void) fprintf(stderr, "%s\n", sccsid);
	}

	nowork = TRUE;
	while ((fgets(fbuf, sizeof(fbuf), cfp)) != NULL) {
		if (fbuf[0] == '*' || fbuf[0] == '\n')
			continue;
		parse_entry (fbuf);
		if (!work_to_do())
			continue;
		batch_it ();
		nowork = FALSE;
	}

	fclose (cfp);
	fclose (tfp);
	unlink (tfile);
	if (vflg == TRUE && nowork == TRUE)
		(void) fprintf(stderr, "no work to do\n");
	exit (0);
}
	
