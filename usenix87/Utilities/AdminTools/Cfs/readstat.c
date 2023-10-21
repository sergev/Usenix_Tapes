/*
 * readstat - reads pathname & stats from statfile on fp
 *	returns  0 if read OK; +1 if EOF; -1 if error
 */

#include "cfs.h"

readstat (fp, statfile, name, sbuf)
FILE   *fp;
char   *statfile;
char   *name;
struct stat *sbuf;
{
    extern char *myname;
    int     nread;
    int     retval = 0;
    char    c;
    int     nc = 0;
    struct stat dummy;

/*
 * read the null-terminated pathname
 */
    while (((nread = fread (&c, 1, 1, fp)) == 1)
	    && (c != '\0')
	    && nc < MAXPATHLEN) {
	name[nc++] = c;
    }
    name[nc] = '\0';
    if (nread == 0) {
	retval = 1;
	if (nc != 0) {
	    fprintf (stderr, "%s: error - truncated statfile %s\n",
		    myname, statfile);
	    retval = -1;
	}
    }
    if ((nread > 0) && (c != '\0')) {
	fprintf (stderr, "%s: error - bad pathname in statfile %s\n",
		myname, statfile);
	retval = -1;
    }
    if (retval == 0) {
/*
 * read the fixed-size statbuf
 */
	nread = fread (sbuf, sizeof (dummy), 1, fp);
	if (nread != 1) {
	    fprintf (stderr, "%s: error - truncated statfile %s\n",
		    myname, statfile);
	    retval = -1;
	}
    }
    return (retval);
}
