#ifndef lint
#ifndef NSCCS
static char sccsid[] = "%W%";
#endif
#endif

rename(from, to)
char *from;	/* Old name */
char *to;	/* New name */
{
	register status = -1;

	if (link(from, to) != -1 && unlink(from) != -1)
		status = 0;
	return(status);
}
