/*
 * openwrite - open a file for write; bomb if unsuccesful
 */

#include <stdio.h>
#include <sys/file.h>

FILE *
openwrite (file)
char   *file;
{
    extern char *myname;
    FILE	*fd;

    fd = fopen (file, "w+");
    if (fd == NULL) {
	fprintf (stderr, "%s: can't open %s for writing\n", myname, file);
	exit (1);
    }
    return (fd);
}
