/*
 * openread - open a file for exclusive read; bomb if unsuccesful
 */

#include <stdio.h>
#include <sys/file.h>

FILE *
openread (file)
char   *file;
{
    extern char *myname;
    FILE	*fp;

    fp = fopen (file, "r");
    if (fp == NULL) {
	fprintf (stderr, "%s: can't open %s for reading\n", myname, file);
	exit (1);
    }
    if (flock (fileno (fp), LOCK_EX | LOCK_NB)) {
	fprintf (stderr, "%s: statfile %s is in use and locked\n",
	    myname, file);
	exit (1);
    }
    rewind(fp);
    return (fp);
}
