#

/*LINTLIBRARY*/

#include "stdio.h"

#ifndef lint
#ifndef NOID
static char	sccsid[] = "@(#)mkdir.c	7.5";
#endif /* !NOID */
#endif /* !lint */

extern FILE *	popen();

static
quote(name, fp)
register char *	name;
register FILE *	fp;
{
	register int	c;

	(void) fputc('\'', fp);
	if (name != NULL)
		while ((c = *name++) != '\0')
			if (c == '\'')
				(void) fprintf(fp, "'\\''");
			else	(void) fputc(c, fp);
	(void) fputc('\'', fp);
}

mkdir(name, mode)
char *	name;
{
	register FILE *	fp;
	register int	oops;

	if ((fp = popen("sh", "w")) == NULL)
		return -1;
	(void) fprintf(fp, "mkdir 2>&- ");
	quote(name, fp);
	(void) fprintf(fp, " && chmod 2>&- %o ", mode);
	quote(name, fp);
	(void) fputc('\n', fp);
	(void) fflush(fp);
	oops = ferror(fp);
	return (pclose(fp) == 0 && !oops) ? 0 : -1;
}
