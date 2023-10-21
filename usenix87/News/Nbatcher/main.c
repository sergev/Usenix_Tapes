/*
 *
 *	main.c - for nbatcher
 *
 *	R.J. Esposito
 *	Bell of Penna.
 *	June 1986
 *
 */

#include <stdio.h>
#include "nbatcher.h"

main()
{
	int	uid;
	FILE	*cfp;
	char	fbuf[BUFSIZ];

	uid = getuid();

	if (uid && uid != NEWSUID)
		xerror ("permission denied - not NEWSUSER\n");

	if (chdir(LIBDIR) < 0)
		xerror ("can't chdir to %s\n", LIBDIR);

	if ((cfp=fopen("nbatcher.ctl", "r")) == NULL)
		xerror ("no `batcher.ctl' file found\n");

	if (isatty(0))
		vflg = TRUE;

	while ((fgets(fbuf, sizeof(fbuf), cfp)) != NULL) {
		if (fbuf[0] == '*' || fbuf[0] == '\n')
			continue;
		parse_entry (fbuf);
		if (!work_to_do())
			continue;
		batch_it ();
	}

	fclose (cfp);
	unlink (tfile);
	exit (0);
}
	
