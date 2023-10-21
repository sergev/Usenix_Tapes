/*
 * ERRPRNT
 * prints error message using second optional parameter
 * in formatting string, the first parameter,
 * and then halts.
 */
errprnt(buf, bp)
char *buf, *bp;
{
extern char *argv0; /* A copy of zeroth parameter - the program's name */

	flush(1);
	close(1); dup(2);
	printf("%s ", argv0);
	printf(buf, bp);
	putchar('\n');
	flush();
	exit(1);
}
