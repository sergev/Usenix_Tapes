/*
 * Some systems do not have this
 * system call.
 *
 * DB Shimell	September 1984
 */

#ifndef lint
#ifndef NSCCS
static char sccsid[] = "%W%";
#endif
#endif

umask(numask)
{
	return(0);
}
