/*
 * ERRPRNTC (portable c library version)
 * prints error message using second optional parameter
 * in formatting string, the first parameter,
 * and then halts.
 */
errprnt(buf, bp1, bp2, bp3)
char *buf, *bp1, *bp2, *bp3;
{
extern char *argv0; /* A copy of zeroth parameter - the program's name */

	printf(2, "%s ", argv0);
	printf(2, buf, bp1, bp2, bp3);
	printf(2, "\n");
	cexit(1);
}
