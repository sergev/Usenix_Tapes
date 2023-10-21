/*
 * Special interface that checks for the environment variable PAGER.  If
 * present, the program specified is executed, otherwise OLD_PAGER_NEW_LOCATION
 * (specified below).  This program should replace /usr/ucb/more (or whatever
 * your default pager is) and more should be moved to OLD_PAGER_NEW_LOCATION.
 * This is essentially a fix for all the programs which *should* check for the
 * environment variable PAGER, but don't - hopefully it will be obsoleted as
 * old programs are update to check for PAGER so we can loose the overhead of
 * reexecuting even this small program ...
 *
 * Casey Leedom (lll-crg.arpa!csustan!casey) - 5/29/86
 */

#ifndef OLD_PAGER_NEW_LOCATION
#	define	OLD_PAGER_NEW_LOCATION	"/usr/ucb/More"
#endif !OLD_PAGER_NEW_LOCATION

void
main(argc, argv)
	int	argc;
	char	**argv;
{
	char	*pager, *getenv();

	if (!(pager = getenv("PAGER")))
		pager = OLD_PAGER_NEW_LOCATION;
	(void) execv(pager, argv);
}
